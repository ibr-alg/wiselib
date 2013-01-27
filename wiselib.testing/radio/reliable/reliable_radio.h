#ifndef RELIABLE_RADIO_H
#define	RELIABLE_RADIO_H

#include "util/pstl/vector_static.h"
#include "util/delegates/delegate.hpp"
#include "../../internal_interface/message/message.h"
#include "reliable_radio_message.h"
#include "reliable_radio_source_config.h"
#include "reliable_radio_default_values_config.h"

namespace wiselib
{
	template<	typename Os_P,
				typename Radio_P,
				typename Clock_P,
				typename Timer_P,
				typename Rand_P,
				typename Debug_P>
	class ReliableRadio_Type
	{
	public:
		typedef Os_P Os;
		typedef Radio_P Radio;
		typedef Timer_P Timer;
		typedef Debug_P Debug;
		typedef Clock_P Clock;
		typedef Rand_P Rand;
		typedef typename Radio::node_id_t node_id_t;
		typedef typename Radio::size_t size_t;
		typedef typename Radio::block_data_t block_data_t;
		typedef typename Radio::message_id_t message_id_t;
		typedef typename Clock::time_t time_t;
		typedef typename Radio::ExtendedData ExtendedData;
		typedef typename Radio::ExtendedData ExData;
		typedef typename Radio::TxPower TxPower;
		typedef typename Timer::millis_t millis_t;
		typedef delegate4<void, node_id_t, size_t, uint8_t*, ExData const&> event_notifier_delegate_t;
		typedef vector_static<Os, event_notifier_delegate_t, RR_MAX_REGISTERED_PROTOCOLS> RegisteredCallbacks_vector;
		typedef typename RegisteredCallbacks_vector::iterator RegisteredCallbacks_vector_iterator;
		typedef Message_Type<Os, Radio, Debug> Message;
		typedef ReliableRadioMessage_Type<Os, Radio, Debug> ReliableRadioMessage;
		typedef vector_static<Os, ReliableRadioMessage, RR_MAX_BUFFERED_MESSAGES> ReliableRadioMessage_vector;
		typedef typename ReliableRadioMessage_vector::iterator ReliableRadioMessage_vector_iterator;
		typedef ReliableRadio_Type<Os, Radio, Clock, Timer, Rand, Debug> self_t;
		// --------------------------------------------------------------------
		ReliableRadio_Type() :
			daemon_period		( RR_RESEND_DAEMON_PERIOD ),
			max_retries			( RR_MAX_RETRIES )
		{};
		// --------------------------------------------------------------------
		~ReliableRadio_Type()
		{};
		// --------------------------------------------------------------------
		void enable_radio()
		{
#ifdef DEBUG_RELIABLE_RADIO_H
			debug().debug( "ReliableRadio - enable - Entering.\n" );
#endif
			radio().enable_radio();
			set_status( RR_ACTIVE_STATUS );
			recv_callback_id_ = radio().template reg_recv_callback<self_t, &self_t::receive>( this );
			daemon();
#ifdef DEBUG_RELIABLE_RADIO_H
			debug().debug( "ReliableRadio - enable - Exiting.\n" );
#endif
		};
		// --------------------------------------------------------------------
		void disable_radio()
		{
			set_status( RR_WAITING_STATUS );
			radio().template unreg_recv_callback( recv_callback_id_ );
			radio().disable_radio();
		};
		// --------------------------------------------------------------------
		void send( node_id_t _dest, size_t _len, block_data_t* _data )
		{
#ifdef DEBUG_RELIABLE_RADIO_H
			debug().debug( "ReliableRadio - send - Entering.\n" );
#endif
			if ( status == RR_ACTIVE_STATUS )
			{
				ReliableRadioMessage reliable_radio_message;
				reliable_radio_message.set_message_id( rand()()%0xff );
				reliable_radio_message.set_payload( _len, _data );
				reliable_radio_message.set_destination( _dest );
				insert_reliable_radio_message( reliable_radio_message );
				Message message;
				message.set_message_id( RR_MESSAGE );
				block_data_t buff[Radio::MAX_MESSAGE_LENGTH];
				message.set_payload( reliable_radio_message.serial_size(), reliable_radio_message.serialize( buff ) );
				radio().send( _dest, message.serial_size(), message.serialize() );
			}
#ifdef DEBUG_RELIABLE_RADIO_H
			debug().debug( "ReliableRadio - send - Exiting.\n" );
#endif
		}
		// --------------------------------------------------------------------
		void receive( node_id_t _from, size_t _len, block_data_t * _msg, ExData const &_ex )
		{
#ifdef DEBUG_RELIABLE_RADIO_H
			debug().debug( "ReliableRadio - receive - Entering.\n"  );
#endif
			if ( status == RR_ACTIVE_STATUS )
			{
				if ( _from != radio().id() )
				{
					Message* msg = (Message*) _msg;
					if ( ( msg->get_message_id() == RR_MESSAGE ) && ( msg->compare_checksum() ) )
					{
#ifdef DEBUG_RELIABLE_RADIO_H
			debug().debug( "ReliableRadio - receive - Received RR_MESSAGE.\n" );
#endif
						ReliableRadioMessage reliable_radio_message;
						reliable_radio_message.de_serialize( msg->get_payload() );
						ReliableRadioMessage reliable_radio_reply;
						reliable_radio_reply.set_message_id( reliable_radio_message.get_message_id() );
						block_data_t buff2[Radio::MAX_MESSAGE_LENGTH];
						block_data_t buff1[Radio::MAX_MESSAGE_LENGTH];
						reliable_radio_reply.set_payload( 0, buff1 );
						Message message;
						message.set_message_id( RR_REPLY );
						message.set_payload( reliable_radio_reply.serial_size(), reliable_radio_reply.serialize( buff2 ) );
						radio().send( _from, message.serial_size(), message.serialize() );
#ifdef DEBUG_RELIABLE_RADIO_H
			debug().debug( "ReliableRadio - receive - Sending RR_REPLY.\n" );
#endif
						if ( !replies_check( reliable_radio_message.get_message_id() ) )
						{
#ifdef DEBUG_RELIABLE_RADIO_H
			debug().debug( "ReliableRadio-receive - RR_MESSAGE WAS NEW, storing.\n" );
#endif
							insert_reliable_radio_reply( reliable_radio_reply );
							for ( RegisteredCallbacks_vector_iterator i = callbacks.begin(); i != callbacks.end(); ++i )
							{
								(*i)( _from, reliable_radio_message.get_payload_size(), reliable_radio_message.get_payload(), _ex);
							}
						}
					}
					else if ( msg->get_message_id() == RR_REPLY )
					{
#ifdef DEBUG_RELIABLE_RADIO_H
			debug().debug( "ReliableRadio - receive - Received RR_REPLY.\n" );
#endif
						ReliableRadioMessage reliable_radio_reply;
						reliable_radio_reply.de_serialize( msg->get_payload() );
						for ( ReliableRadioMessage_vector_iterator i = reliable_radio_messages.begin(); i != reliable_radio_messages.end(); ++i )
						{
							if ( i->get_message_id() == reliable_radio_reply.get_message_id() )
							{
								i->set_counter( max_retries + 2 );
#ifdef DEBUG_RELIABLE_RADIO_H
								debug().debug( "ReliableRadio - receive - Exiting with mark of %d message.\n", i->get_message_id() );
#endif
								return;
							}
						}
					}
				}
			}
#ifdef DEBUG_RELIABLE_RADIO_H
			debug().debug( "ReliableRadio - receive - Exiting.\n" );
#endif
		}
		// --------------------------------------------------------------------
		void daemon( void* _user_data = NULL )
		{
#ifdef DEBUG_RELIABLE_RADIO_H
			debug().debug( "ReliableRadio - daemon - Entering.\n" );
#endif
			if ( status == RR_ACTIVE_STATUS )
			{
				for ( ReliableRadioMessage_vector_iterator i = reliable_radio_messages.begin(); i != reliable_radio_messages.end(); ++i )
				{
					if ( i->get_counter() <= max_retries )
					{
#ifdef DEBUG_RELIABLE_RADIO_H
						debug().debug( "ReliableRadio - daemon - An RR_MESSAGE exists with less than max retries...\n" );
#endif
						Message message;
						message.set_message_id( RR_MESSAGE );
						block_data_t buff[Radio::MAX_MESSAGE_LENGTH];
						message.set_payload( i->serial_size(), i->serialize( buff ) );
						radio().send( i->get_destination(), message.serial_size(), message.serialize() );
#ifdef DEBUG_RELIABLE_RADIO_H
						debug().debug( "ReliableRadio - daemon - An RR_MESSAGE exists with less than max retries... - Sending again...\n" );
#endif
					}
					else if ( i->get_counter() == ( max_retries + 1 ) )
					{
						for ( RegisteredCallbacks_vector_iterator j = callbacks.begin(); j != callbacks.end(); ++j )
						{
							ExData ex;
							Message message;
							message.set_message_id( RR_UNDELIVERED );
							message.set_payload( i->get_payload_size(), i->get_payload() );
							(*j)( i->get_destination(), message.serial_size(), message.serialize(), ex);
						}
					}
					i->inc_counter();
				}
				for ( ReliableRadioMessage_vector_iterator i = reliable_radio_replies.begin(); i != reliable_radio_replies.end(); ++i )
				{
#ifdef DEBUG_RELIABLE_RADIO_H
					debug().debug( "ReliableRadio - daemon -Inside replies vector updating counters...\n" );
#endif
					if ( i->get_counter() <= ( max_retries + 1 ) )
					{
						i->inc_counter();
					}
				}
				timer().template set_timer<self_t, &self_t::daemon> ( daemon_period, this, 0 );
			}
#ifdef DEBUG_RELIABLE_RADIO_H
			debug().debug( "ReliableRadio - daemon - Exiting.\n" );
#endif
		}
		// --------------------------------------------------------------------
		uint8_t insert_reliable_radio_message( ReliableRadioMessage _rrm )
		{
#ifdef DEBUG_RELIABLE_RADIO_H
			debug().debug( "ReliableRadio - insert_reliable_radio_message - Entering.\n" );
#endif
			for ( ReliableRadioMessage_vector_iterator i = reliable_radio_messages.begin(); i != reliable_radio_messages.end(); ++i )
			{
				if ( i->get_counter() > max_retries )
				{
#ifdef DEBUG_RELIABLE_RADIO_H
			debug().debug( "ReliableRadio - insert_reliable_radio_message - Found an obsolete entry, replacing and exiting with success.\n" );
#endif
					*i = _rrm;
					return 1;
				}
			}
			if ( reliable_radio_messages.max_size() > reliable_radio_messages.size() )
			{
				reliable_radio_messages.push_back( _rrm );
#ifdef DEBUG_RELIABLE_RADIO_H
			debug().debug( "ReliableRadio - insert_reliable_radio_message - Enough space to push back, Exiting with success.\n" );
#endif
				return 1;
			}
#ifdef DEBUG_RELIABLE_RADIO_H
			debug().debug( "ReliableRadio - insert_reliable_radio_message - Exiting with failure.\n" );
#endif
			return 0;
		}
		// --------------------------------------------------------------------
		uint8_t insert_reliable_radio_reply( ReliableRadioMessage _rrr )
		{
#ifdef DEBUG_RELIABLE_RADIO_H
			debug().debug( "ReliableRadio - insert_reliable_radio_reply - Entering.\n" );
#endif
			for ( ReliableRadioMessage_vector_iterator i = reliable_radio_replies.begin(); i != reliable_radio_replies.end(); ++i )
			{
				if ( i->get_counter() > ( max_retries + 1 ) )
				{
#ifdef DEBUG_RELIABLE_RADIO_H
					debug().debug( "ReliableRadio - insert_reliable_radio_reply - Found an obsolete entry, replacing and exiting with success.\n" );
#endif
					*i = _rrr;
					return 1;
				}
			}
			if ( reliable_radio_replies.max_size() > reliable_radio_replies.size() )
			{
				reliable_radio_replies.push_back( _rrr );
#ifdef DEBUG_RELIABLE_RADIO_H
			debug().debug( "ReliableRadio - insert_reliable_radio_reply - Enough space to push back, Exiting with success.\n" );
#endif
				return 1;
			}
			return 0;
#ifdef DEBUG_RELIABLE_RADIO_H
			debug().debug( "ReliableRadio - insert_reliable_radio_reply - Exiting with failure.\n" );
#endif
		}
		// --------------------------------------------------------------------
		uint8_t replies_check( message_id_t _msg_id )
		{
			for ( ReliableRadioMessage_vector_iterator i = reliable_radio_replies.begin(); i != reliable_radio_replies.end(); ++i )
			{
				if ( ( i->get_message_id() == _msg_id ) &&  ( i->get_counter() <= ( max_retries + 1 ) ) )
				{
					return 1;
				}
			}
			return 0;
		}
		// --------------------------------------------------------------------
		template<class T, void(T::*TMethod)( node_id_t, size_t, block_data_t*, ExData const& ) >
		uint32_t reg_recv_callback( T *_obj_pnt )
		{
#ifdef DEBUG_RELIABLE_RADIO_H
			debug().debug( "ReliableRadio - reg_recv_callback - Entering.\n" );
#endif
			if ( status == RR_ACTIVE_STATUS )
			{
				if ( callbacks.max_size() == callbacks.size() )
				{
#ifdef DEBUG_RELIABLE_RADIO_H
			debug().debug( "ReliableRadio - reg_recv_callback - Exiting FULL.\n" );
#endif
					return RR_PROT_LIST_FULL;
				}
				callbacks.push_back( event_notifier_delegate_t::template from_method<T, TMethod > ( _obj_pnt ) );
#ifdef DEBUG_RELIABLE_RADIO_H
			debug().debug( "ReliableRadio - reg_recv_callback - Exiting SUCCESS.\n" );
#endif
				return RR_SUCCESS;
			}
			else
			{
#ifdef DEBUG_RELIABLE_RADIO_H
			debug().debug( "ReliableRadio - reg_recv_callback - Exiting INACTIVE.\n" );
#endif
				return RR_INACTIVE;
			}
		}
		// --------------------------------------------------------------------
        int unreg_recv_callback( uint32_t idx )
        {
            return 0;
        }
        // --------------------------------------------------------------------
        size_t reserved_bytes()
        {
        	ReliableRadioMessage rrm;
        	return radio().reserved_bytes() + rrm.serial_size();
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
		void set_daemon_period( millis_t _rdp )
		{
			daemon_period =_rdp;
		}
		// --------------------------------------------------------------------
		uint32_t get_max_retries()
		{
			return max_retries;
		}
		// --------------------------------------------------------------------
		void set_max_retries( uint32_t _mr )
		{
			max_retries = _mr;
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
		int set_power(TxPower _p )
		{
			return radio().set_power( _p );
		}
		// --------------------------------------------------------------------
		TxPower power()
		{
			return radio().power();
		}
		// --------------------------------------------------------------------
		enum reliable_radio_status
		{
			RR_ACTIVE_STATUS,
			RR_WAITING_STATUS,
			RR_STATUS_NUM_VALUES
		};
		enum reliable_radio_errors
		{
			RR_SUCCESS,
			RR_PROT_LIST_FULL,
			RR_INACTIVE,
			RR_ERROR_NUM_VALUES
		};
		enum reliable_radio_message_ids
		{
			RR_MESSAGE = 112,
			RR_REPLY = 113,
			RR_UNDELIVERED = 114
		};
        enum Restrictions
        {
            MAX_MESSAGE_LENGTH = Radio::MAX_MESSAGE_LENGTH
        };
        enum SpecialNodeIds
        {
        	BROADCAST_ADDRESS = Radio::BROADCAST_ADDRESS,
        	NULL_NODE_ID = Radio::NULL_NODE_ID
        };
	private:
		uint32_t recv_callback_id_;
        uint8_t status;
        millis_t daemon_period;
        RegisteredCallbacks_vector callbacks;
        ReliableRadioMessage_vector reliable_radio_messages;
        ReliableRadioMessage_vector reliable_radio_replies;
        uint32_t max_retries;
        Radio * radio_;
        Clock * clock_;
        Timer * timer_;
        Debug * debug_;
        Rand * rand_;
    };
}

#endif
