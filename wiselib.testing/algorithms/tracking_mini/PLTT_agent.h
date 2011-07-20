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
#include "PLTT_config.h"
namespace wiselib
{
	template<	typename Os_P,
				typename Radio_P,
				typename AgentID_P,
				typename Node_P,
				typename IntensitiyNumber_P,
				typename Clock_P,
				typename Debug_P>
	class PLTT_AgentType
	{
	public:
		typedef Os_P Os;
		typedef Radio_P Radio;
		typedef AgentID_P AgentID;
		typedef	Node_P Node;
		typedef IntensitiyNumber_P IntensityNumber;
		typedef Debug_P Debug;
		typedef typename Radio::block_data_t block_data_t;
		typedef typename Radio::size_t size_t;
		typedef Clock_P Clock;
		typedef typename Clock::time_t time_t;
		typedef typename Radio::node_id_t node_id_t;
		typedef PLTT_AgentType<Os, Radio, AgentID, Node, IntensityNumber, Clock, Debug> self_type;
		PLTT_AgentType()
		{}
		PLTT_AgentType( const AgentID& _trid, const Node& _tar, const Node& _tra, const IntensityNumber& _max_inten, AgentID& _raid = 0 )
		{
			set_all( _trid, _tar, _tra, _max_inten );
		}
		PLTT_AgentType( block_data_t* buff, size_t offset = 0 )
		{
			get_from_buffer( buff, offset );
		}
		block_data_t* set_buffer_from( block_data_t* buff, size_t offset = 0 )
		{

			uint8_t AGENT_ID_POS = 0;
			uint8_t TARGET_POS = AGENT_ID_POS + sizeof( AgentID );
			uint8_t TRACKER_POS = TARGET_POS + target.get_buffer_size();
			uint8_t MAX_INTEN_POS = TRACKER_POS + tracker.get_buffer_size();
#ifdef PLTT_AGENT_TRACKING_METRICS
			uint8_t TRACK_START_POS = MAX_INTEN_POS + sizeof( IntensityNumber );
			uint8_t TRACK_END_POS = TRACK_START_POS + sizeof( time_t );
			uint8_t APROX_DETECTION_POS = TRACK_END_POS + sizeof( time_t );
#ifdef OPT_RELIABLE_TRACKING
			uint8_t RELIABLE_AGENT_ID_POS = APROX_DETECTION_POS + sizeof( time_t );
#endif
#else
#ifdef OPT_RELIABLE_TRACKING
			uint8_t RELIABLE_AGENT_ID_POS = MAX_INTEN_POS + sizeof( IntensityNumber );
#endif
#endif
			write<Os, block_data_t, AgentID> (buff + AGENT_ID_POS + offset, agent_id);
			target.set_buffer_from( buff, TARGET_POS + offset );
			tracker.set_buffer_from( buff, TRACKER_POS + offset );
			write<Os, block_data_t, IntensityNumber> (buff + MAX_INTEN_POS + offset, max_intensity);
#ifdef PLTT_AGENT_TRACKING_METRICS
			write<Os, block_data_t, time_t> (buff + TRACK_START_POS, track_start );
			write<Os, block_data_t, time_t> (buff + TRACK_END_POS, track_end );
			write<Os, block_data_t, time_t> (buff +	APROX_DETECTION_POS, aprox_detection );
#endif
#ifdef OPT_RELIABLE_TRACKING
			write<Os, block_data_t, AgentID> (buff + RELIABLE_AGENT_ID_POS, reliable_agent_id );
#endif
			return buff;
		}
		void get_from_buffer( block_data_t* buff, size_t offset = 0 )
		{
			uint8_t AGENT_ID_POS = 0;
			uint8_t TARGET_POS = AGENT_ID_POS + sizeof( AgentID );
			uint8_t TRACKER_POS = TARGET_POS + target.get_buffer_size();
			uint8_t MAX_INTEN_POS = TRACKER_POS + tracker.get_buffer_size();
#ifdef PLTT_AGENT_TRACKING_METRICS
			uint8_t TRACK_START_POS = MAX_INTEN_POS + sizeof( IntensityNumber );
			uint8_t TRACK_END_POS = TRACK_START_POS + sizeof( time_t );
			uint8_t APROX_DETECTION_POS = TRACK_END_POS + sizeof( time_t );
#ifdef OPT_RELIABLE_TRACKING
			uint8_t RELIABLE_AGENT_ID_POS = APROX_DETECTION_POS + sizeof( time_t );
#endif
#else
#ifdef OPT_RELIABLE_TRACKING
			uint8_t RELIABLE_AGENT_ID_POS = MAX_INTEN_POS + sizeof( IntensityNumber );
#endif
#endif
			agent_id = read<Os, block_data_t, AgentID> (buff + AGENT_ID_POS + offset);
			target.get_from_buffer( buff, TARGET_POS + offset );
			tracker.get_from_buffer( buff, TRACKER_POS + offset );
			max_intensity = read<Os, block_data_t, IntensityNumber> (buff + MAX_INTEN_POS + offset);
#ifdef PLTT_AGENT_TRACKING_METRICS
			track_start = read<Os, block_data_t, time_t> (buff + TRACK_START_POS );
			track_end = read<Os, block_data_t, time_t> (buff + TRACK_END_POS );
			aprox_detection = read<Os, block_data_t, time_t> (buff + APROX_DETECTION_POS );
#endif
#ifdef OPT_RELIABLE_TRACKING
			reliable_agent_id = read<Os, block_data_t, AgentID> (buff + RELIABLE_AGENT_ID_POS);
#endif
		}
		size_t get_buffer_size()
		{
			uint8_t AGENT_ID_POS = 0;
			uint8_t TARGET_POS = AGENT_ID_POS + sizeof( AgentID );
			uint8_t TRACKER_POS = TARGET_POS + target.get_buffer_size();
			uint8_t MAX_INTEN_POS = TRACKER_POS + tracker.get_buffer_size();
#ifdef PLTT_AGENT_TRACKING_METRICS
			uint8_t TRACK_START_POS = MAX_INTEN_POS + sizeof( IntensityNumber );
			uint8_t TRACK_END_POS = TRACK_START_POS + sizeof( time_t );
			uint8_t APROX_DETECTION_POS = TRACK_END_POS + sizeof( time_t );
#ifdef OPT_RELIABLE_TRACKING
			uint8_t RELIABLE_AGENT_ID_POS = APROX_DETECTION_POS + sizeof( time_t );
			return RELIABLE_AGENT_ID_POS + sizeof( AgentID );
#endif
			return APROX_DETECTION_POS + sizeof( time_t );
#else
#ifdef OPT_RELIABLE_TRACKING
			uint8_t RELIABLE_AGENT_ID_POS = MAX_INTEN_POS + sizeof( IntensityNumber );
			return RELIABLE_AGENT_ID_POS + sizeof( AgentID );
#endif
			return MAX_INTEN_POS + sizeof( max_intensity );
#endif
		}
		inline self_type& operator=( const self_type& _a)
		{
			agent_id = _a.agent_id;
			target = _a.target;
			tracker = _a.tracker;
			max_intensity = _a.max_intensity;
	#ifdef PLTT_AGENT_TRACKING_METRICS
			aprox_detection = _a.aprox_detection;
			track_start = _a.track_start;
			track_end = _a.track_end;
	#endif
	#ifdef OPT_RELIABLE_TRACKING
			reliable_agent_id = _a.reliable_agent_id;
	#endif
			return *this;
		}
		inline AgentID get_agent_id()
		{
			return agent_id;
		}
		inline void set_agent_id( const AgentID& _trid )
		{
			agent_id = _trid;
		}
#ifdef OPT_RELIABLE_TRACKING
		inline AgentID get_reliable_agent_id()
		{
			return reliable_agent_id;
		}
		inline void set_reliable_agent_id( const AgentID& _trid )
		{
			reliable_agent_id = _trid;
		}
		inline void update_reliable_agent_id()
		{
			++reliable_agent_id;
		}
#endif
		inline Node get_target()
		{
			return target;
		}
		inline void set_target( const Node&_n )
		{
			target = _n;
		}
		inline Node get_tracker()
		{
			return tracker;
		}
		inline void set_tracker( const Node& _n )
		{
			tracker = _n;
		}
		inline void set_max_intensity( const IntensityNumber& _mi )
		{
			max_intensity = _mi;
		}
		inline IntensityNumber get_max_intensity()
		{
			return max_intensity;
		}
		inline void set_all( const AgentID& _trid, const Node& _tar, const Node& _tra, const IntensityNumber& _max_inten, const AgentID _raid )
		{
			agent_id = _trid;
			target = _tar;
			tracker = _tra;
			max_intensity = _max_inten;
			reliable_agent_id = _raid;
		}
		void print( Debug& debug, Clock& clock )
		{
			debug.debug( "agent_id (size %i), %x\n", sizeof( agent_id ), agent_id );
			target.print( debug );
			tracker.print( debug );
			debug.debug( "max_intensity (size %i), %i\n", sizeof( max_intensity ), max_intensity );
#ifdef PLTT_AGENT_TRACKING_METRICS
			debug.debug( "track start secs (size %i) :, %i\n" , sizeof( uint32_t), clock.seconds( track_start ) );
			debug.debug( "track start ms (size %i) : %i\n", sizeof( uint16_t), clock.milliseconds( track_start ) );
			debug.debug( "track end secs (size %i) : %i\n", sizeof( uint32_t ), clock.seconds( track_end ) );
			debug.debug( "track end ms (size %i) : %i\n", sizeof( uint16_t ), clock.milliseconds( track_end ) );
			debug.debug( "aprox detection secs (size %i) : %i\n", sizeof( uint32_t ), clock.seconds( aprox_detection ) );
			debug.debug( "aprox detection ms (size %i) %i\n", sizeof( uint16_t ), clock.milliseconds( aprox_detection ) );
#endif
#ifdef OPT_RELIABLE_TRACKING
			debug.debug( "reliable_agent_id (size %i) %i\n", sizeof( reliable_agent_id ), reliable_agent_id );
#endif
		}
#ifdef PLTT_AGENT_TRACKING_METRICS
		time_t get_track_start( Clock& clock )
		{
			#ifdef ISENSE_APP
			return time_t( clock.seconds( track_start ), clock.milliseconds( track_start ) );
			#endif
			#ifdef SHAWN_APP
			return 0;
			#endif
		}
		time_t get_track_end( Clock& clock )
		{
			#ifdef ISENSE_APP
			return time_t( clock.seconds( track_end ), clock.milliseconds ( track_end ) );
			#endif
			#ifdef SHAWN_APP
			return 0;
			#endif
		}
		time_t get_aprox_detection( Clock& clock )
		{
			#ifdef ISENSE_APP
			return time_t( clock.seconds( aprox_detection), clock.milliseconds( aprox_detection ) );
			#endif
			#ifdef SHAWN_APP
			return 0;
			#endif
		}
		void set_track_start( time_t t )
		{
			track_start = t;
		}
		void set_track_end( time_t t )
		{
			track_end = t;
		}
		void set_aprox_detection( time_t t )
		{
			aprox_detection = t;
		}
		time_t detection_duration( Clock& clock )
		{
			uint32_t sec_dd = clock.seconds( aprox_detection ) - clock.seconds( track_start );
			uint16_t ms_dd;
			if ( clock.milliseconds( aprox_detection ) < clock.milliseconds( track_start ) )
			{
				sec_dd = sec_dd - 1;
				ms_dd = ( 1000 + clock.milliseconds( aprox_detection ) ) - clock.milliseconds( track_start );
			}
			else
			{
				ms_dd = clock.milliseconds( aprox_detection ) - clock.milliseconds( track_start );
			}
			#ifdef ISENSE_APP
			return time_t( sec_dd, ms_dd );
			#endif
			#ifdef SHAWN_APP
			return 0;
			#endif
		}
		time_t track_duration( Clock& clock )
		{
			uint32_t sec_td = clock.seconds( track_end ) - clock.seconds( track_start );
			uint16_t ms_td;
			if ( clock.milliseconds( track_end ) < clock.milliseconds( track_start ) )
			{
				sec_td = sec_td - 1;
				ms_td = ( 1000 + clock.milliseconds( track_end ) ) - clock.milliseconds( track_start );
			}
			else
			{
				ms_td = clock.milliseconds( track_end ) - clock.milliseconds( track_start );
			}
			#ifdef ISENSE_APP
			return time_t( sec_td, ms_td );
			#endif
			#ifdef SHAWN_APP
			return 0;
			#endif
		}
#endif
	private:
		AgentID agent_id;
		Node target;
		Node tracker;
		IntensityNumber max_intensity;
#ifdef PLTT_AGENT_TRACKING_METRICS
		time_t aprox_detection;
		time_t track_start;
		time_t track_end;
#endif
#ifdef OPT_RELIABLE_TRACKING
		AgentID reliable_agent_id;
#endif
	};
}
#endif


