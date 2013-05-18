#ifndef __FRAGMENTING_RADIO_H__
#define	__FRAGMENTING_RADIO_H__

#include "util/pstl/vector_static.h"
#include "util/delegates/delegate.hpp"
#include "../../internal_interface/message/message.h"
#include "fragment.h"
#include "fragmenting_message.h"
#include "fragmenting_radio_source_config.h"
#include "fragmenting_radio_default_values_config.h"

namespace wiselib
{
	template<	typename Os_P,
				typename Radio_P,
				typename Clock_P,
				typename Timer_P,
				typename Rand_P,
				typename Debug_P>
	class FragmentingRadio_Type
	{
	public:
		typedef Os_P Os;
		typedef Radio_P Radio;
		typedef Timer_P Timer;
		typedef Debug_P Debug;
		typedef Clock_P Clock;
		typedef Rand_P Rand;
		typedef typename Radio::node_id_t node_id_t;
		typedef typename Radio::size_t size_t_normal;
		//typedef typename Radio::size_t size_t;
		typedef uint16_t size_t;
		typedef typename Radio::block_data_t block_data_t;
		typedef typename Radio::message_id_t message_id_t;
		typedef typename Clock::time_t time_t;
		typedef typename Radio::ExtendedData ExtendedData;
		typedef typename Radio::ExtendedData ExData;
		typedef typename Radio::TxPower TxPower;
		typedef typename Timer::millis_t millis_t;
		typedef FragmentingRadio_Type<Os, Radio, Clock, Timer, Rand, Debug> FragmentingRadio;
		typedef delegate4<void, node_id_t, size_t, uint8_t*, ExData const&> event_notifier_delegate_t;
		typedef vector_static<Os, event_notifier_delegate_t, FR_MAX_REGISTERED_PROTOCOLS> RegisteredCallbacks_vector;
		typedef typename RegisteredCallbacks_vector::iterator RegisteredCallbacks_vector_iterator;
		typedef FragmentingMessage_Type<Os, Radio, FragmentingRadio, Timer, Debug> FragmentingMessage;
		typedef vector_static<Os, FragmentingMessage, FR_MAX_FRAGMENED_MESSAGES_BUFFERED> FragmentingMessage_vector;
		typedef typename FragmentingMessage_vector::iterator FragmentingMessage_vector_iterator;
		typedef typename FragmentingMessage::Fragment Fragment;
		typedef typename FragmentingMessage::Fragment_vector Fragment_vector;
		typedef typename FragmentingMessage::Fragment_vector_iterator Fragment_vector_iterator;
		typedef FragmentingRadio_Type<Os, Radio, Clock, Timer, Rand, Debug> self_t;
		typedef Message_Type<Os, FragmentingRadio, Debug> Message;
		typedef Message_Type<Os, Radio, Debug> Message_normal;
		// --------------------------------------------------------------------
		FragmentingRadio_Type() :
			status							( FR_WAITING_STATUS ),
			daemon_period					( FR_DAEMON_MILLIS ),
			fragmenting_message_timeout		( FR_FRAGMENTING_MESSAGE_TIMEOUT )
		{};
		// --------------------------------------------------------------------
		~FragmentingRadio_Type()
		{};
		// --------------------------------------------------------------------
		void enable_radio()
		{
#ifdef DEBUG_FRAGMENTING_RADIO_H
			debug().debug( "FragmentingRadio - enable - Entering.\n" );
#endif
			radio().enable_radio();
			set_status( FR_ACTIVE_STATUS );
			recv_callback_id_ = radio().template reg_recv_callback<self_t, &self_t::receive>( this );
#ifdef DEBUG_FRAGMENTING_RADIO_H
			debug().debug( "FragmentingRadio - enable - Exiting.\n" );
#endif
			daemon();
		};
		// --------------------------------------------------------------------
		void disable_radio()
		{
			set_status( FR_WAITING_STATUS );
			radio().template unreg_recv_callback( recv_callback_id_ );
			radio().disable_radio();
		};
		// --------------------------------------------------------------------
		void send( node_id_t _dest, size_t _len, block_data_t* _data )
		{
#ifdef DEBUG_FRAGMENTING_RADIO_H
			debug().debug( "FragmentingRadio - send - Entering.\n" );
#endif
			if ( status == FR_ACTIVE_STATUS )
			{
				if ( Radio::MAX_MESSAGE_LENGTH >= _len )
				{
#ifdef DEBUG_FRAGMENTING_RADIO_H
					debug().debug( "FragmentingRadio - send - Sending normal message (radio max payload lens %d vs %d vs %d).\n", MAX_MESSAGE_LENGTH, Radio::MAX_MESSAGE_LENGTH, _len );
#endif
					Message m;
					m.de_serialize( _data );
					Message_normal mn;
					mn.set_message_id( m.get_message_id() );
					mn.set_payload( m.get_payload_size(), m.get_payload() );
					radio().send( _dest, mn.serial_size(), mn.serialize() );
				}
				else
				{
#ifdef DEBUG_FRAGMENTING_RADIO_H
					debug().debug( "FragmentingRadio - send - Sending fragmenting message (radio max payload lens %d vs %d vs %d).\n", MAX_MESSAGE_LENGTH, Radio::MAX_MESSAGE_LENGTH, _len );
#endif
					if ( Radio::MAX_MESSAGE_LENGTH > reserved_bytes() )
					{
						FragmentingMessage fm;
						Message m;
						size_t internal_reserved_bytes = m.serial_size();
						m.de_serialize( _data );
						size_t pure_payload = _len - internal_reserved_bytes;
						size_t pure_fragment_payload = Radio::MAX_MESSAGE_LENGTH - reserved_bytes();
#ifdef DEBUG_FRAGMENTING_RADIO_H
						debug().debug( "FragmentingRadio - send - Sending fragmenting message - Pure payload %d, pure_fragment_payload %d, reserved_bytes %d, radio().reserved_bytes %d, internal_reserved_bytes %d,  _len %d.\n", pure_payload, pure_fragment_payload, reserved_bytes(), radio().reserved_bytes(), internal_reserved_bytes, _len );
#endif
						fm.set_id( ( rand()() % 0xffff ) );
						fm.set_orig_id( m.get_message_id() );
						fm.vectorize( _data, pure_payload, pure_fragment_payload, internal_reserved_bytes );
#ifdef DEBUG_FRAGMENTING_RADIO_H
						debug().debug( "FragmentingRadio - send - Sending fragmenting message of %d fragments.\n", fm.get_fragmenting_message_ref()->size() );
#endif
						for ( Fragment_vector_iterator it = fm.get_fragmenting_message_ref()->begin(); it != fm.get_fragmenting_message_ref()->end(); ++it )
						{
							Message_normal mf;
							mf.set_message_id( FR_MESSAGE );
							block_data_t buff[Radio::MAX_MESSAGE_LENGTH];
							mf.set_payload( it->serial_size(), it->serialize( buff ) );
							radio().send( _dest, mf.serial_size(), mf.serialize() );
						}
					}
					else
					{
#ifdef DEBUG_FRAGMENTING_RADIO_H
						debug().debug( "FragmentingRadio - send - Message headers exceed maximum payload!\n" );
#endif
					}
				}
			}
#ifdef DEBUG_FRAGMENTING_RADIO_H
			debug().debug( "FragmentingRadio - send - Exiting.\n" );
#endif
		}
		// --------------------------------------------------------------------
		void receive( node_id_t _from, size_t_normal _len, block_data_t * _msg, ExData const &_ex )
		{
#ifdef DEBUG_FRAGMENTING_RADIO_H
			debug().debug( "FragmentingRadio - receive - Entering.\n"  );
#endif
			if ( status == FR_ACTIVE_STATUS )
			{
				if ( _from != radio().id() )
				{
					Message_normal message;
					message.de_serialize( _msg );
					if ( !message.compare_checksum() )
					{
#ifdef DEBUG_FRAGMENTING_RADIO_H
						debug().debug( "FragmentingRadio - receive - Corrupted message!\n"  );
#endif
						return;
					}
					if ( message.get_message_id() == FR_MESSAGE )
					{
						uint8_t flag = 0;
						Fragment f;
						f.de_serialize( message.get_payload() );
						for ( FragmentingMessage_vector_iterator it = fragmenting_messages.begin(); it != fragmenting_messages.end(); ++it )
						{
							if ( ( it->get_id() == f.get_id() ) && ( it->get_active() == 1 ) )
							{
#ifdef DEBUG_FRAGMENTING_RADIO_H
								debug().debug( "FragmentingRadio - receive - Found matching fragment vector f_id : %d, f_fn : %d.\n", f.get_id(), f.get_seq_fragment()  );
#endif
								it->insert_unique( f );
								if ( it->check_completeness() == 1 )
								{
#ifdef DEBUG_FRAGMENTING_RADIO_H
									debug().debug( "FragmentingRadio - receive - Found matching fragment vector - Complete!.\n"  );
#endif
									Message m;
									m.set_message_id( it->get_orig_id() );
									block_data_t buff[MAX_MESSAGE_LENGTH];
									m.set_payload( it->serial_size(), it->de_vectorize( buff ) );
									for ( RegisteredCallbacks_vector_iterator i = callbacks.begin(); i != callbacks.end(); ++i )
									{
										(*i)( _from, m.get_payload_size(), m.serialize(), _ex);
									}
#ifdef DEBUG_FRAGMENTING_RADIO_H
									debug().debug( "FragmentingRadio - receive - Exiting.\n" );
#endif
									it->set_inactive();
									return;
								}
								flag = 1;
							}
						}
						if ( flag == 0 )
						{
#ifdef DEBUG_FRAGMENTING_RADIO_H
							debug().debug( "FragmentingRadio - receive - No matching fragment vector - Making new f_id : %d, f_fn : %d.\n", f.get_id(), f.get_seq_fragment()  );
#endif
							FragmentingMessage fm;
							fm.set_id( f.get_id() );
							fm.set_orig_id( f.get_orig_id() );
							fm.insert_unique( f );
							fm.set_active();
							fm.set_timestamp( clock().seconds( clock().time() ) * 1000 + clock().milliseconds( clock().time() ) );
							uint8_t flag2 = 0;
							for ( FragmentingMessage_vector_iterator it = fragmenting_messages.begin(); it != fragmenting_messages.end(); ++it )
							{
								if ( it->get_active() == 0 )
								{
#ifdef DEBUG_FRAGMENTING_RADIO_H
									debug().debug( "FragmentingRadio - daemon - Replacing inactive.\n" );
#endif
									*it = fm;
									flag2 = 1;
								}
							}
							if ( flag2 == 0 )
							{
#ifdef DEBUG_FRAGMENTING_RADIO_H
								debug().debug( "FragmentingRadio - daemon - Inserting new.\n" );
#endif
								fragmenting_messages.push_back( fm );
							}
						}
					}
					else
					{
#ifdef DEBUG_FRAGMENTING_RADIO_H
						if ( radio().id() == 0x9710 )
						{
							debug().debug( "FragmentingRadio - receive - Other type of ID pushing forward as it is.\n"  );
						}
#endif
						for ( RegisteredCallbacks_vector_iterator i = callbacks.begin(); i != callbacks.end(); ++i )
						{
							Message_normal mn;
							mn.de_serialize( _msg );
							Message m;
							m.set_message_id( mn.get_message_id() );
							m.set_payload( mn.get_payload_size(), mn.get_payload() );
							(*i)( _from, m.serial_size(), m.serialize(), _ex);
						}
					}
				}
			}
#ifdef DEBUG_FRAGMENTING_RADIO_H
			debug().debug( "FragmentingRadio - receive - Exiting.\n" );
#endif
		}
		// --------------------------------------------------------------------
		template<class T, void(T::*TMethod)( node_id_t, size_t, block_data_t*, ExData const& ) >
		uint32_t reg_recv_callback( T *_obj_pnt )
		{
#ifdef DEBUG_FRAGMENTING_RADIO_H
			debug().debug( "FragmentingRadio - reg_recv_callback - Entering.\n" );
#endif
			if ( status == FR_ACTIVE_STATUS )
			{
				if ( callbacks.max_size() == callbacks.size() )
				{
#ifdef DEBUG_FRAGMENTING_RADIO_H
			debug().debug( "FragmentingRadio - reg_recv_callback - Exiting FULL.\n" );
#endif
					return FR_PROT_LIST_FULL;
				}
				callbacks.push_back( event_notifier_delegate_t::template from_method<T, TMethod > ( _obj_pnt ) );
#ifdef DEBUG_FRAGMENTING_RADIO_H
				debug().debug( "FragmentingRadio - reg_recv_callback - Exiting SUCCESS.\n" );
#endif
				return FR_SUCCESS;
			}
			else
			{
#ifdef DEBUG_FRAGMENTING_RADIO_H
			debug().debug( "FragmentingRadio - reg_recv_callback - Exiting INACTIVE.\n" );
#endif
				return FR_INACTIVE;
			}
		}
		// --------------------------------------------------------------------
        int unreg_recv_callback( uint32_t idx )
        {
            return 0;
        }
		// --------------------------------------------------------------------
		void daemon( void* _user_data = NULL )
		{
			if ( status == FR_ACTIVE_STATUS )
			{
				uint32_t current_time = clock().seconds( clock().time() ) * 1000 + clock().milliseconds( clock().time() );
				for ( FragmentingMessage_vector_iterator it = fragmenting_messages.begin(); it != fragmenting_messages.end(); ++it )
				{
					if ( current_time < it->get_timestamp() )
					{
#ifdef DEBUG_FRAGMENTING_RADIO_H
						debug().debug( "FragmentingRadio - daemon - Paradox detected!\n" );
#endif
						it->set_timestamp( current_time );
					}
					if ( ( current_time > it->get_timestamp() + fragmenting_message_timeout ) && ( it->get_active() == 1 ) )
					{
#ifdef DEBUG_FRAGMENTING_RADIO_H
						debug().debug( "FragmentingRadio - daemon - Setting incomplete fragmenting message %d inactive.\n", it->get_id() );
#endif
						it->set_inactive();

					}
				}
				timer().template set_timer<self_t, &self_t::daemon> ( daemon_period, this, 0 );
			}
		}
        // --------------------------------------------------------------------
        size_t reserved_bytes()
        {
        	Fragment f;
        	return ( radio().reserved_bytes() + f.serial_size() );
        };
		// --------------------------------------------------------------------
		uint8_t get_status()
		{
			return status;
		}
		// --------------------------------------------------------------------
		void set_status( uint8_t _st )
		{
			status = _st;
		}
		// --------------------------------------------------------------------
		millis_t get_daemon_period()
		{
			return daemon_period;
		}
		// --------------------------------------------------------------------
		void set_daemon_period( millis_t _dm )
		{
			daemon_period = _dm;
		}
		// --------------------------------------------------------------------
		void init( Radio& _radio, Timer& _timer, Debug& _debug, Clock& _clock, Rand& _rand )
		{
			radio_ = &_radio;
			timer_ = &_timer;
			debug_ = &_debug;
			clock_ = &_clock;
			rand_ = &_rand;
		}
		// --------------------------------------------------------------------
		int set_channel( int _channel )
		{
			return radio().set_channel( _channel );
		}
		// --------------------------------------------------------------------
		int channel()
		{
			return radio().channel();
		}
		// --------------------------------------------------------------------
		int set_power( TxPower _p )
		{
			return radio().set_power( _p );
		}
		// --------------------------------------------------------------------
		TxPower power()
		{
			return radio().power();
		}
		// --------------------------------------------------------------------
		Radio& radio()
		{
			return *radio_;
		}
		// --------------------------------------------------------------------
		Clock& clock()
		{
			return *clock_;
		}
		// --------------------------------------------------------------------
		Timer& timer()
		{
			return *timer_;
		}
		// --------------------------------------------------------------------
		Debug& debug()
		{
			return *debug_;
		}
		// --------------------------------------------------------------------
		Rand& rand()
		{
			return *rand_;
		}
		// --------------------------------------------------------------------
		node_id_t id()
		{
			return radio().id();
		}
		// --------------------------------------------------------------------
		enum reliable_radio_status
		{
			FR_ACTIVE_STATUS,
			FR_WAITING_STATUS,
			FR_STATUS_NUM_VALUES
		};
		enum reliable_radio_errors
		{
			FR_SUCCESS,
			FR_PROT_LIST_FULL,
			FR_INACTIVE,
			FR_ERROR_NUM_VALUES
		};
		enum reliable_radio_message_ids
		{
			FR_MESSAGE = 14,
			FR_REPLY = 24,
			FR_UNDELIVERED = 34
		};
        enum Restrictions
        {
            MAX_MESSAGE_LENGTH = 1024
        };
        enum SpecialNodeIds
        {
        	BROADCAST_ADDRESS = Radio::BROADCAST_ADDRESS,
        	NULL_NODE_ID = Radio::NULL_NODE_ID
        };
	private:
		uint32_t recv_callback_id_;
        uint8_t status;
        RegisteredCallbacks_vector callbacks;
        FragmentingMessage_vector fragmenting_messages;
        millis_t daemon_period;
        millis_t fragmenting_message_timeout;
        Radio * radio_;
        Clock * clock_;
        Timer * timer_;
        Debug * debug_;
        Rand * rand_;
    };
}

#endif
