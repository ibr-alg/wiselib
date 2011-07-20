/***************************************************************************
** This file is part of the generic algorithm library Wiselib.           **
** Copyright (C) 2008,2009 by the Wisebed (www.wisebed.eu) project.      **
**									 									 **
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
#ifndef __PLTT_TRACE_H__
#define __PLTT_TRACE_H__
namespace wiselib
{
	template<	typename Os_P,
				typename Radio_P,
				typename TimesNumber_P,
				typename SecondsNumber_P,
				typename IntensityNumber_P,
				typename Node_P,
				typename NodeID_P,
				typename Debug_P>
	class PLTT_TraceType
	{
	public:
		typedef Os_P Os;
		typedef Radio_P Radio;
		typedef TimesNumber_P TimesNumber;
		typedef SecondsNumber_P SecondsNumber;
		typedef IntensityNumber_P IntensityNumber;
		typedef Node_P Node;
		typedef NodeID_P NodeID;
		typedef Debug_P Debug;
		typedef typename Radio::block_data_t block_data_t;
		typedef typename Radio::size_t size_t;
		typedef PLTT_TraceType<Os, Radio, TimesNumber, SecondsNumber, IntensityNumber, Node, NodeID, Debug> self_type;
		typedef struct PLTT_TraceData_Type
		{
			Node current;
			Node parent;
			Node grandparent;
			NodeID target_id;
			TimesNumber start_time;
			SecondsNumber diminish_seconds;
			IntensityNumber diminish_amount;
			IntensityNumber spread_penalty;
			IntensityNumber intensity;
			PLTT_TraceData_Type():
			target_id		(0),
			start_time		(0),
			diminish_seconds	(0),
			diminish_amount  	(0),
			spread_penalty   	(0),
			intensity        	(0)
			{}
		} PLTT_TraceData;

		inline PLTT_TraceType():
		inhibited  ( 0 )
		{}
		inline PLTT_TraceType(const SecondsNumber& _ds, const IntensityNumber& _da, const IntensityNumber& _sp,	const IntensityNumber& _si, const TimesNumber& _st )
		{
			trace_data.diminish_seconds = _ds;
			trace_data.diminish_amount = _da;
			trace_data.spread_penalty = _sp;
			trace_data.intensity = _si;
			trace_data.start_time = _st;
			inhibited = 0;
		}
		inline PLTT_TraceType( const PLTT_TraceData& _td )
		{
			trace_data = _td;
			inhibited = 0;
		}
		inline PLTT_TraceType( const self_type& _t)
		{
			*this = _t;
		}
		inline PLTT_TraceType( block_data_t* buff, size_t offset = 0 )
		{
			get_from_buffer( buff, offset );
			inhibited = 0;
		}
		inline block_data_t* set_buffer_from( block_data_t* buff, size_t offset = 0 )
		{
			uint8_t CURRENT_NODE_POS = 0;
			uint8_t PARENT_NODE_POS = trace_data.current.get_buffer_size() + CURRENT_NODE_POS;
			uint8_t GRANDPARENT_NODE_POS = trace_data.parent.get_buffer_size() + PARENT_NODE_POS;
			uint8_t TARGET_ID_POS = trace_data.grandparent.get_buffer_size() + GRANDPARENT_NODE_POS;
			uint8_t START_TIME_POS = sizeof( NodeID ) + TARGET_ID_POS;
			uint8_t DIMINISH_SECS_POS = sizeof( TimesNumber ) + START_TIME_POS;
			uint8_t DIMINISH_AMOUNT_POS = sizeof( SecondsNumber ) + DIMINISH_SECS_POS;
			uint8_t SPREAD_PENALTY_POS = sizeof( IntensityNumber ) + DIMINISH_AMOUNT_POS;
			uint8_t INTENSITY_POS = sizeof( IntensityNumber ) + SPREAD_PENALTY_POS;
			trace_data.current.set_buffer_from( buff, CURRENT_NODE_POS + offset );
			trace_data.parent.set_buffer_from( buff, PARENT_NODE_POS + offset);
			trace_data.grandparent.set_buffer_from( buff, GRANDPARENT_NODE_POS + offset);
			write<Os, block_data_t, NodeID>( buff + TARGET_ID_POS + offset, trace_data.target_id );
			write<Os, block_data_t, TimesNumber>( buff + START_TIME_POS + offset, trace_data.start_time );
			write<Os, block_data_t, SecondsNumber>( buff + DIMINISH_SECS_POS + offset, trace_data.diminish_seconds );
			write<Os, block_data_t, IntensityNumber>( buff + DIMINISH_AMOUNT_POS + offset, trace_data.diminish_amount );
			write<Os, block_data_t, IntensityNumber>( buff + SPREAD_PENALTY_POS + offset, trace_data.spread_penalty );
			write<Os, block_data_t, IntensityNumber>( buff + INTENSITY_POS + offset, trace_data.intensity );
			return buff;
		}
		// --------------------------------------------------------------------
		inline void get_from_buffer(block_data_t* buff, size_t offset = 0)
		{
			uint8_t CURRENT_NODE_POS = 0;
			uint8_t PARENT_NODE_POS = trace_data.current.get_buffer_size() + CURRENT_NODE_POS;
			uint8_t GRANDPARENT_NODE_POS = trace_data.parent.get_buffer_size() + PARENT_NODE_POS;
			uint8_t TARGET_ID_POS = trace_data.grandparent.get_buffer_size() + GRANDPARENT_NODE_POS;
			uint8_t START_TIME_POS = sizeof( NodeID ) + TARGET_ID_POS;
			uint8_t DIMINISH_SECS_POS = sizeof( TimesNumber ) + START_TIME_POS;
			uint8_t DIMINISH_AMOUNT_POS = sizeof( SecondsNumber ) + DIMINISH_SECS_POS;
			uint8_t SPREAD_PENALTY_POS = sizeof( IntensityNumber ) + DIMINISH_AMOUNT_POS;
			uint8_t INTENSITY_POS = sizeof( IntensityNumber ) + SPREAD_PENALTY_POS;
			trace_data.current.get_from_buffer( buff, CURRENT_NODE_POS + offset);
			trace_data.parent.get_from_buffer( buff, PARENT_NODE_POS + offset );
			trace_data.grandparent.get_from_buffer( buff, GRANDPARENT_NODE_POS + offset );
			trace_data.target_id = read<Os, block_data_t, NodeID>(buff + TARGET_ID_POS + offset );
			trace_data.start_time = read<Os, block_data_t, TimesNumber>(buff + START_TIME_POS + offset );
			trace_data.diminish_seconds = read<Os, block_data_t, SecondsNumber>(buff + DIMINISH_SECS_POS + offset );
			trace_data.diminish_amount = read<Os, block_data_t, IntensityNumber>(buff + DIMINISH_AMOUNT_POS + offset );
			trace_data.spread_penalty = read<Os, block_data_t, IntensityNumber>(buff + SPREAD_PENALTY_POS + offset );
			trace_data.intensity = read<Os, block_data_t, IntensityNumber>(buff + INTENSITY_POS + offset );
		}
		inline size_t get_buffer_size()
		{
			uint8_t CURRENT_NODE_POS = 0;
			uint8_t PARENT_NODE_POS = trace_data.current.get_buffer_size() + CURRENT_NODE_POS;
			uint8_t GRANDPARENT_NODE_POS = trace_data.parent.get_buffer_size() + PARENT_NODE_POS;
			uint8_t TARGET_ID_POS = trace_data.grandparent.get_buffer_size() + GRANDPARENT_NODE_POS;
			uint8_t START_TIME_POS = sizeof( NodeID ) + TARGET_ID_POS;
			uint8_t DIMINISH_SECS_POS = sizeof( TimesNumber ) + START_TIME_POS;
			uint8_t DIMINISH_AMOUNT_POS = sizeof( SecondsNumber ) + DIMINISH_SECS_POS;
			uint8_t SPREAD_PENALTY_POS = sizeof( IntensityNumber ) + DIMINISH_AMOUNT_POS;
			uint8_t INTENSITY_POS = sizeof( IntensityNumber ) + SPREAD_PENALTY_POS;
			return INTENSITY_POS + sizeof( IntensityNumber );
		}
		inline self_type& operator=( const self_type& _t)
		{
			trace_data = _t.trace_data;
			inhibited = _t.inhibited;
			return *this;
		}
		inline void update_intensity_diminish()
		{
			trace_data.intensity = trace_data.intensity - trace_data.diminish_amount;
			if ( trace_data.intensity < 0 )
			{ trace_data.intensity = 0; }
		}
		inline void update_intensity_penalize()
		{
			trace_data.intensity = trace_data.intensity - trace_data.spread_penalty;
			if ( trace_data.intensity < 0 )
			{ trace_data.intensity = 0; }
		}
		inline void update_path( const Node& _n )
		{
			trace_data.grandparent = trace_data.parent;
			trace_data.parent = trace_data.current;
			trace_data.current = _n;
		}
		inline void update_start_time()	{ trace_data.start_time = trace_data.start_time + 1; }
		inline IntensityNumber get_diminish_amount() { return trace_data.diminish_amount; }
		inline SecondsNumber get_diminish_seconds(){ return trace_data.diminish_seconds; }
		inline IntensityNumber get_intensity(){ return trace_data.intensity; }
		inline NodeID get_target_id(){ return trace_data.target_id; }
		inline Node get_current(){ return trace_data.current; }
		inline Node get_parent(){ return trace_data.parent; }
		inline Node get_grandparent(){ return trace_data.grandparent; }
		inline NodeID get_random_id(){ return random_id; }
		inline NodeID get_furthest_id(){ return furthest_id; }
		inline PLTT_TraceData get_trace_data(){ return trace_data; }
		inline SecondsNumber get_inhibited() { return inhibited; }
		inline TimesNumber get_start_time() { return trace_data.start_time; }
		inline IntensityNumber get_spread_penalty() { return trace_data.spread_penalty; }
		inline Node get_repulsion_point()
		{
			if ( trace_data.grandparent.get_id() != 0 ) { return trace_data.grandparent; }
			if ( trace_data.parent.get_id() != 0 ) { return trace_data.parent; }
			return trace_data.current;
		}
		inline void set_target_id( const NodeID& tarid ){ trace_data.target_id = tarid; }
		inline void set_inhibited(){ inhibited = inhibited + 1;	}
		inline void set_inhibited( const uint8_t& n ){ inhibited = n; }
		inline void set_start_time( const TimesNumber& t ) { trace_data.start_time = t; }
		inline void set_diminish_seconds( const SecondsNumber& s ) { trace_data.diminish_seconds = s; }
		inline void set_diminish_amount( const IntensityNumber &d ) { trace_data.diminish_amount = d; }
		inline void set_spread_penalty( const IntensityNumber &s ) { trace_data.spread_penalty = s; }
		inline void set_intensity( const IntensityNumber &i ) { trace_data.intensity = i; }
		inline void set_random_id( const NodeID& r_id ){ random_id = r_id; }
		inline void set_current( const Node& _c ){ trace_data.current = _c; }
		inline void set_parent( const Node& _p ){ trace_data.parent = _p; }
		inline void set_grandparent( const Node& _g ){ trace_data.grandparent = _g; }
		inline void set_furthest_id( const NodeID& f_id ){ furthest_id = f_id; }
		inline void print( Debug& debug )
		{
			debug.debug( "Trace (size %i) :", get_buffer_size() );
			debug.debug( "diminish_seconds (size %i) : %i", sizeof( trace_data.diminish_seconds ), trace_data.diminish_seconds );
			debug.debug( "diminish_amount (size %i) : %i", sizeof( trace_data.diminish_amount ), trace_data.diminish_amount );
			debug.debug( "spread_penalty (size %i) : %i", sizeof( trace_data.spread_penalty ), trace_data.spread_penalty );
			debug.debug( "intensity (size %i) : %i", sizeof( trace_data.intensity ), trace_data.intensity );
			debug.debug( "start_time (size %i) : %i", sizeof( trace_data.start_time ), trace_data.start_time  );
			debug.debug( "target id (size %i) : %x", sizeof( trace_data.target_id ), trace_data.target_id );
			trace_data.current.print( debug );
			trace_data.parent.print( debug );
			trace_data.grandparent.print( debug );
			debug.debug( "inhibited (size %i) : %i", sizeof( inhibited ), inhibited );
		}
		// --------------------------------------------------------------------
	private:
		PLTT_TraceData trace_data;
		uint8_t inhibited;
		NodeID random_id;
		NodeID furthest_id;
	};
}
#endif
