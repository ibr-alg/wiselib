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

#ifndef __PROTOCOL_PAYLOAD_H__
#define	__PROTOCOL_PAYLOAD_H__

#include "neighbor_discovery_default_values_config.h"

namespace wiselib
{
	template< 	typename Os_P,
				typename Radio_P,
				typename Debug_P>
	class ProtocolPayload_Type
	{
		typedef Os_P Os;
		typedef Radio_P Radio;
		typedef Debug_P Debug;
		typedef typename Radio::node_id_t node_id_t;
		typedef typename Radio::size_t size_t;
		typedef typename Radio::block_data_t block_data_t;
		typedef ProtocolPayload_Type<Os, Radio, Debug> self_type;
	public:
		ProtocolPayload_Type() :
			payload_size	( 0 )
		{}
		// --------------------------------------------------------------------
		ProtocolPayload_Type( uint8_t _pid, size_t _ps, block_data_t* _pd, size_t _offset = 0 )
		{
			if ( _ps <= ND_MAX_PROTOCOL_PAYLOAD_SIZE )
			{
				protocol_id = _pid;
				payload_size = _ps;
				for ( size_t i = 0; i < ND_MAX_PROTOCOL_PAYLOAD_SIZE; i++ )
				{
					payload_data[i] = 0;
				}
				for ( size_t i = 0; i < payload_size; i++ )
				{
					payload_data[i] = _pd[i + _offset];
				}
			}
		}
		// --------------------------------------------------------------------
		~ProtocolPayload_Type()
		{}
		// --------------------------------------------------------------------
		uint8_t get_protocol_id()
		{
			return protocol_id;
		}
		// --------------------------------------------------------------------
		void set_protocol_id( uint8_t _pid )
		{
			protocol_id = _pid;
		}
		// --------------------------------------------------------------------
		size_t get_payload_size()
		{
			return payload_size;
		}
		// --------------------------------------------------------------------
		size_t get_max_payload_size()
		{
			return ND_MAX_PROTOCOL_PAYLOAD_SIZE;
		}
		// --------------------------------------------------------------------
		void set_payload_size( size_t _ps )
		{
			payload_size = _ps;
		}
		// --------------------------------------------------------------------
		block_data_t* get_payload_data()
		{
			return payload_data;
		}
		// --------------------------------------------------------------------
		void set_payload_data( block_data_t* _pd, size_t _offset = 0 )
		{
			if ( payload_size <= ND_MAX_PROTOCOL_PAYLOAD_SIZE )
			{
				for ( size_t i = 0; i < payload_size; i++ )
				{
					payload_data[i] = _pd[i + _offset];
				}
			}
		}
		// --------------------------------------------------------------------
		void set_payload( block_data_t* _pd, size_t _ps, size_t _offset = 0 )
		{
			payload_size = _ps;
			if ( payload_size <= ND_MAX_PROTOCOL_PAYLOAD_SIZE )
			{
				for ( size_t i = 0; i < payload_size; i++ )
				{
					payload_data[i] = _pd[i + _offset];
				}
			}
		}
		// --------------------------------------------------------------------
		ProtocolPayload_Type& operator=( const ProtocolPayload_Type& _pp )
		{
			protocol_id = _pp.protocol_id;
			payload_size = _pp.payload_size;
			if ( payload_size <= ND_MAX_PROTOCOL_PAYLOAD_SIZE )
			{
				for ( size_t i = 0 ; i < payload_size; i++ )
				{
					payload_data[i] = _pp.payload_data[i];
				}
			}
			return *this;
		}
		// --------------------------------------------------------------------
#ifdef DEBUG_PROTOCOL_PAYLOAD_H
		void print( Debug& debug, Radio& radio )
		{
			debug.debug( "-------------------------------------------------------\n" );
			debug.debug( "ProtocolPayload : \n");
			debug.debug( "serial_size : %d\n", serial_size() );
			debug.debug( "protocol_id (size %i) : %d\n", sizeof(protocol_id), protocol_id );
			debug.debug( "max_payload_size : %d\n", ND_MAX_PROTOCOL_PAYLOAD_SIZE );
			debug.debug( "payload_size (size %i) : %d\n", sizeof(payload_size), payload_size );
			if ( payload_size <= ND_MAX_PROTOCOL_PAYLOAD_SIZE )
			{
				for ( size_t i = 0; i < payload_size; i++ )
				{
					debug.debug( "%d:%d\n", i, payload_data[i] );
				}
			}
			debug.debug( "-------------------------------------------------------");
		}
#endif
		// --------------------------------------------------------------------
		block_data_t* serialize( block_data_t* _buff, size_t _offset = 0 )
		{
			if ( payload_size != 0 )
			{
				size_t PROTOCOL_ID_POS = 0;
				size_t PAYLOAD_SIZE_POS = PROTOCOL_ID_POS + sizeof(uint8_t);
				size_t PAYLOAD_DATA_POS = PAYLOAD_SIZE_POS + sizeof(size_t);
				write<Os, block_data_t, uint8_t>( _buff + PROTOCOL_ID_POS + _offset, protocol_id );
				write<Os, block_data_t, size_t>( _buff + PAYLOAD_SIZE_POS + _offset, payload_size );
				if ( payload_size <= ND_MAX_PROTOCOL_PAYLOAD_SIZE )
				{
					for ( size_t i = 0 ; i < payload_size; i++ )
					{
						_buff[PAYLOAD_DATA_POS + i + _offset] = payload_data[i];
					}
				}
			}
			return _buff;

		}
		// --------------------------------------------------------------------
		void de_serialize( block_data_t* _buff, size_t _offset = 0 )
		{
			size_t PROTOCOL_ID_POS = 0;
			size_t PAYLOAD_SIZE_POS = PROTOCOL_ID_POS + sizeof(uint8_t);
			size_t PAYLOAD_DATA_POS = PAYLOAD_SIZE_POS + sizeof(size_t);
			protocol_id = read<Os, block_data_t, uint8_t>( _buff + PROTOCOL_ID_POS + _offset );
			payload_size = read<Os, block_data_t, size_t>( _buff + PAYLOAD_SIZE_POS + _offset );
			if ( payload_size <= ND_MAX_PROTOCOL_PAYLOAD_SIZE )
			{
				for ( size_t i = 0 ; i < payload_size; i++ )
				{
					 payload_data[i] = _buff[PAYLOAD_DATA_POS + i + _offset];
				}
			}
		}
		// --------------------------------------------------------------------
		size_t serial_size()
		{
			if ( payload_size != 0 )
			{
				size_t PROTOCOL_ID_POS = 0;
				size_t PAYLOAD_SIZE_POS = PROTOCOL_ID_POS + sizeof(uint8_t);
				size_t PAYLOAD_DATA_POS = PAYLOAD_SIZE_POS + sizeof(size_t);
				if ( payload_size <= ND_MAX_PROTOCOL_PAYLOAD_SIZE )
				{
					return PAYLOAD_DATA_POS + sizeof( block_data_t) * payload_size;
				}
				else
				{
					return PAYLOAD_SIZE_POS + sizeof(size_t);
				}
			}
			return 0;
		}
		// --------------------------------------------------------------------
	private:
		uint8_t protocol_id;
		size_t payload_size;
		block_data_t payload_data[ND_MAX_PROTOCOL_PAYLOAD_SIZE];
	};
}
#endif
