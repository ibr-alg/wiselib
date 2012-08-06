/**************************************************************************
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

#include "PLTT_source_config.h"

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
		// --------------------------------------------------------------------
		inline PLTT_TraceType():
			start_time			(0),
			diminish_seconds	(0),
			diminish_amount		(0),
			spread_penalty		(0),
			intensity			(0),
			inhibited			(0),
			target_id			(0),
			recipient_1_id		(0),
			recipient_2_id		(0)
		{
			max_intensity = 2;
			for ( IntensityNumber i = 0; i < ( sizeof(max_intensity) * 8 ); i++ )
			{
				max_intensity = 2 * max_intensity;
			}
			max_intensity = max_intensity - 1;
		}
		// --------------------------------------------------------------------
		inline PLTT_TraceType( const SecondsNumber& _ds, const IntensityNumber& _da, const IntensityNumber& _sp, const IntensityNumber& _si, const TimesNumber& _st )
		{
			diminish_seconds = _ds;
			diminish_amount = _da;
			spread_penalty = _sp;
			intensity = _si;
			start_time = _st;
			inhibited = 0;
			max_intensity = 2;
			for ( IntensityNumber i = 0; i < ( sizeof(max_intensity) * 8 ); i++ )
			{
				max_intensity = 2 * max_intensity;
			}
			max_intensity = max_intensity - 1;
			recipient_1_id = 0;
			recipient_2_id = 0;
			target_id = 0;
		}
		// --------------------------------------------------------------------
		inline PLTT_TraceType( const self_type& _t )
		{
			*this = _t;
		}
		// --------------------------------------------------------------------
		inline PLTT_TraceType( block_data_t* _buff, size_t _offset = 0 )
		{
			de_serialize( _buff, _offset );
			inhibited = 0;
			max_intensity = 2;
			for (IntensityNumber i = 0; i < ( sizeof(max_intensity) * 8 ); i++ )
			{
				max_intensity = 2*max_intensity;
			}
			max_intensity = max_intensity - 1;
		}
		// --------------------------------------------------------------------
		inline block_data_t* serialize( block_data_t* _buff, size_t _offset = 0 )
		{
			size_t CURRENT_NODE_POS = 0;
			size_t PARENT_NODE_POS = current.serial_size() + CURRENT_NODE_POS;
			size_t GRANDPARENT_NODE_POS = parent.serial_size() + PARENT_NODE_POS;
			size_t TARGET_ID_POS = grandparent.serial_size() + GRANDPARENT_NODE_POS;
			size_t START_TIME_POS = sizeof(NodeID) + TARGET_ID_POS;
			size_t DIMINISH_SECS_POS = sizeof(TimesNumber) + START_TIME_POS;
			size_t DIMINISH_AMOUNT_POS = sizeof(SecondsNumber) + DIMINISH_SECS_POS;
			size_t SPREAD_PENALTY_POS = sizeof(IntensityNumber) + DIMINISH_AMOUNT_POS;
			size_t INTENSITY_POS = sizeof(IntensityNumber) + SPREAD_PENALTY_POS;
			size_t RECIPIENT_1_ID_POS = sizeof(IntensityNumber) + INTENSITY_POS;
			size_t RECIPIENT_2_ID_POS = sizeof(NodeID) + RECIPIENT_1_ID_POS;
			current.serialize( _buff, CURRENT_NODE_POS + _offset );
			parent.serialize( _buff, PARENT_NODE_POS + _offset);
			grandparent.serialize( _buff, GRANDPARENT_NODE_POS + _offset);
			write<Os, block_data_t, NodeID> ( _buff + TARGET_ID_POS + _offset, target_id );
			write<Os, block_data_t, TimesNumber> ( _buff + START_TIME_POS + _offset, start_time );
			write<Os, block_data_t, SecondsNumber> ( _buff + DIMINISH_SECS_POS + _offset, diminish_seconds );
			write<Os, block_data_t, IntensityNumber> ( _buff + DIMINISH_AMOUNT_POS + _offset, diminish_amount );
			write<Os, block_data_t, IntensityNumber> ( _buff + SPREAD_PENALTY_POS + _offset, spread_penalty );
			write<Os, block_data_t, IntensityNumber> ( _buff + INTENSITY_POS + _offset, intensity );
			write<Os, block_data_t, NodeID> ( _buff + RECIPIENT_1_ID_POS + _offset, recipient_1_id );
			write<Os, block_data_t, NodeID> ( _buff + RECIPIENT_2_ID_POS + _offset, recipient_2_id );
			return _buff;
		}
		// --------------------------------------------------------------------
		inline void de_serialize(block_data_t* _buff, size_t _offset = 0)
		{
			size_t CURRENT_NODE_POS = 0;
			size_t PARENT_NODE_POS = current.serial_size() + CURRENT_NODE_POS;
			size_t GRANDPARENT_NODE_POS = parent.serial_size() + PARENT_NODE_POS;
			size_t TARGET_ID_POS = grandparent.serial_size() + GRANDPARENT_NODE_POS;
			size_t START_TIME_POS = sizeof(NodeID) + TARGET_ID_POS;
			size_t DIMINISH_SECS_POS = sizeof(TimesNumber) + START_TIME_POS;
			size_t DIMINISH_AMOUNT_POS = sizeof(SecondsNumber) + DIMINISH_SECS_POS;
			size_t SPREAD_PENALTY_POS = sizeof(IntensityNumber) + DIMINISH_AMOUNT_POS;
			size_t INTENSITY_POS = sizeof(IntensityNumber) + SPREAD_PENALTY_POS;
			size_t RECIPIENT_1_ID_POS = sizeof(IntensityNumber) + INTENSITY_POS;
			size_t RECIPIENT_2_ID_POS = sizeof(NodeID) + RECIPIENT_1_ID_POS;
			current.de_serialize( _buff, CURRENT_NODE_POS + _offset );
			parent.de_serialize( _buff, PARENT_NODE_POS + _offset );
			grandparent.de_serialize( _buff, GRANDPARENT_NODE_POS + _offset );
			target_id = read<Os, block_data_t, NodeID> ( _buff + TARGET_ID_POS + _offset );
			start_time = read<Os, block_data_t, TimesNumber>( _buff + START_TIME_POS + _offset );
			diminish_seconds = read<Os, block_data_t, SecondsNumber> ( _buff + DIMINISH_SECS_POS + _offset );
			diminish_amount = read<Os, block_data_t, IntensityNumber> ( _buff + DIMINISH_AMOUNT_POS + _offset );
			spread_penalty = read<Os, block_data_t, IntensityNumber> ( _buff + SPREAD_PENALTY_POS + _offset );
			intensity = read<Os, block_data_t, IntensityNumber> ( _buff + INTENSITY_POS + _offset );
			recipient_1_id = read<Os, block_data_t, NodeID> ( _buff + RECIPIENT_1_ID_POS + _offset );
			recipient_2_id = read<Os, block_data_t, NodeID> ( _buff + RECIPIENT_2_ID_POS + _offset );
		}
		// --------------------------------------------------------------------
		inline size_t serial_size()
		{
			size_t CURRENT_NODE_POS = 0;
			size_t PARENT_NODE_POS = current.serial_size() + CURRENT_NODE_POS;
			size_t GRANDPARENT_NODE_POS = parent.serial_size() + PARENT_NODE_POS;
			size_t TARGET_ID_POS = grandparent.serial_size() + GRANDPARENT_NODE_POS;
			size_t START_TIME_POS = sizeof(NodeID) + TARGET_ID_POS;
			size_t DIMINISH_SECS_POS = sizeof(TimesNumber) + START_TIME_POS;
			size_t DIMINISH_AMOUNT_POS = sizeof(SecondsNumber) + DIMINISH_SECS_POS;
			size_t SPREAD_PENALTY_POS = sizeof(IntensityNumber) + DIMINISH_AMOUNT_POS;
			size_t INTENSITY_POS = sizeof(IntensityNumber) + SPREAD_PENALTY_POS;
			size_t RECIPIENT_1_ID_POS = sizeof(IntensityNumber) + INTENSITY_POS;
			size_t RECIPIENT_2_ID_POS = sizeof(NodeID) + RECIPIENT_1_ID_POS;
			return RECIPIENT_2_ID_POS + sizeof(NodeID);
		}
		// --------------------------------------------------------------------
		inline self_type& operator=( const self_type& _t )
		{
			inhibited = _t.inhibited;
			target_id = _t.target_id;
			start_time = _t.start_time;
			diminish_seconds = _t.diminish_seconds;
			spread_penalty = _t.spread_penalty;
			intensity = _t.intensity;
			recipient_1_id = _t.recipient_1_id;
			recipient_2_id = _t.recipient_2_id;
			current = _t.current;
			parent = _t.parent;
			grandparent = _t.grandparent;
			diminish_amount = _t.diminish_amount;
			max_intensity = _t.max_intensity;
			return *this;
		}
		// --------------------------------------------------------------------
		inline void update_intensity_diminish()
		{
			if ( intensity < diminish_amount )
			{
				intensity = 0;
				return;
			}
			intensity = intensity - diminish_amount;
		}
		// --------------------------------------------------------------------
		inline void update_intensity_penalize()
		{
			if ( intensity < spread_penalty )
			{
				intensity = 0;
				return;
			}
			intensity = intensity - spread_penalty;
		}
		// --------------------------------------------------------------------
		inline void update_path( const Node& _n )
		{
			grandparent = parent;
			parent = current;
			current = _n;
		}
		// --------------------------------------------------------------------
		inline void update_start_time()
		{
			start_time = start_time + 1;
		}
		// --------------------------------------------------------------------
		inline IntensityNumber get_diminish_amount()
		{
			return diminish_amount;
		}
		// --------------------------------------------------------------------
		inline SecondsNumber get_diminish_seconds()
		{
			return diminish_seconds;
		}
		// --------------------------------------------------------------------
		inline IntensityNumber get_intensity()
		{
			return intensity;
		}
		// --------------------------------------------------------------------
		inline NodeID get_target_id()
		{
			return target_id;
		}
		// --------------------------------------------------------------------
		inline Node get_current()
		{
			return current;
		}
		// --------------------------------------------------------------------
		inline Node get_parent()
		{
			return parent;
		}
		// --------------------------------------------------------------------
		inline Node get_grandparent()
		{
			return grandparent;
		}
		// --------------------------------------------------------------------
		inline NodeID get_recipient_1_id()
		{
			return recipient_1_id;
		}
		// --------------------------------------------------------------------
		inline NodeID get_recipient_2_id()
		{
			return recipient_2_id;
		}
		// --------------------------------------------------------------------
		inline SecondsNumber get_inhibited()
		{
			return inhibited;
		}
		// --------------------------------------------------------------------
		inline TimesNumber get_start_time()
		{
			return start_time;
		}
		// --------------------------------------------------------------------
		inline IntensityNumber get_spread_penalty()
		{
			return spread_penalty;
		}
		// --------------------------------------------------------------------
		inline IntensityNumber get_max_intensity()
		{
			return max_intensity;
		}
		// --------------------------------------------------------------------
		inline Node get_repulsion_point()
		{
			if ( grandparent.get_id() != 0 ) { return grandparent; }
			if ( parent.get_id() != 0 ) { return parent; }
			return current;
		}
		// --------------------------------------------------------------------
		inline void set_target_id( const NodeID& tarid )
		{
			target_id = tarid;
		}
		// --------------------------------------------------------------------
		inline void set_inhibited()
		{
			inhibited = inhibited + 1;
		}
		// --------------------------------------------------------------------
		inline void set_inhibited( const uint8_t& n )
		{
			inhibited = n;
		}
		// --------------------------------------------------------------------
		inline void set_start_time( const TimesNumber& t )
		{
			start_time = t;
		}
		// --------------------------------------------------------------------
		inline void set_diminish_seconds( const SecondsNumber& s )
		{
			diminish_seconds = s;
		}
		// --------------------------------------------------------------------
		inline void set_diminish_amount( const IntensityNumber &d )
		{
			diminish_amount = d;
		}
		// --------------------------------------------------------------------
		inline void set_spread_penalty( const IntensityNumber &s )
		{
			spread_penalty = s;
		}
		// --------------------------------------------------------------------
		inline void set_intensity( const IntensityNumber &i )
		{
			intensity = i;
		}
		// --------------------------------------------------------------------
		inline void set_current( const Node& _c )
		{
			current = _c;
		}
		// --------------------------------------------------------------------
		inline void set_parent( const Node& _p )
		{
			parent = _p;
		}
		// --------------------------------------------------------------------
		inline void set_grandparent( const Node& _g )
		{
			grandparent = _g;
		}
		// --------------------------------------------------------------------
		inline void set_recipient_1_id( const NodeID& r_1_id )
		{
			recipient_1_id = r_1_id;
		}
		// --------------------------------------------------------------------
		inline void set_recipient_2_id( const NodeID& r_2_id )
		{
			recipient_2_id = r_2_id;
		}
		// --------------------------------------------------------------------
#ifdef	DEBUG_PLTT_TRACE_H
		inline void print( Debug& _debug, Radio& _radio )
		{
			_debug.debug( "-------------------------------------------------------\n" );
			_debug.debug( "PLTT_Trace : \n" );
			_debug.debug( "diminish_seconds (size %i) : %i\n", sizeof(diminish_seconds), diminish_seconds );
			_debug.debug( "diminish_amount (size %i) : %i\n", sizeof(diminish_amount), diminish_amount );
			_debug.debug( "spread_penalty (size %i) : %i\n", sizeof(spread_penalty), spread_penalty );
			_debug.debug( "intensity (size %i) : %i\n", sizeof(intensity), intensity );
			_debug.debug( "max_intensity (size %ud) : %ud\n", sizeof(max_intensity), max_intensity );
			_debug.debug( "start_time (size %i) : %i\n", sizeof(start_time), start_time );
			_debug.debug( "target_id (size %i) : %x", sizeof(target_id), target_id );
			_debug.debug( "recipient_1_id (size %i) : %i\n", sizeof(recipient_1_id), recipient_1_id );
			_debug.debug( "recipient_2_id (size %i) : %i\n", sizeof(recipient_2_id), recipient_2_id );
			_debug.debug( "inhibited (size %i) : %i\n", sizeof(inhibited), inhibited );
			_debug.debug( "current, parent, grandparent : \n" );
			current.print( _debug, _radio );
			parent.print( _debug, _radio );
			grandparent.print( _debug, _radio );
			_debug.debug( "-------------------------------------------------------\n" );
		}
#endif
		// --------------------------------------------------------------------
	private:
		Node current;
		Node parent;
		Node grandparent;
		TimesNumber start_time;
		SecondsNumber diminish_seconds;
		IntensityNumber diminish_amount;
		IntensityNumber spread_penalty;
		IntensityNumber intensity;
		uint8_t inhibited;
		NodeID target_id;
		NodeID recipient_1_id;
		NodeID recipient_2_id;
		IntensityNumber max_intensity;
	};
}
#endif
