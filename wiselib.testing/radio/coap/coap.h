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

#ifndef COAP_H
#define COAP_H

#include "external_interface/external_interface.h"
#include "util/serialization/simple_types.h"
using namespace std;

// Config Tweaks

// maximum length of human readable errors
static const size_t COAP_ERROR_STRING_LEN = 200;

// size of the storage blob of a coap packet. contains options and payload
static const size_t COAP_DEFAULT_STORAGE_SIZE = 127;

// Size of message buffer that saves sent and received messages for a while
static const size_t COAPRADIO_SENT_LIST_SIZE = 10;
static const size_t COAPRADIO_RECEIVED_LIST_SIZE = 10;
static const size_t COAPRADIO_RESOURCES_SIZE = 5;

enum CoapMsgIds
{
	CoapMsgId = 51 // Coap Message Type according to Wiselibs Reserved Message IDs
};

// Constants defined in draft-ietf-core-coap-07
static const uint8_t COAP_VERSION = 1;
static const uint16_t COAP_STD_PORT = 5683;

static const float COAP_RANDOM_FACTOR = 1.5;
static const uint16_t COAP_RESPONSE_TIMEOUT = 2000;
static const uint16_t COAP_MAX_RESPONSE_TIMEOUT = (uint16_t) ( COAP_RESPONSE_TIMEOUT * COAP_RANDOM_FACTOR );
static const uint8_t COAP_MAX_RETRANSMIT = 4;
// Time before an ACK is sent. This is to give the application a chance to send a piggybacked response
static const uint16_t COAP_ACK_GRACE_PERIOD = COAP_RESPONSE_TIMEOUT / 4;

enum CoapType
{
	COAP_MSG_TYPE_CON = 0,
	COAP_MSG_TYPE_NON = 1,
	COAP_MSG_TYPE_ACK = 2,
	COAP_MSG_TYPE_RST = 3
};

static const uint8_t COAP_LONG_OPTION = 15;
static const uint8_t COAP_UNLIMITED_OPTIONS = 15;
static const uint8_t COAP_MAX_DELTA_UNLIMITED = 14;
static const uint8_t COAP_MAX_DELTA_DEFAULT = 15;
static const uint8_t COAP_END_OF_OPTIONS_MARKER = 0xf0;

enum CoapOptionNum
{
	COAP_OPT_NOOPT = 0,
	COAP_OPT_CONTENT_TYPE = 1,
	COAP_OPT_MAX_AGE = 2,
	COAP_OPT_PROXY_URI = 3,
	COAP_OPT_ETAG = 4,
	COAP_OPT_URI_HOST = 5,
	COAP_OPT_LOCATION_PATH = 6,
	COAP_OPT_URI_PORT = 7,
	COAP_OPT_LOCATION_QUERY = 8,
	COAP_OPT_URI_PATH = 9,
	COAP_OPT_TOKEN = 11,
	COAP_OPT_ACCEPT = 12,
	COAP_OPT_IF_MATCH = 13,
	COAP_OPT_FENCEPOST = 14,
	COAP_OPT_URI_QUERY = 15,
	COAP_OPT_IF_NONE_MATCH = 21
};

static const uint8_t COAP_OPT_MAXLEN_FENCEPOST = 0;
static const uint8_t COAP_OPT_MAXLEN_CONTENT_TYPE = 2;
static const uint8_t COAP_OPT_MAXLEN_MAX_AGE = 4;
static const uint8_t COAP_OPT_MAXLEN_ETAG = 8;
static const uint8_t COAP_OPT_MAXLEN_URI_PORT = 2;
static const uint8_t COAP_OPT_MAXLEN_TOKEN = 8;
static const uint8_t COAP_OPT_MAXLEN_ACCEPT = 2;
static const uint8_t COAP_OPT_MAXLEN_IF_MATCH = 8;
static const uint8_t COAP_OPT_MAXLEN_IF_NONE_MATCH = 0;
static const uint16_t COAP_STRING_OPTS_MAXLEN = 270;
static const uint16_t COAP_STRING_OPTS_MINLEN = 1;

static const uint8_t COAP_DEFAULT_MAX_AGE = 60;

// Finding the longest opaque option, out of the three opage options Etag, Token and IfMatch
static const uint8_t COAP_OPT_MAXLEN_OPAQUE = COAP_OPT_MAXLEN_TOKEN;

// message codes
//requests
static const uint8_t COAP_REQUEST_CODE_RANGE_MIN = 1;
static const uint8_t COAP_REQUEST_CODE_RANGE_MAX = 31;

static const uint8_t COAP_RESPONSE_CODE_RANGE_MIN = 64;
static const uint8_t COAP_RESPONSE_CODE_RANGE_MAX = 191;

enum CoapCode
{
	COAP_CODE_EMPTY = 0,
	COAP_CODE_GET = 1,
	COAP_CODE_POST = 2,
	COAP_CODE_PUT = 3,
	COAP_CODE_DELETE = 4,
	//responses
	COAP_CODE_CREATED = 65, // 2.01
	COAP_CODE_DELETED = 66, // 2.02
	COAP_CODE_VALID = 67, // 2.03
	COAP_CODE_CHANGED = 68, // 2.04
	COAP_CODE_CONTENT = 69, // 2.05
	COAP_CODE_BAD_REQUEST = 128, // 4.00
	COAP_CODE_UNAUTHORIZED = 129, // 4.01
	COAP_CODE_BAD_OPTION = 	130, // 4.02
	COAP_CODE_FORBIDDEN = 131, // 4.03
	COAP_CODE_NOT_FOUND = 132, // 4.04
	COAP_CODE_METHOD_NOT_ALLOWED = 133, // 4.05
	COAP_CODE_NOT_ACCEPTABLE = 134, // 4.06
	COAP_CODE_PRECONDITION_FAILED = 140, // 4.12
	COAP_CODE_REQUEST_ENTITY_TOO_LARGE = 141, // 4.13
	COAP_CODE_UNSUPPORTED_MEDIA_TYPE = 143, // 4.15
	COAP_CODE_INTERNAL_SERVER_ERROR	= 160, // 5.00
	COAP_CODE_NOT_IMPLEMENTED = 161, // 5.01
	COAP_CODE_BAD_GATEWAY = 162, // 5.02
	COAP_CODE_SERVICE_UNAVAILABLE = 163, // 5.03
	COAP_CODE_GATEWAY_TIMEOUT = 164, // 5.04
	COAP_CODE_PROXYING_NOT_SUPPORTED = 165 // 5.05
};

enum TimerType
{
	TIMER_NONE,
	TIMER_RETRANSMIT,
	TIMER_ACK
};

static const uint8_t COAP_START_OF_OPTIONS = 4;

static const uint8_t COAP_FORMAT_NONE = 0;
static const uint8_t COAP_FORMAT_UNKNOWN = 255;
static const uint8_t COAP_FORMAT_UINT = 1;
static const uint8_t COAP_FORMAT_STRING = 2;
static const uint8_t COAP_FORMAT_OPAQUE = 3;
static const uint8_t COAP_LARGEST_OPTION_NUMBER = 21;
static const uint8_t COAP_OPTION_ARRAY_SIZE = COAP_LARGEST_OPTION_NUMBER + 1;

static const uint8_t COAP_OPTION_FORMAT[COAP_OPTION_ARRAY_SIZE] =
{
	COAP_FORMAT_UNKNOWN,			// 0: not in use
	COAP_FORMAT_UINT,			// 1: COAP_OPT_CONTENT_TYPE
	COAP_FORMAT_UINT,			// 2: COAP_OPT_MAX_AGE
	COAP_FORMAT_STRING,			// 3: COAP_OPT_PROXY_URI
	COAP_FORMAT_OPAQUE,			// 4: COAP_OPT_ETAG
	COAP_FORMAT_STRING,			// 5: COAP_OPT_URI_HOST
	COAP_FORMAT_STRING,			// 6: COAP_OPT_LOCATION_PATH
	COAP_FORMAT_UINT,			// 7: COAP_OPT_URI_PORT
	COAP_FORMAT_STRING,			// 8: COAP_OPT_LOCATION_QUERY
	COAP_FORMAT_STRING,			// 9: COAP_OPT_URI_PATH
	COAP_FORMAT_UNKNOWN,		// 10: not in use
	COAP_FORMAT_OPAQUE,			// 11: COAP_OPT_TOKEN
	COAP_FORMAT_UINT,			// 12: COAP_OPT_ACCEPT
	COAP_FORMAT_OPAQUE,			// 13: COAP_OPT_IF_MATCH
	COAP_FORMAT_NONE,			// 14: COAP_OPT_FENCEPOST
	COAP_FORMAT_STRING,			// 15: COAP_OPT_URI_QUERY
	COAP_FORMAT_UNKNOWN,		// 16: not in use
	COAP_FORMAT_UNKNOWN,		// 17: not in use
	COAP_FORMAT_UNKNOWN,		// 18: not in use
	COAP_FORMAT_UNKNOWN,		// 19: not in use
	COAP_FORMAT_UNKNOWN,		// 20: not in use
	COAP_FORMAT_NONE			// 21: COAP_OPT_IF_NONE_MATCH
};

static const bool COAP_OPT_CAN_OCCUR_MULTIPLE[COAP_OPTION_ARRAY_SIZE] =
{
	false,			// 0: not in use
	false,			// 1: COAP_OPT_CONTENT_TYPE
	false,			// 2: COAP_OPT_MAX_AGE
	true,			// 3: COAP_OPT_PROXY_URI
	true,			// 4: COAP_OPT_ETAG -- can occur multiple times in Requests, but only once in a response
	false,			// 5: COAP_OPT_URI_HOST
	true,			// 6: COAP_OPT_LOCATION_PATH
	false,			// 7: COAP_OPT_URI_PORT
	true,			// 8: COAP_OPT_LOCATION_QUERY
	true,			// 9: COAP_OPT_URI_PATH
	false,			// 10: not in use
	false,			// 11: COAP_OPT_TOKEN
	true,			// 12: COAP_OPT_ACCEPT
	true,			// 13: COAP_OPT_IF_MATCH
	false,			// 14: COAP_OPT_FENCEPOST
	true,			// 15: COAP_OPT_URI_QUERY
	false,			// 16: not in use
	false,			// 17: not in use
	false,			// 18: not in use
	false,			// 19: not in use
	false,			// 20: not in use
	false			// 21: COAP_OPT_IF_NONE_MATCH
};

namespace wiselib
{
	typedef uint16_t coap_msg_id_t;
	// Size of tokens sent by coapradio.h. This does not affect what size tokens coapradio can receive/process!
	typedef uint32_t coap_token_t;


	class OpaqueData
	{
	public:
		OpaqueData& operator=( const OpaqueData &rhs )
		{
			// avoid self-assignment
			if(this != &rhs)
			{
				set( rhs.value(), rhs.length() );
			}
			return *this;
		}

		bool operator==( const OpaqueData &other ) const
		{
			if( this->length() == other.length() )
			{
				for( size_t i = 0; i < this->length(); ++i )
				{
					if( *( this->value() + i ) != *( other.value() + i ) )
						return false;
				}
				return true;
			}
			return false;
		}

		bool operator!=( const OpaqueData &other ) const
		{
			return !( *this == other );
		}

		OpaqueData()
		{
			length_ = 0;
		}

		OpaqueData( const OpaqueData & rhs)
		{
			*this = rhs;
		}

		OpaqueData( uint8_t * value, size_t length )
		{
			set( value, length );
		}

		~OpaqueData()
		{

		}

		void set( const uint8_t *value, size_t length)
		{
			//TODO: check if length exceeds COAP_OPT_MAXLEN_OPAQUE ?
			memcpy(value_, value, length);
			length_ = length;
		}

		const void get(uint8_t *value, size_t &length) const
		{
			memcpy(value, value_, length_);
			length = length_;
		}

		size_t length() const
		{
			return length_;
		}

		uint8_t * value()
		{
			return value_;
		}

		const uint8_t * value() const
		{
			return value_;
		}

		size_t serialize( uint8_t *datastream ) const
		{
			memcpy( datastream, value_, length_ );
			return length_;
		}

	private:
		size_t length_;
		uint8_t value_[COAP_OPT_MAXLEN_OPAQUE];
	};
}

#endif // COAP_H
