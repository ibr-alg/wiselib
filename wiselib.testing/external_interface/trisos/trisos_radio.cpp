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
#include "external_interface/trisos/trisos_radio.h"
#include "external_interface/trisos/trisos_os.h"
#include <string.h>
#include <stdio.h>
extern "C" {
#include "src/hw/rf/rf.h"
#include "src/hw/rf/compiler_avr.h"
#include "src/sys/sys.h"
#include "contiki.h"
#include "sys/etimer.h"
}

namespace wiselib
{
   	trisos_radio_fp trisos_internal_radio_callback = 0;
   	// -----------------------------------------------------------------------
   	static const int MAX_REGISTERED_RECEIVERS = 10;
   	static trisos_radio_delegate_t receivers[MAX_REGISTERED_RECEIVERS];
	PROCESS( spawn_rf_callback, "spawn rf callback" );
   	// -----------------------------------------------------------------------
	static void rf_call()
	{
		AVR_ENTER_CRITICAL_REGION();
		if( process_post( &spawn_rf_callback, 0, NULL ) == PROCESS_ERR_FULL ) 
		{
			process_poll( &spawn_rf_callback );
		}
		AVR_LEAVE_CRITICAL_REGION();
	}
   	// -----------------------------------------------------------------------
	int trisos_radio_add_receiver( trisos_radio_delegate_t& d )
	{
		for ( int i = 0; i < MAX_REGISTERED_RECEIVERS; i++ )
		{
			if ( !receivers[i] )
			{
				receivers[i] = d;
				return i;
			}
		}

		return -1;
	}
    // -----------------------------------------------------------------------
	void trisos_radio_del_receiver( int idx )
	{
		receivers[idx] = trisos_radio_delegate_t();
	}
	// -----------------------------------------------------------------------
	void trisos_notify_receivers( uint8_t length, uint8_t *data )
	{
		/* Size of the header */
		static const uint8_t HEADER_SIZE = 2 * sizeof(TriSOSRadio<wiselib::TriSOSOsModel>::node_id_t);
		/* Return if only header and not data */
		if( length <= HEADER_SIZE ) return;

		uint16_t addr = sys_get_id();

		uint16_t src = read<TriSOSOsModel, uint8_t, uint16_t>( data );
		uint16_t dst = read<TriSOSOsModel, uint8_t, uint16_t>( data + 2 );

		if ( dst == addr || dst == TriSOSRadio<TriSOSOsModel>::BROADCAST_ADDRESS )
		{
			for ( int i = 0; i < MAX_REGISTERED_RECEIVERS; i++ )
			{
				if ( receivers[i] )
				{
					receivers[i]( src, length - HEADER_SIZE, data + HEADER_SIZE );
				}
			}
		}
	}
   	// -----------------------------------------------------------------------
	void init_trisos_radio( void )
	{
		for ( int i = 0; i < MAX_REGISTERED_RECEIVERS; i++ )
		{
			receivers[i] = trisos_radio_delegate_t();
		}
		/* Init rf callback process */
		process_start( &spawn_rf_callback, NULL );
		/* Configure recieve callback */
		rf_config_packet_recv_callback( rf_call );
	}
	// -----------------------------------------------------------------------
	PROCESS_THREAD( spawn_rf_callback, ev, data )
	{

		uint8_t length = (uint8_t)(TriSOSRadio<wiselib::TriSOSOsModel>::MAX_MESSAGE_LENGTH);
		uint8_t buffer[(uint8_t)(TriSOSRadio<wiselib::TriSOSOsModel>::MAX_MESSAGE_LENGTH)];

		PROCESS_BEGIN();

		while(1)
		{
			PROCESS_YIELD();

			rf_state_t rf_state = rf_receive_data( &length, buffer );
			if( trisos_internal_radio_callback && 
				rf_state == rf_SUCCESS )
			{
				trisos_internal_radio_callback( length, buffer );
			}

			/* Reschedule while still data available in receive buffer */
			/* Commented out: Spawns to many processes! */
			/*if( rf_data_available() )
			{
				rf_call();
			}*/ 
		}

		PROCESS_END();
	}
}
