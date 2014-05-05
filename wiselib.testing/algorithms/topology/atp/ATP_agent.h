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

#ifndef __ATP_AGENT_H__
#define __ATP_AGENT_H__
#include "ATP_source_config.h"
namespace wiselib
{
	template<	typename Os_P,
				typename Radio_P,
				typename AgentID_P,
				typename TimesNumber_P,
				typename Debug_P>
	class ATP_AgentType
	{
	public:
		typedef Os_P Os;
		typedef Radio_P Radio;
		typedef AgentID_P AgentID;
		typedef TimesNumber_P TimesNumber;
		typedef Debug_P Debug;
		typedef typename Radio::block_data_t block_data_t;
		typedef typename Radio::size_t size_t;
		typedef ATP_AgentType<Os, Radio, AgentID, TimesNumber, Debug> self_type;
		// --------------------------------------------------------------------
		inline ATP_AgentType() :
			agent_id	( 0 ),
			hop_count	( 0 )
		{}
		// --------------------------------------------------------------------
		inline ATP_AgentType( const AgentID& _aid ) :
			hop_count	( 0 )
		{
			agent_id = _aid;
		}
		// --------------------------------------------------------------------
		inline block_data_t* serialize( block_data_t* _buff, size_t _offset = 0 )
		{
			size_t AGENT_ID_POS = 0;
			size_t HOP_COUNT_POS = AGENT_ID_POS + sizeof(AgentID);
			write<Os, block_data_t, AgentID> ( _buff + AGENT_ID_POS + _offset, agent_id );
			write<Os, block_data_t, TimesNumber> ( _buff + HOP_COUNT_POS + _offset, hop_count );
			return _buff;
		}
		// --------------------------------------------------------------------
		inline void de_serialize( block_data_t* _buff, size_t _offset = 0 )
		{
			size_t AGENT_ID_POS = 0;
			size_t HOP_COUNT_POS = AGENT_ID_POS + sizeof(AgentID);
			agent_id = read<Os, block_data_t, AgentID> ( _buff + AGENT_ID_POS + _offset );
			hop_count = read<Os, block_data_t, TimesNumber> ( _buff + HOP_COUNT_POS + _offset );
		}
		// --------------------------------------------------------------------
		inline size_t serial_size()
		{
			size_t AGENT_ID_POS = 0;
			size_t HOP_COUNT_POS = AGENT_ID_POS + sizeof(AgentID);
			return HOP_COUNT_POS + sizeof(TimesNumber);
		}
		// --------------------------------------------------------------------
		inline self_type& operator=( const self_type& _a )
		{
			agent_id = _a.agent_id;
			hop_count = _a.hop_count;
			return *this;
		}
		// --------------------------------------------------------------------
		inline AgentID get_agent_id()
		{
			return agent_id;
		}
		// --------------------------------------------------------------------
		inline void set_agent_id( AgentID _aid )
		{
			agent_id = _aid;
		}
		// --------------------------------------------------------------------
		void inc_hop_count()
		{
			hop_count = hop_count + 1;
		}
		// --------------------------------------------------------------------
		TimesNumber get_hop_count()
		{
			return hop_count;
		}
		// --------------------------------------------------------------------
#ifdef DEBUG_ATP_AGENT_H
		inline void print( Debug& _debug, Radio& _radio )
		{
			_debug.debug( "-------------------------------------------------------\n");
			_debug.debug( "ATP_Agent : \n" );
			_debug.debug( "agent_id (size %i) : %x\n", sizeof(agent_id), agent_id );
			_debug.debug( "hop_count (size %i) : %d\n", sizeof(hop_count), hop_count );
			_debug.debug( "-------------------------------------------------------\n");
		}
#endif
		// --------------------------------------------------------------------
	private:
		AgentID agent_id;
		TimesNumber hop_count;
	};
}
#endif


