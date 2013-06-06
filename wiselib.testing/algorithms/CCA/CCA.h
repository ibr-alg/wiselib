/**************************************************************************
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

#ifndef __CCA_H__
#define __CCA_H__

#include "CCA_source_config.h"
#include "CCA_default_values_config.h"
#include "../../internal_interface/message/message.h"

namespace wiselib
{
	template<	typename Os_P,
				typename Radio_P,
				typename Timer_P,
				typename Rand_P,
				typename Clock_P,
				typename Debug_P
			>
	class CCA_Type
	{
	public:
		typedef Os_P Os;
		typedef Radio_P Radio;
		typedef Rand_P Rand;
		typedef typename Rand::rand_t rand_t;
		typedef Debug_P Debug;
		typedef Timer_P Timer;
		typedef Clock_P Clock;
		typedef typename Radio::node_id_t node_id_t;
		typedef typename Radio::size_t size_t;
		typedef typename Radio::block_data_t block_data_t;
		typedef typename Radio::message_id_t message_id_t;
		typedef typename Radio::ExtendedData ExData;
		typedef typename Timer::millis_t millis_t;
		typedef typename Radio::TxPower TxPower;
		typedef typename Clock::time_t time_t;
		typedef Message_Type<Os, Radio, Debug> Message;
		typedef CCA_Type <Os, Radio, Timer, Rand, Clock, Debug> self_type;
		// -----------------------------------------------------------------------
		CCA_Type() :
			radio_callback_id						( 0 ),
			transmission_power_dB					( CCA_H_TRANSMISSION_POWER_DB ),
			status									( WAITING_STATUS )
		{
		}
		// -----------------------------------------------------------------------
		~CCA_Type()
		{
		}
		// -----------------------------------------------------------------------
		void enable( void )
		{
#ifdef DEBUG_CCA_H_ENABLE
			debug().debug( "CCA - enable %x - Entering.\n", radio().id() );
#endif
			radio().enable_radio();
			set_status( ACTIVE_STATUS );
			cca_daemon();
#ifdef DEBUG_CCA_H_ENABLE
			debug().debug( "CCA - enable %x - Exiting.\n", radio().id() );
#endif
		}
		// -----------------------------------------------------------------------
		void receive( node_id_t _from, size_t _len, block_data_t * _msg, ExData const &_ex )
		{
			if ( status == ACTIVE_STATUS)
			{
				message_id_t msg_id = *_msg;
				Message *message = (Message*) _msg;
				if ( !message->compare_checksum() )
				{
					if ( msg_id == CCA_MESSAGE_ID )
					{

					}
				}
			}
		}
		// -----------------------------------------------------------------------
		void cca_daemon( void* user_data = NULL )
		{
#ifdef DEBUG_CCA_H_ENABLE
			debug().debug( "CCA - cca_daemon %x - Entering.\n", radio().id() );
#endif
			if ( status == ACTIVE_STATUS)
			{
				uint8 state = 0;
				uint8 tries = 0;
				millis_t time = 0;
				radio().confirm( state, tries, time );
			}
#ifdef DEBUG_CCA_H_ENABLE
			debug().debug( "CCA - cca_daemon %x - Exiting.\n", radio().id() );
#endif
			timer().template set_timer<self_type, &self_type::cca_daemon> ( 50, this, NULL );
		}
		// -----------------------------------------------------------------------
		void disable( void )
		{
			set_status( WAITING_STATUS );
			radio().unreg_recv_callback( radio_callback_id );
		}
		// -----------------------------------------------------------------------
		void init( Radio& radio, Timer& timer, Debug& debug, Rand& rand, Clock& clock )
		{
			_radio = &radio;
			_timer = &timer;
			_debug = &debug;
			_rand = &rand;
			_clock = &clock;
		}
		// -----------------------------------------------------------------------
		void set_status( int _st )
		{
			status = _st;
		}
		// -----------------------------------------------------------------------
	private:
		Radio& radio()
		{
			return *_radio;
		}
		// -----------------------------------------------------------------------
		Timer& timer()
		{
			return *_timer;
		}
		// -----------------------------------------------------------------------
		Debug& debug()
		{
			return *_debug;
		}
		// -----------------------------------------------------------------------
		Rand& rand()
		{
			return *_rand;
		}
		// -----------------------------------------------------------------------
		Clock& clock()
		{
			return *_clock;
		}
		// -----------------------------------------------------------------------
		enum message_ids
		{
			CCA_MESSAGE_ID = 17
		};
		// -----------------------------------------------------------------------
		Radio* _radio;
		Timer* _timer;
		Debug* _debug;
		Rand* _rand;
		Clock* _clock;
		enum cca_status
		{
			ACTIVE_STATUS,
			WAITING_STATUS,
			CCA_STATUS_NUM_VALUES
		};
		uint32_t radio_callback_id;
		int8_t transmission_power_dB;
		uint8_t status;
	};

}
#endif
