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

#ifndef __PLTT_TARGET_H__
#define __PLTT_TARGET_H__

#include "PLTT_config.h"
#include "PLTT_message.h"

namespace wiselib
{
	template<typename Os_P,
		typename PLTT_Trace_P,
		typename Node_P,
		typename Timer_P,
		typename Radio_P,
		typename Clock_P,
		typename Debug_P>
	class PLTT_TargetType
	{
	public:
		typedef Os_P Os;
		typedef Radio_P Radio;
		typedef Debug_P Debug;
		typedef Node_P Node;
		typedef PLTT_Trace_P PLTT_Trace;
		typedef Timer_P Timer;
		typedef Clock_P Clock;
		typedef PLTT_TargetType<Os, PLTT_Trace, Node, Timer, Radio, Clock, Debug> self_type;
		typedef typename Radio::node_id_t node_id_t;
		typedef typename Radio::size_t size_t;
		typedef typename Radio::block_data_t block_data_t;
		typedef typename Radio::message_id_t message_id_t;
		typedef typename Timer::millis_t millis_t;
		typedef typename PLTT_Trace::PLTT_TraceData PLTT_TraceData;
		typedef typename PLTT_Trace::TimesNumber TimesNumber;
		typedef typename Radio::TxPower TxPower;
		typedef PLTT_MessageType<Os, Radio> Message;
		void init( Radio& radio, Timer& timer, Clock& clock, Debug& debug )
		{
			radio_ = &radio;
			timer_ = &timer;
			debug_ = &debug;
			clock_ = &clock;
		}
		Node* get_self()
		{
			return &self;
		}
		void set_self( Node _n )
		{
			self = _n;
		}
		// -----------------------------------------------------------------------
		PLTT_TargetType()
		{}
		// -----------------------------------------------------------------------
		PLTT_TargetType( PLTT_Trace _t, millis_t _s, int16_t _tp )
		{
			target_trace = _t;
			spread_milis = _s;
			trans_power.set_dB( _tp );
			target_trace.set_start_time( 0 );
		}
		// -----------------------------------------------------------------------
		~PLTT_TargetType()
		{}
		// -----------------------------------------------------------------------
		void enable( void )
		{
			radio().enable_radio();
			target_trace.set_target_id( self.get_id() );
			timer().template set_timer<self_type, &self_type::send_trace>( spread_milis, this, 0 );
		}
		// -----------------------------------------------------------------------
		void disable( void )
		{
			radio().disable();
		}
		// -----------------------------------------------------------------------
		void send( node_id_t destination, size_t len, block_data_t *data )
		{
			radio().set_power( trans_power );
			Message message;
			message.set_msg_id( PLTT_DETECTION_ID );
			message.set_payload( len, data );
			radio().send( destination, message.buffer_size(), (block_data_t*)&message );
		}
		// -----------------------------------------------------------------------
		void send_trace( void* userdata = NULL)
		{
			block_data_t buffer[Radio::MAX_MESSAGE_LENGTH];
			block_data_t* buff = buffer;
			target_trace.set_buffer_from( buff );
			size_t len = target_trace.get_buffer_size();
			send( Radio::BROADCAST_ADDRESS, len, (uint8_t*)buff );
			target_trace.update_start_time();
			timer().template set_timer<self_type, &self_type::send_trace>( spread_milis, this, 0 );
		}
		// -----------------------------------------------------------------------
	private:
		Radio& radio()
		{
			return *radio_; 
		}
		Timer& timer()
		{
			return *timer_; 
		}
		Debug& debug()
		{
			return *debug_; 
		}
		Clock& clock()
		{
			return *clock_;
		}
		Radio * radio_;
		Timer * timer_;
		Debug * debug_;
		Clock * clock_;
		enum MessageIds
		{
			PLTT_DETECTION_ID = 11
		};
		PLTT_Trace target_trace;
		millis_t spread_milis;
		TxPower trans_power;
		Node self;
	};

}
#endif
