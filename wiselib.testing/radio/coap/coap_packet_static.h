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

#ifndef COAP_PACKET_STATIC_H
#define COAP_PACKET_STATIC_H

#include "coap.h"

static const size_t SINGLE_OPTION_NO_HEADER = 0;

namespace wiselib
{
	template<typename OsModel_P,
	typename Radio_P,
	typename String_T,
	size_t storage_size_ = COAP_DEFAULT_STORAGE_SIZE >
	class CoapPacketStatic
	{
	public:
		typedef OsModel_P OsModel;
		typedef typename OsModel_P::Debug Debug;
		typedef Radio_P Radio;
		typedef typename Radio::block_data_t block_data_t;
		typedef String_T string_t;

		typedef CoapPacketStatic<OsModel_P, Radio_P, String_T, storage_size_> self_type;
		typedef self_type* self_pointer_t;
		typedef self_type coap_packet_t;

		enum error_code
		{
			// inherited from concepts::BasicReturnValues_concept
			SUCCESS = OsModel::SUCCESS,
			ERR_NOMEM = OsModel::ERR_NOMEM,
			ERR_UNSPEC = OsModel::ERR_UNSPEC,
			ERR_NOTIMPL = OsModel::ERR_NOTIMPL,
			// coap_packet_t errors
			ERR_WRONG_TYPE,
			ERR_UNKNOWN_OPT,
			ERR_OPT_NOT_SET,
			ERR_OPT_TOO_LONG,
			ERR_METHOD_NOT_APPLICABLE,
			ERR_MULTIPLE_OCCURENCES_OF_OPTION,
			// packet parsing errors
			ERR_OPTIONS_EXCEED_PACKET_LENGTH,
			ERR_UNKNOWN_CRITICAL_OPTION,
			ERR_MULTIPLE_OCCURENCES_OF_CRITICAL_OPTION,
			ERR_EMPTY_STRING_OPTION,
			ERR_NOT_COAP,
			ERR_WRONG_COAP_VERSION
		};

		///@name Construction / Destruction
		///@{
		CoapPacketStatic( );
		CoapPacketStatic( const coap_packet_t &rhs );
		~CoapPacketStatic();
		///@}

		coap_packet_t& operator=(const coap_packet_t &rhs);

		void init();

		/**
		 * Takes a stream of data and tries to parse it into the CoapPacketStatic from which this method is called
		 * @param datastream the serial data to be parsed
		 * @param length length of the datastream
		 * @return CoapPacketStatic::SUCCESS on successful parsing<br>
		 *         CoapPacketStatic::ERR_NOMEM if the message is too large to be stored<br>
		 *         CoapPacketStatic::ERR_NOT_COAP if the message does not appear to be a CoAP message<br>
		 *         CoapPacketStatic::ERR_WRONG_COAP_VERSION if an incompatible CoAP Version is used<br>
		 *         CoapPacketStatic::ERR_OPTIONS_EXCEED_PACKET_LENGTH if the options run out of the packets length<br>
		 *         CoapPacketStatic::ERR_UNKNOWN_CRITICAL_OPTION<br>
		 *         CoapPacketStatic::ERR_MULTIPLE_OCCURENCES_OF_CRITICAL_OPTION<br>
		 *         CoapPacketStatic::ERR_EMPTY_STRING_OPTION<br>
		 *         an unsuccessful parsing attempt may, depending on the type of error, also store additional information about the nature of the failure. This information can be retrieved with get_error_context()
		 */
		int parse_message( block_data_t *datastream, size_t length );

		/**
		 * Returns the CoAP version number of the packet
		 * @return CoAP version number
		 */
		uint8_t version() const;

		/**
		 * Sets the CoAP version number of the packet.<br>
		 * DO NOT set it to anything other than COAP_VERSION unless you know what you are doing!<br>
		 * Note: the corresponding field in the CoAP packet is 2 bit wide. Setting it to anything greater than 3 is pointless.
		 * @param version version number
		 */
		void set_version( uint8_t version );

		/**
		 * Returns the type of the packet. Can be Confirmable, Non-Confirmable, Acknowledgement or Reset.
		 * @return message type
		 */
		CoapType type() const;

		/**
		 * Sets the type of the packet. Can be COAP_MSG_TYPE_CON, COAP_MSG_TYPE_NON, COAP_MSG_TYPE_ACK or COAP_MSG_TYPE_RST.
		 * @param type message type
		 */
		void set_type( CoapType type );

		/**
		 * Returns the code of the packet. Can be a request (1-31), response (64-191) or empty (0). (all other values are reserved)<br>
		 * For details refer to the CoAP Code Registry section of the CoAP draft.
		 * @return code of the message
		 */
		CoapCode code() const;

		/**
		 * Returns whether the packet is a request
		 * @return true if packet is a request, false otherwise
		 */
		bool is_request() const;

		/**
		 * Returns whether the packet is a response
		 * @return true if packet is a response, false otherwise
		 */
		bool is_response() const;

		/**
		 * Sets the code of the packet. Can be a request (1-31), response (64-191) or empty (0). (all other values are reserved)<br>
		 * For details refer to the CoAP Code Registry section of the CoAP draft.
		 * @param code code of the packet
		 */
		void set_code( CoapCode code );

		/**
		 * Returns the message ID by which message duplication can be detected and request/response matching can be done.
		 * @return the message ID
		 */
		coap_msg_id_t msg_id() const;

		/**
		 * Sets the message ID by which message duplication can be detected and request/response matching can be done.
		 * @param msg_id new message ID
		 */
		void set_msg_id( coap_msg_id_t msg_id );

		/**
		 * \brief Returns a uint32_t where every bit that is set indicates the presence of the corresponding option in the packet.
		 * Counting starts at 0, so that shifting can be used and the number of bits shifted is the option number.<br>
		 * For example:<br>
		 * 0x0000 8204 means that Uri-Query, Uri-Path and Max-Age are set.
		 * @return a uint32_t where every bit that is set indicates the presence of the corresponding option in the packet.
		 */
		uint32_t what_options_are_set() const;

		/**
		 * Returns the token by which request/response matching can be done.<br>
		 * If the token option is not set, an zero length OpaqueData object is returned, representing the empty default token.
		 * @param token reference to OpaqueData object, will contain message token afterwards
		 */
		void token( OpaqueData &token );

		/**
		 * Sets the token by which request/response matching can be done.
		 * @param token new message ID
		 */
		void set_token( const OpaqueData &token );

		/**
		 * Returns the Uri-Path option. Returns a zero length string if Uri Path is not set
		 * @return the Uri Path value
		 */
		string_t uri_path();

		/**
		 * Sets Uri-Path value
		 * @param path new Uri-Path
		 */
		int set_uri_path( const string_t &path );

		/**
		 * Sets Uri-Query value
		 * @param query new Uri-Query
		 */
		int set_uri_query( const string_t &query );

		/**
		 * Sets Content-Type value
		 * @param content_type new Content-Type
		 */
		void set_content_type( CoapContentType content_type );

		/**
		 * Returns the Content-Type option.
		 * @return the Content-Type value
		 */
		CoapContentType content_type();

		/**
		 * Retrieves a list of Query segments for Uri-Query of Location-Query
		 * @param optnum COAP_OPT_URI_QUERY or COAP_OPT_LOCATION_QUERY
		 * @param result list where the segments will be stored
		 * @return CoapPacketStatic::SUCCESS on successful retrieval<br>
		 *         CoapPacketStatic::ERR_OPT_NOT_SET if the option is not set<br>
		 *         CoapPacketStatic::ERR_METHOD_NOT_APPLICABLE if optnum is something other than those allowed
		 */
		template<typename list_t>
		int get_query_list( CoapOptionNum optnum, list_t &result );

		/**
		 * Sets Uri-Port value
		 * @param port new Uri-Port
		 */
		void set_uri_port( uint32_t port );

		/**
		 * Returns Uri-Port value. Returns COAP_STD_PORT if port option is not explicitly set
		 * @return Uri-Port value
		 */
		uint32_t uri_port();

		/**
		 * Returns a pointer to the payload section of the packet
		 * @return pointer to payload
		 */
		block_data_t* data();
		const block_data_t* data() const;

		/**
		 * Returns the length of the payload
		 * @return payload length
		 */
		size_t data_length() const;

		/**
		 * Sets the payload
		 * @param data the payload
		 * @param length payload length
		 * @return CoapPacketStatic::ERR_NOMEM if there is not enough space for data this size<br>
		 *         CoapPacketStatic::SUCCESS otherwise
		 */
		int set_data( block_data_t* data , size_t length);

		/**
		 * Sets the option with the given option number
		 * @param option_number Option to set
		 * @param value new value of the option
		 * @return CoapPacketStatic::SUCCESS on Success<br>
		 *         CoapPacketStatic::ERR_UNKNOWN_OPT when an unknown option number is passed<br>
		 *         CoapPacketStatic::ERR_WRONG_TYPE when the option with the given option number is not of uint type<br>
		 *         CoapPacketStatic::ERR_NOMEM when there is not enough memory to store the option
		 */
		int set_option( CoapOptionNum option_number, uint32_t value );
		/**
		 * Sets the option with the given option number
		 * @param option_number Option to set
		 * @param value new value of the option
		 * @return CoapPacketStatic::SUCCESS on Success<br>
		 *         CoapPacketStatic::ERR_UNKNOWN_OPT when an unknown option number is passed<br>
		 *         CoapPacketStatic::ERR_WRONG_TYPE when the option with the given option number is not of string type<br>
		 *         CoapPacketStatic::ERR_NOMEM when there is not enough memory to store the option
		 */
		int set_option( CoapOptionNum option_number, const string_t &value );
		/**
		 * Sets the option with the given option number
		 * @param option_number Option to set
		 * @param value new value of the option
		 * @return CoapPacketStatic::SUCCESS on Success<br>
		 *         CoapPacketStatic::ERR_UNKNOWN_OPT when an unknown option number is passed<br>
		 *         CoapPacketStatic::ERR_WRONG_TYPE when the option with the given option number is not of opaque type<br>
		 *         CoapPacketStatic::ERR_NOMEM when there is not enough memory to store the option
		 */
		int set_option( CoapOptionNum option_number, const OpaqueData &value );

		/**
		 * Appends the passed value to the option with the given option number
		 * @param option_number Option to set
		 * @param value new value of the option
		 * @return CoapPacketStatic::SUCCESS on Success<br>
		 *         CoapPacketStatic::ERR_UNKNOWN_OPT when an unknown option number is passed<br>
		 *         CoapPacketStatic::ERR_WRONG_TYPE when the option with the given option number is not of uint type<br>
		 *         CoapPacketStatic::ERR_NOMEM when there is not enough memory to store the option<br>
		 *         CoapPacketStatic::ERR_MULTIPLE_OCCURENCES_OF_OPTION if the option is already set and not allowed to occur multiple times
		 */
		int add_option( CoapOptionNum option_number, uint32_t value );
		/**
		 * Appends the passed value to the option with the given option number
		 * @param option_number Option to set
		 * @param value new value of the option
		 * @return CoapPacketStatic::SUCCESS on Success<br>
		 *         CoapPacketStatic::ERR_UNKNOWN_OPT when an unknown option number is passed<br>
		 *         CoapPacketStatic::ERR_WRONG_TYPE when the option with the given option number is not of string type<br>
		 *         CoapPacketStatic::ERR_NOMEM when there is not enough memory to store the option<br>
		 *         CoapPacketStatic::ERR_MULTIPLE_OCCURENCES_OF_OPTION if the option is already set and not allowed to occur multiple times
		 */
		int add_option( CoapOptionNum option_number, const string_t &value );
		/**
		 * Appends the passed value to the option with the given option number
		 * @param option_number Option to set
		 * @param value new value of the option
		 * @return CoapPacketStatic::SUCCESS on Success<br>
		 *         CoapPacketStatic::ERR_UNKNOWN_OPT when an unknown option number is passed<br>
		 *         CoapPacketStatic::ERR_WRONG_TYPE when the option with the given option number is not of opaque type<br>
		 *         CoapPacketStatic::ERR_NOMEM when there is not enough memory to store the option<br>
		 *         CoapPacketStatic::ERR_MULTIPLE_OCCURENCES_OF_OPTION if the option is already set and not allowed to occur multiple times
		 */
		int add_option( CoapOptionNum option_number, const OpaqueData &value );

		/**
		 * Retrieves the value of an option
		 * @param option_number option in question
		 * @param value value of the option
		 * @return CoapPacketStatic::SUCCESS on Success<br>
		 *         CoapPacketStatic::ERR_OPT_NOT_SET if the option is not set
		 */
		int get_option( CoapOptionNum option_number, uint32_t &value );
		/**
		 * Retrieves the value of an option
		 * @param option_number option in question
		 * @param value value of the option
		 * @return CoapPacketStatic::SUCCESS on Success<br>
		 *         CoapPacketStatic::ERR_OPT_NOT_SET if the option is not set
		 */
		int get_option( CoapOptionNum option_number, string_t &value );
		/**
		 * Retrieves the value of an option
		 * @param option_number option in question
		 * @param value value of the option
		 * @return CoapPacketStatic::SUCCESS on Success<br>
		 *         CoapPacketStatic::ERR_OPT_NOT_SET if the option is not set
		 */
		int get_option( CoapOptionNum option_number, OpaqueData &value );
		/**
		 * Retrieves the value of all occurences of an option
		 * @param option_number option in question
		 * @param values List to store the value in. Has to be a list type for string_t, uint32_t or OpaqueData, depending on the type of the Option
		 * @return CoapPacketStatic::SUCCESS on Success<br>
		 *         CoapPacketStatic::ERR_OPT_NOT_SET if the option is not set
		 */
		template<typename list_t>
		int get_options( CoapOptionNum option_number, list_t &values );

		/**
		 * Removes all segments of the option from the packet
		 * @return always returns CoapPacketStatic::SUCCESS
		 */
		int remove_option( CoapOptionNum option_number );

		/**
		 * Retrieves current state of the If-None-Match Option
		 * @return true if If-None-Match is set, false otherwise
		 */
		bool opt_if_none_match() const;

		/**
		 * Sets or unsets If-None-Match
		 * @param opt_if_none_match new state of If-None-Match
		 * @return CoapPacketStatic::SUCCESS on success<br>
		 *         CoapPacketStatic::ERR_NOMEM when there is not enough memory to store the option
		 */
		int set_opt_if_none_match( bool opt_if_none_match );

		/**
		 * Returns the number of option *segments* (not options) in the packet.
		 * @return the number of option segments
		 */
		size_t option_count() const;

		/**
		 * Calculates the length of the message if it were serialized in the current sate
		 * @return the expected length of the serialized message
		 */
		size_t serialize_length() const;

		/**
		 * Serializes the packet so it can be sent over the radio.
		 * @param datastream to where the serialized packet will be written.
		 * @return length of the packet
		 */
		size_t serialize( block_data_t *datastream ) const;

		/**
		 * \brief Retrieves more information about packet parsing errors
		 * When a parsing error occurs the CoapPacketStatic class stores the CoapCode that should be returned in reply and the option number of the option where the error occured (if it occured during parsing an option)
		 * @param error_code CoapCode that should be returned in reply
		 * @param error_option option number of the option where the error occured
		 */
		void get_error_context( CoapCode &error_code, CoapOptionNum &error_option);

	private:
		// points to beginning of payload
		block_data_t *payload_;
		// marks the first byte PAST the last option
		block_data_t *end_of_options_;
		size_t data_length_;
		size_t option_count_;
		coap_msg_id_t msg_id_;
		uint8_t type_;
		uint8_t code_;
		// only relevant when an error occurs
		uint8_t error_code_;
		uint8_t error_option_;
		// Coap Version
		uint8_t version_;

		block_data_t* options_[COAP_OPTION_ARRAY_SIZE];
		// contains Options and Data
		block_data_t storage_[storage_size_];

		int add_option(CoapOptionNum num, const block_data_t *serial_opt, size_t len, size_t num_of_opts = SINGLE_OPTION_NO_HEADER );
		int add_end_of_opts_marker();
		void remove_end_of_opts_marker();
		bool is_end_of_opts_marker( block_data_t *option_header);
		void scan_opts( block_data_t *start, uint8_t prev );
		int initial_scan_opts( size_t num_of_opts, size_t message_length );
		uint8_t next_fencepost_delta(uint8_t previous_opt_number) const;
		bool is_fencepost( uint8_t optnum) const;
		bool is_critical( uint8_t option_number ) const;
		size_t optlen(block_data_t * optheader) const;
		size_t make_segments_from_string( const char *cstr, char delimiter, CoapOptionNum optnum, block_data_t *segments, size_t &num_segments ) const;
		void make_string_from_segments( char delimiter, CoapOptionNum optnum, string_t &result );

	};


	// Implementation starts here
	template<typename OsModel_P,
	typename Radio_P,
	typename String_T,
	size_t storage_size_>
	CoapPacketStatic<OsModel_P, Radio_P, String_T, storage_size_>& CoapPacketStatic<OsModel_P, Radio_P, String_T, storage_size_>::operator=(const coap_packet_t &rhs)
	{
		if( &rhs != this )
		{
			memcpy( storage_, rhs.storage_, storage_size_ );
			for( size_t i = 0; i < COAP_OPTION_ARRAY_SIZE; ++i)
			{
				if(rhs.options_[i] != NULL )
				{
					options_[i] = storage_ + ( rhs.options_[i] - rhs.storage_ );
				}
			}
			payload_ = storage_ + ( rhs.payload_ - rhs.storage_ );
			end_of_options_ = storage_ + ( rhs.end_of_options_ - rhs.storage_ );
			data_length_ = rhs.data_length_;
			option_count_ = rhs.option_count_;
			msg_id_ = rhs.msg_id_;
			type_ = rhs.type_;
			code_ = rhs.code_;
			error_code_ = rhs.error_code_;
			error_option_ = rhs.error_option_;
			version_ = rhs.version_;
		}
		return *this;
	}

	template<typename OsModel_P,
	typename Radio_P,
	typename String_T,
	size_t storage_size_>
	CoapPacketStatic<OsModel_P, Radio_P, String_T, storage_size_>::CoapPacketStatic()
	{
		init();
	}

	template<typename OsModel_P,
	typename Radio_P,
	typename String_T,
	size_t storage_size_>
	CoapPacketStatic<OsModel_P, Radio_P, String_T, storage_size_>::CoapPacketStatic( const coap_packet_t &rhs)
	{
		init();
		*this = rhs;
	}

	template<typename OsModel_P,
	typename Radio_P,
	typename String_T,
	size_t storage_size_>
	CoapPacketStatic<OsModel_P, Radio_P, String_T, storage_size_>::~CoapPacketStatic()
	{
	}

	template<typename OsModel_P,
	typename Radio_P,
	typename String_T,
	size_t storage_size_>
	void CoapPacketStatic<OsModel_P, Radio_P, String_T, storage_size_>::init()
	{
		version_ = COAP_VERSION;
		// TODO: sinnvollen Default festlegen und dann ein COAP_MSG_DEFAULT_TYPE Makro anlegen oder so
		type_ = COAP_MSG_TYPE_NON;
		code_ = COAP_CODE_EMPTY;
		msg_id_ = 0;
		for(size_t i = 0; i < COAP_OPTION_ARRAY_SIZE; ++i)
		{
			options_[i] = NULL;
		}

		payload_ = storage_ + storage_size_;
		end_of_options_ = storage_;
		data_length_ = 0;
		option_count_ = 0;

		error_code_ = 0;
		error_option_ = 0;
	}

	template<typename OsModel_P,
	typename Radio_P,
	typename String_T,
	size_t storage_size_>
	int CoapPacketStatic<OsModel_P, Radio_P, String_T, storage_size_>::parse_message( block_data_t *datastream, size_t length )
	{
		// clear everything
		init();

		// can this possibly be a coap packet?
		if( length >= COAP_START_OF_OPTIONS )
		{
			if( ( length - COAP_START_OF_OPTIONS ) > storage_size_ )
				return ERR_NOMEM;
			uint8_t coap_first_byte = read<OsModel , block_data_t , uint8_t >( datastream );
			version_ = coap_first_byte >> 6 ;
			if( version_ != COAP_VERSION )
			{
				error_code_ = COAP_CODE_NOT_IMPLEMENTED;
				error_option_ = COAP_OPT_NOOPT;
				return ERR_WRONG_COAP_VERSION;
			}
			type_ = ( coap_first_byte & 0x30 ) >> 4 ;
			size_t option_count = coap_first_byte & 0x0f;
			code_ = read<OsModel , block_data_t , uint8_t >( datastream +1 );
			msg_id_ = read<OsModel , block_data_t , coap_msg_id_t >( datastream + 2 );

			memcpy( storage_, datastream + COAP_START_OF_OPTIONS, length - COAP_START_OF_OPTIONS );

			int status = initial_scan_opts( option_count, length - COAP_START_OF_OPTIONS );
			return status;
		}
		// can't make any sense of it
		return ERR_NOT_COAP;
	}

	template<typename OsModel_P,
	typename Radio_P,
	typename String_T,
	size_t storage_size_>
	uint8_t CoapPacketStatic<OsModel_P, Radio_P, String_T, storage_size_>::version() const
	{
		return version_;
	}

	template<typename OsModel_P,
	typename Radio_P,
	typename String_T,
	size_t storage_size_>
	void CoapPacketStatic<OsModel_P, Radio_P, String_T, storage_size_>::set_version( uint8_t version )
	{
		version_ = version & 0x03;
	}

	template<typename OsModel_P,
	typename Radio_P,
	typename String_T,
	size_t storage_size_>
	CoapType CoapPacketStatic<OsModel_P, Radio_P, String_T, storage_size_>::type() const
	{
		return (CoapType) type_;
	}

	template<typename OsModel_P,
	typename Radio_P,
	typename String_T,
	size_t storage_size_>
	void CoapPacketStatic<OsModel_P, Radio_P, String_T, storage_size_>::set_type( CoapType type )
	{
		type_ = type;
	}

	template<typename OsModel_P,
	typename Radio_P,
	typename String_T,
	size_t storage_size_>
	CoapCode CoapPacketStatic<OsModel_P, Radio_P, String_T, storage_size_>::code() const
	{
		return (CoapCode) code_;
	}

	template<typename OsModel_P,
	typename Radio_P,
	typename String_T,
	size_t storage_size_>
	void CoapPacketStatic<OsModel_P, Radio_P, String_T, storage_size_>::set_code( CoapCode code )
	{
		code_ = code;
	}

	template<typename OsModel_P,
	typename Radio_P,
	typename String_T,
	size_t storage_size_>
	bool CoapPacketStatic<OsModel_P, Radio_P, String_T, storage_size_>::is_request() const
	{
		return( code_ >= COAP_REQUEST_CODE_RANGE_MIN && code_ <= COAP_REQUEST_CODE_RANGE_MAX );
	}

	template<typename OsModel_P,
	typename Radio_P,
	typename String_T,
	size_t storage_size_>
	bool CoapPacketStatic<OsModel_P, Radio_P, String_T, storage_size_>::is_response() const
	{
		return( code_ >= COAP_RESPONSE_CODE_RANGE_MIN && code_ <= COAP_RESPONSE_CODE_RANGE_MAX );
	}

	template<typename OsModel_P,
	typename Radio_P,
	typename String_T,
	size_t storage_size_>
	uint16_t CoapPacketStatic<OsModel_P, Radio_P, String_T, storage_size_>::msg_id() const
	{
		return msg_id_;
	}

	template<typename OsModel_P,
	typename Radio_P,
	typename String_T,
	size_t storage_size_>
	void CoapPacketStatic<OsModel_P, Radio_P, String_T, storage_size_>::set_msg_id( coap_msg_id_t msg_id )
	{
		msg_id_ = msg_id;
	}

	template<typename OsModel_P,
	typename Radio_P,
	typename String_T,
	size_t storage_size_>
	int CoapPacketStatic<OsModel_P, Radio_P, String_T, storage_size_>::set_data( block_data_t* data , size_t length)
	{
		// we put data at the very end
		block_data_t *payload = storage_ + ( (storage_size_) - length );
		if( payload < end_of_options_  )
			return ERR_NOMEM;
		data_length_ = length;
		payload_ = payload;
		memmove(payload_, data, data_length_ );
		return SUCCESS;
	}

	template<typename OsModel_P,
	typename Radio_P,
	typename String_T,
	size_t storage_size_>
	typename Radio_P::block_data_t * CoapPacketStatic<OsModel_P, Radio_P, String_T, storage_size_>::data()
	{
		return payload_;
	}

	template<typename OsModel_P,
	typename Radio_P,
	typename String_T,
	size_t storage_size_>
	const typename Radio_P::block_data_t * CoapPacketStatic<OsModel_P, Radio_P, String_T, storage_size_>::data() const
	{
		return payload_;
	}

	template<typename OsModel_P,
	typename Radio_P,
	typename String_T,
	size_t storage_size_>
	size_t CoapPacketStatic<OsModel_P, Radio_P, String_T, storage_size_>::data_length() const
	{
		return data_length_;
	}

	template<typename OsModel_P,
	typename Radio_P,
	typename String_T,
	size_t storage_size_>
	uint32_t CoapPacketStatic<OsModel_P, Radio_P, String_T, storage_size_>::what_options_are_set() const
	{
		uint32_t result = 0;
		for( size_t i = 0; i < COAP_OPTION_ARRAY_SIZE; ++i )
		{
			if( options_[i] != NULL )
				result |= 1 << i;
		}
		return result;
	}

	template<typename OsModel_P,
	typename Radio_P,
	typename String_T,
	size_t storage_size_>
	void CoapPacketStatic<OsModel_P, Radio_P, String_T, storage_size_>::token( OpaqueData &token )
	{
		if( get_option( COAP_OPT_TOKEN, token ) != SUCCESS )
		{
			token = OpaqueData();
		}
	}

	template<typename OsModel_P,
	typename Radio_P,
	typename String_T,
	size_t storage_size_>
	void CoapPacketStatic<OsModel_P, Radio_P, String_T, storage_size_>::set_token( const OpaqueData &token )
	{
		remove_option( COAP_OPT_TOKEN );
		if( token != OpaqueData() )
		{
			add_option( COAP_OPT_TOKEN, token );
		}
	}

	template<typename OsModel_P,
	typename Radio_P,
	typename String_T,
	size_t storage_size_>
	String_T CoapPacketStatic<OsModel_P, Radio_P, String_T, storage_size_>::uri_path()
	{
		string_t path = string_t();
		get_option( COAP_OPT_URI_PATH, path );
		return path;
	}

	template<typename OsModel_P,
	typename Radio_P,
	typename String_T,
	size_t storage_size_>
	void CoapPacketStatic<OsModel_P, Radio_P, String_T, storage_size_>::set_content_type(CoapContentType ctype)
	{
		remove_option( COAP_OPT_CONTENT_TYPE );
		if ( ctype != COAP_CONTENT_TYPE_NONE )
			add_option( COAP_OPT_CONTENT_TYPE, ctype );
	}

	template<typename OsModel_P,
	typename Radio_P,
	typename String_T,
	size_t storage_size_>
	CoapContentType CoapPacketStatic<OsModel_P, Radio_P, String_T, storage_size_>::content_type()
	{
		CoapContentType ctype;
		int status = get_option( COAP_OPT_CONTENT_TYPE, &ctype );

		if ( status == ERR_OPT_NOT_SET )
		{
			return COAP_CONTENT_TYPE_NONE;
		}
		else
		{
			return ctype;
		}
	}

	template<typename OsModel_P,
	typename Radio_P,
	typename String_T,
	size_t storage_size_>
	int CoapPacketStatic<OsModel_P, Radio_P, String_T, storage_size_>::set_uri_path( const string_t &path )
	{
		return set_option( COAP_OPT_URI_PATH, path );
	}

	template<typename OsModel_P,
	typename Radio_P,
	typename String_T,
	size_t storage_size_>
	int CoapPacketStatic<OsModel_P, Radio_P, String_T, storage_size_>::set_uri_query( const string_t &query )
	{
		return set_option( COAP_OPT_URI_QUERY, query );
	}

	template<typename OsModel_P,
	typename Radio_P,
	typename String_T,
	size_t storage_size_>
	template<typename list_t>
	int CoapPacketStatic<OsModel_P, Radio_P, String_T, storage_size_>
	::get_query_list( CoapOptionNum optnum, list_t &result )
	{
		if ( optnum != COAP_OPT_LOCATION_QUERY
		     && optnum != COAP_OPT_URI_QUERY )
			return ERR_METHOD_NOT_APPLICABLE;

		if( options_[optnum] == NULL )
			return ERR_OPT_NOT_SET;

		result.clear();
		string_t curr_segment;
		block_data_t swap;
		block_data_t *pos = options_[optnum];
		block_data_t *nextpos;
		size_t value_start;
		size_t curr_segment_len;
		do
		{
			curr_segment_len = optlen( pos );

			if( curr_segment_len < 15 )
				value_start = 1;
			else
				value_start = 2;

			nextpos = pos + curr_segment_len + value_start;
			swap = *(nextpos);
			*nextpos = (block_data_t) '\0';
			curr_segment = (char*) ( pos + value_start );
			result.push_back( curr_segment );
			*nextpos = swap;

			pos = nextpos;
		} while ( pos < end_of_options_ && ( swap & 0xf0 ) == 0 );

		return SUCCESS;
	}

	template<typename OsModel_P,
	typename Radio_P,
	typename String_T,
	size_t storage_size_>
	void CoapPacketStatic<OsModel_P, Radio_P, String_T, storage_size_>::set_uri_port( uint32_t port )
	{
		remove_option( COAP_OPT_URI_PORT );
		if( port != COAP_STD_PORT )
		{
			add_option( COAP_OPT_URI_PORT, port );
		}
	}

	template<typename OsModel_P,
	typename Radio_P,
	typename String_T,
	size_t storage_size_>
	uint32_t CoapPacketStatic<OsModel_P, Radio_P, String_T, storage_size_>::uri_port()
	{
		uint32_t port = COAP_STD_PORT;
		get_option( COAP_OPT_URI_PORT, port );
		return port;
	}

	template<typename OsModel_P,
	typename Radio_P,
	typename String_T,
	size_t storage_size_>
	int CoapPacketStatic<OsModel_P, Radio_P, String_T, storage_size_>::set_option( CoapOptionNum option_number, uint32_t value )
	{
		remove_option(option_number);
		return add_option( option_number, value );
	}

	template<typename OsModel_P,
	typename Radio_P,
	typename String_T,
	size_t storage_size_>
	int CoapPacketStatic<OsModel_P, Radio_P, String_T, storage_size_>::set_option( CoapOptionNum option_number, const string_t &value )
	{
		remove_option(option_number);
		return add_option( option_number, value );
	}

	template<typename OsModel_P,
	typename Radio_P,
	typename String_T,
	size_t storage_size_>
	int CoapPacketStatic<OsModel_P, Radio_P, String_T, storage_size_>::set_option( CoapOptionNum option_number, const OpaqueData &value )
	{
		remove_option(option_number);
		return add_option( option_number, value );
	}

	template<typename OsModel_P,
		typename Radio_P,
		typename String_T,
		size_t storage_size_>
	int CoapPacketStatic<OsModel_P, Radio_P, String_T, storage_size_>::add_option( CoapOptionNum option_number, uint32_t value )
	{
		if( option_number > COAP_LARGEST_OPTION_NUMBER )
		{
			return ERR_UNKNOWN_OPT;
		}
		if( COAP_OPTION_FORMAT[option_number] != COAP_FORMAT_UINT )
		{
			return ERR_WRONG_TYPE;
		}
		block_data_t serial[4];
		size_t highest_non_zero_byte = 5;
		serial[ 3 ] = value & 0x000000ff;
		serial[ 2 ] = (value & 0x0000ff00) >> 8;
		serial[ 1 ] = (value & 0x00ff0000) >> 16;
		serial[ 0 ] = (value & 0xff000000) >> 24;
		if( serial[0] != 0 )
			highest_non_zero_byte = 1;
		else if ( serial[1] != 0 )
			highest_non_zero_byte = 2;
		else if ( serial[2] != 0 )
			highest_non_zero_byte = 3;
		else if ( serial[3] != 0 )
			highest_non_zero_byte = 4;

		return add_option( option_number,
		                   serial + highest_non_zero_byte - 1,
		                   5 - highest_non_zero_byte );
	}

	template<typename OsModel_P,
		typename Radio_P,
		typename String_T,
		size_t storage_size_>
	int CoapPacketStatic<OsModel_P, Radio_P, String_T, storage_size_>::add_option( CoapOptionNum option_number, const string_t &value )
	{
		if( option_number > COAP_LARGEST_OPTION_NUMBER )
		{
			return ERR_UNKNOWN_OPT;
		}
		if( COAP_OPTION_FORMAT[option_number] != COAP_FORMAT_STRING )
		{
			return ERR_WRONG_TYPE;
		}
		if( value.length() == 0 )
		{
			return SUCCESS;
		}

		// reserve some additional space for option headers
		block_data_t insert[ value.length() + 20 ];
		size_t num_segments = 0;
		size_t serial_len = 0;
		size_t segment_start = 0;

		const char* c_str = (const_cast<string_t&>(value)).c_str();

		if( option_number == COAP_OPT_LOCATION_PATH
		   || option_number  == COAP_OPT_URI_PATH )
		{
			if( (size_t) value.length() > segment_start && c_str[segment_start] == '/' )
				++segment_start;
			if( (size_t) value.length() > segment_start )
			{
				serial_len = make_segments_from_string(c_str + segment_start,
				          '/', option_number, insert, num_segments );
			}
			else
				return SUCCESS;
		}
		else if ( option_number == COAP_OPT_LOCATION_QUERY
		         || option_number  == COAP_OPT_URI_QUERY )
		{
			if( (size_t) value.length() > segment_start && c_str[segment_start] == '/' )
				++segment_start;
			if( (size_t) value.length() > segment_start && c_str[segment_start] == '?' )
				++segment_start;
			if( (size_t) value.length() > segment_start )
			{
				serial_len = make_segments_from_string( c_str + segment_start,
				             '&', option_number, insert, num_segments );
			}
			else
				return SUCCESS;
		}
		else
		{
			if( value.length() <= COAP_STRING_OPTS_MAXLEN )
			{
				memcpy(insert, c_str, value.length());
				serial_len = value.length();
				num_segments = 0;
			}
			else
			{
				size_t copylen = 0;
				do
				{
					*(insert + serial_len) = COAP_LONG_OPTION;
					++serial_len;
					*(insert + serial_len) = 0xff;
					++serial_len;
					copylen = value.length() - num_segments * 270;
					if( copylen > 270 )
						copylen = 270;
					memcpy( insert + serial_len,
					        c_str + segment_start, copylen);
					serial_len += copylen;
					++num_segments;
					segment_start += copylen;
				} while( segment_start < (size_t) value.length() );
			}
		}

		return add_option( option_number, insert, serial_len, num_segments );
	}

	template<typename OsModel_P,
		typename Radio_P,
		typename String_T,
		size_t storage_size_>
	int CoapPacketStatic<OsModel_P, Radio_P, String_T, storage_size_>::add_option( CoapOptionNum option_number, const OpaqueData &value)
	{
		if( option_number > COAP_LARGEST_OPTION_NUMBER )
		{
			return ERR_UNKNOWN_OPT;
		}
		if( COAP_OPTION_FORMAT[option_number] != COAP_FORMAT_OPAQUE )
		{
			return ERR_WRONG_TYPE;
		}

		return add_option( option_number, value.value(), value.length() );
	}

	template<typename OsModel_P,
	typename Radio_P,
	typename String_T,
	size_t storage_size_>
	int CoapPacketStatic<OsModel_P, Radio_P, String_T, storage_size_>::get_option( CoapOptionNum option_number, uint32_t &value )
	{
		if( options_[option_number] != NULL )
		{
			value = 0;
			block_data_t *raw = options_[option_number];
			size_t len = *raw & 0xf;
			size_t pos = 1;

			switch( len )
			{
			case 4:
				value = *(raw + pos);
				++pos;
			case 3:
				value = (value << 8) | *(raw + pos);
				++pos;
			case 2:
				value = (value << 8) | *(raw + pos);
				++pos;
			case 1:
				value = (value << 8) | *(raw + pos);
				++pos;
			case 0:
				// do nothing
				break;
			default:
				// uint option longer than 4. SHOULD NOT HAPPEN!!!
				return ERR_OPT_TOO_LONG;
			}
			return SUCCESS;
		}
		return ERR_OPT_NOT_SET;
	}

	template<typename OsModel_P,
	typename Radio_P,
	typename String_T,
	size_t storage_size_>
	int CoapPacketStatic<OsModel_P, Radio_P, String_T, storage_size_>::get_option( CoapOptionNum option_number, string_t &value )
	{
		if( options_[option_number] == NULL )
			return ERR_OPT_NOT_SET;

		if( option_number == COAP_OPT_LOCATION_PATH
				|| option_number  == COAP_OPT_URI_PATH )
		{
			make_string_from_segments ( '/', option_number, value );
		}
		else if ( option_number == COAP_OPT_LOCATION_QUERY
				|| option_number  == COAP_OPT_URI_QUERY )
		{
			make_string_from_segments ( '&', option_number, value );
		}
		else
		{
			// TODO: testen ob das funtioniert
			make_string_from_segments ( '\0', option_number, value );
		}
		return SUCCESS;
	}

	template<typename OsModel_P,
	typename Radio_P,
	typename String_T,
	size_t storage_size_>
	int CoapPacketStatic<OsModel_P, Radio_P, String_T, storage_size_>::get_option( CoapOptionNum option_number, OpaqueData &value )
	{
		if( options_[option_number] == NULL )
			return ERR_OPT_NOT_SET;

		size_t len = optlen( options_[option_number] );
		size_t value_start = (len >= 15) ? 2 : 1;

		value.set( options_[option_number] + value_start, len );

		return SUCCESS;
	}

	template<typename OsModel_P,
	typename Radio_P,
	typename String_T,
	size_t storage_size_>
	template<typename list_t>
	int CoapPacketStatic<OsModel_P, Radio_P, String_T, storage_size_>::get_options( CoapOptionNum option_number, list_t &values )
	{
		if( option_number > COAP_LARGEST_OPTION_NUMBER )
			return ERR_UNKNOWN_OPT;
		if( options_[option_number] == NULL )
			return ERR_OPT_NOT_SET;

		values.clear();

		block_data_t *pos = options_[option_number];

		// FIXME the following code never worked, this needs to be done with template specialization.
		// But how to specialize on the list_t? wiselib::list_static takes 3 params: Os, Value_type and SIZE
		// Os is given and Value_Type is used for specialization but SIZE varies...
		/*
		if (COAP_OPTION_FORMAT[option_number] == COAP_FORMAT_STRING)
		{
			string_t curr_segment;
			block_data_t swap;
			block_data_t *nextpos;
			size_t value_start;
			size_t curr_segment_len;
			do
			{
				curr_segment_len = optlen(pos);

				if (curr_segment_len < 15)
					value_start = 1;
				else
					value_start = 2;

				nextpos = pos + curr_segment_len + value_start;
				swap = *(nextpos);
				*nextpos = (block_data_t) '\0';
				curr_segment = (char*) (pos + value_start);
				values.push_back(curr_segment);
				*nextpos = swap;

				pos = nextpos;
			} while (pos < end_of_options_ && (swap & 0xf0) == 0);
		}
		else if (COAP_OPTION_FORMAT[option_number] == COAP_FORMAT_UINT)
		{
			uint32_t curr_value;
			size_t len;
			size_t offset;
			do
			{
				curr_value = 0;
				len = pos & 0x0f;
				offset = 1;

				switch (len)
				{
				case 4:
					curr_value = *(pos + offset);
					++offset;
				case 3:
					curr_value = (curr_value << 8) | *(pos + offset);
					++offset;
				case 2:
					curr_value = (curr_value << 8) | *(pos + offset);
					++offset;
				case 1:
					curr_value = (curr_value << 8) | *(pos + offset);
					++offset;
				case 0:
					// do nothing
					break;
				default:
					// uint option longer than 4. SHOULD NOT HAPPEN!!!
					return ERR_OPT_TOO_LONG;
				}
				values.push_back(curr_value);
				pos += offset;
			} while (pos < end_of_options_ && (*pos & 0xf0) == 0);
		}
		*/
		if (COAP_OPTION_FORMAT[option_number] == COAP_FORMAT_OPAQUE)
		{
			OpaqueData curr_value;
			do
			{
				size_t len = optlen(pos);
				size_t value_start = (len >= 15) ? 2 : 1;

				curr_value.set(pos + value_start, len);
				values.push_back(curr_value);
				pos += value_start + len;
			} while (pos < end_of_options_ && (*pos & 0xf0) == 0);
		}
		else
		{
			return ERR_METHOD_NOT_APPLICABLE;
		}

		return SUCCESS;
	}

	template<typename OsModel_P,
	typename Radio_P,
	typename String_T,
	size_t storage_size_>
	int CoapPacketStatic<OsModel_P, Radio_P, String_T, storage_size_>::remove_option( CoapOptionNum option_number )
	{
		block_data_t *removal_start = options_[option_number];
		size_t removal_len = 0;
		size_t num_segments = 0;
		size_t curr_segment_len;
		uint8_t max_delta = COAP_MAX_DELTA_DEFAULT;
		if( option_count_ >= COAP_UNLIMITED_OPTIONS )
		{
			max_delta = COAP_MAX_DELTA_UNLIMITED;
		}

		if( removal_start != NULL )
		{
			uint8_t prev = (option_number - ( ((*removal_start) & 0xf0 ) >> 4));
			uint8_t next = 0;
			do
			{
				++num_segments;
				curr_segment_len = *(removal_start + removal_len) & 0x0f;
				if( curr_segment_len == COAP_LONG_OPTION )
				{
					curr_segment_len += *(removal_start + 1) + 2;
				}
				else
				{
					++curr_segment_len;
				}

				removal_len += curr_segment_len;
			} while( (removal_start + removal_len) < end_of_options_
			         && ( *(removal_start + removal_len) & 0xf0 ) == 0 );

			// if the removed option is the last option and the one
			// before is a fencepost, remove the fencepost
			if( (removal_start + removal_len) >= end_of_options_ )
			{
				if(is_fencepost( prev ))
				{
					removal_start = options_[ prev ];
					removal_len = (size_t) (end_of_options_ - removal_start);
					options_[ prev ] = NULL;
					++num_segments;
				}
			}
			else
			{
				// insert a new fencepost if necessary
				next = option_number + ( ( (*(removal_start + removal_len)) & 0xf0 ) >> 4);
				if( ( next - prev ) > max_delta
				    && !is_end_of_opts_marker(removal_start + removal_len))
				{
					uint8_t fencepost_delta = next_fencepost_delta( option_number
							- ( ( (*removal_start) & 0xf0 ) >> 4) );
					*removal_start = fencepost_delta << 4;
					++removal_start;
					--removal_len;
					--num_segments;
					prev += fencepost_delta;
				}

				// move following options
				memmove( removal_start,
				         removal_start + removal_len,
				         size_t (end_of_options_ - (removal_start + removal_len) ) );
				*removal_start = (*removal_start & 0x0f) | ( ( next - prev ) << 4 );
			}

			options_[ option_number ] = NULL;

			end_of_options_ -= removal_len;
			option_count_ -= num_segments;

			scan_opts( removal_start, prev );

			// remove End of Options marker if there are fewer than 15 options
			// now, OR if there are exactly 15 and one of them is a fencepost
			// inserted because a delta of exactly 15 would have to be used
			// otherwise
			if( ( option_count_ + num_segments >= COAP_UNLIMITED_OPTIONS
			    && option_count_ < COAP_UNLIMITED_OPTIONS )
			    || ( option_count_ == COAP_UNLIMITED_OPTIONS
			       && options_[COAP_OPT_FENCEPOST] != NULL
			       && ( *(options_[COAP_OPT_FENCEPOST]) & 0xf0 )
			          + ( *(options_[COAP_OPT_FENCEPOST] + 1) & 0xf0 )
			          == COAP_END_OF_OPTIONS_MARKER ) )
			{
				remove_end_of_opts_marker();
			}
		}
		return SUCCESS;
	}

	template<typename OsModel_P,
	typename Radio_P,
	typename String_T,
	size_t storage_size_>
	bool CoapPacketStatic<OsModel_P, Radio_P, String_T, storage_size_>::opt_if_none_match() const
	{
		return options_[COAP_OPT_IF_NONE_MATCH] != NULL;
	}

	template<typename OsModel_P,
	typename Radio_P,
	typename String_T,
	size_t storage_size_>
	int CoapPacketStatic<OsModel_P, Radio_P, String_T, storage_size_>::set_opt_if_none_match( bool opt_if_none_match )
	{
		if( opt_if_none_match )
		{
			return add_option(COAP_OPT_IF_NONE_MATCH , NULL, 0 );
		}
		else
		{
			return remove_option( COAP_OPT_IF_NONE_MATCH );
		}
	}

	template<typename OsModel_P,
	typename Radio_P,
	typename String_T,
	size_t storage_size_>
	size_t CoapPacketStatic<OsModel_P, Radio_P, String_T, storage_size_>
	::option_count() const
	{
//		size_t count = 0;
//		block_data_t * position = (block_data_t*) storage_;
//		size_t len = 0;
//		while( position < end_of_options_ )
//		{
//			len = (*position) & 0x0f;
//			if( len == 15 )
//			{
//				len = 16 + ( *(position + 1) );
//			}
//			position += len + 1;
//			++count;
//		}
//		return count;
		return option_count_;
	}

	template<typename OsModel_P,
	typename Radio_P,
	typename String_T,
	size_t storage_size_>
	size_t CoapPacketStatic<OsModel_P, Radio_P, String_T, storage_size_>::serialize_length() const
	{
		// header (4 bytes) + options + payload
		return (size_t) ( 4 +
		         ( end_of_options_ - storage_ ) + data_length_ );
	}

	template<typename OsModel_P,
	typename Radio_P,
	typename String_T,
	size_t storage_size_>
	size_t CoapPacketStatic<OsModel_P, Radio_P, String_T, storage_size_>
	::serialize( block_data_t *datastream ) const
	{
		if( option_count_ >= COAP_UNLIMITED_OPTIONS )
		{
			datastream[0] = ((version() & 0x03) << 6) | ((type() & 0x03) << 4) | COAP_UNLIMITED_OPTIONS;
		}
		else
		{
			datastream[0] = ((version() & 0x03) << 6) | ((type() & 0x03) << 4) | (( option_count()) & 0x0f);
		}
		datastream[1] = code();
		datastream[2] = (this->msg_id() & 0xff00) >> 8;
		datastream[3] = (this->msg_id() & 0x00ff);

		size_t len = 4;
		memcpy( datastream + len, storage_, (size_t) (end_of_options_ - storage_) );
		len += (size_t) (end_of_options_ - storage_);
		memcpy( datastream + len,
		        payload_, data_length_ );
		len += data_length_;

		return len;
	}

	template<typename OsModel_P,
	typename Radio_P,
	typename String_T,
	size_t storage_size_>
	void CoapPacketStatic<OsModel_P, Radio_P, String_T, storage_size_>::get_error_context( CoapCode &error_code, CoapOptionNum &error_option)
	{
		error_code = (CoapCode) error_code_;
		error_option = (CoapOptionNum) error_option_;
	}

//-----------------------------------------------------------------------------
// Private methods start here

	template<typename OsModel_P,
	typename Radio_P,
	typename String_T,
	size_t storage_size_>
	int CoapPacketStatic<OsModel_P, Radio_P, String_T, storage_size_>::add_option(CoapOptionNum num, const block_data_t *serial_opt, size_t len, size_t num_of_opts)
	{
		if( options_[num] != NULL && !COAP_OPT_CAN_OCCUR_MULTIPLE[num] )
			return ERR_MULTIPLE_OCCURENCES_OF_OPTION;
		CoapOptionNum prev = (CoapOptionNum) 0;
		CoapOptionNum next = (CoapOptionNum) 0;
		uint8_t fencepost = 0;
		uint8_t max_delta = COAP_MAX_DELTA_DEFAULT;
		if( option_count_ + num_of_opts >= COAP_UNLIMITED_OPTIONS
		    || ( num_of_opts == SINGLE_OPTION_NO_HEADER
		       && option_count_ + 1 >= COAP_UNLIMITED_OPTIONS ) )
		{
			max_delta = COAP_MAX_DELTA_UNLIMITED;
		}
		// only add header when a single option is added
		size_t overhead_len = 0;
		if( num_of_opts == SINGLE_OPTION_NO_HEADER )
		{
			overhead_len = 1;
			if( len >= COAP_LONG_OPTION )
				++overhead_len;
		}

		size_t fenceposts_omitted_len = 0;

		block_data_t *put_here = end_of_options_;
		block_data_t *next_opt_start = put_here;
		// there are options set
		if( put_here > storage_ )
		{

			// look for the next bigger option - this is where we need to start
			// moving things further back
			for( size_t i = (size_t) num + 1; i < COAP_OPTION_ARRAY_SIZE; ++i )
			{
				if( options_[i] != NULL )
				{
					next = (CoapOptionNum) i;
					put_here = options_[i];

					if( is_fencepost( next ) )
					{
						// if the delta to the option after the fencepost is
						// small enough, we can ommit the fencepost
						CoapOptionNum nextnext = (CoapOptionNum) ( next +
						        // get the next option header
								// considering the fenceposts length is only a
								// precaution, it should be zero
						        ( ( *( options_[next]
						        + ( *(options_[next]) & 0x0f ) + 1 )
						        // bitwise AND and shift to get the delta
						        & 0xf0) >> 4 ));

						if( nextnext - num <= max_delta )
						{
							options_[next] = NULL;
							next = nextnext;
						}
					}
					next_opt_start = options_[next];

					break;
				}
			}

			// look for previous option - can be the same option we're inserting
			if( next != 0 )
			{
				prev = (CoapOptionNum) ( next - (CoapOptionNum) ( ( *(options_[next]) & 0xf0 ) >> 4 ));
			}
			else
			{
				for( size_t i = (size_t) num; i > 0; --i )
				{
					if( options_[i] != NULL )
					{
						prev = (CoapOptionNum) i;
						break;
					}
				}
			}

			if( is_fencepost( prev ) )
			{
				// if the delta to the option before the fencepost is
				// small enough, we can ommit the fencepost
				CoapOptionNum prevprev = (CoapOptionNum) ( prev -
						( ( *( options_[prev] ) && 0xf0) >> 4 ) );
				if( num - prevprev <= max_delta )
				{
					put_here = options_[prev];
					options_[prev] = NULL;
					prev = prevprev;
				}
			}

			fenceposts_omitted_len = next_opt_start - put_here;
		}

		if( num - prev > max_delta )
		{
			++overhead_len;
			fencepost = next_fencepost_delta( prev );
		}

		// move following options back
		size_t bytes_needed = len + overhead_len - fenceposts_omitted_len;
		if( end_of_options_ + bytes_needed >= payload_ )
			return ERR_NOMEM;
		if( put_here < end_of_options_ )
		{
			// correcting delta of following option
			*next_opt_start = ( *next_opt_start & 0x0f )
			                  | (block_data_t) ((next - num) << 4);

			memmove( next_opt_start + bytes_needed,
			        next_opt_start,
			        (size_t) (end_of_options_ - next_opt_start));
		}

		memcpy( put_here + (bytes_needed - len), serial_opt, len );
		end_of_options_ += bytes_needed;
		if( fencepost != 0)
		{
			*put_here = fencepost << 4;
			prev = (CoapOptionNum) fencepost;
			options_[fencepost] = put_here;
			++put_here;
			++option_count_;
		}
		// if multiple options are inserted only add delta, otherwise add size too
		*put_here = ( *put_here & 0x0f ) | ((num - prev) << 4);
		if( num_of_opts == SINGLE_OPTION_NO_HEADER )
		{
			if( len < COAP_LONG_OPTION )
			{
				*put_here = ( *put_here & 0xf0 ) | (len & 0x0f);
			}
			else
			{
				*put_here = ( *put_here & 0xf0 ) | 0x0f;
				*(put_here + 1) = (len - COAP_LONG_OPTION);
			}
		}

		// if we just appended we don't want to overwrite the option's pointer
		if( prev == num )
		{
			put_here += len;
		}

		scan_opts( put_here, prev );

		if( num_of_opts == SINGLE_OPTION_NO_HEADER )
			++option_count_;
		else
			option_count_ += num_of_opts;


		// remove all deltas equal to COAP_END_OF_OPTIONS_MARKER if
		// inserting the options resulted in crossing the
		//  COAP_UNLIMITED_OPTIONS "border"
		if( option_count_ >= COAP_UNLIMITED_OPTIONS
		    && ( option_count_ - num_of_opts < COAP_UNLIMITED_OPTIONS
		         || ( num_of_opts == SINGLE_OPTION_NO_HEADER
		              && ( option_count_ - 1 ) < COAP_UNLIMITED_OPTIONS ) ) )
		{
			int status = add_end_of_opts_marker();
			if( status != SUCCESS )
				return status;
		}

		return SUCCESS;
	}

	template<typename OsModel_P,
	typename Radio_P,
	typename String_T,
	size_t storage_size_>
	int CoapPacketStatic<OsModel_P, Radio_P, String_T, storage_size_>::add_end_of_opts_marker()
	{
		uint8_t prev = 0;
		block_data_t *move_back_start;
		for( size_t i = 0; i < COAP_OPTION_ARRAY_SIZE; ++i)
		{
			if( options_[i] != NULL )
			{
				if( ( i - prev ) == COAP_MAX_DELTA_DEFAULT )
				{
					if( end_of_options_ + 1 > payload_ )
						return ERR_NOMEM;
					move_back_start = options_[i];
					memmove( ( move_back_start + 1 ),
					         move_back_start,
					         (size_t) ( end_of_options_ - move_back_start ) );
					options_[i] = move_back_start + 1;
					*move_back_start = next_fencepost_delta( prev ) << 4;
					uint8_t fencepost = ( prev + next_fencepost_delta( prev ) );
					*(options_[i]) = (*(options_[i]) & 0x0f) | ( ( i - fencepost ) << 4 );
					options_[fencepost] = move_back_start;
					++end_of_options_;
					++option_count_;
				}
				prev = i;
			}
		}
		if( end_of_options_ + 1 > payload_ )
			return ERR_NOMEM;
		*end_of_options_ = COAP_END_OF_OPTIONS_MARKER;
		++end_of_options_;

		scan_opts( storage_, 0 );

		return SUCCESS;
	}

	template<typename OsModel_P,
	typename Radio_P,
	typename String_T,
	size_t storage_size_>
	void CoapPacketStatic<OsModel_P, Radio_P, String_T, storage_size_>::remove_end_of_opts_marker()
	{
		// look for fenceposts that were inserted because the delta was
		// equal to COAP_END_OF_OPTIONS_MARKER and remove them, as they are
		// no longer necessary
		uint8_t prev;
		uint8_t next;
		for( size_t i = COAP_OPT_FENCEPOST;
		     i < COAP_LARGEST_OPTION_NUMBER;
		     i += COAP_OPT_FENCEPOST )
		{
			if( options_[i] != NULL )
			{
				prev = i - ( ( *options_[i] & 0xf0 ) >> 4 );
				next = i + ( ( *(options_[i] + 1) & 0xf0 ) >> 4 );
				if( next - prev == COAP_MAX_DELTA_DEFAULT )
				{
					memmove( options_[i],
					         (options_[i] + 1),
					         (size_t) ( end_of_options_ - (options_[i] + 1) ) );
					options_[next] = options_[i];
					options_[i] = NULL;
					*options_[next] = ((*options_[next]) & 0x0f)
					                  | COAP_END_OF_OPTIONS_MARKER;
					--option_count_;
					--end_of_options_;
				}
			}
		}

		scan_opts( storage_, 0 );

		// remove COAP_END_OF_OPTIONS_MARKER
		--end_of_options_;
	}

	template<typename OsModel_P,
	typename Radio_P,
	typename String_T,
	size_t storage_size_>
	bool CoapPacketStatic<OsModel_P, Radio_P, String_T, storage_size_>::is_end_of_opts_marker(block_data_t *opthead)
	{
		if( option_count_ >= COAP_UNLIMITED_OPTIONS && *opthead == COAP_END_OF_OPTIONS_MARKER )
			return true;
		return false;
	}

	template<typename OsModel_P,
	typename Radio_P,
	typename String_T,
	size_t storage_size_>
	void CoapPacketStatic<OsModel_P, Radio_P, String_T, storage_size_>::scan_opts( block_data_t *start, uint8_t prev)
	{
		uint8_t delta = 0;
		while( start < end_of_options_ )
		{
			if( is_end_of_opts_marker( start ) )
				break;
			if( (delta = (((*start) & 0xf0) >> 4)) != 0 )
			{
				if( prev + delta > COAP_LARGEST_OPTION_NUMBER )
					break;
				options_[ prev + delta ] = start;
				prev = prev + delta;
			}
			size_t len = *start & 0x0f;
			if( len == COAP_LONG_OPTION )
			{
				len += *(start + 1);
				++len;
			}
			start += len + 1;
		}
	}

	template<typename OsModel_P,
	typename Radio_P,
	typename String_T,
	size_t storage_size_>
	int CoapPacketStatic<OsModel_P, Radio_P, String_T, storage_size_>::initial_scan_opts( size_t num_of_opts, size_t message_length )
	{
		block_data_t *curr_position = storage_;
		option_count_ = 0;
		uint8_t current = 0;
		uint8_t previous = 0;
		size_t opt_length;
		while( ( option_count_ < num_of_opts  || num_of_opts == COAP_UNLIMITED_OPTIONS ) )
		{
			// end of options
			if( is_end_of_opts_marker( curr_position ) )
				break;

			current = previous + ( ( *curr_position & 0xf0) >> 4);

			// length of option plus header
			opt_length = *curr_position & 0x0f;
			if( opt_length == COAP_LONG_OPTION )
				opt_length = *(curr_position + 1) + 17;
			else
				++opt_length;

			if( current == previous
			    && !COAP_OPT_CAN_OCCUR_MULTIPLE[current] )
			{
				// option is critical
				if ( is_critical( current ) )
				{
					error_code_ = COAP_CODE_BAD_OPTION;
					error_option_ = current;
					return ERR_MULTIPLE_OCCURENCES_OF_CRITICAL_OPTION;
				}
				// otherwise ignore
				++option_count_;
				previous = current;
				curr_position += opt_length;
				continue;
			}

			// check for unknown options
			if ( ( current > COAP_LARGEST_OPTION_NUMBER )
			     || ( COAP_OPTION_FORMAT[current] == COAP_FORMAT_UNKNOWN ) )
			{
				// option is critical
				if ( is_critical( current ) )
				{
					error_code_ = COAP_CODE_BAD_OPTION;
					error_option_ = current;
					return ERR_UNKNOWN_CRITICAL_OPTION;
				}
				// otherwise ignore
				++option_count_;
				previous = current;
				curr_position += opt_length;
				continue;
			}

			if( COAP_OPTION_FORMAT[current] == COAP_FORMAT_STRING
			    && opt_length == 0 )
			{
				if ( is_critical( current ) )
				{
					error_code_ = COAP_CODE_BAD_OPTION;
					error_option_ = current;
					return ERR_EMPTY_STRING_OPTION;
				}
				// otherwise ignore
				++option_count_;
				previous = current;
				curr_position += opt_length;
				continue;
			}

			if( current != previous )
			{
				options_[ current ] = curr_position;
			}

			++option_count_;
			previous = current;
			curr_position += opt_length;

			if( (curr_position >= (storage_ + storage_size_)
			    || curr_position >= ( storage_ + message_length ))
			    && ( option_count_ < num_of_opts ) )
			{
				error_code_ = COAP_CODE_BAD_REQUEST;
				error_option_ = current;
				return ERR_OPTIONS_EXCEED_PACKET_LENGTH;
			}
		}

		end_of_options_ = curr_position;

		// Rest is data
		if( curr_position < ( storage_ + message_length ) )
		{
			int status = set_data(curr_position,
					message_length - ( curr_position - storage_ ) );
			if( status != SUCCESS )
				return status;
		}

		return SUCCESS;
	}

	template<typename OsModel_P,
	typename Radio_P,
	typename String_T,
	size_t storage_size_>
	uint8_t CoapPacketStatic<OsModel_P, Radio_P, String_T, storage_size_>::next_fencepost_delta(uint8_t previous_opt_number) const
	{
		return ( COAP_OPT_FENCEPOST - ( (previous_opt_number) % COAP_OPT_FENCEPOST ) );
	}

	template<typename OsModel_P,
	typename Radio_P,
	typename String_T,
	size_t storage_size_>
	bool CoapPacketStatic<OsModel_P, Radio_P, String_T, storage_size_>::is_fencepost(uint8_t optnum) const
	{
		return (optnum > 0 && optnum % 14 == 0);
	}

	template<typename OsModel_P,
	typename Radio_P,
	typename String_T,
	size_t storage_size_>
	bool CoapPacketStatic<OsModel_P, Radio_P, String_T, storage_size_>::is_critical( uint8_t option_number ) const
	{
		// odd option numbers are critical
		return( option_number & 0x01 );
	}

	template<typename OsModel_P,
	typename Radio_P,
	typename String_T,
	size_t storage_size_>
	size_t CoapPacketStatic<OsModel_P, Radio_P, String_T, storage_size_>::optlen(block_data_t * optheader) const
	{
		size_t len = *optheader & 0x0f;
		if( len == COAP_LONG_OPTION )
		{
			len += *(optheader + 1);
		}
		return len;
	}

	template<typename OsModel_P,
	typename Radio_P,
	typename String_T,
	size_t storage_size_>
	size_t CoapPacketStatic<OsModel_P, Radio_P, String_T, storage_size_>
	::make_segments_from_string( const char *cstr, char delimiter, CoapOptionNum optnum, block_data_t *result, size_t &num_segments ) const
	{
		size_t result_pos = 0;
		size_t position = 0;
		size_t segment_start = 0;
		size_t length = 0;
		num_segments = 0;
		if(cstr[position] == '\0')
			return 0;
		for( ; position <= strlen(cstr); ++position )
		{
			if( cstr[position] == delimiter ||
			    ( cstr[position] == '\0' && cstr[position - 1] != delimiter ) )
			{
				if( (length = position - segment_start ) == 0)
					return 0;

				if( length >= COAP_LONG_OPTION && length <= COAP_STRING_OPTS_MAXLEN )
				{
					*(result + result_pos) = 0x0f;
					++result_pos;
					*(result + result_pos) = (block_data_t) length - COAP_LONG_OPTION;
					++result_pos;
				}
				else if ( length < COAP_LONG_OPTION )
				{
					*(result + result_pos) = (block_data_t) length;
					++result_pos;
				}
				else
					return 0;

				memcpy(result + result_pos, cstr + segment_start, length );
				result_pos += length;
				++num_segments;
				segment_start = position + 1;
			}
		}

		return result_pos;
	}

	template<typename OsModel_P,
	typename Radio_P,
	typename String_T,
	size_t storage_size_>
	void CoapPacketStatic<OsModel_P, Radio_P, String_T, storage_size_>
	::make_string_from_segments( char delimiter, CoapOptionNum optnum, string_t &result )
	{
		result = "";
		if( options_[optnum] != NULL )
		{
			char terminated_delimiter[] = { delimiter, '\0' };
			block_data_t swap;
			block_data_t *pos = options_[optnum];
			block_data_t *nextpos;
			size_t value_start;
			size_t curr_segment_len;
			do
			{
				curr_segment_len = optlen( pos );

				if( curr_segment_len < 15 )
					value_start = 1;
				else
					value_start = 2;

				nextpos = pos + curr_segment_len + value_start;
				swap = *(nextpos);
				*nextpos = (block_data_t) '\0';
				result.append( (char*) ( pos + value_start ) );
				*nextpos = swap;

				if( (nextpos < end_of_options_ ) && ( swap & 0xf0 ) == 0 )
				{
					result.append( terminated_delimiter );
				}
				pos = nextpos;
			} while ( pos < end_of_options_ && ( swap & 0xf0 ) == 0 );
		}

	}

}




#endif // COAP_PACKET_STATIC_H
