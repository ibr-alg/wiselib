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

#include <external_interface/external_interface.h>

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
		typename Debug_P = typename OsModel_P::Debug
	>
	class SkyLightDataProvider {
		public:
			typedef SkyLightDataProvider self_type;
			typedef self_type* self_pointer_t;

			typedef OsModel_P OsModel;
			typedef typename OsModel::size_type size_type;
			typedef typename OsModel::block_data_t block_data_t;

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

			SkyLightDataProvider() : debug_(0) {
			}

			int init(
					const char *ov,
					Timer& timer,
					Debug& debug
			) {
				ov_ = ov;
				timer_ = &timer;
				debug_ = &debug;

				saved_value_ = 0;
				sensor_value_ = 0;

				SENSORS_ACTIVATE(light_sensor);

				check();
				return SUCCESS;
			}

			int init() {
				return SUCCESS;
			}

			Debug& debug() { return *debug_; }

		private:
			void check() {
				assert(debug_ != 0);
				assert(timer_ != 0);
			}

			void take_measurement(void *_ = 0) {
				unsigned v = light_sensor.value(LIGHT_SENSOR_PHOTOSYNTHETIC);
				sensor_value_ = (1.0 - LIGHT_ALPHA / 100.0) * sensor_value_;

				if(Math::abs(sensor_value_ - saved_value_) > UPDATE_THRESHOLD) {
					update_rdf();
				}

				timer_->set_timer<self_type, &self_type::take_measurement>(MEASUREMENT_INTERVAL, this, 0);
			}

			void update_rdf() {
				Tuple t;
				t.set(0, ov_);
				t.set(1, "<http://spitfire-project.eu/ontology/ns/value>");

				// delete old tuple
				iterator iter = tuple_store().begin(&t);
				while(iter != tuple_store().end()) {
					iter = tuple_store().erase(iter);
				}

				char buffer[20];
				buffer[0] = '"';
				int l = ftoa(sizeof(buffer - 2), buffer + 1, sensor_value_, 3);
				assert(l != -1 && l < sizeof(buffer) - 1);
				buffer[l] = '"';
				buffer[l + 1] = '\0';

				t.set(2, buffer);
				tuple_store().insert(t);
			}

			float saved_value_;
			float sensor_value_;

			typename Debug::self_pointer_t debug_;
			typename Timer::self_pointer_t timer_;
	};

} // namespace wiselib

#endif // __SKY_LIGHT_DATA_PROVIDER_H__
