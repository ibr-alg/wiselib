#ifndef __RELIABLE_RADIO_MESSAGE_H__
#define	__RELIABLE_RADIO_MESSAGE_H__

#include "reliable_radio_source_config.h"

namespace wiselib
{
	template<	typename Os_P,
				typename Radio_P,
				typename Debug_P>
	class ReliableRadioMessage_Type
	{
	public:
		typedef Os_P Os;
		typedef Radio_P Radio;
		typedef Debug_P Debug;
		typedef typename Radio::message_id_t message_id_t;
		typedef typename Radio::block_data_t block_data_t;
		typedef typename Radio::node_id_t node_id_t;
		typedef typename Radio::size_t size_t;
		typedef ReliableRadioMessage_Type<Os, Radio, Debug> self_t;
		// --------------------------------------------------------------------
		ReliableRadioMessage_Type() :
			message_id				( 0 ),
			counter					( 0 ),
			payload_size			( 0 ),
			destination				( 0 )
		{};
		// --------------------------------------------------------------------
		~ReliableRadioMessage_Type()
		{};
		// --------------------------------------------------------------------
		self_t& operator=( const self_t& _rrm )
		{
			message_id = _rrm.message_id;
			counter = _rrm.counter;
			payload_size = _rrm.payload_size;
			destination = _rrm.destination;
			memcpy( payload, _rrm.payload, payload_size );
			return *this;
		}
		// --------------------------------------------------------------------
		message_id_t get_message_id()
		{
			return message_id;
		}
		// --------------------------------------------------------------------
		void set_message_id( message_id_t _msg_id )
		{
			message_id = _msg_id;
		}
		// --------------------------------------------------------------------
		void set_payload( size_t _len, block_data_t* _buff )
		{
			payload_size = _len;
			memcpy( payload, _buff, _len );
		}
		// --------------------------------------------------------------------
		block_data_t* get_payload()
		{
			return payload;
		}
		// --------------------------------------------------------------------
		size_t get_payload_size()
		{
			return payload_size;
		}
		// --------------------------------------------------------------------
		void set_destination( node_id_t _nid )
		{
			destination = _nid;
		}
		// --------------------------------------------------------------------
		node_id_t get_destination()
		{
			return destination;
		}
		// --------------------------------------------------------------------
		uint32_t get_counter()
		{
			return counter;
		}
		// --------------------------------------------------------------------
		void inc_counter()
		{
			counter = counter + 1;
		}
		// --------------------------------------------------------------------
		void set_counter( uint32_t _c )
		{
			counter = _c;
		}
		// --------------------------------------------------------------------
		block_data_t* serialize( block_data_t* _buff, size_t _offset = 0 )
		{
			size_t MSG_ID_POS = 0;
			size_t DATA_LEN_POS = MSG_ID_POS + sizeof(message_id_t);
			size_t DATA_POS = DATA_LEN_POS + sizeof(size_t);
			write<Os, block_data_t, message_id_t>( _buff + MSG_ID_POS + _offset,  message_id );
			write<Os, block_data_t, size_t>( _buff + DATA_LEN_POS + _offset, payload_size );
			memcpy( _buff + DATA_POS + _offset, payload, payload_size );
			return _buff;
		}
		// --------------------------------------------------------------------
		void de_serialize( block_data_t* _buff, size_t _offset = 0 )
		{
			size_t MSG_ID_POS = 0;
			size_t DATA_LEN_POS = MSG_ID_POS + sizeof(message_id_t);
			size_t DATA_POS = DATA_LEN_POS + sizeof(size_t);
			message_id = read<Os, block_data_t, message_id_t>( _buff + MSG_ID_POS + _offset );
			payload_size = read<Os, block_data_t, size_t>( _buff + DATA_LEN_POS + _offset );
			memcpy( payload, _buff + DATA_POS + _offset, payload_size );
		}
		// --------------------------------------------------------------------
		size_t serial_size()
		{
			size_t MSG_ID_POS = 0;
			size_t DATA_LEN_POS = MSG_ID_POS + sizeof(message_id_t);
			size_t DATA_POS = DATA_LEN_POS + sizeof(size_t);
			return DATA_POS + payload_size;
		}
		// --------------------------------------------------------------------
#ifdef DEBUG_RELIABLE_RADIO_H
		void print( Debug& _debug, Radio& _radio )
		{
			_debug.debug( "-------------------------------------------------------\n");
			_debug.debug( "ReliableRadioMessage : \n" );
			_debug.debug( "message_id (size %i) : %d\n", sizeof(message_id_t), message_id );
			_debug.debug( "counter (size %i) : %d\n", sizeof(uint32_t), counter );
			_debug.debug( "destination (size %i) : %d\n", sizeof(node_id_t), destination );
			_debug.debug( "payload_size (size %i) : %d\n", sizeof(size_t), payload_size );
			_debug.debug( "payload: \n");
			for ( size_t i = 0; i < payload_size; i++ )
			{
				_debug.debug("%d", payload[i] );
			}
			_debug.debug( "-------------------------------------------------------\n");
		}
#endif
		// --------------------------------------------------------------------
	private:
		message_id_t message_id;
		uint32_t counter;
		block_data_t payload[Radio::MAX_MESSAGE_LENGTH];
		size_t payload_size;
		node_id_t destination;
    };
}
#endif
