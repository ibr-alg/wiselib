#ifndef __FRAGMENT_H__
#define	__FRAGMENT_H__

#include "fragmenting_radio_source_config.h"

namespace wiselib
{
	template<	typename Os_P,
				typename Radio_P,
				typename Debug_P>
	class Fragment_Type
	{
	public:
		typedef Os_P Os;
		typedef Radio_P Radio;
		typedef Debug_P Debug;
		typedef typename Radio::message_id_t message_id_t;
		typedef typename Radio::size_t size_t;
		typedef typename Radio::block_data_t block_data_t;
		typedef typename Radio::node_id_t node_id_t;
		typedef Fragment_Type<Os, Radio, Debug> self_t;
		// --------------------------------------------------------------------
		Fragment_Type() :
			id						( 0 ),
			orig_id					( 0 ),
			seq_fragment			( 0 ),
			total_fragments			( 0 ),
			payload_size			( 0 )
		{};
		// --------------------------------------------------------------------
		~Fragment_Type()
		{};
		// --------------------------------------------------------------------
		self_t& operator=( const self_t& _frm )
		{
			id = _frm.id;
			orig_id = _frm.orig_id;
			seq_fragment = _frm.seq_fragment;
			payload_size = _frm.payload_size;
			total_fragments = _frm.total_fragments;
			memcpy( payload, _frm.payload, payload_size );
			return *this;
		}
		// --------------------------------------------------------------------
		uint16_t get_id()
		{
			return id;
		}
		// --------------------------------------------------------------------
		void set_id( uint16_t _id )
		{
			id = _id;
		}
		// --------------------------------------------------------------------
		message_id_t get_orig_id()
		{
			return orig_id;
		}
		// --------------------------------------------------------------------
		void set_orig_id( message_id_t _o_id )
		{
			orig_id = _o_id;
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
		void set_total_fragments( size_t _tf )
		{
			total_fragments = _tf;
		}
		// --------------------------------------------------------------------
		size_t get_total_fragments()
		{
			return total_fragments;
		}
		// --------------------------------------------------------------------
		size_t get_seq_fragment()
		{
			return seq_fragment;
		}
		// --------------------------------------------------------------------
		void set_seq_fragment( size_t _sf )
		{
			seq_fragment = _sf;
		}
		// --------------------------------------------------------------------
		block_data_t* serialize( block_data_t* _buff, size_t _offset = 0 )
		{
			size_t ID_POS = 0;
			size_t ORIG_ID_POS = ID_POS + sizeof(uint16_t);
			size_t SEQ_FRAGMENT_POS = ORIG_ID_POS + sizeof(message_id_t);
			size_t TOTAL_FRAGMENTS_POS = SEQ_FRAGMENT_POS + sizeof(size_t);
			size_t PAYLOAD_SIZE_POS = TOTAL_FRAGMENTS_POS + sizeof(size_t);
			size_t PAYLOAD_POS = PAYLOAD_SIZE_POS + sizeof(size_t);
			write<Os, block_data_t, uint16_t>( _buff + ID_POS + _offset, id );
			write<Os, block_data_t, message_id_t> ( _buff + ORIG_ID_POS + _offset, orig_id );
			write<Os, block_data_t, size_t>( _buff + SEQ_FRAGMENT_POS + _offset, seq_fragment );
			write<Os, block_data_t, size_t>( _buff + TOTAL_FRAGMENTS_POS + _offset, total_fragments );
			write<Os, block_data_t, size_t>( _buff + PAYLOAD_SIZE_POS + _offset, payload_size );
			memcpy( _buff + PAYLOAD_POS + _offset, payload, payload_size );
			return _buff;
		}
		// --------------------------------------------------------------------
		void de_serialize( block_data_t* _buff, size_t _offset = 0 )
		{
			size_t ID_POS = 0;
			size_t ORIG_ID_POS = ID_POS + sizeof(uint16_t);
			size_t SEQ_FRAGMENT_POS = ORIG_ID_POS + sizeof(message_id_t);
			size_t TOTAL_FRAGMENTS_POS = SEQ_FRAGMENT_POS + sizeof(size_t);
			size_t PAYLOAD_SIZE_POS = TOTAL_FRAGMENTS_POS + sizeof(size_t);
			size_t PAYLOAD_POS = PAYLOAD_SIZE_POS + sizeof(size_t);
			id = read<Os, block_data_t, uint16_t>( _buff + ID_POS + _offset );
			orig_id = read<Os, block_data_t, message_id_t> ( _buff + ORIG_ID_POS + _offset );
			seq_fragment = read<Os, block_data_t, size_t>( _buff + SEQ_FRAGMENT_POS + _offset );
			total_fragments = read<Os, block_data_t, size_t>( _buff + TOTAL_FRAGMENTS_POS + _offset );
			payload_size = read<Os, block_data_t, size_t>( _buff + PAYLOAD_SIZE_POS + _offset );
			memcpy( payload, _buff + PAYLOAD_POS + _offset, payload_size );
		}
		// --------------------------------------------------------------------
		size_t serial_size()
		{
			size_t ID_POS = 0;
			size_t ORIG_ID_POS = ID_POS + sizeof(uint16_t);
			size_t SEQ_FRAGMENT_POS = ORIG_ID_POS + sizeof(message_id_t);
			size_t TOTAL_FRAGMENTS_POS = SEQ_FRAGMENT_POS + sizeof(size_t);
			size_t PAYLOAD_SIZE_POS = TOTAL_FRAGMENTS_POS + sizeof(size_t);
			size_t PAYLOAD_POS = PAYLOAD_SIZE_POS + sizeof(size_t);
			return PAYLOAD_POS + payload_size;
		}
		// --------------------------------------------------------------------
#ifdef DEBUG_FRAGMENT_H
		void print( Debug& _debug, Radio& _radio )
		{
			_debug.debug( "-------------------------------------------------------\n");
			_debug.debug( "Fragment : \n" );
			_debug.debug( "id (size %i) : %d\n", sizeof(uint16_t), id );
			_debug.debug( "orig_id (size %i) : %d\n", sizeof(message_id_t), id );
			_debug.debug( "seq_fragment (size %i) : %d\n", sizeof(size_t), seq_fragment );
			_debug.debug( "total_fragments (size %i) : %d\n", sizeof(size_t), total_fragments );
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
		uint16_t id;
		message_id_t orig_id;
		size_t seq_fragment;
		size_t total_fragments;
		block_data_t payload[Radio::MAX_MESSAGE_LENGTH];
		size_t payload_size;
    };
}
#endif
