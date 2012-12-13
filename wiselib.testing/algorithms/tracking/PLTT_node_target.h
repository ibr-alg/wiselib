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

#ifndef __PLTT_NODE_TARGET_H__
#define __PLTT_NODE_TARGET_H__

#include "PLTT_source_config.h"

namespace wiselib
{
	template<	typename Os_P,
				typename Radio_P,
				typename NodeID_P,
				typename IntensityNumber_P,
				typename Debug_P>
	class PLTT_NodeTargetType
	{
	public:
		typedef Os_P Os;
		typedef Radio_P Radio;
		typedef	NodeID_P NodeID;
		typedef IntensityNumber_P IntensityNumber;
		typedef Debug_P Debug;
		typedef typename Radio::block_data_t block_data_t;
		typedef typename Radio::size_t size_t;
		typedef PLTT_NodeTargetType<Os, Radio, NodeID, IntensityNumber, Debug> self_type;
		// --------------------------------------------------------------------
		PLTT_NodeTargetType()
		{}
		// --------------------------------------------------------------------
		PLTT_NodeTargetType( const self_type& _nt )
		{
			*this = _nt;
		}
		// --------------------------------------------------------------------
		PLTT_NodeTargetType( block_data_t* _buff, size_t _offset = 0 )
		{
			de_serialize( _buff, _offset );
		}
		// --------------------------------------------------------------------
		PLTT_NodeTargetType( const NodeID& _id, const IntensityNumber& _in )
		{
			set_all( _id, _in );
		}
		// --------------------------------------------------------------------
		inline block_data_t* serialize( block_data_t* buff, size_t offset = 0 )
		{
			uint8_t TARGET_ID_POS = 0;
			uint8_t INTENSITY_POS = TARGET_ID_POS + sizeof(NodeID);
			write<Os, block_data_t, NodeID>( buff + TARGET_ID_POS + offset, target_id );
			write<Os, block_data_t, IntensityNumber>( buff + INTENSITY_POS + offset, intensity );
			return buff;
		}
		// --------------------------------------------------------------------
		inline void de_serialize(block_data_t* _buff, size_t _offset = 0 )
		{
			uint8_t TARGET_ID_POS = 0;
			uint8_t INTENSITY_POS = TARGET_ID_POS + sizeof(NodeID);
			target_id = read<Os, block_data_t, NodeID> ( _buff + TARGET_ID_POS + _offset );
			intensity = read<Os, block_data_t, IntensityNumber> ( _buff + INTENSITY_POS + _offset );
		}
		// --------------------------------------------------------------------
		inline size_t serial_size()
		{
			uint8_t TARGET_ID_POS = 0;
			uint8_t INTENSITY_POS = TARGET_ID_POS + sizeof(NodeID);
			return INTENSITY_POS + sizeof(IntensityNumber);
		}
		// --------------------------------------------------------------------
		inline self_type& operator=( const self_type& _nt )
		{
			target_id = _nt.target_id;
			intensity = _nt.intensity;
			return *this;
		}
		// --------------------------------------------------------------------
		inline void set_target_id( const NodeID& _tid )
		{
			target_id = _tid;
		}
		// --------------------------------------------------------------------
		inline void set_intensity( const IntensityNumber& _i )
		{
			intensity = _i;
		}
		// --------------------------------------------------------------------
		inline void set_all( const NodeID& _tid, const IntensityNumber& _i )
		{
			target_id = _tid;
			intensity = _i;
		}
		// --------------------------------------------------------------------
		inline NodeID get_target_id()
		{
			return target_id;
		}
		// --------------------------------------------------------------------
		inline IntensityNumber get_intensity()
		{
			return intensity;
		}
		// --------------------------------------------------------------------
#ifdef DEBUG_PLTT_NODE_TARGET_H
		inline void print( Debug& _debug, Radio& _radio )
		{
			_debug.debug( "-------------------------------------------------------\n" );
			_debug.debug( "NodeTarget : \n" );
			_debug.debug( "target_id (size %i) : %x\n", sizeof( NodeID ), target_id );
			_debug.debug( "intensity (size %i) : %i\n", sizeof( IntensityNumber ), intensity );
			_debug.debug( "-------------------------------------------------------\n" );
		}
#endif
		// --------------------------------------------------------------------
	private:
		NodeID target_id;
		IntensityNumber intensity;
	};
}
#endif
