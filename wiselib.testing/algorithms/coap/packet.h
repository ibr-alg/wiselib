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

/*
 * File:   coap.h
 * Author: Dimitrios Giannakopoulos
 */

#ifndef PACKET_H
#define  PACKET_H

#ifdef ENABLE_URI_QUERIES
#include "query.h"
typedef wiselib::Queries<wiselib::OSMODEL, wiselib::StaticString> queries_t;
#endif

typedef enum {
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

typedef enum {
    GET = 1,
    POST = 2,
    PUT = 4,
    DELETE = 8
} app_method_t;

typedef enum {
    COAP_GET = 1,
    COAP_POST,
    COAP_PUT,
    COAP_DELETE
} coap_method_t;

typedef enum {
    CON,
    NON,
    ACK,
    RST
} coap_message_type_t;

typedef enum {
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

typedef enum {
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

namespace wiselib {
#ifdef DEBUG_OPTION

    template<typename Debug_P>
#endif
    class CoapPacket {
    public:
#ifdef DEBUG_OPTION
        typedef Debug_P Debug;

        void init(Debug& debug) {
            debug_ = &debug;
            version_ = COAP_VERSION;
            type_ = 0;
            opt_count_ = 0;
            code_ = 0;
            mid_ = 0;
            options_ = 0x00;
            uri_path_len_ = 0;
            payload_len_ = 0;
        }
#else

        void init() {
            version_ = COAP_VERSION;
            type_ = 0;
            opt_count_ = 0;
            code_ = 0;
            mid_ = 0;
            options_ = 0x00;
            uri_path_len_ = 0;
            payload_len_ = 0;
        }
#endif

        /**
         * Returns the version of COAP.
         * @return the version of the message.
         */
        inline uint8_t version_w() {
            return version_;
        }

        /**
         * Get the type of the message.
         * @return Confirmable (0), Non-Confirmable (1), Acknowledgement (2) or Reset (3).
         */
        inline uint8_t type_w() {
            return type_;
        }

        /**
         * Checks if the Message is confirmable.
         * @return
         */
        inline bool is_confirmable() {
            return type_ == CON;
        }

        /**
         * Returns the number of options in the Message.
         * @return # of options.
         */
        inline uint8_t opt_count_w() {
            return opt_count_;
        }

        /**
         * Get the Code of the message.
         * @return request (1-31) or a response (64-191), or is empty (0). All other code values are reserved.
         */
        inline uint8_t code_w() {
            return code_;
        }

        /**
         * Helper Function.
         * Checks if a Message is a Request.
         */
        inline bool is_request() {
            return ( code_w() >= 1 && code_w() <= 4);
        }

        inline uint16_t mid_w() {
            return mid_;
        }

        inline uint32_t is_option(uint8_t opt) {
            return options_ & 1L << opt;
        }

        inline uint8_t content_type_w() {
            return content_type_;
        }

        /*
                 inline uint32_t max_age_w() {
                    return max_age_;
                 }
         */
        inline uint16_t uri_host_w() {
            return uri_host_;
        }

        inline uint16_t uri_port_w() {
            return uri_port_;
        }

        inline uint8_t uri_path_len_w() {
            return uri_path_len_;
        }

        inline char* uri_path_w() {
            return uri_path_;
        }

        inline uint16_t observe_w() {
            return observe_;
        }

        inline uint8_t token_len_w() {
            return token_len_;
        }

        inline uint8_t* token_w() {
            return token_;
        }

        inline uint16_t accept_w() {
            return accept_;
        }
#ifdef ENABLE_URI_QUERIES

        inline queries_t* uri_queries() {
            return &uri_queries_;
        }
#endif

        inline uint32_t block2_num_w() {
            return block2_num_;
        }

        inline uint8_t block2_more_w() {
            return block2_more_;
        }

        inline uint16_t block2_size_w() {
            return block2_size_;
        }

        inline uint32_t block2_offset_w() {
            return block2_offset_;
        }

        inline uint8_t payload_len_w() {
            return payload_len_;
        }

        inline uint8_t* payload_w() {
            return payload_;
        }

        inline void set_version(uint8_t version) {
            version_ = version;
        }

        inline void set_type(uint8_t type) {
            type_ = type;
        }

        inline void set_opt_count(uint8_t opt_count) {
            opt_count_ = opt_count;
        }

        inline void set_code(uint8_t code) {
            code_ = code;
        }

        inline void set_mid(uint16_t mid) {
            mid_ = mid;
        }

        inline void set_option(uint8_t opt) {
            options_ |= 1L << opt;
        }

        inline void set_content_type(uint8_t content_type) {
            content_type_ = content_type;
        }

        /*
                 inline void set_max_age( uint32_t max_age ) {
                    max_age_ = max_age;
                 }
         */
        inline void set_uri_host(uint16_t uri_host) {
            uri_host_ = uri_host;
        }

        inline void set_uri_port(uint16_t uri_port) {
            uri_port_ = uri_port;
        }

        inline void set_uri_path_len(uint8_t uri_path_len) {
            uri_path_len_ = uri_path_len;
        }

        inline void set_uri_path(char* uri_path) {
            uri_path_ = uri_path;
        }

        inline void set_observe(uint16_t observe) {
            observe_ = observe;
        }

        inline void set_token_len(uint8_t token_len) {
            token_len_ = token_len;
        }

        inline void set_token(uint8_t* token) {
            memcpy(token_, token, token_len_);
        }

        inline void set_accept(uint16_t accept) {
            accept_ = accept;
        }
#ifdef ENABLE_URI_QUERIES

        inline void set_uri_query(queries_t uri_queries) {
            uri_queries_ = uri_queries;
        }
#endif

        inline void set_block2_num(uint32_t block2_num) {
            block2_num_ = block2_num;
        }

        inline void set_block2_more(uint8_t block2_more) {
            block2_more_ = block2_more;
        }

        inline void set_block2_size(uint16_t block2_size) {
            block2_size_ = block2_size;
        }

        inline void set_block2_offset(uint32_t block2_offset) {
            block2_offset_ = block2_offset;
        }

        inline void set_payload_len(uint8_t payload_len) {
            payload_len_ = payload_len;
        }

        inline void set_payload(uint8_t* payload) {
            payload_ = payload;
        }

        coap_status_t buffer_to_packet(uint8_t len, uint8_t* buf) {
            //header
            version_ = (COAP_HEADER_VERSION_MASK & buf[1]) >> COAP_HEADER_VERSION_SHIFT;
            type_ = (COAP_HEADER_TYPE_MASK & buf[1]) >> COAP_HEADER_TYPE_SHIFT;
            opt_count_ = (COAP_HEADER_OPT_COUNT_MASK & buf[1]) >> COAP_HEADER_OPT_COUNT_SHIFT;
            code_ = buf[2];
            mid_ = buf[3] << 8 | buf[4];

            //options
            uint8_t *current_opt = buf + COAP_HEADER_LEN + 1;
            if (opt_count_) {

                uint16_t opt_len = 0;
                uint8_t opt_index = 0;
                uint8_t current_delta = 0;
                for (opt_index = 0; opt_index < opt_count_; opt_index++) {

                    current_delta += current_opt[0] >> 4;
                    //get option length
                    if ((0x0F & current_opt[0]) < 15) {
                        opt_len = 0x0F & current_opt[0];
                        current_opt += 1; //point to option value
                    } else {
                        opt_len = current_opt[1] + 15;
                        current_opt += 2; //point to option value
                    }
                    if (current_delta == 14 && opt_len == 0) // fence post
                    {
                        continue;
                    }

                    switch (current_delta) {
                        case CONTENT_TYPE:
                            set_option(CONTENT_TYPE);
                            DBG_O(debug().debug("OPTION:CONTENT TYPE"));
                            content_type_ = get_int_opt_value(current_opt, opt_len, false);
                            break;
                        case MAX_AGE:
                            DBG_O(debug().debug("OPTION:MAX AGE"));
                            set_option(MAX_AGE);
                            //max_age_ = get_int_opt_value( current_opt, opt_len, false );
                            break;
                        case PROXY_URI:
                            DBG_O(debug().debug("OPTION:PROXY URI"));
                            set_option(PROXY_URI);
                            break;
                        case ETAG:
                            DBG_O(debug().debug("OPTION:ETAG"));
                            set_option(ETAG);
                            break;
                        case URI_HOST:
                            // based on id, not ip-literal, converting ascii to node id
                            set_option(URI_HOST);
                            DBG_O(debug().debug("OPTION:URI HOST"));
                            uri_host_ = get_int_opt_value(current_opt, opt_len, true);
                            DBG_O(debug().debug("HOST: %d, %x", uri_host_, uri_host_));
                            break;
                        case LOCATION_PATH:
                            DBG_O(debug().debug("OPTION:LOCATION PATH"));
                            set_option(LOCATION_PATH);
                            break;
                        case URI_PORT:
                            DBG_O(debug().debug("OPTION:URI PORT"));
                            set_option(URI_PORT);
                            uri_port_ = get_int_opt_value(current_opt, opt_len, false);
                            break;
                        case LOCATION_QUERY:
                            DBG_O(debug().debug("OPTION:LOCATION QUERY"));
                            set_option(LOCATION_QUERY);
                            break;
                        case URI_PATH:
                            set_option(URI_PATH);
                            DBG_O(debug().debug("OPTION:URI PATH"));
                            merge_options(&uri_path_, &uri_path_len_, current_opt, opt_len, '/');
                            break;
                        case OBSERVE:
                            set_option(OBSERVE);
                            DBG_O(debug().debug("OPTION:OBSERVE"));
                            observe_ = get_int_opt_value(current_opt, opt_len, false);
                            break;
                        case TOKEN:
                            set_option(TOKEN);
                            DBG_O(debug().debug("OPTION:TOKEN"));
                            token_len_ = opt_len; // may fix
                            memcpy(token_, current_opt, opt_len); // may fix
                            break;
                        case ACCEPT:
                            DBG_O(debug().debug("OPTION:ACCEPT"));
                            set_option(ACCEPT);
                            accept_ = get_int_opt_value(current_opt, opt_len, false);
                            break;
                        case IF_MATCH:
                            DBG_O(debug().debug("OPTION:IF MATCH"));
                            set_option(IF_MATCH);
                            break;
                        case MAX_OFE:
                            DBG_O(debug().debug("OPTION:MAX OFE"));
                            set_option(MAX_OFE);
                            break;
                        case URI_QUERY:
                            DBG_O(debug().debug("OPTION:URI QUERY"));
                            set_option(URI_QUERY);
#ifdef ENABLE_URI_QUERIES
                            uri_queries_.add_query((char*) current_opt, opt_len);
#endif
                            break;
                        case BLOCK2:
                            DBG_O(debug().debug("OPTION:BLOCK2"));
                            set_option(BLOCK2);
                            block2_num_ = get_int_opt_value(current_opt, opt_len, false);
                            block2_more_ = (block2_num_ & 0x08) >> 3;
                            block2_size_ = 16 << (block2_num_ & 0x07);
                            block2_offset_ = (block2_num_ & ~0x0F) << (block2_num_ & 0x07);
                            block2_num_ >>= 4;
                            break;
                        case BLOCK1:
                            DBG_O(debug().debug("OPTION:BLOCK1"));
                            set_option(BLOCK1);
                            break;
                        case IF_NONE_MATCH:
                            DBG_O(debug().debug("OPTION:IF NONE MATCH"));
                            set_option(IF_NONE_MATCH);
                            break;
                        default:
                            DBG_O(debug().debug("OPTION:BAD OPTION"));
                            return BAD_OPTION;
                    }
                    current_opt += opt_len; // point to next option delta
                }
            }
            //get payload
            payload_ = current_opt;
            payload_len_ = len - (payload_ - buf);

            return NO_ERROR;
        }

        uint8_t packet_to_buffer(uint8_t *buf) {
            //options
            uint8_t current_delta = 0;
            uint8_t buf_index = COAP_HEADER_LEN + 1;
            if (is_option(CONTENT_TYPE)) {
                buf_index += set_int_opt_value(CONTENT_TYPE, current_delta, &buf[buf_index], content_type_);
                current_delta = CONTENT_TYPE;
                opt_count_ += 1;
            }
            if (is_option(MAX_AGE)) {
                //buf_index += set_int_opt_value( MAX_AGE, current_delta, &buf[buf_index], max_age_ );
                current_delta = MAX_AGE;
                opt_count_++;
            }
            if (is_option(URI_HOST)) {
                buf_index += set_int_opt_value(URI_HOST, current_delta, &buf[buf_index], uri_host_);
                current_delta = URI_HOST;
                opt_count_++;
            }
            if (is_option(URI_PORT)) {
                buf_index += set_int_opt_value(URI_PORT, current_delta, &buf[buf_index], uri_port_);
                current_delta = URI_PORT;
                opt_count_++;
            }
            if (is_option(URI_PATH)) {
                char seperator[] = "/";
                buf_index += split_option(URI_PATH, current_delta, &buf[buf_index], seperator);
                current_delta = URI_PATH;
            }
            if (is_option(OBSERVE)) {
                buf_index += set_int_opt_value(OBSERVE, current_delta, &buf[buf_index], observe_);
                current_delta = OBSERVE;
                opt_count_ += 1;
            }
            if (is_option(TOKEN)) {
                buf[buf_index++] = (TOKEN - current_delta) << 4 | token_len_;
                memcpy(&buf[buf_index], token_, token_len_);
                current_delta = TOKEN;
                buf_index += token_len_;
                opt_count_ += 1;
            }
            if (is_option(ACCEPT)) {
                buf_index += set_int_opt_value(ACCEPT, current_delta, &buf[buf_index], accept_);
                current_delta = ACCEPT;
                opt_count_ += 1;
            }
            if (is_option(BLOCK2)) {
                uint32_t block = block2_num_ << 4;
                if (block2_more_)
                    block |= 0x8;
                block |= 0xF & (power_of_two(block2_size_) - 4);
                buf_index += set_int_opt_value(BLOCK2, current_delta, &buf[buf_index], block);
                current_delta = BLOCK2;
                opt_count_ += 1;
            }
            //header
            buf[0] = WISELIB_MID_COAP_RESP; /// personal use
            buf[1] = COAP_HEADER_VERSION_MASK & version_ << COAP_HEADER_VERSION_SHIFT;
            buf[1] |= COAP_HEADER_TYPE_MASK & type_ << COAP_HEADER_TYPE_SHIFT;
            buf[1] |= COAP_HEADER_OPT_COUNT_MASK & opt_count_ << COAP_HEADER_OPT_COUNT_SHIFT;
            buf[2] = code_;
            buf[3] = 0xFF & (mid_ >> 8);
            buf[4] = 0xFF & mid_;
            //payload
            memcpy(&buf[buf_index], payload_, payload_len_);
            return buf_index + payload_len_;
        }
    protected:

        uint8_t add_fence_opt(uint8_t opt, uint8_t *current_delta, uint8_t *buf) {
            uint8_t i = 0;
            while (opt - *current_delta > 15) {
                uint8_t delta = 14 - (*current_delta % 14);
                set_opt_header(delta, 0, &buf[i++]);
                *current_delta += delta;
                opt_count_++;
            }
            return i;
        }

        uint8_t set_opt_header(uint8_t delta, size_t len, uint8_t *buf) {
            if (len < 15) {
                buf[0] = delta << 4 | len;
                return 1;
            } else {
                buf[0] = delta << 4 | 0x0F;
                buf[1] = len - 15;
                return 2;
            }
        }

        uint8_t set_int_opt_value(uint8_t opt, uint8_t current_delta, uint8_t *buf, uint32_t value) {
            uint8_t i = add_fence_opt(opt, &current_delta, buf);
            uint8_t start_i = i;

            //uint8_t *option = &buf[i];

            if (0xFF000000 & value) buf[++i] = (uint8_t) (0xFF & value >> 24);
            if (0xFFFF0000 & value) buf[++i] = (uint8_t) (0xFF & value >> 16);
            if (0xFFFFFF00 & value) buf[++i] = (uint8_t) (0xFF & value >> 8);
            if (0xFFFFFFFF & value) buf[++i] = (uint8_t) (0xFF & value);

            i += set_opt_header(opt - current_delta, i - start_i, &buf[start_i]);
            return i;
        }

        uint32_t get_int_opt_value(uint8_t *value, uint16_t length, bool hexAscii) {
            uint32_t var = 0;
            int i = 0;
            while (i < length) {
                if (hexAscii == false) {
                    var <<= 8;
                    var |= 0xFF & value[i++];
                } else {
                    var *= 16;
                    if (value[i] >= 0x41 && value[i] <= 0x5a)
                        var += value[i] - 0x41 + 10;
                    else if (value[i] >= 0x61 && value[i] <= 0x7a)
                        var += value[i] - 0x61 + 10;
                    else if (value[i] >= 0x30 && value[i] <= 0x39)
                        var += value[i] - 0x30;
                    i++;
                }
            }
            return var;
        }

        static void merge_options(char **dst, size_t *dst_len, uint8_t *value, uint16_t length, char seperator) {
            if (*dst_len > 0) {
                (*dst)[*dst_len] = seperator;
                *dst_len += 1;
                memmove((*dst) + (*dst_len), value, length);
                *dst_len += length;
            } else {
                *dst = (char *) value;
                *dst_len = length;
            }
        }

        uint8_t split_option(uint8_t opt, uint8_t current_delta, uint8_t* buf, char* seperator) {
            uint8_t index = 0;
            uint8_t buf_last = 0;
            uint8_t path_last = 0;
            uint8_t shift = 0;
            while (index < uri_path_len_) {
                index += mystrcspn(&uri_path_[index], seperator) + 1;
                if (index - path_last - 1 > 15) // large option
                {
                    buf[buf_last] = (opt - current_delta) << 4 | 15;
                    buf[buf_last + 1] = index - path_last - 1 - 15;
                    memcpy(&buf[buf_last + 2], &uri_path_[path_last], index - path_last - 1);
                    buf_last = index + (++shift);
                } else {
                    buf[buf_last] = (opt - current_delta) << 4 | (index - path_last - 1);
                    memcpy(&buf[buf_last + 1], &uri_path_[path_last], index - path_last - 1);
                    buf_last = index + shift;
                }
                path_last = index;
                current_delta = opt;
                opt_count_ += 1;
            }
            return buf_last;
        }

        uint8_t power_of_two(uint16_t num) {
            uint8_t i = 0;
            while (num != 1) {
                num >>= 1;
                i++;
            }
            return i;
        }
    private:
        uint8_t version_;
        uint8_t type_;
        uint8_t opt_count_;
        uint8_t code_;
        uint16_t mid_;

        uint32_t options_;

        uint8_t content_type_; // 1
        //uint32_t max_age_; // 2
        //TODO...
        //size_t proxy_uri_len_; // 3
        //char *proxy_uri_; // 3
        //uint8_t etag_len_; // 4
        //uint8_t etag[8]_; // 4
        //size_t uri_host_len_; // 5
        uint16_t uri_host_; // 5
        uint16_t uri_port_; // 7
        //TODO...
        size_t uri_path_len_; // 9
        char *uri_path_; // 9
        uint16_t observe_; // 10
        uint8_t token_len_; // 11
        uint8_t token_[8]; // 11
        uint16_t accept_; // 12
        //TODO...
        //uint8_t if_match_len_; // 13
        //uint8_t if_match_[8]; // 13
#ifdef ENABLE_URI_QUERIES
        queries_t uri_queries_; // 15
#endif
        // block2 17
        uint32_t block2_num_; // 17
        uint8_t block2_more_; // 17
        uint16_t block2_size_; // 17
        uint32_t block2_offset_; // 17
        //uint32_t block1_num_; // 19
        //uint8_t block1_more_; // 19
        //uint16_t block1_size_; // 19
        //uint32_t block1_offset_; // 19
        //uint8_t if_none_match; // 21

        uint8_t payload_len_;
        uint8_t *payload_;
#ifdef DEBUG_OPTION
        Debug * debug_;

        Debug& debug() {
            return *debug_;
        }
#endif
    };
}
#endif
