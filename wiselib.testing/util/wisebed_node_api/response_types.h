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

#ifndef RESPONSE_TYPES_H_
#define RESPONSE_TYPES_H_

	enum ResponseType{
		COMMAND_SUCCESS = 0,
		WRONG_PARAMETER_LIST = 1,
		INSUFFICIENT_PARAMETER_LIST = 2,
		UNKNOWN_PARAMETER = 3,
		COMMAND_LOCKED = 4,
		UNKNOWN_DESTINATION_ID = 5,
		SENSOR_NOT_AVAILIBLE = 6,
		SENSOR_BUSY = 7,
		SENSOR_LOCKED = 8,
		NOTYETIMPLEMENTED = 126,
		UNKNOWN_ERROR = 127
	};

#endif /* RESPONSE_TYPES_H_ */
