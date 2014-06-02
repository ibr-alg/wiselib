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
#ifndef __SKY_LIGHT_DATA_PROVIDER_H__
#define __SKY_LIGHT_DATA_PROVIDER_H__

#if defined(CONTIKI)
	extern "C" {
		#include <string.h>
		#include <contiki.h>
		#include <netstack.h>
		#include <dev/light-sensor.h>
	}
#endif
#include <external_interface/external_interface.h>
#include <util/standalone_math.h>
#include <util/string_util.h>

namespace wiselib {

	/**
	 * @brief SkyLightDataProvider implementation of @ref SkyLightDataProvider_concept "SkyLightDataProvider Concept".
	 *
	 * @ingroup SkyLightDataProvider_concept
	 * @ingroup basic_algorithm_concept
	 *
	 */
	template<
		typename OsModel_P,
		typename Tuplestore_P,
		typename Timer_P = typename OsModel_P::Timer,
		typename Debug_P = typename OsModel_P::Debug
	>
	class SkyLightDataProvider {
		public:
			typedef SkyLightDataProvider self_type;
			typedef self_type* self_pointer_t;

			typedef OsModel_P OsModel;
			typedef typename OsModel::size_t size_type;
			typedef typename OsModel::size_t size_t;
			typedef typename OsModel::block_data_t block_data_t;
			typedef StandaloneMath<OsModel> Math;

			typedef Tuplestore_P TupleStore;
			typedef typename TupleStore::Tuple Tuple;
			typedef typename TupleStore::iterator iterator;
			typedef Debug_P Debug;
			typedef Timer_P Timer;

			enum ErrorCodes {
				SUCCESS = OsModel::SUCCESS,
				ERR_UNSPEC = OsModel::ERR_UNSPEC,
				ERR_NOMEM = OsModel::ERR_NOMEM,
				ERR_BUSY = OsModel::ERR_BUSY,
				ERR_NOTIMPL = OsModel::ERR_NOTIMPL,
				ERR_NETDOWN = OsModel::ERR_NETDOWN,
				ERR_HOSTUNREACH = OsModel::ERR_HOSTUNREACH
			};

			enum {
				MEASUREMENT_INTERVAL = 5000,
				UPDATE_THRESHOLD = 1,
				LIGHT_ALPHA = 50,
			};

			SkyLightDataProvider() : timer_(0), debug_(0) {
			}

			int init(
					const char *ov,
					TupleStore& tuple_store,
					Timer& timer,
					Debug& debug
			) {
				ov_ = ov;
				tuple_store_ = &tuple_store;
				timer_ = &timer;
				debug_ = &debug;

				saved_value_ = 0;
				sensor_value_ = 0;
				enabled_ = true;

				SENSORS_ACTIVATE(light_sensor);

				take_measurement();

				check();
				return SUCCESS;
			}

			//int init() {
				//return SUCCESS;
			//}

			void disable() {
				enabled_ = false;
			}

			TupleStore& tuple_store() { return *tuple_store_; }

			Debug& debug() { return *debug_; }
			Timer& timer() { return *timer_; }

		private:
			void check() {
				assert(debug_ != 0);
				assert(timer_ != 0);
			}

			/**
			 * @param p user data, a value != 0 indicates the first run (and
			 *   guarantees a value to be inserted into the tuple store).
			 */
			void take_measurement(void *p = (void*)1) {
				unsigned v = light_sensor.value(LIGHT_SENSOR_PHOTOSYNTHETIC);
				vvv = v;
				sensor_value_ = (
						((1.0 - (float)LIGHT_ALPHA / 100.0) * (float)sensor_value_) +
						(((float)LIGHT_ALPHA / 100.0) * (float)v)
				);

				if(p != 0 || Math::abs(sensor_value_ - saved_value_) > UPDATE_THRESHOLD) {
					update_rdf();
				}

				if(enabled_) {
					timer_->template set_timer<self_type, &self_type::take_measurement>(MEASUREMENT_INTERVAL, this, 0);
				}
			}

			void update_rdf() {
				Tuple t;

				t.set(0, (block_data_t*)ov_);
				t.set(1, (block_data_t*)"<http://spitfire-project.eu/ontology/ns/value>");

				// delete old tuple
				iterator iter = tuple_store().begin(&t, BIN(011));
				while(iter != tuple_store().end()) {
#if ENABLE_DEBUG
						debug_->debug("erase (%d %d %d)", (int)iter->get_key(0), (int)iter->get_key(1), (int)iter->get_key(2));
#endif
					iter = tuple_store().erase(iter);
				}

				char buffer[20];
				buffer[0] = '"';
				int l = ftoa(sizeof(buffer) - 2, buffer + 1, sensor_value_, 3);
				assert(l != -1 && l < (int)sizeof(buffer) - 1);
				buffer[l] = '"';
				buffer[l + 1] = '\0';
				saved_value_ = sensor_value_;

				t.set(2, (block_data_t*)buffer);

#if ENABLE_DEBUG
				debug_->debug("+ (%s %s %s) %d %d", (char*)t.get(0), (char*)t.get(1), (char*)t.get(2), (int)sensor_value_, (int)vvv);
#endif
				tuple_store().insert(t);
			}

			float saved_value_;
			int vvv;
			float sensor_value_;
			const char *ov_;
			bool enabled_;

			typename TupleStore::self_pointer_t tuple_store_;
			typename Timer::self_pointer_t timer_;
			typename Debug::self_pointer_t debug_;
	};

} // namespace wiselib

#endif // __SKY_LIGHT_DATA_PROVIDER_H__
