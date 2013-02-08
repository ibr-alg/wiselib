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
#ifndef __PRIVACY_MSG_H__
#define __PRIVACY_MSG_H__

namespace wiselib
{
	template<	typename Os_P,
				typename Radio_P>
	class PrivacyMessageType
	{
	public:
		typedef Os_P Os;
		typedef Radio_P Radio;
		typedef typename Radio::block_data_t block_data_t;
		typedef typename Radio::size_t size_t;
		typedef typename Radio::message_id_t message_id_t;
		typedef PrivacyMessageType<Os, Radio> self_type;
		// --------------------------------------------------------------------
		inline PrivacyMessageType()
		{
			set_msg_id( 0 );
			size_t len = 0;
			write<Os, block_data_t, size_t>( buff + PAYLOAD_LEN_POS, len );
		}
		// --------------------------------------------------------------------
		inline message_id_t msg_id()
		{
			return read<Os, block_data_t, message_id_t>( buff + MSG_ID_POS );
		};
		// --------------------------------------------------------------------
		inline void set_msg_id( message_id_t msg_id )
		{
			write<Os, block_data_t, message_id_t>( buff + MSG_ID_POS, msg_id );
		}
		// --------------------------------------------------------------------
		inline uint16_t request_id()
		{
			return read<Os, block_data_t, uint16_t>( buff + REQ_ID_POS );
		}
		// --------------------------------------------------------------------
		inline void set_request_id( uint16_t req_id )
		{
			write<Os, block_data_t, uint16_t>( buff + REQ_ID_POS, req_id );
		}
		// --------------------------------------------------------------------
		inline size_t payload_size()
		{
			return read<Os, block_data_t, size_t>( buff + PAYLOAD_LEN_POS );
		}
		// --------------------------------------------------------------------
		inline block_data_t* payload()
		{ 
			return buff + PAYLOAD_POS;
		}
		// --------------------------------------------------------------------
		inline void set_payload( size_t len, block_data_t *buf )
		{
			write<Os, block_data_t, size_t>( buff + PAYLOAD_LEN_POS, len );
			for (size_t i = 0; i < len; ++i )
			{
				write<Os, block_data_t, block_data_t>( buff + PAYLOAD_POS + i, *( buf + i ) );
			}
		}
		// --------------------------------------------------------------------
		inline size_t buffer_size()
		{
			return PAYLOAD_POS + payload_size();
		}
		// --------------------------------------------------------------------
		inline block_data_t* buffer()
		{
			return buff;
		}
		// --------------------------------------------------------------------
		inline self_type& operator=( const self_type& _p )
		{
			for ( size_t i = 0; i < Radio::MAX_MESSAGE_LENGTH; i++ )
			{
				buff[i] = _p.buff[i];
			}
			return *this;
		}
		// --------------------------------------------------------------------
	private:
		enum data_positions
		{
			MSG_ID_POS = 0,
			REQ_ID_POS = MSG_ID_POS + sizeof( message_id_t),
			PAYLOAD_LEN_POS = REQ_ID_POS + sizeof( uint16_t ),
			PAYLOAD_POS = PAYLOAD_LEN_POS + sizeof( size_t )
		};
		block_data_t buff[Radio::MAX_MESSAGE_LENGTH];
	};
}
#endif
