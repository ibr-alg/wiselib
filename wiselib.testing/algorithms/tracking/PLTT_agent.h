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

#ifndef __PLTT_AGENT_H__
#define __PLTT_AGENT_H__

#include "PLTT_default_values_config.h"
#include "PLTT_source_config.h"

namespace wiselib
{
	template<	typename Os_P,
				typename Radio_P,
				typename AgentID_P,
				typename IntensitiyNumber_P,
				typename Position_P,
				typename TimesNumber_P,
				typename Debug_P>
	class PLTT_AgentType
	{
	public:
		typedef Os_P Os;
		typedef Radio_P Radio;
		typedef AgentID_P AgentID;
		typedef IntensitiyNumber_P IntensityNumber;
		typedef Position_P Position;
		typedef TimesNumber_P TimesNumber;
		typedef Debug_P Debug;
		typedef typename Radio::block_data_t block_data_t;
		typedef typename Radio::size_t size_t;
		typedef typename Radio::node_id_t node_id_t;
		typedef PLTT_AgentType<Os, Radio, AgentID, IntensityNumber, Position, TimesNumber, Debug> self_type;
		// --------------------------------------------------------------------
		inline PLTT_AgentType()
		{}
		// --------------------------------------------------------------------
		inline PLTT_AgentType( const AgentID& _aid, const node_id_t& _tid, const node_id_t& _trid, const IntensityNumber& _mi ) :
				payload_size 		(0),
				start_millis		(0),
#ifdef CONFIG_PLTT_TRACKER_H_AGENT_BUFFERING
				count				(0),
#endif
				hop_count			(0),
				trace_id			(0),
				tracker_trace_id	(0),
				target_lqi			(0),
				target_rssi			(0)
		{
			agent_id = _aid;
			target_id = _tid;
			tracker_id = _trid;
			max_intensity = _mi;
		}
		// --------------------------------------------------------------------
		inline block_data_t* serialize( block_data_t* _buff, size_t _offset = 0 )
		{
			size_t AGENT_ID_POS = 0;
			size_t TARGET_ID_POS = AGENT_ID_POS + sizeof(AgentID);
			size_t TRACKER_ID_POS = TARGET_ID_POS + sizeof(node_id_t);
			size_t MAX_INTEN_POS = TRACKER_ID_POS + sizeof(node_id_t);
			size_t START_MILLIS_POS = MAX_INTEN_POS + sizeof(IntensityNumber);
			size_t HOP_COUNT_POS = START_MILLIS_POS + sizeof(uint32_t);
			size_t TRACE_ID_POS = HOP_COUNT_POS + sizeof(uint32_t);
			size_t TRACKER_TRACE_ID_POS = TRACE_ID_POS + sizeof(TimesNumber);
			size_t TARGET_LQI_POS = TRACKER_TRACE_ID_POS + sizeof(TimesNumber);
			size_t TARGET_RSSI_POS = TARGET_LQI_POS + sizeof(uint8_t);
			size_t TARGET_POSITION_POS = TARGET_RSSI_POS + sizeof(uint8_t);
			size_t PAYLOAD_SIZE_POS = TARGET_POSITION_POS + target_position.serial_size();
			size_t PAYLOAD_POS = PAYLOAD_SIZE_POS + sizeof(size_t);
			write<Os, block_data_t, AgentID> ( _buff + AGENT_ID_POS + _offset, agent_id );
			write<Os, block_data_t, node_id_t> ( _buff + TARGET_ID_POS + _offset, target_id );
			write<Os, block_data_t, node_id_t> ( _buff + TRACKER_ID_POS + _offset, tracker_id );
			write<Os, block_data_t, IntensityNumber> ( _buff + MAX_INTEN_POS + _offset, max_intensity );
			write<Os, block_data_t, uint32_t> ( _buff + START_MILLIS_POS + _offset, start_millis );
			write<Os, block_data_t, uint32_t> ( _buff + HOP_COUNT_POS + _offset, hop_count );
			write<Os, block_data_t, TimesNumber> ( _buff + TRACE_ID_POS + _offset, trace_id );
			write<Os, block_data_t, uint8_t> ( _buff + TARGET_LQI_POS + _offset, target_lqi );
			write<Os, block_data_t, uint8_t> ( _buff + TARGET_RSSI_POS + _offset, target_rssi );
			write<Os, block_data_t, TimesNumber> ( _buff + TRACKER_TRACE_ID_POS + _offset, tracker_trace_id );
			target_position.serialize( _buff + _offset, TARGET_POSITION_POS );
			write<Os, block_data_t, size_t> ( _buff + PAYLOAD_SIZE_POS + _offset, payload_size );
			memcpy( _buff + PAYLOAD_POS + _offset, payload, payload_size );
			return _buff;
		}
		// --------------------------------------------------------------------
		inline void de_serialize( block_data_t* _buff, size_t _offset = 0 )
		{
			size_t AGENT_ID_POS = 0;
			size_t TARGET_ID_POS = AGENT_ID_POS + sizeof(AgentID);
			size_t TRACKER_ID_POS = TARGET_ID_POS + sizeof(node_id_t);
			size_t MAX_INTEN_POS = TRACKER_ID_POS + sizeof(node_id_t);
			size_t START_MILLIS_POS = MAX_INTEN_POS + sizeof(IntensityNumber);
			size_t HOP_COUNT_POS = START_MILLIS_POS + sizeof(uint32_t);
			size_t TRACE_ID_POS = HOP_COUNT_POS + sizeof(uint32_t);
			size_t TRACKER_TRACE_ID_POS = TRACE_ID_POS + sizeof(TimesNumber);
			size_t TARGET_LQI_POS = TRACKER_TRACE_ID_POS + sizeof(TimesNumber);
			size_t TARGET_RSSI_POS = TARGET_LQI_POS + sizeof(uint8_t);
			size_t TARGET_POSITION_POS = TARGET_RSSI_POS + sizeof(uint8_t);
			size_t PAYLOAD_SIZE_POS = TARGET_POSITION_POS + target_position.serial_size();
			size_t PAYLOAD_POS = PAYLOAD_SIZE_POS + sizeof(size_t);
			agent_id = read<Os, block_data_t, AgentID> ( _buff + AGENT_ID_POS + _offset );
			target_id = read<Os, block_data_t, node_id_t> ( _buff + TARGET_ID_POS + _offset );
			tracker_id = read<Os, block_data_t, node_id_t> ( _buff + TRACKER_ID_POS + _offset );
			max_intensity = read<Os, block_data_t, IntensityNumber> ( _buff + MAX_INTEN_POS + _offset );
			start_millis = read<Os, block_data_t, uint32_t> ( _buff + START_MILLIS_POS + _offset );
			hop_count = read<Os, block_data_t, uint32_t> ( _buff + HOP_COUNT_POS + _offset );
			trace_id = read<Os, block_data_t, TimesNumber> ( _buff + TRACE_ID_POS + _offset );
			target_lqi = read<Os, block_data_t, uint8_t> ( _buff + TARGET_LQI_POS + _offset );
			target_rssi = read<Os, block_data_t, uint8_t> ( _buff + TARGET_RSSI_POS + _offset );
			tracker_trace_id = read<Os, block_data_t, TimesNumber> ( _buff + TRACKER_TRACE_ID_POS + _offset );
			target_position.de_serialize( _buff + _offset, TARGET_POSITION_POS );
			payload_size = read<Os, block_data_t, size_t> ( _buff + PAYLOAD_SIZE_POS + _offset );
			memcpy( payload, _buff + PAYLOAD_POS + _offset, payload_size );
		}
		// --------------------------------------------------------------------
		inline size_t serial_size()
		{
			size_t AGENT_ID_POS = 0;
			size_t TARGET_ID_POS = AGENT_ID_POS + sizeof(AgentID);
			size_t TRACKER_ID_POS = TARGET_ID_POS + sizeof(node_id_t);
			size_t MAX_INTEN_POS = TRACKER_ID_POS + sizeof(node_id_t);
			size_t START_MILLIS_POS = MAX_INTEN_POS + sizeof(IntensityNumber);
			size_t HOP_COUNT_POS = START_MILLIS_POS + sizeof(uint32_t);
			size_t TRACE_ID_POS = HOP_COUNT_POS + sizeof(uint32_t);
			size_t TRACKER_TRACE_ID_POS = TRACE_ID_POS + sizeof(TimesNumber);
			size_t TARGET_LQI_POS = TRACKER_TRACE_ID_POS + sizeof(TimesNumber);
			size_t TARGET_RSSI_POS = TARGET_LQI_POS + sizeof(uint8_t);
			size_t TARGET_POSITION_POS = TARGET_RSSI_POS + sizeof(uint8_t);
			//size_t TARGET_POSITION_POS = TRACKER_TRACE_ID_POS + sizeof(TimesNumber);
			size_t PAYLOAD_SIZE_POS = TARGET_POSITION_POS + target_position.serial_size();
			size_t PAYLOAD_POS = PAYLOAD_SIZE_POS + sizeof(size_t);
			return PAYLOAD_POS + payload_size;
		}
		// --------------------------------------------------------------------
		inline self_type& operator=( const self_type& _a )
		{
			agent_id = _a.agent_id;
			target_id = _a.target_id;
			tracker_id = _a.tracker_id;
			max_intensity = _a.max_intensity;
			payload_size = _a.payload_size;
			start_millis = _a.start_millis;
#ifdef CONFIG_PLTT_TRACKER_H_AGENT_BUFFERING
			count = _a.count;
#endif
			hop_count = _a.hop_count;
			trace_id = _a.trace_id;
			tracker_trace_id = _a.tracker_trace_id;
			target_position = _a.target_position;
			target_lqi = _a.target_lqi;
			target_rssi = _a.target_rssi;
			memcpy( payload, _a.payload, payload_size );
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
		inline node_id_t get_target_id()
		{
			return target_id;
		}
		// --------------------------------------------------------------------
		inline void set_target_id( node_id_t _tid )
		{
			target_id = _tid;
		}
		// --------------------------------------------------------------------
		inline node_id_t get_tracker_id()
		{
			return tracker_id;
		}
		// --------------------------------------------------------------------
		inline void set_tracker_id( node_id_t _trid )
		{
			tracker_id = _trid;
		}
		// --------------------------------------------------------------------
		inline void set_max_intensity( IntensityNumber _mi )
		{
			max_intensity = _mi;
		}
		// --------------------------------------------------------------------
		inline IntensityNumber get_max_intensity()
		{
			return max_intensity;
		}
		// --------------------------------------------------------------------
		inline void set_payload( size_t _len, block_data_t* _buff )
		{
			payload_size = _len;
			memcpy( payload, _buff, _len );
		}
		// --------------------------------------------------------------------
		inline block_data_t* get_payload()
		{
			return payload;
		}
		// --------------------------------------------------------------------
		inline size_t get_payload_size()
		{
			return payload_size;
		}
		// --------------------------------------------------------------------
		inline void set_start_millis( uint32_t _sm )
		{
			start_millis = _sm;
		}
		// --------------------------------------------------------------------
		inline uint32_t get_start_millis()
		{
			return start_millis;
		}
		// --------------------------------------------------------------------
		inline void switch_dest()
		{
			node_id_t buff;
			buff = target_id;
			target_id = tracker_id;
			tracker_id = buff;
		}
		// --------------------------------------------------------------------
#ifdef CONFIG_PLTT_TRACKER_H_AGENT_BUFFERING
		void inc_count()
		{
			count = count + 1;
		}
		// --------------------------------------------------------------------
		uint32_t get_count()
		{
			return count;
		}
#endif
		// --------------------------------------------------------------------
		void inc_hop_count()
		{
			hop_count = hop_count + 1;
		}
		// --------------------------------------------------------------------
		uint32_t get_hop_count()
		{
			return hop_count;
		}
		// --------------------------------------------------------------------
		void set_target_position( Position _tp )
		{
			target_position = _tp;
		}
		// --------------------------------------------------------------------
		Position get_target_position()
		{
			return target_position;
		}
		// --------------------------------------------------------------------
		void set_trace_id( TimesNumber _tid )
		{
			trace_id = _tid;
		}
		// --------------------------------------------------------------------
		TimesNumber get_trace_id()
		{
			return trace_id;
		}
		// --------------------------------------------------------------------
		void set_tracker_trace_id( TimesNumber _ttid )
		{
			tracker_trace_id = _ttid;
		}
		// --------------------------------------------------------------------
		TimesNumber get_tracker_trace_id()
		{
			return tracker_trace_id;
		}
		// --------------------------------------------------------------------
		uint8_t get_target_lqi()
		{
			return target_lqi;
		}
		// --------------------------------------------------------------------
		void set_target_lqi( uint8_t _tlqi )
		{
			target_lqi = _tlqi;
		}
		// --------------------------------------------------------------------
		uint8_t get_target_rssi()
		{
			return target_rssi;
		}
		// --------------------------------------------------------------------
		void set_target_rssi( uint8_t _trssi )
		{
			target_rssi = _trssi;
		}
		// --------------------------------------------------------------------
#ifdef DEBUG_PLTT_AGENT_H
		inline void print( Debug& _debug, Radio& _radio )
		{
			_debug.debug( "-------------------------------------------------------\n");
			_debug.debug( "PLTT_Agent : \n" );
			_debug.debug( "agent_id (size %i) : %x\n", sizeof(agent_id), agent_id );
			_debug.debug( "max_intensity (size %i) : %i\n", sizeof(max_intensity), max_intensity );
			_debug.debug( "current target id (size %i) : %x\n", sizeof(target_id), target_id );
			_debug.debug( "current tracker id (size %i) : %x\n", sizeof(tracker_id), tracker_id );
			_debug.debug( "millis_t (size %i) : %d\n", sizeof(start_millis), start_millis );
#ifdef CONFIG_PLTT_TRACKER_H_AGENT_BUFFERING
			_debug.debug( "count (size %i) : %d\n", sizeof(count), count );
#endif
			_debug.debug( "hop_count (size %i) : %d\n", sizeof(hop_count), hop_count );
			_debug.debug( "payload_size (size %i) : %i\n", sizeof(size_t), payload_size );
			_debug.debug( "trace_id (size %i) : %i\n", sizeof(trace_id), trace_id );
			_debug.debug( "tracker_trace_id (size %i) : %i\n", sizeof(tracker_trace_id), tracker_trace_id );
			_debug.debug( "target_lqi (size %i) : %i\n", sizeof(target_lqi), target_lqi );
			_debug.debug( "target_rssi (size %i) : %i\n", sizeof(target_rssi), target_rssi );
			target_position.print( _debug, _radio );
			_debug.debug( "payload : \n" );
			for ( size_t i = 0; i < payload_size; i++ )
			{
				_debug.debug( " %d", payload[i] );
			}
			_debug.debug( "\n");
			_debug.debug( "-------------------------------------------------------\n");
		}
#endif
		// --------------------------------------------------------------------
	private:
		AgentID agent_id;
		node_id_t target_id;
		node_id_t tracker_id;
		IntensityNumber max_intensity;
		uint32_t start_millis;
		block_data_t payload[PLTT_AGENT_H_MAX_PAYLOAD];
		size_t payload_size;
#ifdef CONFIG_PLTT_TRACKER_H_AGENT_BUFFERING
		uint32_t count;
#endif
		uint32_t hop_count;
		TimesNumber trace_id;
		TimesNumber tracker_trace_id;
		uint8_t target_lqi;
		uint8_t target_rssi;
		Position target_position;
	};
}
#endif


