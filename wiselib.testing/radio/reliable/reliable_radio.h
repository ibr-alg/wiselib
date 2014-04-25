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
		typedef vector_static<Os, ReliableRadioMessage, RR_MAX_BUFFERED_REPLIES> ReliableRadioReply_vector;
		typedef typename ReliableRadioReply_vector::iterator ReliableRadioReply_vector_iterator;
		typedef ReliableRadio_Type<Os, Radio, Clock, Timer, Rand, Debug> self_t;
		// --------------------------------------------------------------------
		ReliableRadio_Type() :
			daemon_period		( RR_RESEND_DAEMON_PERIOD ),
			status				( RR_WAITING_STATUS ),
			max_retries			( RR_MAX_RETRIES ),
			seq					( 0 )
		{};
		// --------------------------------------------------------------------
		~ReliableRadio_Type()
		{};
		// --------------------------------------------------------------------
		void enable_radio()
		{
			if ( status == RR_WAITING_STATUS )
			{
#ifdef DEBUG_RELIABLE_RADIO_H
				debug().debug( "ReliableRadio - enable %x- Entering.\n", radio().id() );
#endif
				radio().enable_radio();
				set_status( RR_ACTIVE_STATUS );
				recv_callback_id_ = radio().template reg_recv_callback<self_t, &self_t::receive>( this );
				daemon();
#ifdef DEBUG_RELIABLE_RADIO_H
				debug().debug( "ReliableRadio - enable %x - Exiting.\n", radio().id() );
#endif
			}
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
			//***
			block_data_t bufffff[Radio::MAX_MESSAGE_LENGTH];
			Message mmm;
			mmm.de_serialize( _data );
			//***
			if ( ( mmm.get_message_id() == 41 ) || ( mmm.get_message_id() == 51 ) )
			{
				debug().debug( "ReliableRadio - send %d- Entering.\n", radio().id() );
			}
#endif
			if ( status == RR_ACTIVE_STATUS )
			{
				if ( _dest == BROADCAST_ADDRESS )
				{
					radio().send( _dest, _len, _data);
					return;
				}
				ReliableRadioMessage reliable_radio_message;
#ifdef CONFIG_RELIABLE_RADIO_H_RANOM_ID
				reliable_radio_message.set_message_id( rand()()%0xffff );
#else
				reliable_radio_message.set_message_id( seq );
				seq = seq + 1;
#endif
				reliable_radio_message.set_payload( _len, _data );
				reliable_radio_message.set_destination( _dest );
				uint8_t res = insert_reliable_radio_message( reliable_radio_message );
				if ( res == 1 )
				{
					Message message;
					message.set_message_id( RR_MESSAGE );
					block_data_t buff[Radio::MAX_MESSAGE_LENGTH];
					message.set_payload( reliable_radio_message.serial_size(), reliable_radio_message.serialize( buff ) );
					radio().send( _dest, message.serial_size(), message.serialize() );
	#ifdef DEBUG_RELIABLE_RADIO_H
					if ( ( mmm.get_message_id() == 41 ) || ( mmm.get_message_id() == 51 ) )
					{
						debug().debug( "ReliableRadio - send %d - Message was sent to %d with rand id [%d] of size %d \n", radio().id(), _dest, reliable_radio_message.get_message_id(), message.serial_size() );
					}
	#endif
				}
	#ifdef DEBUG_RELIABLE_RADIO_H
				if ( ( mmm.get_message_id() == 41 ) || ( mmm.get_message_id() == 51 ) )
				{
					debug().debug( "ReliableRadio - send %d - Exiting.\n", radio().id() );
				}
	#endif
				else
				{
					for ( RegisteredCallbacks_vector_iterator j = callbacks.begin(); j != callbacks.end(); ++j )
					{
						ExData ex;
						Message message;
						message.set_message_id( RR_MESSAGE_BUFFER_FULL );
						message.set_payload( _len, _data );
						(*j)( _dest, message.serial_size(), message.serialize(), ex);
					}
				}
			}
		}
		// --------------------------------------------------------------------
		void receive( node_id_t _from, size_t _len, block_data_t * _msg, ExData const &_ex )
		{
#ifdef DEBUG_RELIABLE_RADIO_H
			debug().debug( "ReliableRadio - receive %x - Entering.\n", radio().id() );
#endif
			if ( status == RR_ACTIVE_STATUS )
			{
				if ( _from != radio().id() )
				{
					Message* msg = (Message*) _msg;
					if ( ( msg->get_message_id() == RR_MESSAGE ) && ( msg->compare_checksum() ) )
					{
#ifdef DEBUG_RELIABLE_RADIO_H
						debug().debug( "ReliableRadio - receive %x - Received RR_MESSAGE.\n", radio().id() );
#endif
						ReliableRadioMessage reliable_radio_message;
						reliable_radio_message.de_serialize( msg->get_payload() );
//@
//#ifdef DEBUG_RELIABLE_RADIO_H
//						block_data_t* buffff = reliable_radio_message.get_payload();
//						Message mmmmm;
//						mmmmm.de_serialize( buffff );
//						if ( ( mmmmm.get_message_id() == 41 ) || ( mmmmm.get_message_id() == 51 ) )
//						{
//							debug().debug( "RR-rec %x - Rec RR_MESSAGE of [%x] from %x of size %d.\n", radio().id(), reliable_radio_message.get_message_id(), _from, _len );
//						}
//#endif
						ReliableRadioMessage reliable_radio_reply;
						reliable_radio_reply.set_message_id( reliable_radio_message.get_message_id() );
						reliable_radio_reply.set_counter( reliable_radio_message.get_counter() );
						block_data_t buff2[Radio::MAX_MESSAGE_LENGTH];
						block_data_t buff1[Radio::MAX_MESSAGE_LENGTH];
//@
//#ifdef DEBUG_RELIABLE_RADIO_H
						//**
						//size_t olds = reliable_radio_reply.serial_size();
						//**
//#endif
						//used uint32 offset to store specific ids for deep debugs
						reliable_radio_reply.set_payload( sizeof(node_id_t)/* + sizeof(uint32_t)*/, buff1 );
						reliable_radio_reply.set_destination( _from );
						reliable_radio_reply.reply_destination_write();
//@
//#ifdef DEBUG_RELIABLE_RADIO_H
//						//**
//						if ( ( mmmmm.get_message_id() == 41 ) )//&& ( _len != 187 ) )
//						{
//							reliable_radio_reply.reply_internal_message_id_write( 41 );
//						}
//						else if  ( mmmmm.get_message_id() == 51 )
//						{
//							reliable_radio_reply.reply_internal_message_id_write( 51 );
//						}
						//**
//#endif
//@
//#ifdef DEBUG_RELIABLE_RADIO_H
//						if ( ( mmmmm.get_message_id() == 41 ) || ( mmmmm.get_message_id() == 51 ) )
//						{
//							reliable_radio_reply.reply_destination_read();
//							debug().debug( "RR-rec %x - Send RR_REPLY of [%x] to %x vs %x with sizes[%d + %d, %d].\n", radio().id(), reliable_radio_reply.get_message_id(), _from, reliable_radio_reply.get_destination(), olds, sizeof(node_id_t) + sizeof(uint32_t), reliable_radio_reply.serial_size() );
//						}
//#endif
						if ( replies_check( reliable_radio_reply.get_message_id(), _from ) == 0 )
						{
//@
//#ifdef DEBUG_RELIABLE_RADIO_H
//							if ( ( mmmmm.get_message_id() == 41 ) || ( mmmmm.get_message_id() == 51 ) )
//							{
//								debug().debug( "ReliableRadio-receive %x - RR_MESSAGE WAS NEW of [%x] from %x, going for storing.\n", radio().id(), reliable_radio_message.get_message_id(), _from );
//							}
//#endif
//@
//#ifdef DEBUG_RELIABLE_RADIO_H
//
//							if ( ( mmmmm.get_message_id() == 41 ) || ( mmmmm.get_message_id() == 51 ) )
//							{
//								if ( res == 1 )
//								{
//									debug().debug( "RR-rec %x - RR_MESSAGE WAS NEW of [%x] from %x, STORED.\n", radio().id(), reliable_radio_message.get_message_id(), _from );
//								}
//								else
//								{
//									debug().debug( "RR-rec %x - RR_MESSAGE WAS NEW of [%x] from %x, NOT STORED.\n", radio().id(), reliable_radio_message.get_message_id(), _from );
//								}
//							}
//#endif
							//this loop could be included only for res 1... due to the fact that the reliable node partners might go into an endless ping pong loop (no buffer - no knowledge...)
							uint8_t res = insert_reliable_radio_reply( reliable_radio_reply );
							if ( res == 1 )
							{
								Message message;
								message.set_message_id( RR_REPLY );
								message.set_payload( reliable_radio_reply.serial_size(), reliable_radio_reply.serialize( buff2 ) );
								radio().send( _from, message.serial_size(), message.serialize() );
								for ( RegisteredCallbacks_vector_iterator i = callbacks.begin(); i != callbacks.end(); ++i )
								{
									(*i)( _from, reliable_radio_message.get_payload_size(), reliable_radio_message.get_payload(), _ex);
								}
							}
							else
							{
								//debug().debug("%x",_from);
								//callback for buffer failure
//								for ( ReliableRadioReply_vector_iterator i = reliable_radio_replies.begin(); i != reliable_radio_replies.end(); ++i )
//								{
//									debug().debug("%x:%x:[%d:%d]", radio().id(), i->get_message_id(), i->get_destination(), i->get_counter() );
//								}
								for ( RegisteredCallbacks_vector_iterator j = callbacks.begin(); j != callbacks.end(); ++j )
								{
									Message message;
									message.set_message_id( RR_REPLY_BUFFER_FULL );
									message.set_payload( _len, msg->get_payload() );
									(*j)( _from, message.serial_size(), message.serialize(), _ex);
								}
							}
						}
//@
//#ifdef DEBUG_RELIABLE_RADIO_H
//						else
//						{
//							if ( ( mmmmm.get_message_id() == 41 ) || ( mmmmm.get_message_id() == 51 ) )
//							{
//								debug().debug( "RR - rec %x - RR_MESSAGE WAS NOT!!!! NEW of [%x] from %x.\n", radio().id(), reliable_radio_message.get_message_id(), _from );
//							}
//						}
//#endif
					}
					else if ( ( msg->get_message_id() == RR_REPLY ) && ( msg->compare_checksum() ) )
					{
#ifdef DEBUG_RELIABLE_RADIO_H
						debug().debug( "ReliableRadio - receive %x - Received RR_REPLY.\n", radio().id() );
#endif
						ReliableRadioMessage reliable_radio_reply;
						reliable_radio_reply.de_serialize( msg->get_payload() );
						reliable_radio_reply.reply_destination_read();
//@
//#ifdef DEBUG_RELIABLE_RADIO_H
//						if ( ( reliable_radio_reply.reply_internal_message_id_read() == 41 ) ||  ( reliable_radio_reply.reply_internal_message_id_read() == 51 ) )
//						{
//							debug().debug( "RR - rec %x - Rec RR_REPLY [%x] from %x.\n", radio().id(), reliable_radio_reply.get_message_id(), _from );
//						}
//						uint8_t found_flag = 0;
//#endif
						for ( ReliableRadioMessage_vector_iterator i = reliable_radio_messages.begin(); i != reliable_radio_messages.end(); ++i )
						{
#ifdef DEBUG_RELIABLE_RADIO_H
							//if ( ( reliable_radio_reply.reply_internal_message_id_read() == 41 ) ||  ( reliable_radio_reply.reply_internal_message_id_read() == 51 ) )
							//{
							//	debug().debug( "ReliableRadio - receive %x - Received RR_REPLY [%x] - in the loops with [%x vs %x] with deliver status [%d] from %x.\n", radio().id(),  reliable_radio_reply.get_message_id(), i->get_message_id(), reliable_radio_reply.get_message_id(), i->get_delivered(), _from );
							//}
#endif
							if ( ( i->get_message_id() == reliable_radio_reply.get_message_id() ) && ( i->get_delivered() == 0 ) && ( i->get_destination() == _from ) )
							{
								i->set_delivered();
//@
//#ifdef DEBUG_RELIABLE_RADIO_H
//								if ( ( reliable_radio_reply.reply_internal_message_id_read() == 41 ) ||  ( reliable_radio_reply.reply_internal_message_id_read() == 51 ) )
//								{
//									debug().debug( "RR - rec %x - Received RR_REPLY [%x] - Exiting as delivered from %x.\n", radio().id(), i->get_message_id(), _from );
//									found_flag = 1;
//								}
//#endif
								return;
							}
						}
//@
//#ifdef DEBUG_RELIABLE_RADIO_H
//						if ( ( found_flag == 0 ) && ( ( reliable_radio_reply.reply_internal_message_id_read() == 41 ) ||  ( reliable_radio_reply.reply_internal_message_id_read() == 51 ) ) )
//						{
//							debug().debug( "RR - rec %x - Received RR_REPLY [%x] - Exiting not found from %x with possible obsolete reply.\n", radio().id(),  reliable_radio_reply.get_message_id(), _from );
//						}
//						//debug().debug( "ReliableRadio - receive %x - Exiting from %x with possible obsolete reply.\n", radio().id(), _from );
//#endif
					}
					else if ( msg->compare_checksum() )
					{
						for ( RegisteredCallbacks_vector_iterator j = callbacks.begin(); j != callbacks.end(); ++j )
						{
							(*j)( _from, _len, _msg, _ex);
						}
					}
				}
			}
#ifdef DEBUG_RELIABLE_RADIO_H
			//debug().debug( "ReliableRadio - receive %x - Exiting (from %x).\n", radio().id(), _from );
#endif
		}
		// --------------------------------------------------------------------
#ifdef CONFIG_RELIABLE_RADIO_H_TIGHT_DAEMON_CONTROL
		void daemon_scheduler( void* _data = NULL )
		{
#ifdef DEBUG_RELIABLE_RADIO_H
			debug().debug("ReliableRadio - daemon_scheduler - Entering scheduler\n", radio().id() );
#endif
			uint32_t backoff_daemon_period = rand()() % ( daemon_period - 50 );
			time_t current_time = clock().time();
#ifdef DEBUG_RELIABLE_RADIO_H
			debug().debug("DAE_SCH:%x:%d:%d:%d:%d\n", radio().id(), clock().seconds( current_time ), clock().milliseconds( current_time ), backoff_daemon_period, daemon_period );
#endif
			timer().template set_timer<self_t, &self_t::daemon>( backoff_daemon_period, this, 0 );
			timer().template set_timer<self_t, &self_t::daemon_scheduler> ( daemon_period, this, 0 );
#ifdef DEBUG_RELIABLE_RADIO_H
			debug().debug("ReliableRadio - daemon_scheduler - Exiting scheduler\n", radio().id() );
#endif
		}
#endif
		// --------------------------------------------------------------------
		void daemon( void* _user_data = NULL )
		{
#ifdef DEBUG_RELIABLE_RADIO_H
			debug().debug( "ReliableRadio - daemon %x - Entering.\n", radio().id() );
#endif
			if ( status == RR_ACTIVE_STATUS )
			{
				for ( ReliableRadioMessage_vector_iterator i = reliable_radio_messages.begin(); i != reliable_radio_messages.end(); ++i )
				{
//@
//#ifdef DEBUG_RELIABLE_RADIO_H
					block_data_t* buffff = i->get_payload();
					Message mmmmm;
					mmmmm.de_serialize( buffff );
//#endif
					if ( ( i->get_counter() < max_retries ) && ( i->get_delivered() == 0 ) )
					{
//@
//#ifdef DEBUG_RELIABLE_RADIO_H
//						if ( ( mmmmm.get_message_id() == 41 ) || ( mmmmm.get_message_id() == 51 ) )
//						{
//							debug().debug( "RR - dae %x - RR_MESS [%x] exists with less than max retr\n", radio().id(), i->get_message_id() );
//						}
//#endif
						debug().debug("RRR:%x",radio().id());
						Message message;
						message.set_message_id( RR_MESSAGE );
						block_data_t buff[Radio::MAX_MESSAGE_LENGTH];
						message.set_payload( i->serial_size(), i->serialize( buff ) );
						if ( i->get_counter() > max_retries / 2 )
						{
							int old_db = radio().power().to_dB();
							if ( old_db < -6 )
							{
//@
//#ifdef DEBUG_RELIABLE_RADIO_H
//								if ( ( mmmmm.get_message_id() == 41 ) || ( mmmmm.get_message_id() == 51 ) )
//								{
//									debug().debug("RR - dae %x - [%x], Inc rad [%d, %d], for re-trans\n", radio().id(), i->get_message_id(), old_db, old_db + 6 );
//								}
//#endif
								TxPower tp;
								tp.set_dB( old_db + 6 );
								radio().set_power( tp );
							}
							radio().send( i->get_destination(), message.serial_size(), message.serialize() );
							i->inc_counter();
							TxPower tp;
							tp.set_dB( old_db );
							radio().set_power( tp );
//#ifdef CONFIG_RELIABLE_RADIO_H_TIGHT_DAEMON_CONTROL
//							break;
//#endif
						}
						else
						{
//@
//#ifdef DEBUG_RELIABLE_RADIO_H
///							if ( ( mmmmm.get_message_id() == 41 ) || ( mmmmm.get_message_id() == 51 ) )
//							{
//								debug().debug( "RR - dae %x - RR_MESS [%x] exists with less than max retries - Sending again to %d [%d]\n", radio().id(),  i->get_message_id(), i->get_destination(), i->get_message_id() );
//							}
//#endif
							radio().send( i->get_destination(), message.serial_size(), message.serialize() );
							i->inc_counter();
//#ifdef CONFIG_RELIABLE_RADIO_H_TIGHT_DAEMON_CONTROL
//							break;
//#endif
						}
					}
					else if ( ( i->get_counter() ==  max_retries ) && ( i->get_delivered() == 0 ) )
					{
//@
//#ifdef DEBUG_RELIABLE_RADIO_H
//						if ( ( mmmmm.get_message_id() == 41 ) || ( mmmmm.get_message_id() == 51 ) )
//						{
//							debug().debug( "RR - dae %d - RR_MESS [%x] exists with max retries %d of [%d] with deliver status [%d] from %x - Undeliv\n", radio().id() ,  i->get_message_id(), max_retries + 1, i->get_message_id(), i->get_delivered(), i->get_destination() );
//						}
//#endif
						for ( RegisteredCallbacks_vector_iterator j = callbacks.begin(); j != callbacks.end(); ++j )
						{
							ExData ex;
							Message message;
							message.set_message_id( RR_UNDELIVERED );
							message.set_payload( i->get_payload_size(), i->get_payload() );
							(*j)( i->get_destination(), message.serial_size(), message.serialize(), ex);
							i->inc_counter();
						}
					}
				}
				for ( ReliableRadioReply_vector_iterator i = reliable_radio_replies.begin(); i != reliable_radio_replies.end(); ++i )
				{
#ifdef DEBUG_RELIABLE_RADIO_H
					debug().debug( "ReliableRadio - daemon %x - Inside replies vector updating counters...\n", radio().id() );
#endif
					if ( i->get_counter() < max_retries  )
					{
						i->inc_counter();
					}
				}
#ifndef CONFIG_RELIABLE_RADIO_H_TIGHT_DAEMON_CONTROL

#ifdef CONFIG_RELIABLE_RADIO_H_RANDOM_DAEMON_OFFSET
				millis_t offset = rand()() % (daemon_period / 3);
#else
				millis_t offset = 0;
#endif
				timer().template set_timer<self_t, &self_t::daemon> ( daemon_period + offset, this, 0 );
#endif
			}
#ifdef DEBUG_RELIABLE_RADIO_H
			debug().debug( "ReliableRadio - daemon %x Exiting.\n", radio().id() );
#endif
		}
		// --------------------------------------------------------------------
		uint8_t insert_reliable_radio_message( ReliableRadioMessage _rrm )
		{
//#ifdef DEBUG_RELIABLE_RADIO_H
//			debug().debug( "ReliableRadio - insert_reliable_radio_message %x - Entering.\n", radio().id() );
			Message mmm;
			mmm.de_serialize( _rrm.get_payload() );

//#endif
			if ( reliable_radio_messages.max_size() > reliable_radio_messages.size() )
			{
				reliable_radio_messages.push_back( _rrm );

#ifdef DEBUG_RELIABLE_RADIO_H
//				if ( mmm.get_message_id() == 41 )
//				{
				debug().debug( "ReliableRadio - insert_reliable_radio_message %x - Enough space to push back, Exiting with success.\n", radio().id() );
//				}
#endif
				return 1;
			}
			for ( ReliableRadioMessage_vector_iterator i = reliable_radio_messages.begin(); i != reliable_radio_messages.end(); ++i )
			{
#ifdef DEBUG_RELIABLE_RADIO_H
//				if ( mmm.get_message_id() == 41 )
//				{
				debug().debug( "ReliableRadio - insert_reliable_radio_message %x rr_buffer[%d vs %d] - In the loops looking.... \n", radio().id(), reliable_radio_messages.max_size(), reliable_radio_messages.size() );
//				}
#endif
				if ( ( i->get_counter() > max_retries ) || ( i->get_delivered() == 1 ) )
				{
#ifdef DEBUG_RELIABLE_RADIO_H
//					if ( mmm.get_message_id() == 41 )
//					{
					debug().debug( "ReliableRadio - insert_reliable_radio_message %x - Found an obsolete entry, replacing and exiting with success.\n", radio().id() );
//					}
#endif
					*i = _rrm;
					return 1;
				}
			}
//#ifdef DEBUG_RELIABLE_RADIO_H
			debug().debug( "ins_r_r_mes %x- Exit with fail [%x].\n", radio().id(), _rrm.get_message_id(), mmm.get_message_id()  );
//#endif
			return 0;
		}
		// --------------------------------------------------------------------
		uint8_t insert_reliable_radio_reply( ReliableRadioMessage _rrr )
		{
#ifdef DEBUG_RELIABLE_RADIO_H
			debug().debug( "ReliableRadio - insert_reliable_radio_reply %x - Entering.\n", radio().id() );
#endif
			if ( reliable_radio_replies.max_size() > reliable_radio_replies.size() )
			{
				reliable_radio_replies.push_back( _rrr );
#ifdef DEBUG_RELIABLE_RADIO_H
			debug().debug( "ReliableRadio - insert_reliable_radio_reply %x - Enough space to push back, Exiting with success.\n", radio().id() );
#endif
				return 1;
			}
			for ( ReliableRadioReply_vector_iterator i = reliable_radio_replies.begin(); i != reliable_radio_replies.end(); ++i )
			{
				if ( i->get_counter() >= max_retries )
				{
#ifdef DEBUG_RELIABLE_RADIO_H
					debug().debug( "ReliableRadio - insert_reliable_radio_reply %x - Found an obsolete entry, replacing and exiting with success.\n", radio().id() );
#endif
					*i = _rrr;
					return 1;
				}
			}
//#ifdef DEBUG_RELIABLE_RADIO_H
			debug().debug( "ins_r_rep %x - Exit fail [%x].\n", radio().id(), _rrr.get_message_id() );
//#endif
			return 0;
		}
		// --------------------------------------------------------------------
		uint8_t replies_check( uint32_t _msg_id, node_id_t _from )
		{
			for ( ReliableRadioReply_vector_iterator i = reliable_radio_replies.begin(); i != reliable_radio_replies.end(); ++i )
			{
#ifdef DEBUG_RELIABLE_RADIO_H
				if ( i->reply_internal_message_id_read() == 41 )
				{
					debug().debug( "Replies_check - checking in loop %x - Reply with [msg_id %d, dest %x] vs inc param [msg_id %d, dest %x]\n", radio().id(), i->get_message_id(), i->get_destination(), _msg_id, _from );
				}
#endif
				i->reply_destination_read();
#ifdef DEBUG_RELIABLE_RADIO_H
				if ( i->reply_internal_message_id_read() == 41 )
				{
					debug().debug( "Replies_check - checking in loop %x - Reply with [msg_id %d, dest %x] vs inc param [msg_id %d, dest %x]\n", radio().id(), i->get_message_id(), i->get_destination(), _msg_id, _from );
				}
#endif
				if ( ( i->get_message_id() == _msg_id ) && ( i->get_destination() == _from ) && ( i->get_delivered() == 0 ) )
				{
#ifdef DEBUG_RELIABLE_RADIO_H
					debug().debug( "Replies_check - checking in loop %d - Reply with [msg_id %d, dest %x, del %d] vs inc param [msg_id %d, dest %x ]\n", radio().id(), i->get_message_id(), i->get_destination(), i->get_delivered(), _msg_id, _from );
#endif
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
			debug().debug( "ReliableRadio - reg_recv_callback %x - Entering.\n", radio().id() );
#endif
			if ( status == RR_ACTIVE_STATUS )
			{
				if ( callbacks.max_size() == callbacks.size() )
				{
#ifdef DEBUG_RELIABLE_RADIO_H
			debug().debug( "ReliableRadio - reg_recv_callback %x - Exiting FULL.\n", radio().id() );
#endif
					return RR_PROT_LIST_FULL;
				}
				callbacks.push_back( event_notifier_delegate_t::template from_method<T, TMethod > ( _obj_pnt ) );
#ifdef DEBUG_RELIABLE_RADIO_H
			debug().debug( "ReliableRadio - reg_recv_callback %x - Exiting SUCCESS.\n", radio().id() );
#endif
				return RR_SUCCESS;
			}
			else
			{
#ifdef DEBUG_RELIABLE_RADIO_H
			debug().debug( "ReliableRadio - reg_recv_callback %x - Exiting INACTIVE.\n", radio().id() );
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
			RR_MESSAGE = 14,
			RR_REPLY = 24,
			RR_UNDELIVERED = 34,
			RR_MESSAGE_BUFFER_FULL = 44,
			RR_REPLY_BUFFER_FULL = 54
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
        millis_t daemon_period;
        uint8_t status;
        RegisteredCallbacks_vector callbacks;
        ReliableRadioMessage_vector reliable_radio_messages;
        ReliableRadioReply_vector reliable_radio_replies;
        uint32_t max_retries;
        uint32_t seq;
        Radio * radio_;
        Clock * clock_;
        Timer * timer_;
        Debug * debug_;
        Rand * rand_;
    };
}

#endif
