#ifndef __DELAYED_MESSAGE_DELIVERY_H__
#define __DELAYED_MESSAGE_DELIVERY_H__

#include <stdint.h>
#include <sstream>

#include "util/serialization/serialization.h"

#include "roomba_statistics.h"

#include "external_interface/pc/roomba.h"
#include "external_interface/external_interface.h"

#include "algorithms/neighbor_discovery/echo.h"
#include "algorithms/neighbor_discovery/pgb_payloads_ids.h"
#include "algorithms/end_to_end_communication/end_to_end_communication_msg.h"

template< 	typename OsModel_P,
			typename NeighborDiscovery_P,
			typename Roomba_P,
			typename Radio_P = typename OsModel_P::Radio,
			typename Debug_P = typename OsModel_P::Debug,
			typename Timer_P = typename OsModel_P::Timer,
			typename Clock_P = typename OsModel_P::Clock
			>
class RoombaMessageDelivery {
	public:
		typedef OsModel_P Os;
		typedef NeighborDiscovery_P NeighborDiscovery;
		typedef Roomba_P Roomba;
		typedef Radio_P Radio;
		typedef Debug_P Debug;
		typedef Timer_P Timer;
		typedef Clock_P Clock;

		typedef RoombaMessageDelivery<Os,NeighborDiscovery, Roomba, Radio, Debug, Timer> self_type;
		typedef self_type* self_pointer_t;

		typedef typename wiselib::CommunicationMessage<Os, Radio> msg_t;

		typedef typename Radio::block_data_t block_data_t;
		typedef typename Radio::size_t size_t;
		typedef typename Radio::node_id_t node_id_t;
		typedef typename Radio::message_id_t message_id_t;

		enum {
			MAX_NUMBER_OF_STORED_MESSAGES = 255, // maximum number of stored messages
		};

		enum {
			MAX_NUMBER_OF_ROOMBAS = 10, 	// maximum number of Roombas in system, 
							//gives size for roomba-interchange table
		};

		void init( NeighborDiscovery& neighbor_discovery, 
			   Roomba& roomba, 
			   Radio& radio, 
			   Debug& debug, 
			   Timer& timer, 
			   Clock& clock
		) {
			stopped_ = true;

			radio_ = &radio;
			debug_ = &debug;
			timer_ = &timer;
			clock_ = &clock;
			roomba_ = &roomba;
			neighbor_discovery_ = &neighbor_discovery;

			init();
		}


		void init()
		{
			// enable statistics output
			typename Os::Clock clock;
			starting_time_ = clock.seconds( clock.time() );
			std::ostringstream time_string;
			time_string << "statistics_" << starting_time_ << ".txt";
			statistics_ = new RoombaStatistics( time_string.str() );

			radio_->enable_radio();
			radio_recv_callback_id_ = radio_->template reg_recv_callback<self_type, &self_type::on_receive>( this );

			neighbor_discovery_->enable();
			if( neighbor_discovery_->register_payload_space( MOBILITY ) )
				debug_->debug( "Could not register payload space in neighbor discovery module!\n" );

			uint8_t data = 1;
			neighbor_discovery_->set_payload( MOBILITY, &data, sizeof( data ) );

			uint8_t flags = NeighborDiscovery::NEW_NB_BIDI;
			neighbor_discovery_->template reg_event_callback<self_type, &self_type::on_new_neighbor>( MOBILITY, flags, this );
			flags = NeighborDiscovery::NEW_PAYLOAD_BIDI;
			neighbor_discovery_->template reg_event_callback<self_type, &self_type::on_new_payload>( MOBILITY, flags, this );


			// Initially no messages are stored.
			for( uint16_t i=0; i < MAX_NUMBER_OF_STORED_MESSAGES; ++i )
			{
				stored_messages_[i].used = false;
			}
			number_of_stored_messages_ = 0;

			// Initially no Roomba is known
			for( uint16_t i=0; i < MAX_NUMBER_OF_ROOMBAS; ++i )
			{
				roomba_interchanged_msg_table_[i].used = false;
			}

			timer_->template set_timer<self_type, &self_type::on_time>( 1000, this, 0 );

			stopped_ = false;
		}

		int destruct()
		{
			stopped_ = true;

			//Unregister callbacks
			neighbor_discovery_->unregister_payload_space( MOBILITY );
			neighbor_discovery_->unreg_recv_callback( MOBILITY );
			radio_->unregister_recv_callback( radio_recv_callback_id_ );

			neighbor_discovery_->disable();
			radio_->disable_radio();

			for( uint16_t i=0; i < MAX_NUMBER_OF_STORED_MESSAGES; ++i )
			{
				stored_messages_[i].used = false;
			}
			number_of_stored_messages_ = 0;

			statistics_->close_statistics();

			return Os::SUCCES;
		}

		/** \brief returns number of stored messages
		 *
		 * \return number_of_stored_messages_
		 */
		uint16_t number_of_stored_messages()
		{
			return number_of_stored_messages_;
		}

	protected:
		/** \brief callback for new neighbor
		 *
		 * Delivers all the stored messages which have a destination within the current neighborhood.
		 * \param event
		 * \param from
		 * \param len
		 * \param data&
		 * \return void
		 */
		void on_new_neighbor( uint8_t event, node_id_t from, uint8_t len, uint8_t* data ) {
			debug_->debug( "on_new_neighbor %d\n", from );

			if( !stopped_ )
			{
				if( ( event & NeighborDiscovery::NEW_NB_BIDI ) != 0 )
				{
					for( int i=0; i < MAX_NUMBER_OF_STORED_MESSAGES; i++ )
					{
						if( stored_messages_[i].used )
						{
							msg_t* msg = &stored_messages_[i].message;

							if( msg->dest() == from )
							{
								debug_->debug( "Delivering message to %d, size: %d.\n", msg->dest(), msg->buffer_size() );
								// Deliver stored message
								radio_->send( msg->dest(), msg->buffer_size(), (block_data_t*)msg );

								stored_messages_[i].used = false;
								number_of_stored_messages_--;
								
								//Save statistics output
								typename Os::Clock clock;
								statistics_->print_statistics(
									clock.seconds( clock.time() ) - starting_time_,
									number_of_stored_messages_,
									-1
								);
							}
						}
					}

					debug_->debug( "number of stored messages: %d\n", number_of_stored_messages_);
				}
			}
		}

		void on_new_payload( uint8_t event, node_id_t from, uint8_t len, uint8_t* data ) {
			//debug_->debug( "on_new_payload %d, %d, %d\n", event, from, *data );

			uint16_t i;
			if( *data ) {
				for( i=0; i<MAX_NUMBER_OF_ROOMBAS; ++i )
				{
					if ( // return if at pos i is roomba
							roomba_interchanged_msg_table_[i].used == true &&
							from == roomba_interchanged_msg_table_[i].roomba_id
					) {
						return;
					}
				}

				// hence, at this point we know that the roomba did not get the data, yet
				for( i=0; i < MAX_NUMBER_OF_ROOMBAS; ++i )
				{
					if( roomba_interchanged_msg_table_[i].used == false )
					{
					  roomba_interchanged_msg_table_[i].used = true;
					  roomba_interchanged_msg_table_[i].roomba_id = from;

					  break;
					}
				}
				if( i == MAX_NUMBER_OF_ROOMBAS ) // All slots are used.
				{
					// Drop first one, but give error message!
					// this should not happen!
					roomba_interchanged_msg_table_[0].used = true;
					roomba_interchanged_msg_table_[0].roomba_id = from;

					debug_->debug( "Too many roombas in system for proper data dump exchange!\n" );
				}
			}

			// now send all stored messages to other roomba
			for( i=0; i<MAX_NUMBER_OF_STORED_MESSAGES; ++i )
			{
				msg_t* msg = &stored_messages_[i].message;
				radio_->send( msg->dest(), msg->buffer_size(), (block_data_t*)msg );
			}
		}

		/** \brief Callback for time timer
		 *
		 * Increases the store_duration of the stored messages.
		 */
		void on_time( void* userdata )
		{
			//debug_->debug( "on_time: trying to deliver %d messages\n", number_of_stored_messages_ );
			if( !stopped_ )
			{
				for( int i=0; i < MAX_NUMBER_OF_STORED_MESSAGES; i++ )
				{
					if( stored_messages_[i].used )
					{
						msg_t* msg = &stored_messages_[i].message;

						if( neighbor_discovery_->is_neighbor_bidi( msg->dest() ) ) 
						{
							debug_->debug( "Delivering message to %d, size: %d.\n", msg->dest(), msg->buffer_size() );
							radio_->send( msg->dest(), msg->buffer_size(), (block_data_t*)msg );
							stored_messages_[i].used = false;
							number_of_stored_messages_--;
							
							//Save statistics output
							typename Os::Clock clock;
							statistics_->print_statistics(
								clock.seconds( clock.time() ) - starting_time_,
								number_of_stored_messages_,
								-1
							);
						} else {
							stored_messages_[i].store_duration++;
						}
					}
				}
			}

			if( number_of_stored_messages_ > 0 )
			{
				debug_->debug( "\a" );
			}

			timer_->template set_timer<self_type, &self_type::on_time>( 1000, this, 0 );
		}



		/** \brief Callback for message receive
		 *
		 * Stores received CommunicationMessages for delivery into buffer.
		 * \param id is node-id of message origin
		 * \param size is size of message
		 * \param &data is pointer to message-data
		 * \return void
		 */
		void on_receive( node_id_t id, size_t size, block_data_t* data) {
			debug_->debug( "on_receive from %d, msg_id: %d\n", id, wiselib::read<Os, block_data_t, message_id_t>( data ) );

			message_id_t msg_id = wiselib::read<Os, block_data_t, message_id_t>( data );

			if( msg_id == msg_t::END_TO_END_MESSAGE )
			{
				uint16_t i;		// run variable that is used for several for loops
				msg_t* msg = (msg_t*)data;

				debug_->debug( "\aReceived message from %d with destination %d with size %d.\n", id, msg->dest(), msg->buffer_size() );

				// check if received message is already contained inside buffer, if so do not add
				for( i=0; i < MAX_NUMBER_OF_STORED_MESSAGES; ++i )
				{
					if( stored_messages_[i].used &&
						stored_messages_[i].message.seq_no() == msg->seq_no() &&
						stored_messages_[i].message.source() == msg->source()
					){
						return;
					}
				}

				// if destination is in roomba's range, directly deliver
				if( neighbor_discovery_->is_neighbor_bidi( msg->dest() ) )
				{
					debug_->debug( "Delivering message directly to %d, size: %d.\n", msg->dest(), msg->buffer_size() );
					radio_->send( msg->dest(), msg->buffer_size(), (block_data_t*)msg );

					statistics_->print_statistics_comment( "direct message delivery at time:" );

					//Save statistics output
					typename Os::Clock clock;
					statistics_->print_statistics(
						clock.seconds( clock.time() ) - starting_time_,
						number_of_stored_messages_,
						0
					);
					return;
				}

				/*
				 * Put message into buffer. This is done by find the first empty slot. If no slot is empty, the oldest
				 * message will be deleted and the new message will be put to that position.
				 */
				uint16_t cur_max = 0;
				uint16_t cur_max_index = 0;

				for( i=0; i < MAX_NUMBER_OF_STORED_MESSAGES; ++i )
				{
					if( !stored_messages_[i].used )
					{
						break;
					} else {
						if( stored_messages_[i].store_duration > cur_max )
						{
							cur_max = stored_messages_[i].store_duration;
							cur_max_index = i;
						}
					}
				}

				if( i == MAX_NUMBER_OF_STORED_MESSAGES ) // All slots are used.
				{
					// Drop oldest message by the new message.
					i = cur_max_index;
					number_of_stored_messages_--;

					debug_->debug( "Could not deliver message from %d to %d. (buffer overflow)\n",
							stored_messages_[cur_max_index].message.source(),
							stored_messages_[cur_max_index].message.dest() );
					statistics_->print_statistics_comment( "Message buffer overflowed: deleted oldest message!" );
				}

				//Store the message in slot i
				stored_messages_[i].used = true;
				stored_messages_[i].store_duration = 0;
				memcpy( &stored_messages_[i].message, msg, msg->buffer_size() );
				number_of_stored_messages_++;

				debug_->debug( "number of stored messages: %d\n", number_of_stored_messages_);

				// clear list of roombas that got current state of message buffer
				for( i=0; i < MAX_NUMBER_OF_ROOMBAS; ++i )
				{
					roomba_interchanged_msg_table_[i].used = false;
				}

				//Save statistics output
				typename Os::Clock clock;
				statistics_->print_statistics(
					clock.seconds( clock.time() ) - starting_time_,
					number_of_stored_messages_,
					1
				);
			}
		}

		struct message_entry
		{
			bool used;
			uint16_t store_duration;
			msg_t message;
		};

		struct roomba_interchanged_msg_entry
		{
			bool used;
			node_id_t roomba_id;
		};

		Radio* radio_;
		int starting_time_;							// starting time that shall be substracted from current event time for statistics
		typename Debug::self_pointer_t debug_;
		typename Timer::self_pointer_t timer_;
		typename Clock::self_pointer_t clock_;
		typename Roomba::self_pointer_t roomba_;
		NeighborDiscovery* neighbor_discovery_;
		RoombaStatistics* statistics_;

		roomba_interchanged_msg_entry roomba_interchanged_msg_table_[MAX_NUMBER_OF_ROOMBAS]; // saves for current buffer which roombas alread got buffer dump
		message_entry stored_messages_[MAX_NUMBER_OF_STORED_MESSAGES];
		uint16_t number_of_stored_messages_;
		int radio_recv_callback_id_;
		bool stopped_;
};

#endif //DELAYED_MESSAGE_DELIVERY
