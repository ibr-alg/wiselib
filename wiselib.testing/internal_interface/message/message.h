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

#ifndef __MESSAGE_H__
#define __MESSAGE_H__
#include "message_source_config.h"
#include "util/serialization/simple_types.h"

namespace wiselib
{
	template<	typename OsModel_P,
				typename Radio_P,
				typename Debug_P>
	class Message_Type
	{
	public:
		typedef OsModel_P OsModel;
		typedef Radio_P Radio;
		typedef Debug_P Debug;
		typedef typename Radio::block_data_t block_data_t;
		typedef typename Radio::size_t size_t;
		typedef typename Radio::message_id_t message_id_t;
		// --------------------------------------------------------------------
		inline Message_Type()
		{
			set_message_id( 0 );
			set_payload( 0, buffer );
		}
		// --------------------------------------------------------------------
		inline message_id_t get_message_id()
		{
			size_t MESSAGE_ID_POS = 0;
			return read<OsModel, block_data_t, message_id_t> ( buffer + MESSAGE_ID_POS );
		};
		// --------------------------------------------------------------------
		inline void set_message_id( message_id_t _id )
		{
			size_t MESSAGE_ID_POS = 0;
			write<OsModel, block_data_t, message_id_t> ( buffer + MESSAGE_ID_POS, _id );
		}
		// --------------------------------------------------------------------
		inline size_t get_payload_size()
		{
			size_t MESSAGE_ID_POS = 0;
#ifdef MESSAGE_H_FLETCHER_CHECKSUM
			size_t CSUM_POS = MESSAGE_ID_POS + sizeof(message_id_t);
			size_t PAYLOAD_SIZE_POS = CSUM_POS + sizeof(uint16_t);
#else
			size_t PAYLOAD_SIZE_POS = MESSAGE_ID_POS + sizeof(message_id_t);
#endif
			return read<OsModel, block_data_t, size_t> (buffer + PAYLOAD_SIZE_POS );
		}
		// --------------------------------------------------------------------
		inline block_data_t* get_payload()
		{
			size_t MESSAGE_ID_POS = 0;
#ifdef MESSAGE_H_FLETCHER_CHECKSUM
			size_t CSUM_POS = MESSAGE_ID_POS + sizeof(message_id_t);
			size_t PAYLOAD_SIZE_POS = CSUM_POS + sizeof(uint16_t);
#else
			size_t PAYLOAD_SIZE_POS = MESSAGE_ID_POS + sizeof(message_id_t);
#endif
			size_t PAYLOAD_POS = PAYLOAD_SIZE_POS + sizeof(size_t);
			return buffer + PAYLOAD_POS;
		}
		// --------------------------------------------------------------------
		inline void set_payload( size_t _len, block_data_t* _buff, size_t _offset = 0 )
		{
			size_t MESSAGE_ID_POS = 0;
#ifdef MESSAGE_H_FLETCHER_CHECKSUM
			size_t CSUM_POS = MESSAGE_ID_POS + sizeof(message_id_t);
			size_t PAYLOAD_SIZE_POS = CSUM_POS + sizeof(uint16_t);
			uint16_t csum = fletcher16_checksum( _buff + _offset, _len);
			write<OsModel, block_data_t, uint16_t> ( buffer + _offset + CSUM_POS, csum );
#else
			size_t PAYLOAD_SIZE_POS = MESSAGE_ID_POS + sizeof(message_id_t);
#endif
			size_t PAYLOAD_POS = PAYLOAD_SIZE_POS + sizeof(size_t);
			write<OsModel, block_data_t, size_t> ( buffer + _offset + PAYLOAD_SIZE_POS, _len );
			memcpy( buffer + PAYLOAD_POS, _buff, _len );
		}
		// --------------------------------------------------------------------
		inline size_t serial_size()
		{
			size_t MESSAGE_ID_POS = 0;
#ifdef MESSAGE_H_FLETCHER_CHECKSUM
			size_t CSUM_POS = MESSAGE_ID_POS + sizeof(message_id_t);
			size_t PAYLOAD_SIZE_POS = CSUM_POS + sizeof(uint16_t);
#else
			size_t PAYLOAD_SIZE_POS = MESSAGE_ID_POS + sizeof(message_id_t);
#endif
			size_t PAYLOAD_POS = PAYLOAD_SIZE_POS + sizeof(size_t);
			return PAYLOAD_POS + get_payload_size();
		}
		// --------------------------------------------------------------------
		inline block_data_t* serialize( block_data_t* _buff = NULL )
		{
			return buffer;
		}
		// --------------------------------------------------------------------
		inline void de_serialize( block_data_t* _buff, size_t _offset = 0 )
		{
			memcpy( buffer + _offset, _buff, Radio::MAX_MESSAGE_LENGTH );
		}
		// --------------------------------------------------------------------
		Message_Type& operator=( const Message_Type& _msg )
		{
			size_t len = _msg.serial_size();
			memcpy( buffer, _msg.buffer, len );
			return *this;
		}
		// --------------------------------------------------------------------
		inline void print( Debug& _debug, Radio& _radio )
		{
			_debug.debug( "-------------------------------------------------------\n" );
			_debug.debug( "Message : \n" );
			_debug.debug( "serial_size: %d\n", serial_size() );
			_debug.debug( "message_id (size %i) : %d\n", sizeof(message_id_t), get_message_id() );
#ifdef MESSAGE_H_FLETCHER_CHECKSUM
			_debug.debug( "checksum (size %i) : %d\n", sizeof(uint16_t), csum() );
#endif
			_debug.debug( "payload_size (size %i): %d\n", sizeof(size_t), get_payload_size() );
			_debug.debug( "payload : \n");
			for (size_t i = 0; i < get_payload_size(); i++ )
			{
				_debug.debug( "%d", get_payload()[i] );
			}
			_debug.debug( "-------------------------------------------------------\n" );
		}
		// --------------------------------------------------------------------
#ifdef MESSAGE_H_FLETCHER_CHECKSUM
		inline uint16_t fletcher16_checksum( uint8_t const* _data, size_t _bytes )
		{
		        uint16_t sum1 = 0xff, sum2 = 0xff;
		        while ( _bytes )
		        {
		                size_t tlen = _bytes > 20 ? 20 : _bytes;
		                _bytes -= tlen;
		                do
		                {
		                        sum2 += sum1 += *_data++;
		                }
		                while ( --tlen );
		                sum1 = ( sum1 & 0xff ) + ( sum1 >> 8 );
		                sum2 = ( sum2 & 0xff ) + ( sum2 >> 8 );
		        }
		        sum1 = ( sum1 & 0xff ) + ( sum1 >> 8 );
		        sum2 = ( sum2 & 0xff ) + ( sum2 >> 8 );
		        return sum2 << 8 | sum1;
		}
		// --------------------------------------------------------------------
		inline uint8_t compare_checksum()
		{
			size_t MESSAGE_ID_POS = 0;
			size_t CSUM_POS = MESSAGE_ID_POS + sizeof(message_id_t);
			size_t PAYLOAD_SIZE_POS = CSUM_POS + sizeof(uint16_t);
			size_t PAYLOAD_POS = PAYLOAD_SIZE_POS + sizeof(size_t);
			uint16_t csum_transmitted = read<OsModel, block_data_t, uint16_t> ( buffer + CSUM_POS );
			uint16_t csum_actual = fletcher16_checksum( buffer + PAYLOAD_POS, get_payload_size() );
			return ( csum_transmitted == csum_actual );
		}
		// --------------------------------------------------------------------
		inline uint16_t csum()
		{
			size_t MESSAGE_ID_POS = 0;
			size_t CSUM_POS = MESSAGE_ID_POS + sizeof(message_id_t);
			return read<OsModel, block_data_t, uint16_t> ( buffer + CSUM_POS );
		}
#endif
	private:
		block_data_t buffer[Radio::MAX_MESSAGE_LENGTH];
	};
}
#endif
