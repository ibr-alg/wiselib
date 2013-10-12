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
			
			/*
			
			<http://uberdust.cti.gr/rest/testbed/1/node/urn:wisebed:ctitestbed:0x190/capability/urn:wisebed:node:capability:pir/>
        a       <http://www.w3.org/2002/07/owl#Thing> , <http://www.w3.org/2000/01/rdf-schema#Resource> , <http://spitfire-project.eu/ontology/ns/OV> , <http://purl.oclc.org/NET/ssnx/ssn#ObservationValue> , <http://qudt.org/schema/qudt#QuantityValue> ;
        <http://purl.oclc.org/NET/ssnx/ssn#observedProperty>
                <http://spitfire-project.eu/property/Movement> ;
        <http://purl.oclc.org/NET/ssnx/ssn#observes>
                <http://spitfire-project.eu/property/Movement> ;
        <http://purl.org/dc/terms/#date>
                "13-10-11T14:21:42Z" ;
        <http://qudt.org/schema/qudt#quantityKind>
                <http://spitfire-project.eu/property/Movement> ;
        <http://spitfire-project.eu/ontology/ns/obs>
                <http://spitfire-project.eu/property/Movement> ;
        <http://spitfire-project.eu/ontology/ns/value>
                "1.0"^^<http://www.w3.org/2001/XMLSchema#float> .
				
			*/
				
			/**
			 * @me assumed to be long-lived.
			 */
			void init(isense::PirSensor *sensor, char *me, TupleStore *ts, Registry *reg, Aggregator *agg) {
				sensor_ = sensor;
				tuple_store_ = ts;
				registry_ = reg;
				aggregator_ = agg;
				
				// Some RDF URIs
				
				uri_me_ = me;
				uri_observes_ = "<http://spitfire-project.eu/ontology/ns/obs>";
				uri_movement_ = "<http://spitfire-project.eu/property/Movement>";
				uri_has_value_ = "<http://spitfire-project.eu/ontology/ns/value>";
				sensor_type_ = tuple_store_->dictionary().insert((::uint8_t*)uri_movement_);
				uom_ = tuple_store_->dictionary().insert((::uint8_t*)"<http://spitfire-project.eu/uom/Boolean>");
				
				ins(uri_me_, uri_observes_, uri_movement_);
				
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
				//timeout_id_  = 0xff;
				GET_OS.add_task(this, (void*)0x02);
				timeout_id_ = GET_OS.add_timeout_in(5000, this, NULL);
			}
			
			void handle_sensor() {
				if(timeout_id_ == 0xff) {
					// we timed out before, now be on again!
					GET_OS.add_task(this, (void*)0x01); // on!
				}
				else {
					// the value didnt change, but make sure to update the
					// aggregate
					GET_OS.add_task(this, (void*)0x01); // on!
					
					// we are still in on-state, so nothing to do
					// except, cancel the off-timeout, and start a new one
					GET_OS.remove_timeout(timeout_id_, this);
				}
				timeout_id_ = GET_OS.add_timeout_in(5000, this, NULL);
			}
			
			///@}
		
			
			void update(bool value) {
				GET_OS.debug("PIR %d", (int)value);
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
					float f = value;
					Value v = *reinterpret_cast<Value*>(&f);
					aggregator_->aggregate(rit->first, sensor_type_, uom_, (Value)value, Aggregator::FLOAT);
				}
				//GET_OS.debug("/sens");
			}
			
		private:
			
			void ins(const char* s, const char* p, const char* o) {
				TupleT t;
				t.set(0, (block_data_t*)const_cast<char*>(s));
				t.set(1, (block_data_t*)const_cast<char*>(p));
				t.set(2, (block_data_t*)const_cast<char*>(o));
				tuple_store_->insert(t);
			}
			
			char *uri_me_;
			char *uri_has_value_;
			char *uri_observes_;
			char *uri_movement_;
			
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

