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

extern "C"
{
	#include "contiki.h"
	#include "process.h" 
	#include "interfaces/acc-adxl345.h"
	#include <stdio.h>
}

#include "contiki_gyro_listener.h"

namespace wiselib
{
	static contiki_gyro_delegate_t receiver;
	static struct etimer gyro_timer;

	PROCESS( gyro_event_process, "Gyro Event Listener" );

	PROCESS_THREAD(gyro_event_process, ev, data)
	{
		PROCESS_EXITHANDLER( return stopContikiGyroListening() );
		PROCESS_BEGIN( );

		adxl345_init( );
		etimer_set( &gyro_timer, CLOCK_SECOND / 64);

		for ( ; ; )
		{
			PROCESS_WAIT_EVENT();

			if( ev == PROCESS_EVENT_TIMER )
			{
				if( data == &gyro_timer )
				{
					receiver( adxl345_get_acceleration( ) );
					etimer_reset(&gyro_timer);
				}
			}
		}

		PROCESS_END();
	}

	void initContikiGyroListening()
	{
		receiver = contiki_gyro_delegate_t();
		process_start( &gyro_event_process, 0);
	}

	int stopContikiGyroListening()
	{
		contiki_gyro_delete_receiver();
		return 0;
	}

	void contiki_gyro_set_receiver( contiki_gyro_delegate_t& d )
	{
		receiver = d;
	}

	void contiki_gyro_delete_receiver()
	{
		receiver = contiki_gyro_delegate_t();
	}
}

// vim: noexpandtab:ts=3:sw=3
