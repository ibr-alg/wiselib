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

/* 
 * File:   request_types.h
 * Author: maxpagel
 *
 * Created on 28. Dezember 2010, 10:30
 */

#ifndef _REQUEST_TYPES_H
#define	_REQUEST_TYPES_H
enum RequestTypes
{
    GET_SENSORS          = 0,
    GET_NODE_META_DATA   = 1,
    GET_SENSOR_VALUE     = 2,
    GET_SENSOR_META_DATA = 3,
    SUBSCRIBE            = 4,
    CANCEL_SUBSCRIPTION  = 5,
    POST_SENSOR_META_DATA = 6

};

#endif	/* _REQUEST_TYPES_H */

