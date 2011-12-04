/*
 * File:   coap.h
 * Author: Dimitrios Giannakopoulos
 */

#ifndef PACKET_H
#define  PACKET_H

typedef enum
{
   CONTENT_TYPE = 1,
   MAX_AGE = 2,
   PROXY_URI = 3,
   ETAG = 4,
   URI_HOST = 5,
   LOCATION_PATH = 6,
   URI_PORT = 7,
   LOCATION_QUERY = 8,
   URI_PATH = 9,
   OBSERVE = 10,
   TOKEN = 11,
   ACCEPT = 12,
   IF_MATCH = 13,
   MAX_OFE = 14,
   URI_QUERY = 15,
   BLOCK2 = 17,
   BLOCK1 = 19,
   IF_NONE_MATCH = 21
} coap_option_t;

typedef enum
{
   GET = 1,
   POST,
   PUT,
   DELETE
} coap_method_t;

typedef enum
{
   CON,
   NON,
   ACK,
   RST
} coap_message_type_t;

typedef enum
{
   TEXT_PLAIN = 0,
   TEXT_XML = 1,
   TEXT_CSV = 2,
   TEXT_HTML = 3,
   IMAGE_GIF = 21,
   IMAGE_JPEG = 22,
   IMAGE_PNG = 23,
   IMAGE_TIFF = 24,
   AUDIO_RAW = 25,
   VIDEO_RAW = 26,
   APPLICATION_LINK_FORMAT = 40,
   APPLICATION_XML = 41,
   APPLICATION_OCTET_STREAM = 42,
   APPLICATION_RDF_XML = 43,
   APPLICATION_SOAP_XML = 44,
   APPLICATION_ATOM_XML = 45,
   APPLICATION_XMPP_XML = 46,
   APPLICATION_EXI = 47,
   APPLICATION_FASTINFOSET = 48,
   APPLICATION_SOAP_FASTINFOSET = 49,
   APPLICATION_JSON = 50,
   APPLICATION_X_OBIX_BINARY = 51
} coap_content_type_t;

typedef enum
{
   NO_ERROR = 0,

   CREATED = 65,
   DELETED = 66,
   VALID = 67,
   CHANGED = 68,
   CONTENT = 69,

   BAD_REQUEST = 128,
   UNATHORIZED = 129,
   BAD_OPTION = 130,
   FORBIDDEN = 131,
   NOT_FOUND = 132,
   METHOD_NOT_ALLOWED = 133,
   PRECONDITION_FAILED = 140,
   REQUEST_ENTITY_TOO_LARGE = 141,
   UNSUPPORTED_MEDIA_TYPE = 143,

   INTERNAL_SERVER_ERROR = 160,
   NOT_IMPLEMENTED = 161,
   BAD_GATEWAY = 162,
   SERVICE_UNAVAILABLE = 163,
   GATEWAY_TIMEOUT = 164,
   PROXYING_NOT_SUPPORTED = 165
} coap_status_t;

namespace wiselib
{
   class CoapPacket
   {
      public:
         void init( void )
         {
            version_ = COAP_VERSION;
            type_ = 0;
            opt_count_ = 0;
            code_ = 0;
            mid_ = 0;
            options_ = 0x00;
            uri_path_len_ = 0;
            payload_len_ = 0;
         }
         uint8_t version_w()
         {
            return version_;
         }
         uint8_t type_w()
         {
            return type_;
         }
         uint8_t opt_count_w()
         {
            return opt_count_;
         }
         uint8_t code_w()
         {
            return code_;
         }
         uint16_t mid_w()
         {
            return mid_;
         }
         uint32_t is_option( uint8_t opt )
         {
            return options_ & 1L << opt;
         }
         uint8_t content_type_w()
         {
            return content_type_;
         }
         uint16_t uri_host_w()
         {
            return uri_host_;
         }
         uint8_t uri_path_len_w()
         {
            return uri_path_len_;
         }
         char* uri_path_w()
         {
            return uri_path_;
         }
         uint16_t observe_w()
         {
            return observe_;
         }
         uint8_t token_len_w()
         {
            return token_len_;
         }
         uint8_t* token_w()
         {
            return token_;
         }
         uint32_t block2_num_w()
         {
            return block2_num_;
         }
         uint8_t block2_more_w()
         {
            return block2_more_;
         }
         uint16_t block2_size_w()
         {
            return block2_size_;
         }
         uint32_t block2_offset_w()
         {
            return block2_offset_;
         }
         uint8_t payload_len_w()
         {
            return payload_len_;
         }
         uint8_t* payload_w()
         {
            return payload_;
         }

         void set_version( uint8_t version )
         {
            version_ = version;
         }
         void set_type( uint8_t type )
         {
            type_ = type;
         }
         void set_opt_count( uint8_t opt_count )
         {
            opt_count_ = opt_count;
         }
         void set_code( uint8_t code )
         {
            code_ = code;
         }
         void set_mid( uint16_t mid )
         {
            mid_ = mid;
         }
         void set_option( uint8_t opt )
         {
            options_ |= 1L << opt;
         }
         void set_content_type( uint8_t content_type )
         {
            content_type_ = content_type;
         }
         void set_uri_host( uint16_t uri_host )
         {
            uri_host_ = uri_host;
         }
         void set_uri_path_len( uint8_t uri_path_len )
         {
            uri_path_len_ = uri_path_len;
         }
         void set_uri_path( char* uri_path )
         {
            uri_path_ = uri_path;
         }
         void set_observe( uint16_t observe )
         {
            observe_ = observe;
         }
         void set_token_len( uint8_t token_len )
         {
            token_len_ = token_len;
         }
         void set_token( uint8_t* token )
         {
            memcpy( token_, token, token_len_ );
         }
         void set_block2_num(uint32_t block2_num)
         {
            block2_num_ = block2_num;
         }
         void set_block2_more(uint8_t block2_more)
         {
            block2_more_ = block2_more;
         }
         void set_block2_size(uint16_t block2_size)
         {
            block2_size_ = block2_size;
         }
         void set_block2_offset(uint32_t block2_offset)
         {
            block2_offset_ = block2_offset;
         }
         void set_payload_len( uint8_t payload_len )
         {
            payload_len_ = payload_len;
         }
         void set_payload( uint8_t* payload )
         {
            payload_ = payload;
         }

         coap_status_t buffer_to_packet( uint8_t len, uint8_t* buf )
         {
            //header
            version_ = ( COAP_HEADER_VERSION_MASK & buf[1] ) >> COAP_HEADER_VERSION_SHIFT;
            type_ = ( COAP_HEADER_TYPE_MASK & buf[1] ) >> COAP_HEADER_TYPE_SHIFT;
            opt_count_ = ( COAP_HEADER_OPT_COUNT_MASK & buf[1] ) >> COAP_HEADER_OPT_COUNT_SHIFT;
            code_ = buf[2];
            mid_ = buf[3] << 8 | buf[4];

            //options
            uint8_t *current_opt = buf + COAP_HEADER_LEN + 1;
            if( opt_count_ )
            {

               uint16_t opt_len = 0;
               uint8_t opt_index = 0;
               uint8_t current_delta = 0;
               for ( opt_index = 0; opt_index < opt_count_; opt_index++ )
               {

                  current_delta += current_opt[0] >> 4;
                  if ( ( current_delta == 14 ) || ( current_delta == 28 ) || ( current_delta == 42 ) )
                  {
                     current_opt += 1; //point to next option
                     continue;
                  }
                  //get option length
                  if ( ( 0x0F & current_opt[0] ) < 15 )
                  {
                     opt_len = 0x0F & current_opt[0];
                     current_opt += 1; //point to option value
                  }
                  else
                  {
                     opt_len = current_opt[1] + 15;
                     current_opt += 2; //point to option value
                  }

                  switch ( current_delta )
                  {
                     case CONTENT_TYPE:
                        set_option( CONTENT_TYPE );
                        content_type_ = get_int_opt_value( current_opt, opt_len );
                        break;
                     case MAX_AGE:
                        set_option( MAX_AGE );
                        //max_age_ = get_int_opt_value(current_opt, opt_len);
                        break;
                     case PROXY_URI:
                        set_option( PROXY_URI );
                        break;
                     case ETAG:
                        set_option( ETAG );
                        break;
                     case URI_HOST:
                        // based on id, not ip-literal
                        set_option( URI_HOST );
                        uri_host_ = *current_opt << 8 | *( current_opt + 1 );
                        break;
                     case LOCATION_PATH:
                        set_option( LOCATION_PATH );
                        break;
                     case URI_PORT:
                        set_option( URI_PORT );
                        break;
                     case LOCATION_QUERY:
                        set_option( LOCATION_QUERY );
                        break;
                     case URI_PATH:
                        set_option( URI_PATH );
                        merge_options( &uri_path_, &uri_path_len_, current_opt, opt_len, '/' );
                        break;
                     case OBSERVE:
                        set_option( OBSERVE );
                        observe_ = get_int_opt_value( current_opt, opt_len );
                        break;
                     case TOKEN:
                        set_option( TOKEN );
                        token_len_ = opt_len; // may fix
                        memcpy( token_, current_opt, opt_len ); // may fix
                        break;
                     case ACCEPT:
                        set_option( ACCEPT );
                        break;
                     case IF_MATCH:
                        set_option( IF_MATCH );
                        break;
                     case MAX_OFE:
                        set_option( MAX_OFE );
                        break;
                     case URI_QUERY:
                        set_option( URI_QUERY );
                        break;
                     case BLOCK2:
                        set_option( BLOCK2 );
                        block2_num_ = get_int_opt_value( current_opt, opt_len );
                        block2_more_ = ( block2_num_ & 0x08 ) >> 3;
                        block2_size_ = 16 << ( block2_num_ & 0x07 );
                        block2_offset_ = ( block2_num_ & ~0x0F ) << ( block2_num_ & 0x07 );
                        block2_num_ >>= 4;
                        break;
                     case BLOCK1:
                        set_option( BLOCK1 );
                        break;
                     case IF_NONE_MATCH:
                        set_option( IF_NONE_MATCH );
                        break;
                     default:
                        return BAD_OPTION;

                  }
                  current_opt += opt_len; // point to next option delta
               }
            }
            //get payload
            payload_ = current_opt;
            payload_len_ = len - ( payload_ - buf );

            return NO_ERROR;
         }

         uint8_t packet_to_buffer( uint8_t *buf )
         {
            //options
            uint8_t current_delta = 0;
            uint8_t buf_index = COAP_HEADER_LEN + 1;
            if ( is_option( CONTENT_TYPE ) )
            {
               buf[buf_index++] = 0x01 << 4 | 0x01; // always
               buf[buf_index++] = content_type_;
               current_delta = CONTENT_TYPE;
               opt_count_ += 1;
            }
            if ( is_option( URI_HOST ) )
            {
               buf[buf_index++] = ( URI_HOST - current_delta ) << 4 | 0x02;
               buf[buf_index++] = 0xFF & ( uri_host_ >> 8 );
               buf[buf_index++] = 0xFF & uri_host_;
               current_delta = URI_HOST;
               opt_count_ += 1;
            }
            if ( is_option( URI_PATH ) )
            {
               char seperator[] = "/";
               buf_index += split_option( URI_PATH, current_delta, &buf[buf_index], seperator );
               current_delta = URI_PATH;
            }
            if ( is_option( OBSERVE ) )
            {
               buf[buf_index++] = ( OBSERVE - current_delta ) << 4 | 0x02;
               buf[buf_index++] = 0xFF & observe_ >> 8;
               buf[buf_index++] = 0xFF & observe_;
               current_delta = OBSERVE;
               opt_count_ += 1;
            }
            if ( is_option( TOKEN ) )
            {
               buf[buf_index++] = ( TOKEN - current_delta ) << 4 | token_len_;
               memcpy( &buf[buf_index], token_, token_len_ );
               current_delta = TOKEN;
               buf_index += token_len_;
               opt_count_ += 1;
            }
            if ( is_option( BLOCK2 ) )
            {
               buf_index += add_fence_opt(BLOCK2, &current_delta, buf + buf_index);
               uint32_t block = block2_num_ << 4;
               if ( block2_more_ )
                  block |= 0x8;
               block |= 0xF & (block2_size_ >> 4);
               if ( block2_num_ > 4095 )
               {
                  buf[buf_index++] = ( BLOCK2 - current_delta ) << 4 | 0x03;
                  buf[buf_index++] = 0xFF & (block >> 16);
                  buf[buf_index++] = 0xFF & (block >> 8);
                  buf[buf_index++] = 0xFF & block;
               }
               else if ( block2_num_ > 7 )
               {
                  buf[buf_index++] = ( BLOCK2 - current_delta ) << 4 | 0x02;
                  buf[buf_index++] = 0xFF & (block >> 8);
                  buf[buf_index++] = 0xFF & block;
               }
               else
               {
                  buf[buf_index++] = ( BLOCK2 - current_delta ) << 4 | 0x01;
                  buf[buf_index++] = 0xFF & block;
               }
               opt_count_ += 1;
            }
            //header
            buf[0] = WISELIB_MID_COAP;
            buf[1] = COAP_HEADER_VERSION_MASK & version_ << COAP_HEADER_VERSION_SHIFT;
            buf[1] |= COAP_HEADER_TYPE_MASK & type_ << COAP_HEADER_TYPE_SHIFT;
            buf[1] |= COAP_HEADER_OPT_COUNT_MASK & opt_count_ << COAP_HEADER_OPT_COUNT_SHIFT;
            buf[2] = code_;
            buf[3] = 0xFF & ( mid_ >> 8 );
            buf[4] = 0xFF & mid_;
            //payload
            memmove( &buf[buf_index], payload_, payload_len_ );
            return buf_index + payload_len_;
         }
      protected:
         uint8_t add_fence_opt(uint8_t opt, uint8_t *delta, uint8_t *buf)
         {
            if ( opt - *delta > 15 )
            {
               buf[0] = 14 << 4;
               *delta = 14;
               return 1;
            }
            return 0;
         }
         uint8_t get_int_opt_value( uint8_t *value, uint16_t length )
         {
            uint32_t var = 0;
            int i = 0;
            while ( i < length )
            {
               var <<= 8;
               var |= 0xFF & value[i++];
            }
            return var;
         }
         static void merge_options( char **dst, uint8_t *dst_len, uint8_t *value, uint16_t length, char seperator )
         {
            if ( *dst_len > 0 )
            {
               ( *dst )[*dst_len] = seperator;
               *dst_len += 1;
               memmove( ( *dst ) + ( *dst_len ), value, length );
               *dst_len += length;
            }
            else
            {
               *dst = ( char * ) value;
               *dst_len = length;
            }
         }
         uint8_t split_option( uint8_t opt, uint8_t current_delta, uint8_t* buf, char* seperator )
         {
            uint8_t index = 0;
            uint8_t buf_last = 0;
            uint8_t path_last = 0;
            uint8_t shift = 0;
            while( index < uri_path_len_ )
            {
               index += strcspn( &uri_path_[index], seperator ) + 1;
               if ( index - path_last - 1 > 15 ) // large option
               {
                  buf[buf_last] = ( opt - current_delta ) << 4 | 15;
                  buf[buf_last + 1] = index - path_last - 1 - 15;
                  memcpy( &buf[buf_last + 2], &uri_path_[path_last], index - path_last - 1 );
                  buf_last = index + ( ++shift );
               }
               else
               {
                  buf[buf_last] = ( opt - current_delta ) << 4 | ( index - path_last - 1 );
                  memcpy( &buf[buf_last + 1], &uri_path_[path_last], index - path_last - 1 );
                  buf_last = index + shift;
               }
               path_last = index;
               current_delta = opt;
               opt_count_ += 1;
            }
            return buf_last;
         }
      private:
         uint8_t version_;
         uint8_t type_;
         uint8_t opt_count_;
         uint8_t code_;
         uint16_t mid_;

         uint32_t options_;

         uint8_t content_type_; // 1
         //uint8_t max_age; // 2
         //TODO...
         //size_t uri_host_len; // 5
         uint16_t uri_host_; // 5
         //TODO...
         uint8_t uri_path_len_; // 9
         char *uri_path_; // 9
         uint16_t observe_; // 10
         uint8_t token_len_; // 11
         uint8_t token_[8]; // 11
         //TODO...
         //size_t uri_query_len_; // 15
         //char *uri_query_; // 15
         uint32_t block2_num_;
         uint8_t block2_more_;
         uint16_t block2_size_;
         uint32_t block2_offset_;
         //uint8_t if_none_match; // 21

         uint8_t payload_len_;
         uint8_t *payload_;
   };
}
#endif
