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
#ifndef __PLTT_RELIABLE_AGENT_H__
#define __PLTT_RELIABLE_AGENT_H__
#include "PLTT_config.h"
namespace wiselib
{
	template<	typename Os_P,
				typename Radio_P,
				typename Agent_P,
				typename Time_P,
				typename Debug_P>
	class PLTT_ReliableAgentType
	{
	public:
		typedef Os_P Os;
		typedef Radio_P Radio;
		typedef Agent_P Agent;
		typedef Time_P Time;
		typedef Debug_P Debug;
		typedef typename Radio::node_id_t node_id_t;
		typedef typename Radio::message_id_t message_id_t;
		typedef typename Time::millis_t millis_t;
		typedef PLTT_ReliableAgentType<Os, Radio, Agent, Time, Debug> self_type;
		PLTT_ReliableAgentType()
		{}
		PLTT_ReliableAgentType( Agent _a, node_id_t _r, millis_t _et, millis_t _rt, message_id_t _m )
		{
			set_all( _a, _r, _et, _rt, _m );
		}
		inline self_type& operator=( const self_type& _ra)
		{
			agent = _ra.agent;
			receiver = _ra.receiver;
			exp_time = _ra.exp_time;
			rec_time = _ra.rec_time;
			message_id = _ra.message_id;
			return *this;
		}
		Agent get_agent()
		{
			return agent;
		}
		node_id_t get_receiver()
		{
			return receiver;
		}
		millis_t get_exp_time()
		{
			return exp_time;
		}
		millis_t get_rec_time()
		{
			return rec_time;
		}
		message_id_t get_message_id()
		{
			return message_id;
		}
		void set_agent( Agent _a )
		{
			agent = _a;
		}
		void set_receiver( node_id_t _r )
		{
			receiver = _r;
		}
		void set_exp_time( millis_t _et )
		{
			exp_time = _et;
		}
		void set_rec_time( millis_t _rt )
		{
			rec_time = _rt;
		}
		void set_message_id( message_id_t _m )
		{
			message_id = _m;
		}
		inline void set_all( Agent _a, node_id_t _r, millis_t _et , millis_t _rt, message_id_t _m )
		{
			agent = _a;
			receiver = _r;
			exp_time = _et;
			rec_time = _rt;
			message_id = _m;
		}
		void print( Debug& debug )
		{
			debug.debug( "reliable agent (size %i)\n", sizeof( self_type ) );
			agent.print( debug );
			debug.debug( "receiver (size %i): %x\n", sizeof( node_id_t ), receiver );
			debug.debug( "exp time (size %i): %i\n", sizeof( millis_t ), exp_time );
			debug.debug( "recurring time (size %i): %i\n", sizeof( millis_t ), rec_time );
			debug.debug( "message_id (size %i): %i\n", sizeof (message_id_t ), message_id );
		}
	private:
		Agent agent;
		node_id_t receiver;
		millis_t exp_time;
		millis_t rec_time;
		message_id_t message_id;
	};
}
#endif


