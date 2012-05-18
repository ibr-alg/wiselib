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

#ifndef COAP_SERVICE_STATIC_H
#define COAP_SERVICE_STATIC_H

#include "coap.h"
#include "coap_packet_static.h"
#include "util/delegates/delegate.hpp"
#include "util/pstl/vector_static.h"
#include "util/pstl/static_string.h"
#include "util/pstl/string_dynamic.h"
#include "util/pstl/list_static.h"

#define COAP_SERVICE_TEMPLATE_PREFIX	template<typename OsModel_P, \
		typename Radio_P, \
		typename Timer_P, \
		typename Rand_P, \
		typename String_T, \
		bool preface_msg_id_, \
		bool human_readable_errors_, \
		typename coap_packet_t_, \
		typename OsModel_P::size_t sent_list_size_, \
		typename OsModel_P::size_t received_list_size_, \
		typename OsModel_P::size_t resources_list_size_>

#define COAP_SERVICE_T	CoapServiceStatic<OsModel_P, Radio_P, Timer_P, Rand_P, String_T, preface_msg_id_, human_readable_errors_, coap_packet_t_, sent_list_size_, received_list_size_, resources_list_size_>

namespace wiselib {


/**
 * \brief This class provides an interface to sending CoAP requests and exposing resources via CoAP.
 * For requesting remote resources have a look at get(), put(), post(), del() and request()<br>
 * For sharing resources via CoAP have a look at reg_resource_callback() and reply()<br>
 * Don't forget to call init() and enable_radio() before you do anything else!<br>
 * This implementation implements many basic features of version 9 of the <a href="https://datatracker.ietf.org/doc/draft-ietf-core-coap/"> CoAP draft</a><br>
 * Known Bugs:
 * - Does not work on iSense JN5148 nodes
 * - Does not work on iSense JN5139R1 nodes when compiled with size optimization (-Os). Optimization levels -O1 and -O2 work.
 * <br>
 *
 * \tparam String_T String Type, wiselib::StaticString works, wiselib::string_dynamic might work, but hasn't been tested
 * \tparam preface_msg_id_ Determines whether a CoAP Packet starts with the message ID <br>
 * 		CoapMsgId - as defined in <a href="https://github.com/ibr-alg/wiselib/wiki/Reserved-message-ids">the Wiselib's Reserved Message IDs</a>
 * \tparam human_readable_errors_ if set to true errors will return a human readable error message in the body. Otherwise the body will contain two int16_t that detail the nature of the error and the option number of the option that caused the error.
 * \tparam coap_packet_t_ type of the coap_packet. Write your own implementation if you like ;) Mainly this parameter is meant to be used for controlling the storage_size_ parameter of CoapPacketStatic
 * \tparam sent_list_size_ size of the message buffer that holds messages sent by CoapServiceStatic
 * \tparam received_list_size_ size of the message buffer that holds messages received by CoapServiceStatic
 * \tparam resources_list_size_ determines how many resources can be registered at CoapServiceStatic
 */
template<typename OsModel_P,
	typename Radio_P = typename OsModel_P::Radio,
	typename Timer_P = typename OsModel_P::Timer,
	typename Rand_P = typename OsModel_P::Rand,
	typename String_T = wiselib::StaticString,
	bool preface_msg_id_ = false,
	bool human_readable_errors_ = false,
	typename coap_packet_t_ = typename wiselib::CoapPacketStatic<OsModel_P, Radio_P, String_T>::coap_packet_t,
	typename OsModel_P::size_t sent_list_size_ = COAPRADIO_SENT_LIST_SIZE,
	typename OsModel_P::size_t received_list_size_ = COAPRADIO_RECEIVED_LIST_SIZE,
	typename OsModel_P::size_t resources_list_size_ = COAPRADIO_RESOURCES_SIZE>
	class CoapServiceStatic
	{

	public:
		// Type definitions
		typedef OsModel_P OsModel;

		typedef Radio_P Radio;

		typedef Timer_P Timer;
		typedef Rand_P Rand;
		typedef String_T string_t;

		typedef typename OsModel::size_t os_size_t;

		typedef typename Radio::node_id_t node_id_t;
		typedef typename Radio::size_t size_t;
		typedef typename Radio::block_data_t block_data_t;
		typedef typename Radio::message_id_t message_id_t;

		typedef COAP_SERVICE_T self_t;
		typedef self_t self_type;
		typedef self_t* self_pointer_t;
		typedef self_t CoapServiceStatic_t;

		typedef coap_packet_t_ coap_packet_t;

		enum error_codes
		{
			// inherited from concepts::BasicReturnValues_concept
			SUCCESS = OsModel::SUCCESS,
			ERR_UNSPEC = OsModel::ERR_UNSPEC,
			ERR_NOTIMPL = OsModel::ERR_NOTIMPL
		};

		enum path_compare
		{
			EQUAL,
			LHS_IS_SUBRESOURCE,
			RHS_IS_SUBRESOURCE,
			NOT_EQUAL
		};

		class ReceivedMessage
		{
		public:

			ReceivedMessage& operator=( const ReceivedMessage &rhs )
			{
				// avoid self-assignment
				if(this != &rhs)
				{
					message_ = rhs.message_;
					correspondent_ = rhs.correspondent_;
					ack_ = rhs.ack_;
					response_ = rhs.response_;
				}
				return *this;
			}

			ReceivedMessage()
			{
				message_ = coap_packet_t();
				ack_ = NULL;
				response_ = NULL;
			}

			ReceivedMessage( const ReceivedMessage &rhs )
			{
				*this = rhs;
			}

			ReceivedMessage( const coap_packet_t &packet, node_id_t from )
			{
				message_ = packet;
				correspondent_ = from;
				ack_ = NULL;
				response_ = NULL;
			}

			/**
			 * Gets the CoapPacketStatic Object the was received
			 * @return received message
			 */
			coap_packet_t & message() const
			{
				return message_;
			}

			/**
			 * Gets the CoapPacketStatic Object the was received
			 * @return received message
			 */
			coap_packet_t & message()
			{
				return message_;
			}

			/**
			 * Gets sender of the message
			 * @return sender of the CoAP message
			 */
			node_id_t correspondent() const
			{
				return correspondent_;
			}

			/**
			 * Gets pointer to the ACK message, if one was sent
			 * @return Pointer to ACK message, NULL if no ACK was sent (yet)
			 */
			coap_packet_t * ack_sent() const
			{
				return ack_;
			}

			/**
			 * Gets pointer to a message sent in response to this received
			 * message (via CoapServiceStatic::reply() )
			 * @return Pointer to response, NULL if no response was sent
			 */
			coap_packet_t * response_sent() const
			{
				return response_;
			}

		private:
			friend class COAP_SERVICE_T;
			coap_packet_t message_;
			// in this case the sender
			node_id_t correspondent_;
			coap_packet_t *ack_;
			coap_packet_t *response_;
			// TODO: empfangszeit? (Freshness)

			void set_message( const coap_packet_t &message)
			{
				message_ = message;
			}

			void set_correspondent( node_id_t correspondent)
			{
				correspondent_ = correspondent;
			}

			void set_ack_sent( coap_packet_t *ack )
			{
				ack_ = ack;
			}

			void set_response_sent(coap_packet_t *response)
			{
				response_ = response;
			}
		};

		typedef delegate1<void, ReceivedMessage&> coapreceiver_delegate_t;

		CoapServiceStatic();
		~CoapServiceStatic();

		int destruct();
		int init( Radio& radio, Timer& timer, Rand& rand );
		int enable_radio();
		int disable_radio();
		node_id_t id ();
		int send (node_id_t receiver, size_t len, block_data_t *data );
		void receive(node_id_t from, size_t len, block_data_t * data);

		/**
		 * Sends the %Message passed AS IT IS. That means: no sanity checks, no message ID or token generation.
		 * Most likely this is NOT what you want, use get(), put(), post(), del(), request() or reply() instead
		 * @param receiver node ID of the receiver
		 * @param message Packet to send
		 * @param callback delegate for responses from the receiver
		 * @return packet sent
		 */
		template<class T, void (T::*TMethod)(ReceivedMessage&)>
		coap_packet_t* send_coap_as_is(node_id_t receiver, const coap_packet_t & message, T *callback);

		/**
		 * Generates a MessageID and sends the %Message.
		 * Most likely this is NOT what you want, use get(), put(), post(), del(), request() or reply() instead
		 * @param receiver node ID of the receiver
		 * @param message packet to send
		 * @param callback delegate for responses from the receiver
		 * @return packet sent
		 */
		template<class T, void (T::*TMethod)(ReceivedMessage&)>
		coap_packet_t* send_coap_gen_msg_id(node_id_t receiver, coap_packet_t & message, T *callback);

		/**
		 * Generates a MessageID and Token and sends the %Message.
		 * Most likely this is NOT what you want, use get(), put(), post(), del(), request() or reply() instead
		 * @param receiver node ID of the receiver
		 * @param message packet to send
		 * @param callback delegate for responses from the receiver
		 * @return packet sent
		 */
		template<class T, void (T::*TMethod)(ReceivedMessage&)>
		coap_packet_t* send_coap_gen_msg_id_token(node_id_t receiver, coap_packet_t & message, T *callback);
		
		/**
		 * Sends an RST in response to the MessageID passed
		 * @param receiver node ID of the receiver
		 * @param id message ID to reset
		 * @return RST message sent
		 */
		coap_packet_t* rst( node_id_t receiver, coap_msg_id_t id );

		/**
		 * Registers a resource. Whenever a request contains an Uri-Path that
		 * equals the resource_path or is a subresource of it, it will be passed
		 * to the callback
		 * @param resource_path path of the resource
		 * @param callback Delegate to call when a request for the resource is received
		 * @return index for unregistering a resource
		 */
		template<class T, void (T::*TMethod)(ReceivedMessage&)>
		int reg_resource_callback( string_t resource_path, T *callback );

		/**
		 * Unregisters a resource
		 * @param idx index of the resource to unregister. This index is what
		 * reg_resource_callback() returns
		 * @return always returns CoapServiceStatic::SUCCESS
		 */
		int unreg_resource_callback( int idx );

		/**
		 * Sends a GET request
		 * @param receiver server to send the request to
		 * @param uri_path Uri-Path to be requested, use "" for empty path
		 * @param uri_query Uri-Query to be requested, use "" for empty path
		 * @param callback Delegate that is called when a response is received
		 * @param confirmable set to true if a CON message should be send
		 * @param uri_host use if server hosts several virtual hosts
		 * @param uri_port use if a port other than COAP_STD_PORT is to be used
		 * @return the packet sent
		 */
		template<class T, void (T::*TMethod)(ReceivedMessage&)>
		coap_packet_t* get(node_id_t receiver,
					const string_t &uri_path,
					const string_t &uri_query,
					T *callback,
					bool confirmable = false,
					const string_t &uri_host = string_t(),
					uint16_t uri_port = COAP_STD_PORT);

		/**
		 * Sends a PUT request
		 * @param receiver server to send the request to
		 * @param uri_path Uri-Path to be requested, use "" for empty path
		 * @param uri_query Uri-Query to be requested, use "" for empty path
		 * @param callback Delegate that is called when a response is received
		 * @param payload body of the request
		 * @param payload_length length of body
		 * @param confirmable set to true if a CON message should be send
		 * @param uri_host use if server hosts several virtual hosts
		 * @param uri_port use if a port other than COAP_STD_PORT is to be used
		 * @return the packet sent
		 */
		template<class T, void (T::*TMethod)(ReceivedMessage&)>
		coap_packet_t* put(node_id_t receiver,
					const string_t &uri_path,
					const string_t &uri_query,
					T *callback,
					uint8_t* payload = NULL,
					size_t payload_length = 0,
					bool confirmable = false,
					const string_t &uri_host = string_t(),
					uint16_t uri_port = COAP_STD_PORT);

		/**
		 * Sends a POST request
		 * @param receiver server to send the request to
		 * @param uri_path Uri-Path to be requested, use "" for empty path
		 * @param uri_query Uri-Query to be requested, use "" for empty path
		 * @param callback Delegate that is called when a response is received
		 * @param payload body of the request
		 * @param payload_length length of body
		 * @param confirmable set to true if a CON message should be send
		 * @param uri_host use if server hosts several virtual hosts
		 * @param uri_port use if a port other than COAP_STD_PORT is to be used
		 * @return the packet sent
		 */
		template<class T, void (T::*TMethod)(ReceivedMessage&)>
		coap_packet_t* post(node_id_t receiver,
					const string_t &uri_path,
					const string_t &uri_query,
					T *callback,
					uint8_t* payload = NULL,
					size_t payload_length = 0,
					bool confirmable = false,
					const string_t &uri_host = string_t(),
					uint16_t uri_port = COAP_STD_PORT);

		/**
		 * Sends a DELETE request
		 * @param receiver server to send the request to
		 * @param uri_path Uri-Path to be requested, use "" for empty path
		 * @param uri_query Uri-Query to be requested, use "" for empty path
		 * @param callback Delegate that is called when a response is received
		 * @param confirmable set to true if a CON message should be send
		 * @param uri_host use if server hosts several virtual hosts
		 * @param uri_port use if a port other than COAP_STD_PORT is to be used
		 * @return the packet sent
		 */
		template<class T, void (T::*TMethod)(ReceivedMessage&)>
		coap_packet_t* del(node_id_t receiver,
					const string_t &uri_path,
					const string_t &uri_query,
					T *callback,
					bool confirmable = false,
					const string_t &uri_host = string_t(),
					uint16_t uri_port = COAP_STD_PORT);

		/**
		 * Sends a request, the type is determined by the code parameter
		 * @param receiver server to send the request to
		 * @param code type of the request (typically GET, PUT, POST or DELETE)
		 * @param uri_path Uri-Path to be requested, use "" for empty path
		 * @param uri_query Uri-Query to be requested, use "" for empty path
		 * @param callback Delegate that is called when a response is received
		 * @param payload body of the request
		 * @param payload_length length of body
		 * @param confirmable set to true if a CON message should be send
		 * @param uri_host use if server hosts several virtual hosts
		 * @param uri_port use if a port other than COAP_STD_PORT is to be used
		 * @return the packet sent
		 */
		template<class T, void (T::*TMethod)(ReceivedMessage&)>
		coap_packet_t* request(node_id_t receiver,
					CoapCode code,
					const string_t &uri_path,
					const string_t &uri_query,
					T *callback,
					uint8_t* payload = NULL,
					size_t payload_length = 0,
					bool confirmable = false,
					const string_t &uri_host = string_t(),
					uint16_t uri_port = COAP_STD_PORT);

		/**
		 * Sends a reply to a received message.
		 * @param req_msg received message that triggered this reply. Note that it has to be the reference that was passed to the application in the callback. Do not pass a copy here, because it would break mechanisms like piggybacked ACKs and retransmissions.
		 * @param payload body of the reply
		 * @param payload_length length of the body
		 * @param code Code of the reply, COAP_CODE_CONTENT by default
		 */
		coap_packet_t* reply( ReceivedMessage& req_msg,
				uint8_t* payload,
				size_t payload_length,
				CoapCode code = COAP_CODE_CONTENT );

	private:
#ifdef BOOST_TEST_DECL
		// *cough* ugly hackery
		// this is probably frowned upon, but I'd like some insight into
		// private members when testing
	public:
#endif
		class SentMessage
		{
		public:
			SentMessage()
			{
				retransmit_count_ = 0;
				ack_received_ = false;
				sender_callback_ = coapreceiver_delegate_t();
				response_ = NULL;
			}

			coap_packet_t & message() const
			{
				return message_;
			}

			coap_packet_t & message()
			{
				return message_;
			}

			void set_message(const coap_packet_t &message)
			{
				message_ = message;
			}

			node_id_t correspondent() const
			{
				return correspondent_;
			}

			void set_correspondent( node_id_t correspondent)
			{
				correspondent_ = correspondent;
			}

			uint8_t retransmit_count() const
			{
				return retransmit_count_;
			}

			void set_retransmit_count(uint8_t retransmit_count)
			{
				retransmit_count_ = retransmit_count;
			}

			uint16_t retransmit_timeout() const
			{
				return retransmit_timeout_;
			}

			void set_retransmit_timeout( uint16_t retransmit_timeout )
			{
				retransmit_timeout_ = retransmit_timeout;
			}

			uint16_t increase_retransmit_count()
			{
				++retransmit_count_;
				retransmit_timeout_ *= 2;
				return retransmit_timeout_;
			}

			bool ack_received() const
			{
				return ack_received_;
			}

			void set_ack_received(bool ack_received)
			{
				ack_received_ = ack_received;
			}

			ReceivedMessage * response_received() const
			{
				return response_;
			}

			void set_response_received(ReceivedMessage & response)
			{
				response_ = &response;
			}

			coapreceiver_delegate_t sender_callback() const
			{
				return sender_callback_;
			}

			void set_sender_callback( const coapreceiver_delegate_t &callback )
			{
				sender_callback_ = callback;
			}

		private:
			coap_packet_t message_;
			// in this case the receiver
			node_id_t correspondent_;
			uint8_t retransmit_count_;
			uint16_t retransmit_timeout_;
			bool ack_received_;
			ReceivedMessage * response_;
			coapreceiver_delegate_t sender_callback_;
		};

		class CoapResource
		{
		public:
			bool operator==( const CoapResource &other ) const
			{
				return ( this->resource_path() == other.resource_path() && this->callback() == other.callback() );
			}

			bool operator!=( const CoapResource &other ) const
			{
				return !( *this == other );
			}

			CoapResource()
			{
				resource_path_ = string_t();
				callback_ = coapreceiver_delegate_t();
			}

			CoapResource( string_t path, coapreceiver_delegate_t callback)
			{
				set_resource_path( path );
				set_callback( callback );
			}

			void set_resource_path( string_t path)
			{
				resource_path_ = path;
			}

			string_t resource_path() const
			{
				return resource_path_;
			}

			void set_callback( coapreceiver_delegate_t callback )
			{
				callback_ = callback;
			}

			coapreceiver_delegate_t callback() const
			{
				return callback_;
			}

		private:
			string_t resource_path_;
			coapreceiver_delegate_t callback_;
		};

		typedef list_static<OsModel, ReceivedMessage, received_list_size_> received_list_t;
		typedef list_static<OsModel, SentMessage, sent_list_size_> sent_list_t;

		Radio *radio_;
		Timer *timer_;
		Rand *rand_;
		int recv_callback_id_; // callback for receive function
		sent_list_t sent_;
		received_list_t received_;
		vector_static<OsModel, CoapResource, resources_list_size_> resources_;

		coap_msg_id_t msg_id_;
		coap_token_t token_;

		CoapServiceStatic( const self_type &rhs );

		coap_msg_id_t msg_id();
		coap_token_t token();

		template <typename T, list_size_t N>
		T * queue_message(T message, list_static<OsModel_P, T, N> &queue);

		template <typename T, list_size_t N>
		T* find_message_by_id (node_id_t correspondent, coap_msg_id_t id, list_static<OsModel_P, T, N> &queue);
		template <typename T, list_size_t N>
		T* find_message_by_token (node_id_t correspondent, const OpaqueData& token, const list_static<OsModel_P, T, N> &queue);

		void handle_response( ReceivedMessage& message, SentMessage *request = NULL );

		void handle_request( ReceivedMessage& message );

		void ack(ReceivedMessage& message );

		void ack_timeout ( void * message );
		void retransmit_timeout ( void * message );

		void error_response( int error, ReceivedMessage& message );

		void receive_coap(ReceivedMessage& message);

		int path_cmp( const string_t &lhs, const string_t &rhs);

	};


// public

	COAP_SERVICE_TEMPLATE_PREFIX
	COAP_SERVICE_T::CoapServiceStatic()
	{
		//init();
	}

	COAP_SERVICE_TEMPLATE_PREFIX
	COAP_SERVICE_T::~CoapServiceStatic()
	{

	}

	COAP_SERVICE_TEMPLATE_PREFIX
	int COAP_SERVICE_T::destruct()
	{
		return SUCCESS;
	}

	COAP_SERVICE_TEMPLATE_PREFIX
	int COAP_SERVICE_T::init(Radio& radio,
				Timer& timer,
				Rand& rand )
	{
		radio_ = &radio;
		timer_ = &timer;
		rand_ = &rand;

		rand_->srand( radio_->id() );

		// random initial message ID and token
		msg_id_ = (*rand_)();
		token_ = (*rand_)();
		return SUCCESS;
	}

	COAP_SERVICE_TEMPLATE_PREFIX
	int COAP_SERVICE_T::enable_radio()
	{
		//enable normal radio
		radio_->enable_radio();
		// register receive callback to normal radio
		recv_callback_id_ = radio_->template reg_recv_callback<self_t,
			&self_t::receive > ( this );

		return SUCCESS;
	}

	COAP_SERVICE_TEMPLATE_PREFIX
	int COAP_SERVICE_T::disable_radio()
	{
		radio_->unreg_recv_callback(recv_callback_id_);
		return SUCCESS;
	}

	COAP_SERVICE_TEMPLATE_PREFIX
	typename Radio_P::node_id_t COAP_SERVICE_T::id ()
	{
		return radio_->id();
	}

	COAP_SERVICE_TEMPLATE_PREFIX
	int COAP_SERVICE_T::send (node_id_t receiver, size_t len, block_data_t *data )
	{
		if( preface_msg_id_ )
		{
			block_data_t buf[len+1];
			buf[0] = CoapMsgId;
			memcpy( buf + 1, data, len );
			return radio_->send(receiver, len + 1, buf);
		}
		else
		{
			return radio_->send(receiver, len, data);
		}

	}

	COAP_SERVICE_TEMPLATE_PREFIX
	template <class T, void (T::*TMethod)( typename COAP_SERVICE_T::ReceivedMessage& ) >
	coap_packet_t_ * COAP_SERVICE_T::send_coap_as_is(node_id_t receiver, const coap_packet_t & message, T *callback)
	{
		block_data_t buf[message.serialize_length()];

		size_t len = message.serialize(buf);
		int status = send(receiver, len, buf);

		if(status != SUCCESS )
			return NULL;

		SentMessage & sent = *( queue_message(SentMessage(), sent_) );
		sent.set_correspondent( receiver );
		sent.set_message( message );
		sent.set_sender_callback( coapreceiver_delegate_t::template from_method<T, TMethod>( callback ) );
		uint16_t response_timeout = (uint16_t) ((*rand_)( (COAP_MAX_RESPONSE_TIMEOUT - COAP_RESPONSE_TIMEOUT) ) + COAP_RESPONSE_TIMEOUT);
		sent.set_retransmit_timeout( response_timeout );

		if( message.type() == COAP_MSG_TYPE_CON )
		{
			timer_->template set_timer<self_type, &self_type::retransmit_timeout>( sent.retransmit_timeout(), this, &sent );
		}

		return &(sent.message());
	}

	COAP_SERVICE_TEMPLATE_PREFIX
	template <class T, void (T::*TMethod)( typename COAP_SERVICE_T::ReceivedMessage& ) >
	coap_packet_t_ * COAP_SERVICE_T::send_coap_gen_msg_id(node_id_t receiver, coap_packet_t & message, T *callback)
	{
		message.set_msg_id( this->msg_id() );
		return send_coap_as_is<T, TMethod>( receiver, message, callback );
	}

	COAP_SERVICE_TEMPLATE_PREFIX
	template <class T, void (T::*TMethod)( typename COAP_SERVICE_T::ReceivedMessage& ) >
	coap_packet_t_ * COAP_SERVICE_T::send_coap_gen_msg_id_token(node_id_t receiver, coap_packet_t & message, T *callback)
	{
		OpaqueData token;
		coap_token_t raw_token = this->token();
		token.set( ( uint8_t* ) &raw_token, sizeof( coap_token_t ) );
		message.set_token( token );
		return send_coap_gen_msg_id<T, TMethod>( receiver, message, callback );
	}

	COAP_SERVICE_TEMPLATE_PREFIX
	void COAP_SERVICE_T::receive(node_id_t from, size_t len, block_data_t * data)
	{
		// do not receive own messages
		if (radio_->id() == from) {
			return;
		}
		if (len > 0 )
		{
			size_t msg_id_t_size = 0;

			// only relevant if preface_msg_id_ is true
			message_id_t msg_id = read<OsModel, block_data_t, message_id_t>( data );
			if( preface_msg_id_ )
			{
				msg_id_t_size = sizeof( message_id_t );
			}
			if( ( preface_msg_id_ && msg_id == CoapMsgId ) || !preface_msg_id_ )
			{
				coap_packet_t packet;

				int err_code = packet.parse_message( data + msg_id_t_size, len - msg_id_t_size );

				if( err_code == SUCCESS )
				{
					ReceivedMessage *deduplication;
					// Only act if this message hasn't been received yet
					if( (deduplication = find_message_by_id(from, packet.msg_id(), received_)) == NULL )
					{
						ReceivedMessage& received_message = *( queue_message( ReceivedMessage( packet, from ), received_ ) );

						SentMessage *request;

						if ( packet.type() == COAP_MSG_TYPE_RST )
						{
							request = find_message_by_id( from, packet.msg_id(), sent_ );
							if( request != NULL )
								(*request).sender_callback()( received_message );
							return;
						}
						else if( packet.type() == COAP_MSG_TYPE_ACK )
						{
							request = find_message_by_id( from, packet.msg_id(), sent_ );

							if ( request != NULL )
							{
								(*request).set_ack_received( true );
								// piggy-backed response, give it to whoever sent the request
								if( packet.is_response() )
									handle_response( received_message, request );
							}
						}
						else
						{
							if( packet.is_request() )
							{
								handle_request( received_message );
							}
							else if ( packet.is_response() )
							{
								handle_response( received_message );
							}
							else if( packet.type() == COAP_MSG_TYPE_CON )
							{
								char * error_description = NULL;
								int len = 0;
								if( human_readable_errors_ )
								{
									char error_description_str[COAP_ERROR_STRING_LEN];
									len = sprintf( error_description, "Unknown Code %i", packet.code() );
									error_description = error_description_str;
								}
								reply( received_message, (block_data_t*) error_description, len, COAP_CODE_NOT_IMPLEMENTED );
							}
						}
					}
					else
					{
						// if it's confirmable we might want to hurry sending an ACK
						if( packet.type() == COAP_MSG_TYPE_CON )
							ack( *deduplication );
						// if the response was piggybacked it was already resent by the line above
						if( deduplication->response_sent() != NULL
								&& deduplication->response_sent()->type() != COAP_MSG_TYPE_ACK)
						{
							block_data_t buf[ deduplication->response_sent()->serialize_length() ];

							deduplication->response_sent()->serialize(buf);
							send(deduplication->correspondent(), deduplication->response_sent()->serialize_length(), buf);
						}
					}
				}
				else if( err_code == coap_packet_t::ERR_NOT_COAP
				         || err_code == coap_packet_t::ERR_WRONG_COAP_VERSION )
				{
					// ignore
					// wrong Coap Version is a good indicator for "isn't
					// actually CoAP", so better ignore
				}
				else
				{
					ReceivedMessage& received_error = *( queue_message( ReceivedMessage( packet, from ), received_ ) );
					error_response( err_code, received_error );
				}
			}
		}
	}

	COAP_SERVICE_TEMPLATE_PREFIX
	coap_packet_t_ * COAP_SERVICE_T::rst( node_id_t receiver, coap_msg_id_t id )
	{
		coap_packet_t rstp;
		rstp.set_type( COAP_MSG_TYPE_RST );
		rstp.set_msg_id( id );
		return send_coap_as_is<self_type, &self_type::receive_coap>( receiver, rstp, this );
	}

	COAP_SERVICE_TEMPLATE_PREFIX
	template <class T, void (T::*TMethod)( typename COAP_SERVICE_T::ReceivedMessage& ) >
	int COAP_SERVICE_T::reg_resource_callback( string_t resource_path, T *callback )
	{

		if ( resources_.empty() )
			resources_.assign( COAPRADIO_RESOURCES_SIZE, CoapResource() );

		for ( unsigned int i = 0; i < resources_.size(); ++i )
		{
			if ( resources_.at(i) == CoapResource() )
			{
				resources_.at(i).set_resource_path( resource_path );
				resources_.at(i).set_callback( coapreceiver_delegate_t::template from_method<T, TMethod>( callback ) );
				return i;
			}
		}

		return -1;
	}

	COAP_SERVICE_TEMPLATE_PREFIX
	int COAP_SERVICE_T::unreg_resource_callback( int idx )
	{
		resources_.at(idx) = CoapResource();
		return SUCCESS;
	}


	COAP_SERVICE_TEMPLATE_PREFIX
	template <class T, void (T::*TMethod)( typename COAP_SERVICE_T::ReceivedMessage& ) >
	coap_packet_t_ * COAP_SERVICE_T::get(node_id_t receiver,
			const string_t &uri_path,
			const string_t &uri_query,
			T *callback,
			bool confirmable,
			const string_t &uri_host,
			uint16_t uri_port)
	{
		return request<T, TMethod>( receiver, COAP_CODE_GET ,uri_path, uri_query, callback, NULL, 0, confirmable, uri_host, uri_port );
	}

	COAP_SERVICE_TEMPLATE_PREFIX
	template <class T, void (T::*TMethod)( typename COAP_SERVICE_T::ReceivedMessage& ) >
	coap_packet_t_ * COAP_SERVICE_T::put(node_id_t receiver,
			const string_t &uri_path,
			const string_t &uri_query,
			T *callback,
			uint8_t* payload,
			size_t payload_length,
			bool confirmable,
			const string_t &uri_host,
			uint16_t uri_port)
	{
		return request<T, TMethod>( receiver, COAP_CODE_PUT ,uri_path, uri_query, callback, payload, payload_length, confirmable, uri_host, uri_port );
	}

	COAP_SERVICE_TEMPLATE_PREFIX
	template <class T, void (T::*TMethod)( typename COAP_SERVICE_T::ReceivedMessage& ) >
	coap_packet_t_ * COAP_SERVICE_T::post(node_id_t receiver,
			const string_t &uri_path,
			const string_t &uri_query,
			T *callback,
			uint8_t* payload,
			size_t payload_length,
			bool confirmable,
			const string_t &uri_host,
			uint16_t uri_port)
	{
		return request<T, TMethod>( receiver, COAP_CODE_POST ,uri_path, uri_query, callback, payload, payload_length, confirmable, uri_host, uri_port );
	}

	COAP_SERVICE_TEMPLATE_PREFIX
	template <class T, void (T::*TMethod)( typename COAP_SERVICE_T::ReceivedMessage& ) >
	coap_packet_t_ * COAP_SERVICE_T::del(node_id_t receiver,
			const string_t &uri_path,
			const string_t &uri_query,
			T *callback,
			bool confirmable,
			const string_t &uri_host,
			uint16_t uri_port)
	{
		return request<T, TMethod>( receiver, COAP_CODE_DELETE ,uri_path, uri_query, callback, NULL, 0, confirmable, uri_host, uri_port );
	}

	COAP_SERVICE_TEMPLATE_PREFIX
	template <class T, void (T::*TMethod)( typename COAP_SERVICE_T::ReceivedMessage& ) >
	coap_packet_t_ * COAP_SERVICE_T::request(node_id_t receiver,
				CoapCode code,
				const string_t &uri_path,
				const string_t &uri_query,
				T *callback,
				uint8_t* payload,
				size_t payload_length,
				bool confirmable,
				const string_t &uri_host,
				uint16_t uri_port)
	{
		coap_packet_t pack;

		pack.set_code( code );
		if( !pack.is_request() )
		{
			// TODO ordentlichen Fehler schmeißen?
			return NULL;
		}


		pack.set_uri_path( uri_path );
		pack.set_uri_query( uri_query );

		pack.set_data( payload, payload_length );

		confirmable ? pack.set_type( COAP_MSG_TYPE_CON ) : pack.set_type( COAP_MSG_TYPE_NON );

		if( string_t() != uri_host  )
		{
			pack.set_option( COAP_OPT_URI_HOST, uri_host );
		}

		pack.set_uri_port( uri_port );

		return send_coap_gen_msg_id_token<T, TMethod>(receiver, pack, callback );
	}

	COAP_SERVICE_TEMPLATE_PREFIX
	coap_packet_t_ * COAP_SERVICE_T::reply(ReceivedMessage &req_msg,
				uint8_t* payload,
				size_t payload_length,
				CoapCode code )
	{
		coap_packet_t *sendstatus = NULL;
		coap_packet_t & request = req_msg.message();
		coap_packet_t reply;
		OpaqueData token;
		request.token( token );

		reply.set_token( token );

		if( request.type() == COAP_MSG_TYPE_CON || request.type() == COAP_MSG_TYPE_NON )
			reply.set_type( request.type() );
		else
			return NULL;
		reply.set_code( code );
		reply.set_data( payload, payload_length );

		if( request.type() == COAP_MSG_TYPE_CON && req_msg.ack_sent() == NULL )
		{
			// no ACK has been sent yet, piggybacked response is possible
			reply.set_type( COAP_MSG_TYPE_ACK );
			reply.set_msg_id( request.msg_id() );
			sendstatus = send_coap_as_is<self_type, &self_type::receive_coap>( req_msg.correspondent(), reply, this );
			if( sendstatus != NULL )
			{
				req_msg.set_ack_sent( sendstatus );
				req_msg.set_response_sent( sendstatus );
			}
		}
		else
		{
			sendstatus = send_coap_gen_msg_id<self_type, &self_type::receive_coap>( req_msg.correspondent(), reply, this );
			if( sendstatus != NULL )
			{
				req_msg.set_response_sent( sendstatus );
			}
		}

		return sendstatus;
	}


// private
	COAP_SERVICE_TEMPLATE_PREFIX
	coap_msg_id_t COAP_SERVICE_T::msg_id()
	{
		return(msg_id_++);
	}

	COAP_SERVICE_TEMPLATE_PREFIX
	coap_token_t COAP_SERVICE_T::token()
	{
		return(token_++);
	}

	COAP_SERVICE_TEMPLATE_PREFIX
	template <typename T, list_size_t N>
	T * COAP_SERVICE_T::queue_message(T message, list_static<OsModel_P, T, N> &queue)
	{
		if( queue.full() )
		{
			queue.pop_back();
		}
		queue.push_front( message );
		return &(queue.front());
	}

	COAP_SERVICE_TEMPLATE_PREFIX
	template <typename T, list_size_t N>
	T* COAP_SERVICE_T::find_message_by_id
			(node_id_t correspondent, coap_msg_id_t id, list_static<OsModel_P, T, N> &queue)
	{
		typename list_static<OsModel_P, T, N>::iterator it = queue.begin();
		for(; it != queue.end(); ++it)
		{
			if( (*it).message().msg_id() == id && (*it).correspondent() == correspondent )
				return &(*it);
		}

		return NULL;

	}

	COAP_SERVICE_TEMPLATE_PREFIX
	template <typename T, list_size_t N>
	T* COAP_SERVICE_T::find_message_by_token
		(node_id_t correspondent, const OpaqueData &token, const list_static<OsModel_P, T, N> &queue)
	{
		OpaqueData current_token;
		typename list_static<OsModel_P, T, N>::iterator it = queue.begin();
		for(; it != queue.end(); ++it)
		{
			if( (*it).correspondent() == correspondent )
			{
				(*it).message().token( current_token );
				if( current_token == token )
				{
					return &(*it);
				}
			}
		}
		return NULL;
	}

	// the request-pointer can be a candidate for a matching request, determined by a previous search by message id.
	// If it doesn't turn out to be matching, find_message_by_token has to be called
	COAP_SERVICE_TEMPLATE_PREFIX
	void COAP_SERVICE_T::handle_response( ReceivedMessage& message, SentMessage *request )
	{
		OpaqueData request_token, response_token;
		message.message().token( response_token );

		// see if the given request candidate is the matching request, otherwise find the matching request by token
		if( request != NULL )
		{
			(*request).message().token(request_token);
		}

		if( request == NULL || request_token != response_token )
		{
			request = find_message_by_token( message.correspondent(), response_token, sent_ );
			if( request == NULL )
			{
				// can't match response
				if( message.message().type() == COAP_MSG_TYPE_CON )
				{
					message.set_response_sent( rst( message.correspondent(), message.message().msg_id() ) );
				}
				return;
			}
			(*request).message().token(request_token);
		}

		if( message.message().type() == COAP_MSG_TYPE_CON )
		{
			ack( message );
		}

		if( request->sender_callback() && request->sender_callback().obj_ptr() != NULL )
		{
			(*request).set_response_received( message );
			(*request).sender_callback()( message );
		}

	}

	COAP_SERVICE_TEMPLATE_PREFIX
	void COAP_SERVICE_T::handle_request( ReceivedMessage& message )
	{
		// don't send an ACK right away, instead wait a little to give the
		// resource a chance to send a piggybacked response
		if( message.message().type() == COAP_MSG_TYPE_CON )
		{
			timer_->template set_timer<self_type, &self_type::ack_timeout>( COAP_ACK_GRACE_PERIOD, this, &message );
		}

		string_t available_res;
		// TODO: we're looking at the first path segment only, subresources should be handled by their parents
		string_t request_res = message.message().uri_path();
		bool resource_found = false;
		for(size_t i = 0; i < resources_.size(); ++i )
		{
			if( resources_.at(i) != CoapResource() && resources_.at(i).callback() && resources_.at(i).callback().obj_ptr() != NULL )
			{
				available_res = resources_.at(i).resource_path();
				// in order to match a resource, the requested uri must match a resource, or it must be a sub-element of a resource,
				// which means the next symbol in the request must be a slash
				if( path_cmp( request_res, available_res ) == EQUAL
				    || path_cmp( request_res, available_res ) == LHS_IS_SUBRESOURCE )
				{
					resources_.at(i).callback()( message );
					resource_found = true;
				}
			}
		}
		if( !resource_found )
		{

			char * error_description = NULL;
			int len = 0;
			if( human_readable_errors_ )
			{
				char error_description_str[COAP_ERROR_STRING_LEN];
				len = sprintf(error_description, "Resource %s not found.", request_res.c_str() );
				error_description = error_description_str;
			}
			reply( message, (uint8_t*) error_description, len, COAP_CODE_NOT_FOUND );
		}
	}

	COAP_SERVICE_TEMPLATE_PREFIX
	void COAP_SERVICE_T::ack( ReceivedMessage& message )
	{
		if( message.ack_sent() != NULL )
		{
			block_data_t buf[ message.ack_sent()->serialize_length() ];

			message.ack_sent()->serialize(buf);
			send(message.correspondent(), message.ack_sent()->serialize_length(), buf);
			return;
		}
		coap_packet_t ackp;
		ackp.set_type( COAP_MSG_TYPE_ACK );
		ackp.set_msg_id( message.message().msg_id() );
		coap_packet_t * sent = send_coap_as_is<self_type, &self_type::receive_coap>(message.correspondent(), ackp, this );
		message.set_ack_sent( sent );
	}

	COAP_SERVICE_TEMPLATE_PREFIX
	void COAP_SERVICE_T::ack_timeout ( void * message )
	{
		ReceivedMessage *sent = (ReceivedMessage*) message;
		if( sent->ack_sent() == NULL )
		{
			ack( (*sent) );
		}
	}

	COAP_SERVICE_TEMPLATE_PREFIX
	void COAP_SERVICE_T::retransmit_timeout ( void * message )
	{
		SentMessage *sent = (SentMessage*) message;
		if( !sent->ack_received() && sent->response_received() == NULL)
		{
			size_t length = sent->message().serialize_length();
			block_data_t buf[length];
			sent->message().serialize(buf);
			int status = send(sent->correspondent(), length, buf);

			if(status != SUCCESS )
			{
				timer_->template set_timer<self_type, &self_type::retransmit_timeout>( 1000, this, message );
				return;
			}

			uint16_t retransmit_time = sent->increase_retransmit_count();
			if(sent->retransmit_count() < COAP_MAX_RETRANSMIT )
			{
				timer_->template set_timer<self_type, &self_type::retransmit_timeout>( retransmit_time, this, message );
			}
		}
	}

	COAP_SERVICE_TEMPLATE_PREFIX
	void COAP_SERVICE_T::error_response( int error, ReceivedMessage& message )
	{
		coap_packet_t packet = message.message();
		CoapCode err_coap_code;
		CoapOptionNum err_optnum;
		packet.get_error_context( err_coap_code, err_optnum);
		if(err_coap_code == 0)
		{
			err_coap_code = COAP_CODE_INTERNAL_SERVER_ERROR;
		}
		block_data_t * error_description = NULL;
		int len = 0;
		if( human_readable_errors_ )
		{
			char error_description_str[COAP_ERROR_STRING_LEN];
			switch( error)
			{
			case coap_packet_t::ERR_OPTIONS_EXCEED_PACKET_LENGTH:
				len = sprintf(error_description_str, "Error: Options exceed packet length, last parsed option: %i", err_optnum );
				break;
			case coap_packet_t::ERR_UNKNOWN_CRITICAL_OPTION:
				len = sprintf(error_description_str, "Error: Unknown critical option %i ", err_optnum );
				break;
			case coap_packet_t::ERR_MULTIPLE_OCCURENCES_OF_CRITICAL_OPTION:
				len = sprintf(error_description_str, "Error: Undue multiple occurences of option %i ", err_optnum );
				break;
			case coap_packet_t::ERR_EMPTY_STRING_OPTION:
				len = sprintf(error_description_str, "Error: Empty String option %i ", err_optnum );
				break;
			case coap_packet_t::ERR_NOT_COAP:
				// should not happen as these are already sorted out in receive()
				break;
			case coap_packet_t::ERR_WRONG_COAP_VERSION:
				// should not happen as these are already sorted out in receive()
				break;
			default:
				len = sprintf(error_description_str, "Error: Unknown error %i, last option before error: %i ", error, err_optnum );
			}
			error_description = (block_data_t *) error_description_str;
		}
		else
		{
			block_data_t error_description_uint[ 2 * sizeof( int16_t ) ];
			int16_t transmit_error = (int16_t) error;
			int16_t transmit_optnum = (int16_t) err_optnum;
			len = write<OsModel , block_data_t , int16_t >( error_description_uint, transmit_error );
			len += write<OsModel , block_data_t , int16_t >( error_description_uint + len, transmit_optnum );
			error_description = error_description_uint;
		}
		reply( message, error_description, len, err_coap_code );
	}

	COAP_SERVICE_TEMPLATE_PREFIX
	void COAP_SERVICE_T::receive_coap(ReceivedMessage& message)
	{
		//TODO
	}

	COAP_SERVICE_TEMPLATE_PREFIX
	int COAP_SERVICE_T::path_cmp(const string_t &lhs, const string_t &rhs)
	{
		for( size_t i = 0; ; ++i )
		{
			if( i == lhs.length() )
			{
				if( i == rhs.length() )
					return EQUAL;
				else if( rhs[i] == '/' )
					return LHS_IS_SUBRESOURCE;
				else
					return NOT_EQUAL;
			}
			if( i == rhs.length() )
			{
				if( lhs[i] == '/' )
					return RHS_IS_SUBRESOURCE;
				else
					return NOT_EQUAL;
			}

			if( lhs[i] != rhs[i] )
				return NOT_EQUAL;
		}
	}
}


#endif // COAP_SERVICE_STATIC_H
