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
#ifndef TINYOS_WISELIB_GLUE_H
#define TINYOS_WISELIB_GLUE_H

#include <stdint.h>

// --------------------------------------------------------------------------
// --------------------------------------------------------------------------
// --- Wiselib Glue Code to be called by TinyOs-NCs (e.g., for callbacks)
// --------------------------------------------------------------------------
// --------------------------------------------------------------------------

// --------------------------------------------------------------------------
// ----- Radio
// --------------------------------------------------------------------------
void tinyos_external_receive( uint16_t from, uint8_t len, uint8_t *data, uint8_t lqi );

// --------------------------------------------------------------------------
// ----- Timer
// --------------------------------------------------------------------------
void tinyos_timer_fired( int idx );

// --------------------------------------------------------------------------
// ----- Uart
// --------------------------------------------------------------------------
#ifndef WISELIB_BUILD_ONLY_STABLE
void tinyos_glue_uart_rcv_byte(uint8_t data);
#endif


// --------------------------------------------------------------------------
// --------------------------------------------------------------------------
// --- Functions provided by TinyOs-NCs to be called by Wiselib Glue Code
// --- (each of these functions is declared as "@C() @spontaneous()")
// --------------------------------------------------------------------------
// --------------------------------------------------------------------------

// --------------------------------------------------------------------------
// ----- Debug
// --------------------------------------------------------------------------
void tinyos_glue_debug(char *msg);

// --------------------------------------------------------------------------
// ----- Timer
// --------------------------------------------------------------------------
int tinyos_get_free_timer();
void tinyos_register_timer( int idx, uint32_t millis );

// --------------------------------------------------------------------------
// ----- Radio
// --------------------------------------------------------------------------
enum TinyOsRadioErrorCodes
{
   TINYOS_RADIO_SUCCESS = 0,
   TINYOS_RADIO_BUSY = 1,
   TINYOS_RADIO_PACKET_TOO_LARGE = 2,
   TINYOS_RADIO_ERR_UNSPEC = 3
};

void tinyos_init_radio_module();
uint16_t tinyos_get_nodeid();
int tinyos_send_message( uint16_t id, uint8_t len, const uint8_t *data );

// --------------------------------------------------------------------------
// ----- Clock
// --------------------------------------------------------------------------
uint32_t tinyos_get_time();

// --------------------------------------------------------------------------
// ----- Uart
// --------------------------------------------------------------------------
#ifndef WISELIB_BUILD_ONLY_STABLE
int tinyos_glue_uart_write( uint8_t len, uint8_t *buf );
#endif

#endif
