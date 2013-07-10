/***************************************************************************
 ** This file is part of the generic algorithm library Wiselib.           **
 ** Copyright (C) 2012 by the Wisebed (www.wisebed.eu) project.           **
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
#ifndef ARDUINO_TYPES_H
#define ARDUINO_TYPES_H

// default port that the radio listens to
#define PORT 1337

//uint8_t mac[] = {0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED};
#define MAC "\xde\xad\xbe\xef\xfe\xed"
#define WISELIB_IP 0x0300A8C0
#define WISELIB_IP_ROUTER 0x0100A8C0
#define WISELIB_SUBNET 0x00FFFFFF
#define WISELIB_DNS 0x0100A8C0

#define HOST_NAME "arduino"
#define ZEROCONF_DISCOVER_SERVICE "_wiselib"
#define ZEROCONF_PUBLISH_SERVICE "_node._wiselib"

#define DEFAULT_BAUD_RATE 38400
#define RXD 6
#define TXD 7

#endif
