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

// vim: set noexpandtab ts=4 sw=4:

#ifndef PC_CLOCK_H
#define PC_CLOCK_H

// TODO: De-Boostification

#include "boost/date_time/posix_time/posix_time.hpp"

namespace wiselib {
	template<typename OsModel_P>
	class PCClockModel {
		public:
			typedef OsModel_P OsModel;
			
			typedef PCClockModel<OsModel> self_type;
			typedef self_type* self_pointer_t;
			
			typedef boost::posix_time::ptime time_t;
			typedef time_t value_t;
			typedef uint16_t micros_t;
			typedef uint16_t millis_t;
			typedef uint32_t seconds_t;
			
			enum States
			{
				READY = OsModel::READY,
				NO_VALUE = OsModel::NO_VALUE,
				INACTIVE = OsModel::INACTIVE
			};
			
			PCClockModel();
			
			int state();
			time_t time();
			//void set_time(time_t); // not supported
			micros_t microseconds(time_t);
			millis_t milliseconds(time_t);
			seconds_t seconds(time_t);
	};
	
	template<typename OsModel_P>
	PCClockModel<OsModel_P>::
	PCClockModel() {
	}
		
	template<typename OsModel_P>
	int PCClockModel<OsModel_P>::
	state() {
		return READY;
	}
	
	template<typename OsModel_P>
	typename PCClockModel<OsModel_P>::time_t PCClockModel<OsModel_P>::
	time() {
		return boost::posix_time::microsec_clock::universal_time();
	}
	
	template<typename OsModel_P>
	typename PCClockModel<OsModel_P>::micros_t PCClockModel<OsModel_P>::
	microseconds(time_t t) {
		return t.time_of_day().total_microseconds() % 1000;
	}
	
	template<typename OsModel_P>
	typename PCClockModel<OsModel_P>::millis_t PCClockModel<OsModel_P>::
	milliseconds(time_t t) {
		return t.time_of_day().total_milliseconds() % 1000;
	}
	
	template<typename OsModel_P>
	typename PCClockModel<OsModel_P>::seconds_t PCClockModel<OsModel_P>::
	seconds(time_t t) {
		return t.time_of_day().total_seconds();
	}
	
} // namespace wiselib

#endif // PC_CLOCK_H

