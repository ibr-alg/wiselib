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
// vim: set noexpandtab ts=3 sw=3:

#ifndef TRISOS_CLOCK_H
#define TRISOS_CLOCK_H

#include "src/sys/sys.h"
#include "external_interface/trisos/trisos_os.h"

namespace wiselib {
	template<typename OsModel_P>
	class TriSOSClockModel {
		public:
			typedef OsModel_P OsModel;
			
			typedef TriSOSClockModel<OsModel> self_type;
			typedef self_type* self_pointer_t;
			
			typedef uint32_t time_t;
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
			
			TriSOSClockModel();
			TriSOSClockModel(TriSOSOsModel& os);
			
			int state();
			time_t time();
			//void set_time(time_t); // not supported
			micros_t microseconds(time_t);
			millis_t milliseconds(time_t);
			seconds_t seconds(time_t);
	};
	
	template<typename OsModel_P>
	TriSOSClockModel<OsModel_P>::
	TriSOSClockModel() {
	}
	
	template<typename OsModel_P>
	TriSOSClockModel<OsModel_P>::
	TriSOSClockModel(TriSOSOsModel& os) {
	}
	
	template<typename OsModel_P>
	int TriSOSClockModel<OsModel_P>::
	state() {
		return READY;
	}
	
	template<typename OsModel_P>
	typename TriSOSClockModel<OsModel_P>::time_t TriSOSClockModel<OsModel_P>::
	time() {
		return sys_get_system_time_ms();
	}
	
	template<typename OsModel_P>
	typename TriSOSClockModel<OsModel_P>::micros_t TriSOSClockModel<OsModel_P>::
	microseconds(time_t t) {
		return 0;
	}
	
	template<typename OsModel_P>
	typename TriSOSClockModel<OsModel_P>::millis_t TriSOSClockModel<OsModel_P>::
	milliseconds(time_t t) {
		return t%1000;
	}
	
	template<typename OsModel_P>
	typename TriSOSClockModel<OsModel_P>::seconds_t TriSOSClockModel<OsModel_P>::
	seconds(time_t t) {
		return t/1000;
	}
	
} // namespace wiselib

#endif // TRISOS_CLOCK_H

