/***************************************************************************
 ** This file is part of the generic algorithm library Wiselib.           **
 ** Copyright (C) 2008,2009 by the Wisebed (www.wisebed.eu) project.      **
 **                                                                       **
 ** The Wiselib is free software: you can redistribute it and/or modify   **
 ** it under the terms of the GNU Lesser General Public License as        **
 ** published by the Free Software Foundation, either version 3 of the    **
 ** License, or (at your option) any later version.                       **
 **                                                                       **
 ** The Wiselib is distributed in the hope that it will be useful,        **
 ** but WITHOUT ANY WARRANTY; without even the implied warranty of        **
 ** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         **
 ** GNU Lesser General Public License for more details.                   **
 **                                                                       **
 ** You should have received a copy of the GNU Lesser General Public      **
 ** License along with the Wiselib.                                       **
 ** If not, see <http://www.gnu.org/licenses/>.                           **
 ***************************************************************************/

#ifndef ISENSE_PIR_DATA_PROVIDER_H
#define ISENSE_PIR_DATA_PROVIDER_H

#include <isense/modules/security_module/pir_sensor.h>
#include <isense/util/get_os.h>
#include <util/pstl/string_utils.h>
#include <isense/timeout_handler.h>
#include <isense/protocols/ip/coap/coap_resource.h>
#include <isense/timeout_handler.h>
#include <isense/task.h>

namespace wiselib {
	
	/**
	 * @brief
	 * 
	 * @ingroup
	 * 
	 * @tparam 
	 */
	template<
		typename OsModel_P,
		typename TupleStore_P,
		typename Registry_P,
		typename Aggregator_P
	>
	class IsensePirDataProvider : isense::SensorHandler, public isense::TimeoutHandler, isense::Task {
		
		public:
			typedef OsModel_P OsModel;
			typedef typename OsModel::block_data_t block_data_t;
			typedef typename OsModel::size_t size_type;
			typedef TupleStore_P TupleStore;
			typedef typename TupleStore::Tuple Tuple;
			typedef Registry_P Registry;
			typedef Aggregator_P Aggregator;
			
			/**
			 * @me assumed to be long-lived.
			 */
			void init(isense::PirSensor *sensor, char *me, TupleStore *ts, Registry *reg, Aggregator *agg) {
				sensor_ = sensor;
				uri_me_ = me;
				tuple_store_ = ts;
				registry_ = reg;
				aggregator_ = agg;
				
				uri_has_value_ = "<http://www.ontologydesignpatterns.org/ont/dul/hasValue>";
				sensor_type_ = tuple_store_->dictionary().insert((::uint8_t*)"<http://spitfire-project.eu/property/Occupancy>");
				uom_ = tuple_store_->dictionary().insert((::uint8_t*)"<http://spitfire-project.eu/uom/Boolean>");
				
				timeout_id_ = 0xff;
				
				sensor_->set_sensor_handler(this);
				sensor_->set_pir_sensor_int_interval(1000);
				sensor_->enable();
			}
			
			///@name isense stuff
			///@{
			
			void execute(void* userdata) {
				if(userdata == (void*)0x01) { // on
					update(true);
				}
				else if(userdata == (void*)0x02) { // off
					update(false);
				}
			}
			
			void timeout(void* userdata) {
				// after timeout auto-off
				timeout_id_  = 0xff;
				GET_OS.add_task(this, (void*)0x02);
			}
			
			void handle_sensor() {
				if(timeout_id_ == 0xff) {
					// we timed out before, now be on again!
					GET_OS.add_task(this, (void*)0x01); // on!
				}
				else {
					// we are still in on-state, so nothing to do
					// except, cancel the off-timeout, and start a new one
					GET_OS.remove_timeout(timeout_id_, this);
				}
				timeout_id_ = GET_OS.add_timeout_in(5000, this, NULL);
			}
			
			///@}
		
			
			void update(bool value) {
				Tuple t;
				
				// DELETE (:me :hasValue *)
				
				t.set(uri_me_, uri_has_value_, 0);
				typename TupleStore::iterator it = tuple_store_->begin(&t, BIN(011));
				if(it != tuple_store_->end()) {
					tuple_store_->erase(it);
				}
					
				t.set(uri_me_, uri_has_value_, value ? "1" : "0");
				tuple_store_->insert(t);
				
				for(typename Registry::iterator rit = registry_->begin(); rit != registry_->end(); ++rit) {
					GET_OS.debug("aggr");
					float f = value;
					Value v = *reinterpret_cast<Value*>(&f);
					aggregator_->aggregate(rit->first, sensor_type_, uom_, (Value)value, Aggregator::FLOAT);
				}
			}
			
		private:
			char *uri_me_;
			char *uri_has_value_;
			typename TupleStore::Dictionary::key_type sensor_type_;
			typename TupleStore::Dictionary::key_type uom_;
			
			TupleStore *tuple_store_;
			::uint8_t timeout_id_;
			isense::PirSensor *sensor_;
			Registry *registry_;
			Aggregator *aggregator_;
		
	}; // IsensePirDataProvider
}

#endif // ISENSE_PIR_DATA_PROVIDER_H

