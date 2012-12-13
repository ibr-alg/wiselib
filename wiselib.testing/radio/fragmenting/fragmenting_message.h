#ifndef __FRAGMENTING_MESSAGE_H__
#define	__FRAGMENTING_MESSAGE_H__

#include "fragmenting_radio_source_config.h"
#include "fragmenting_radio_default_values_config.h"
#include "fragment.h"

namespace wiselib
{
	template<	typename Os_P,
				typename Radio_P,
				typename FragmentingRadio_P,
				typename Timer_P,
				typename Debug_P>
	class FragmentingMessage_Type
	{
	public:
		typedef Os_P Os;
		typedef Radio_P Radio;
		typedef FragmentingRadio_P FragmentingRadio;
		typedef Timer_P Timer;
		typedef Debug_P Debug;
		typedef typename Radio::message_id_t message_id_t;
		typedef typename Radio::block_data_t block_data_t;
		typedef typename Radio::node_id_t node_id_t;
		typedef typename Radio::size_t size_t_normal;
		typedef typename FragmentingRadio::size_t size_t;
		typedef typename Timer::millis_t millis_t;
		typedef Fragment_Type<Os, Radio, Debug> Fragment;
		typedef vector_static<Os, Fragment, FR_MAX_FRAGMENTS> Fragment_vector;
		typedef typename Fragment_vector::iterator Fragment_vector_iterator;
		typedef FragmentingMessage_Type<Os, Radio, FragmentingRadio, Timer, Debug> self_t;
		// --------------------------------------------------------------------
		FragmentingMessage_Type() :
			id				( 0 ),
			orig_id			( 0 ),
			timestamp		( 0 ),
			active			( 0 )
		{};
		// --------------------------------------------------------------------
		~FragmentingMessage_Type()
		{};
		// --------------------------------------------------------------------
		self_t& operator=( const self_t& _fm )
		{
			id = _fm.id;
			orig_id = _fm.orig_id;
			timestamp = _fm.timestamp;
			active = _fm.active;
			fragmenting_message = _fm.fragmenting_message;
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
		void set_orig_id( message_id_t o_id )
		{
			orig_id = o_id;
		}
		// --------------------------------------------------------------------
		uint32_t get_timestamp()
		{
			return timestamp;
		}
		// --------------------------------------------------------------------
		void set_timestamp( uint32_t _t )
		{
			timestamp = _t;
		}
		// --------------------------------------------------------------------
		uint8_t get_active()
		{
			return active;
		}
		// --------------------------------------------------------------------
		void set_active()
		{
			active = 1;
		}
		// --------------------------------------------------------------------
		void set_inactive()
		{
			active = 0;
		}
		// --------------------------------------------------------------------
		Fragment_vector* get_fragmenting_message_ref()
		{
			return &fragmenting_message;
		}
		// --------------------------------------------------------------------
		uint8_t check_completeness()
		{
			if ( fragmenting_message.size() != 0 )
			{
				if ( fragmenting_message.size() == fragmenting_message.at( 0 ).get_total_fragments() )
				{
					return 1;
				}
				else
				{
					return 2;
				}
			}
			return 0;
		}
		// --------------------------------------------------------------------
		void insert_unique( Fragment _f )
		{
			if ( check_completeness() == 0 )
			{
				fragmenting_message.push_back( _f );
			}
			else if ( check_completeness() == 2 )
			{
				uint8_t flag = 0;
				for ( Fragment_vector_iterator it = fragmenting_message.begin(); it != fragmenting_message.end(); ++it )
				{
					if ( _f.get_seq_fragment() == it->get_seq_fragment() )
					{
						flag = 1;
					}
				}
				if ( flag == 0 )
				{
					fragmenting_message.push_back( _f );
				}
			}
		}
		// --------------------------------------------------------------------
		void vectorize( block_data_t* _buff, size_t _len, size_t_normal _mr_len, size_t _offset = 0 )
		{
			size_t number_of_fragments = _len / _mr_len;
			size_t_normal last_fragment_bytes = _len % _mr_len;
			if ( last_fragment_bytes != 0 )
			{
				number_of_fragments = number_of_fragments + 1;
			}

			size_t i;
			for ( i = 0; i < number_of_fragments -1; i++ )
			{
				Fragment f;
				f.set_id( id );
				f.set_orig_id( orig_id );
				f.set_seq_fragment( i );
				f.set_total_fragments( number_of_fragments );
				f.set_payload( _mr_len, _buff + _offset + ( i *_mr_len ) );
				fragmenting_message.push_back( f );
			}
			if ( last_fragment_bytes != 0 )
			{
				Fragment f;
				f.set_id( id );
				f.set_orig_id( orig_id );
				f.set_seq_fragment( i );
				f.set_total_fragments( number_of_fragments );
				f.set_payload( last_fragment_bytes, _buff + _offset + ( i * _mr_len ) );
				fragmenting_message.push_back( f );
			}
		}
		// --------------------------------------------------------------------
		block_data_t* de_vectorize( block_data_t* _buff, size_t _offset = 0 )
		{
			size_t i = 0;
			for ( Fragment_vector_iterator it = fragmenting_message.begin(); it != fragmenting_message.end(); ++it )
			{
				memcpy( _buff + _offset + i, it->get_payload(), it->get_payload_size() );
				i = it->get_payload_size() + i;
			}
			return _buff;
		}
		// --------------------------------------------------------------------
		size_t serial_size()
		{
			size_t i = 0;
			for ( Fragment_vector_iterator it = fragmenting_message.begin(); it != fragmenting_message.end(); ++it )
			{
				i = it->get_payload_size() + i;
			}
			return i;
		}
		// --------------------------------------------------------------------
#ifdef DEBUG_FRAGMENTING_MESSAGE_H
		void print( Debug& _debug, FragmentingRadio& _radio )
		{
			_debug.debug( "-------------------------------------------------------\n");
			_debug.debug( "FragmentingMessage : \n" );
			_debug.debug( "id (size %i) : %d\n", sizeof(uint16_t), id );
			_debug.debug( "orig_id (size %i) : %d\n", sizeof(message_id_t), orig_id );
			_debug.debug( "timestamp (size %i) : %d\n", sizeof(uint32_t), timestamp );
			for ( Fragment_vector_iterator i = fragmenting_message.begin(); i != fragmenting_message.end(); ++i )
			{
				i->print( _debug, _radio.radio() );
			}
			_debug.debug( "-------------------------------------------------------\n");
		}
#endif
		// --------------------------------------------------------------------
	private:
		uint16_t id;
		message_id_t orig_id;
		uint32_t timestamp;
		uint8_t active;
		Fragment_vector fragmenting_message;
    };
}
#endif
