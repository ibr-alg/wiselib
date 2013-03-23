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

#ifndef COMMAND_TYPES_H_
#define COMMAND_TYPES_H_

/**
 * Well-known message/command types for WISEBED.
 */

      enum CommandTypes
      {
         DEBUG_MESSAGE = 10,
         VIRTUAL_LINK_MESSAGE = 11,
         BYTE_MESSAGE = 12,
         FLASH_MESSAGE = 13,
         ENABLE_NODE = 20,
         DISABLE_NODE = 21,
         RESET_NODE = 22,
         SET_START_TIME = 23,
         SET_VIRTUAL_ID = 24,
         IS_NODE_ALIVE = 25,
         GET_ID = 26,
         SET_VIRTUAL_LINK = 30,
         DESTROY_VIRTUAL_LINK = 31,
         ENABLE_PHYSICAL_LINK = 32,
         DISABLE_PHYSICAL_LINK = 33,
         GET_PROPERTY_VALUE = 40,
         GET_NEIGHBORHOOD = 41,
         NODE_OUTPUT_TEXT = 50,
         NODE_OUTPUT_BYTE = 51,
         NODE_OUTPUT_VIRTUAL_LINK = 52,
         REMOTE_UART_MESSAGE = 60,
         REMOTE_UART_SINK_REQUEST = 61,
         REMOTE_UART_SINK_RESPONSE = 62,
         REMOTE_UART_SET_SINK = 63,
         REMOTE_UART_KEEP_ALIVE = 64,
         SSD_REST_REQUEST = 70,
         SSD_REST_RESPONSE = 71,
         IOS_LINK_MESSAGE = 105,
         BEACON_SYNC_MESSAGE = 110,
         SENSORDATAMESSAGE = 115
                 

      };

#endif /* COMMAND_TYPES_H_ */
