/*
 * File:   coap.h
 * Author: Dimitrios Giannakopoulos
 */

#ifndef COAP_H
#define	COAP_H
// wiselib defines
#define WISELIB_MID_COAP                    51
// end of wiselib defines
// CONFIGURATION
#define CONF_MAX_RESOURCES                  10
#define CONF_MAX_OBSERVERS                  10
#define CONF_MAX_MSG_LEN                    112
#define CONF_MAX_PAYLOAD_LEN                108
#define CONF_PIGGY_BACKED                   1
#define CONF_MAX_RETRANSMIT_SLOTS           8

#define CONF_COAP_RESPONSE_TIMEOUT          2
#define CONF_COAP_RESPONSE_RANDOM_FACTOR    1.5
#define CONF_COAP_MAX_RETRANSMIT_TRIES      4
// END OF CONFIGURATION
// CURRENT COAP DEFINES
#define COAP_VERSION                        1
#define COAP_HEADER_VERSION_MASK            0xC0
#define COAP_HEADER_VERSION_SHIFT           6
#define COAP_HEADER_TYPE_MASK               0x30
#define COAP_HEADER_TYPE_SHIFT              4
#define COAP_HEADER_OPT_COUNT_MASK          0x0F
#define COAP_HEADER_OPT_COUNT_SHIFT         0
#define COAP_HEADER_LEN                     4
// END OF CURRENT COAP DEFINES
#include "packet.h"
#include "resource.h"
#include <string.h>

typedef wiselib::ResourceController<wiselib::StaticString> resource_t;
typedef wiselib::CoapPacket coap_packet_t;

namespace wiselib
{
template<typename OsModel_P, typename Radio_P, typename Timer_P, typename Debug_P, typename Rand_P>
class Coap
{
public:
    typedef OsModel_P OsModel;
    typedef Radio_P Radio;
    typedef Timer_P Timer;
    typedef Debug_P Debug;
    typedef Rand_P Rand;

    typedef typename OsModel_P::Clock Clock;
    typedef typename Radio::node_id_t node_id_t;
    typedef typename Radio::size_t size_t;
    typedef typename Radio::block_data_t block_data_t;
    typedef typename Radio::message_id_t message_id_t;
    typedef typename Clock::time_t time_t;

    void init(Radio& radio, Timer& timer, Debug& debug, uint16_t rand, resource_t *resources)
    {
        radio_ = &radio;
        timer_ = &timer;
        debug_ = &debug;
        mid_ = rand;

        resources_ = resources;

        memset(retransmit_mid_, 0, sizeof (retransmit_mid_));
        memset(retransmit_timeout_and_tries_, 0, sizeof (retransmit_timeout_and_tries_));
        memset(retransmit_size_, 0, sizeof (retransmit_size_));
        memset(retransmit_packet_, 0, sizeof (retransmit_packet_));

        memset(observe_id_, 0, sizeof(observe_id_));
        memset(observe_token_, 0, sizeof(observe_token_));
        memset(observe_resource_, 0, sizeof(observe_resource_));

    }

    void receiver(size_t *len, block_data_t *buf, node_id_t *from)
    {
        coap_status_t coap_error_code;
        coap_packet_t msg;
        msg.init();

        coap_error_code = msg.buffer_to_packet(*len, buf);
        //debug().debug("Version: %d",msg.version_w());
        //debug().debug("Type: %d",msg.type_w());
        //debug().debug("Code: %d",msg.code_w());
        //debug().debug("MID: %d",msg.mid_w());
        //debug().debug("Host: %d",msg.uri_host_w());
        //return;
        if (msg.version_w() != COAP_VERSION)
        {
            coap_error_code = BAD_REQUEST;
        }
        if (msg.type_w() > 3)
        {
            coap_error_code = BAD_REQUEST;
        }
        if (coap_error_code == NO_ERROR)
        {
            if ((msg.is_option(URI_HOST)) && (msg.uri_host_w() != radio().id()))
            {
                return;
            }

            uint8_t data[CONF_MAX_PAYLOAD_LEN];
            block_data_t buf[CONF_MAX_MSG_LEN];
            uint8_t response_size;
            uint8_t data_len;
            coap_packet_t response;

            memset(data, 0, CONF_MAX_PAYLOAD_LEN);
            memset(buf, 0, CONF_MAX_MSG_LEN);

            switch (msg.type_w())
            {
            case CON:
                debug().debug("RECEIVED CON MSG");
                response.init();
                //coap_init_message(response, data, buf, &data_len);
                if (CONF_PIGGY_BACKED == 0)
                {
                    debug().debug("ACTION: Sent ACK\n");
                    response.set_type(ACK);
                    response.set_mid(msg.mid_w());
                    response_size = response.packet_to_buffer(buf);
                    radio().send(*from, response_size, buf);

                    response.init();
                    memset(buf, 0, CONF_MAX_MSG_LEN);
                    //coap_init_message(response, data, buf, &data_len);
                    response.set_type(CON);
                    response.set_mid(mid_++);
                }
                else
                {
                    response.set_type(ACK);
                    response.set_mid(msg.mid_w());
                }
                break;
            case NON:
                debug().debug("RECEIVED NON MSG");
                response.init();
                //coap_init_message(response, data, buf, &data_len);
                response.set_type(NON);
                response.set_mid(0);
                break;
            case ACK:
                debug().debug("RECEIVED ACK MSG");
                coap_unregister_con_msg(msg.mid_w(), 0);
                //coap_refresh_observer(msg->mid);
                break;
            case RST:
                debug().debug("RECEIVED RST MSG");
                coap_unregister_con_msg(msg.mid_w(), 0);
                //coap_remove_observer(msg->mid);
                break;
            default:
                debug().debug("SWITCH msg->type WTF???\n");
                break;
            }
            if (msg.code_w() >= 1 && msg.code_w() <= 31)
            {
                switch (msg.code_w())
                {
                case GET:
                    // check uri path, return status, pointer with data, value with length
                    debug().debug("COAP GET REQUEST");
                    coap_error_code = get_resource(data, &data_len, &response, msg.uri_path_w(), msg.uri_path_len_w());
                    debug().debug("STATUS: %d\tDATA_LEN: %d\tCONTENT_TYPE: %d", coap_error_code, data_len, response.content_type_w());
                    debug_data(data, data_len);

                    response.set_code(coap_error_code);
                    response.set_option(URI_HOST);
                    response.set_option(TOKEN);
                    response.set_uri_host(*from);
                    response.set_token_len(msg.token_len_w());
                    response.set_token(msg.token_w());
                    response.set_payload(data);
                    response.set_payload_len(data_len);
                    break;
                case PUT:
                    coap_error_code = METHOD_NOT_ALLOWED;
                    break;
                case POST:
                    coap_error_code = METHOD_NOT_ALLOWED;
                    break;
                case DELETE:
                    coap_error_code = METHOD_NOT_ALLOWED;
                    break;
                default:
                    coap_error_code = METHOD_NOT_ALLOWED;
                }
                response_size = response.packet_to_buffer(buf);
                if (response.type_w() == CON)
                {
                    coap_register_con_msg(response.mid_w(), buf, response_size);
                }
                radio().send(response.uri_host_w(), response_size, buf);
                debug().debug("ACTION: Sent Response");
            } // end of handle requests
        } // end if coap_error_code == NO_ERROR
    }

    void coap_register_con_msg(uint16_t mid, uint8_t *buf, uint8_t size)
    {
        uint8_t i = 0;
        while (i < CONF_MAX_RETRANSMIT_SLOTS)
        {
            if (retransmit_mid_[i] == 0)
            {
                retransmit_register_[i] = 1;
                retransmit_mid_[i] = mid;
                retransmit_timeout_and_tries_[i] = (CONF_COAP_RESPONSE_TIMEOUT << 4) | 0x00;
                retransmit_size_[i] = size;
                memcpy(retransmit_packet_[i], buf, size);
                timer().template set_timer<Coap, &Coap::coap_retransmit_loop > (1000 * (retransmit_timeout_and_tries_[i] >> 4), this, (void *) i);
                return;
            }
            i++;
        }
    }

    void coap_unregister_con_msg(uint16_t mid, uint8_t flag)
    {
        uint8_t i = 0;
        while (i < CONF_MAX_RETRANSMIT_SLOTS)
        {
            if (retransmit_mid_[i] == mid)
            {
                if (flag == 1)
                {
                    retransmit_register_[i] = 0;
                    retransmit_mid_[i] = 0x0000;
                    memset(retransmit_packet_[i], 0, retransmit_size_[i]);
                    retransmit_size_[i] = 0x00;
                    retransmit_timeout_and_tries_[i] = 0x00;
                    //debug().debug("Cleared CON msg, mid: %u\n",mid);
                    return;
                }
                else
                {
                    //debug().debug("Unregistered CON msg, mid: %u\n",mid);
                    retransmit_register_[i] = 0;
                    return;
                }
            }
            i++;
        }
    }

    void coap_retransmit_loop(void *i)
    {
        if (retransmit_register_[(int)i] == 1)
        {
            uint8_t timeout_factor = 0x01;
            retransmit_timeout_and_tries_[(int) i] += 1;
            timeout_factor = timeout_factor << (0x0F & retransmit_timeout_and_tries_[(int) i]);

            if ((0x0F & retransmit_timeout_and_tries_[(int) i]) == CONF_COAP_MAX_RETRANSMIT_TRIES)
            {
                coap_unregister_con_msg(retransmit_mid_[(int) i], 1);
                return;
            }
            else
            {
                debug().debug("RETRANSMIT!! %d", (int) i);
                radio().send(Radio::BROADCAST_ADDRESS, retransmit_size_[(int) i], retransmit_packet_[(int) i]);
                timer().template set_timer<Coap, &Coap::coap_retransmit_loop > (timeout_factor * 1000 * (retransmit_timeout_and_tries_[(int) i] >> 4), this, (void *) i);
            }
        }
        else
        {
            coap_unregister_con_msg(retransmit_mid_[(int) i], 1);
        }
    }

    coap_status_t get_resource(uint8_t *data, uint8_t *data_len, coap_packet_t *response, char* uri_path, uint8_t uri_path_len)
    {
        if (!strncmp(uri_path, ".well-known/core", sizeof (".well-known/core") - 1) || uri_path_len == 0)
        {
            coap_resource_discovery((char *)data, data_len);
            response->set_content_type(APPLICATION_LINK_FORMAT);
            response->set_option(CONTENT_TYPE);
            return CONTENT;
        }
        uint8_t i;
        char *return_value;
        for (i=0; i<CONF_MAX_RESOURCES; i++)
        {
            if (!strncmp(uri_path, resources_[i].name(), uri_path_len))
            {
                response->set_content_type(resources_[i].content_type());
                response->set_option(CONTENT_TYPE);
                // get sensor data
                debug().debug("SENSOR INDEX: %d", resources_[i].sensor_index());
                debug().debug("SENSOR LENGTH: %d", resources_[i].resource_len());

                return_value = resources_[i].value(resources_[i].sensor_index());
                //debug_->debug( "sensor %d: %d %d %d", resources_[i].sensor_index(), return_value[0], return_value[1], return_value[2] );
                *data_len = resources_[i].resource_len();
                strncpy((char *)data, return_value, *data_len);
                return CONTENT;
            }
        }
        *data_len = 0;
        return NOT_FOUND;
    }

    void coap_resource_discovery(char *data, uint8_t *data_len)
    {
        uint8_t index = 0;
        strcpy(data + index, "<.well-known/core>");
        index = sizeof ("<.well-known/core>") - 1;

        uint8_t i;
        for (i=0; i<CONF_MAX_RESOURCES; i++)
        {
            if (resources_[i].is_set() == true)
            {
                strcpy(data + index, ",<");
                index += 2;
                strcpy(data + index, resources_[i].name());
                index += resources_[i].name_length();
                strcpy(data + index, ">");
                index++;
            }
        }
        *data_len = index;
    }
    void debug_payload(const uint8_t * payload, size_t length)
    {
        char buffer[1024];
        int bytes_written = 0;
        bytes_written += sprintf(buffer + bytes_written, "pl(");
        for (size_t i = 0; i < length; i++)
        {
            bytes_written += sprintf(buffer + bytes_written, "%x|", payload[i]);
        }
        bytes_written += sprintf(buffer + bytes_written, ")");
        buffer[bytes_written] = '\0';
        debug().debug("%s", buffer);
    }
    void debug_data(const uint8_t * payload, size_t length)
    {
        char buffer[1024];
        int bytes_written = 0;
        bytes_written += sprintf(buffer + bytes_written, "DATA: (");
        for (size_t i = 0; i < length; i++)
        {
            bytes_written += sprintf(buffer + bytes_written, "%d", payload[i]);
        }
        bytes_written += sprintf(buffer + bytes_written, ")");
        buffer[bytes_written] = '\0';
        debug().debug("%s", buffer);
    }
    /*

            void coap_update_observer(node_id_t *id, uint8_t *token, uint8_t token_len, uint8_t resource)
            {
                uint8_t i, free_slot = 0;
                for(i=0; i<CONF_MAX_OBSERVERS; i++)
                {
                    if ((observe_id_[i] == *id) && (observe_resource_[i] == resource))
                    {
                        //update token
                        return;
                    }
                    if (observe_id_[i] == 0x0000)
                    {
                        free_slot = i+1;
                    }
                }
                if (free_slot != 0)
                {
                    observe_id_[free_slot-1] = *id;
                    memcpy(observe_token_[free_slot-1], token, token_len);
                    observe_resource_[free_slot-1] = resource;
                }
            }
        */
private:
    Radio * radio_;
    Timer * timer_;
    Debug * debug_;
    uint16_t mid_;

    resource_t * resources_;

    uint16_t retransmit_mid_[CONF_MAX_RETRANSMIT_SLOTS];
    uint8_t retransmit_register_[CONF_MAX_RETRANSMIT_SLOTS];
    uint8_t retransmit_timeout_and_tries_[CONF_MAX_RETRANSMIT_SLOTS];
    size_t retransmit_size_[CONF_MAX_RETRANSMIT_SLOTS];
    block_data_t retransmit_packet_[CONF_MAX_RETRANSMIT_SLOTS][CONF_MAX_MSG_LEN];

    uint16_t observe_id_[CONF_MAX_OBSERVERS];
    uint8_t observe_token_[8][CONF_MAX_OBSERVERS];
    uint8_t observe_resource_[CONF_MAX_OBSERVERS];

    Radio& radio()
    {
        return *radio_;
    }

    Timer& timer()
    {
        return *timer_;
    }

    Debug& debug()
    {
        return *debug_;
    }
};
}
#endif	/* COAP_H */
