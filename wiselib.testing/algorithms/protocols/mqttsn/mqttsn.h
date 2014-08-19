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

#include <util/base_classes/radio_base.h>
#include <external_interface/external_interface.h>
#include "mqttsn_messages.h"
#include "mqttsn_topics.h"
#include "util/delegates/delegate.hpp"
#include "std_config_testing.h"

#ifndef __ALGORITHMS_MQTTSN_H__
#define __ALGORITHMS_MQTTSN_H__

#ifdef MQTTSN_DEBUG
  #define DEBUG(...)  debug_->debug(__VA_ARGS__)
#else
  #define DEBUG(...)
#endif

namespace wiselib
{
   /**
   *\brief MQTT-SN client implementation
   *
   *\ingroup basic_algorithm_concept
   *\ingroup radio_concept
   *
   * This class represents MQTT-SN client
   *
   */

   template<typename OsModel_P,
       typename Radio_P = typename OsModel_P::Radio,
       typename Timer_P = typename OsModel_P::Timer,
       typename Debug_P = typename OsModel_P::Debug,
       typename Rand_P  = typename OsModel_P::Rand>
   class MqttSn
           :  public RadioBase<OsModel_P, typename Radio_P::node_id_t, typename Radio_P::size_t, typename Radio_P::block_data_t>
   {

   public:
       typedef OsModel_P OsModel;
       typedef Timer_P   Timer;
       typedef Radio_P   Radio;
       typedef Debug_P   Debug;
       typedef Rand_P    Rand;

       typedef typename Radio::node_id_t node_id_t;
       typedef typename Radio::block_data_t block_data_t;
       typedef typename Radio::size_t size_t;    
#if defined(CONTIKI) || defined(ISENSE) || defined(SHAWN)
       typedef typename Radio::ExtendedData ExData;
#endif

       typedef MqttSn<OsModel, Radio, Timer, Debug, Rand> self_type;
       typedef self_type* self_pointer_t;

       /**
        * \brief Enumeration for return codes
        */
       enum RETURN_CODE
       {
           ACCEPTED = 0x00,
           REJECTED_CONGESTION = 0x01,
           REJECTED_INVALID_TOPIC_ID = 0x02,
           REJECTED_NOT_SUPPORTED = 0x03
       };

       /**
        * \brief Enumeration for timers (seconds)
        */
       enum TIMERS
       {
           /**
            * \brief Time to wait before sending GWINFO to another client
           T_GW_INFO = 5,

           /**
            * \brief Time after message is resent after rejected congestion error
            */
           T_WAIT =  1,

           /**
            * \brief Time to wait for a gateway response
            */
           T_RETRY = 10,

           /**
            * \brief Value of keep alive timer in CONNECT message
            */
           T_DURATION = 100
       };

       /**
        * \brief Enumeration for counters
        */
       enum COUNTERS
       {
           /**
            * \brief Number of maximum missed ADVERTISE messages
            */
           N_ADV = 3,

           /**
            * \brief Number of maximum missed acknowledgments from gateway
            */
           N_RETRY = 3
       };

       /**
        * \brief Enumeration for array sizes
        */
       enum ARRAY_SIZE
       {
           /**
            * \brief Number of maximum amout of stored gateways
            */
           GATEWAY_MAX = 10,

           /**
            * \brief Number of maximum amout of registered topics
            */
           REG_TOPIC_MAX = 5,

           /**
            * \brief Number of maximum amout of subscribed topics
            */
           SUB_TOPIC_MAX = 5
       };

       /**
        * \brief Enumeration for message ids
        */
       enum MSG_IDS
       {
           /**
            * \brief Id of register message
            */
           REG_MSG_ID = 0x01,

           /**
            * \brief Id of subscribe message
            */
           SUB_MSG_ID = 0x02,

           /**
            * \brief Id of unsubscribe message
            */
           UNSUB_MSG_ID = 0x03,

           /**
            * \brief Id of publish message
            */
           PUB_MSG_ID = 0x04
       };

       /**
        * \brief Enumeration for error codes
        */
       enum ERROR_CODES
       {
          SUCCESS = OsModel::SUCCESS,
          ERR_UNSPEC = OsModel::ERR_UNSPEC
       };

       typedef wiselib::FlexStaticString<MqttSnHeader<OsModel, Radio>::TOPIC_NAME_SIZE> TopicNameString;
       typedef wiselib::FlexStaticString<MqttSnHeader<OsModel, Radio>::DATA_SIZE> DataString;
       typedef wiselib::MqttSnTopics<OsModel, Radio, REG_TOPIC_MAX> RegTopics;
       typedef wiselib::MqttSnTopics<OsModel, Radio, SUB_TOPIC_MAX> SubTopics;

       typedef delegate2<void, const char*, block_data_t*> publish_delegate_t;

       /**
        * \brief Represents MQTT gateway
        */
       struct Gateway
       {
           uint32_t next_advertise;
           uint8_t gateway_id;
           node_id_t gateway_address;
           uint8_t missed_adv;
           int16_t signal_str;
           bool is_connected;
           bool is_monitored;
       };

       /**
        * \brief Structure connecting topic_name with associated delegate
        */
       struct TopicDelegate
       {
           TopicNameString topic_name;
           publish_delegate_t topic_publish_delegate_;
       };

       /**
        * \brief Initializes MQTTSN client
        * \param radio - radio facet
        * \param debug - debug facet
        * \param timer - timer facet
        * \param rand  - rand facet
        * \return - SUCCESS if initialization was successfull
        */
       int init( Radio &radio, Debug &debug, Timer &timer, Rand &rand );

       /**
        * \brief Used to deinitialize client
        */
       void destruct();

       /**
        * \brief Starts gateway searching process by perodically sending SEARCHGW message
        */
       void send_search_gw();

       /**
        * \brief Connects with gateway
        */
       void send_connect();

       /**
        * \brief Sends WILLTOPIC message with Will Topic assigned to will_topic_ member
        */
       void send_will_topic();

       /**
        * \brief Sends WILLMSG message with Will Msg assigned to will_msg_ member
        */
       void send_will_msg();

       /**
        * \brief Sends WILLTOPICUPD message
        * \param topic_name - Will Topic to update
        */
       void send_will_topic_update( TopicNameString topic_name );

       /**
        * \brief Sends WILLMSGUPD message
        * \param data - Will Msg to update
        */
       void send_will_msg_update( DataString data);

       /**
        * \brief Deletes Will Data on server
        */
       void send_delete_will();

       /**
        * \brief Sends REGISTER messsage
        * \param topic_name - name of a registered topic
        */
       void send_register( TopicNameString topic_name );

       /**
        * \brief Sends PUBLISH message
        * \param topic_name - topic name associated with message
        * \param data - data to sent
        * \param duplicate - indicates if message is set for the first time, false by default
        */
       void send_publish( TopicNameString topic_name, block_data_t* data, size_t length, bool duplicate );

       /**
        * \brief Sends SUBSCRIBE message
        * \param topic_name - topic name to subscribe
        * \param duplicate - indicates if message is set for the first time, false by default
        */
       void send_subscribe( TopicNameString topic_name, bool duplicate );

       /**
        * \brief Sends UNSUBSCRIBE message
        * \param topic_name - topic name to unsubscribe
        * \param duplicate - indicates if message is set for the first time, false by default
        */
       void send_unsubscribe( TopicNameString topic_name, bool duplicate );

       /**
        * \brief Sends PINGREQ message
        */
       void send_pingreq();

       /**
        * \brief Disconnects with gateway
        */
       void send_disconnect();

       /**
        * \brief Sets Will Topic and Will Msg
        * \param will_topic - topic to be set as Will Topic
        * \param will_msg - message to be set as Will Msg
        *
        * If will_msg_ and will_topic_ is set client connects with Will Flag
        */
       void set_will_connect( TopicNameString will_topic, DataString will_msg );

       /**
        * \brief Registers global callback for receiving PUBLISH message
        */
       template<class T, void ( T::*TMethod )( const char* topic_name, block_data_t* data )>
       int reg_recv_publish( T* obj_pnt );

       /**
        * \brief Registers topic specific callback for receiving PUBLISH message
        * \param topic_name - topic name associated with callback
        */
       template<class T, void ( T::*TMethod )( const char* topic_name, block_data_t* data )>
       int reg_recv_publish_topic( T* obj_pnt, TopicNameString topic_name );

       /**
        * \brief Unregisters global callback for receiving PUBLISH message
        * \return
        */
       int unreg_recv_publish();

       /**
        * \brief Unregisters topic specific callback for receiving PUBLISH message
        * \param topic_name - topic name associated with callback
        * \return
        */
       int unreg_recv_publish_topic( TopicNameString topic_name );

       /**
        * \brief Unregisters all topic specific callbacks
        * \return
        */
       int unreg_all_recv_publish_topics();

   public:
       typename Radio::self_pointer_t radio_;
       typename Timer::self_pointer_t timer_;
       typename Debug::self_pointer_t debug_;
       typename Rand::self_pointer_t rand_;

       /**
        * \brief Contains registered topics
        */
       RegTopics registered_topics_;

       /**
        * \brief Constains subscribed topics
        */
       SubTopics subscribed_topics_;

       /**
        * \brief Contains gateways known by client
        */
       Gateway gateways_[GATEWAY_MAX];

       /**
        * \brief Constains active gateway id
        */
       uint8_t active_gateway_;

       /**
        * \brief Counter of missed PINGRESP messages
        */
       uint8_t keep_alive_counter_;

       /**
        * \brief Value of interval between sending SEARCHGW messages
        */
       node_id_t interval_search_gw_;

       /**
        * \brief Indicates if SEARCHGW message was received by a client
        */
       bool recv_search_gw_;

       /**
        * \brief Indicates if registration process is in progress
        */
       bool register_in_progress_;

       /**
        * \brief Indicates if subscription/unsubscription process is in progress
        */
       bool subscription_in_progress_;

       /**
        * \brief Indicates if publishing process is in progress
        */
       bool publishing_in_progress_;

       /**
        * \brief Contains Will Topic value
        */
       TopicNameString will_topic_;

       /**
        * \brief Contains Will Msg value
        */
       DataString will_msg_;

       /**
        * \brief Buffer for topic id fiels in PUBLISH message
        */
       TopicNameString publish_topic_id_buffer_;

       /**
        * \brief Buffer for data field in PUBLISH message
        */
       block_data_t publish_data_buffer_[MqttSnHeader<OsModel, Radio>::DATA_SIZE];

       // DELEGATES

       /**
        * \brief Delegate for global callback
        */
       publish_delegate_t global_publish_delegate_;

       /**
        * \brief Delegates for topic specific callback
        */
       TopicDelegate topic_delegates_[SUB_TOPIC_MAX];

       // RADIO SPECIFIC FUNCTIONS

       /**
        * \brief Enables radio
        */
       void enable_radio();

       /**
        * @brief Disables radio
        */
       void disable_radio();

       /**
        * \brief Callback function for servicing received MQTT message
        * \param from - id of sender node
        * \param size - size of message
        * \param data - message data
        * \param ex - extended radio data
        */
#if defined(CONTIKI) || defined(ISENSE) || defined(SHAWN)
       void on_receive( node_id_t from, size_t size, block_data_t* data, ExData const &ex );
#else
       void on_receive( node_id_t from, size_t size, block_data_t* data );
#endif

       // RECEIVING MESSAGES

       /**
        * \brief Services received ADVERTISE message
        * \param gw_id - gateway id
        * \param duration - indicates when next ADVERTISE message should be received
        * \param rssi - value of rssi
        */
       void recv_advertise( uint8_t gw_id, node_id_t gateway_address, uint16_t duration, int16_t rssi );

       /**
        * \brief Services received SEARCHGW message
        * \param from - id of sender node
        */
       void recv_search_gw( node_id_t from );

       /**
        * \brief Services received GWINFO message
        * \param gw_id - gateway id
        * \param rssi - value of rssi
        */
       void recv_gw_info( uint8_t gw_id, node_id_t gateway_address, int16_t rssi );

       /**
        * \brief Services received CONNACK message
        * \param return_code - return code from received message
        */
       void recv_conn_ack( uint8_t return_code );

       /**
        * \brief Services received SUBACK message
        * \param return_code - return code from received message
        * \param topic_id - topic id from received message
        * \param msg_id - msg id from from received message
        */
       void recv_sub_ack( uint8_t return_code, uint16_t topic_id, uint16_t msg_id );

       /**
        * \brief Services received UNSUBACK message
        * \param msg_id - msg id from received message
        */
       void recv_unsub_ack( uint16_t msg_id );

       /**
        * \brief Services received WILLTOPICREQ message
        */
       void recv_will_topic_req();

       /**
        * \brief Services received WILLMSGREQ message
        */
       void recv_will_msg_req();

       /**
        * \brief Services received REGISTER message
        * \param return_code - return code from received message
        */
       void recv_register( uint16_t topic_id, TopicNameString topic_name );

       /**
        * \brief Services received REGACK message
        * \param return_code - return code from received message
        * \param topic_id - topic id from received message
        * \param msg_id - msg id from received message
        */
       void recv_reg_ack( uint8_t return_code, uint16_t topic_id, uint16_t msg_id );

       /**
        * \brief Services received PUBLISH message
        * \param topic_id - topic id from received message
        * \param data - data from received message
        */
       void recv_publish( uint16_t topic_id, block_data_t *data );

       /**
        * \brief Services received PUBACK message
        * \param return_code - return code from received message
        * \param msg_id - msg id from received message
        */
       void recv_pub_ack( uint8_t return_code, uint16_t msg_id );

       /**
        * \brief Services received PINGRESP message
        */
       void recv_pingresp();

       /**
        * \brief Services received PINGREQ message
        */
       void recv_pingreq();

       /**
        * \brief Services received DISCONNECT message
        */
       void recv_disconnect();

       /**
        * \brief Services received WILLTOPICRESP message
        */
       void recv_will_topic_resp();

       /**
        * \brief Services received WILLMSGRESP message
        */
       void recv_will_msg_resp();

       // AUXILIARY FUNCTIONS
       /**
        * \brief Sets gateway with best rssi value from known gateways
        */
       void set_active_gateway();

       /**
        * \brief Starts gateway monitoring procedure
        * \param index - index of gateway
        */
       void monitor_gateway( uint8_t index );

       /**
        * \brief Starts keep alive procedure
        */
       void keep_alive();

       /**
        * \brief Checs if any gateway is known
        * \return true if any gateway is known, false is no gateway is known
        */
       bool is_any_gateway();

       /**
        * \brief Checks if particular gateway is known
        * \param gw_id - gateway id
        * \return  true if gateway is known, false if gateway is not known
        */
       bool is_this_gateway( uint8_t gw_id );

       /**
        * \brief Checks if client is connected
        * \return true if client is connected, false if client is not connected
        */
       bool is_connected();

       /**
         * \brief Deletes gateway from gateway list
         * \param index - index of gateway to delete
         */
       void delete_gateway( uint8_t index );

       // CALLBACKS

       /**
        * \brief Callback for gateway monitoring procedure
        * \param index - index of gateway
        */
       void monitor_gateway_callback( void* index );

       /**
        * \brief Callback for keep alive procedure
        */
       void keep_alive_callback( void* );

       /**
        * \brief Callback for sending SERACHGW message
        */
       void send_search_gw_callback( void* );

       /**
        * \brief Callback for sending CONNECT message
        */
       void send_connect_callback( void* );

       /**
        * \brief Callback for sending REGISTER message
        */
       void send_register_callback( void* );

       /**
        * \brief Callback for sending SUBSCRIBE message
        */
       void send_subscribe_callback( void* );

       /**
        * \brief Callback for sending PUBLISH message
        */
       void send_publish_callback( void* );

   };

   // -----------------------------------------------------------------------

   template<typename OsModel,
            typename Radio,
            typename Timer,
            typename Debug,
            typename Rand>
   int
   MqttSn<OsModel, Radio, Timer, Debug, Rand>::
   init( Radio &radio, Debug &debug, Timer &timer, Rand &rand )
   {
       radio_ = &radio;
       timer_ = &timer;
       debug_ = &debug;
       rand_ = &rand;

       DEBUG("Init | Radio id: %x", radio_->id() );

       recv_search_gw_ = false;
       register_in_progress_ = false;
       subscription_in_progress_ = false;
       publishing_in_progress_ = false;
       active_gateway_ = 0;
       keep_alive_counter_ = 0;

       // to prevent broadcast storms random time for sending SEARCHGW must be assigned
#if !(defined(ARDUINO))
       rand_->srand( ( *rand_ )( 100 ) );
       interval_search_gw_ = ( *rand_ )( radio_->id() );
#else
       interval_search_gw_ = static_cast<node_id_t>( radio_->id() );
#endif
       DEBUG( "Init | interval_search_gw: %d", interval_search_gw_);

       for( uint8_t i = 0; i < GATEWAY_MAX; ++i )
       {
           gateways_[i].gateway_id = 0;
           gateways_[i].gateway_address = 0;
           gateways_[i].is_connected = false;
           gateways_[i].missed_adv = 0;
           gateways_[i].next_advertise = 0;
           gateways_[i].signal_str = 0;
           gateways_[i].is_monitored = false;
       }

       radio_->template reg_recv_callback<self_type, &self_type::on_receive>( this );

       send_search_gw();

       return SUCCESS;
   }

   // -----------------------------------------------------------------------

   template<typename OsModel,
            typename Radio,
            typename Timer,
            typename Debug,
            typename Rand>
   void
   MqttSn<OsModel, Radio, Timer, Debug, Rand>::
   destruct()
   {
       disable_radio();
   }

   // -----------------------------------------------------------------------

   template<typename OsModel,
            typename Radio,
            typename Timer,
            typename Debug,
            typename Rand>
   void
   MqttSn<OsModel, Radio, Timer, Debug, Rand>::
   enable_radio()
   {
       radio_->enable_radio();
   }

   // -----------------------------------------------------------------------

   template<typename OsModel,
            typename Radio,
            typename Timer,
            typename Debug,
            typename Rand>
   void
   MqttSn<OsModel, Radio, Timer, Debug, Rand>::
   disable_radio()
   {
       radio_->disable_radio();
   }

   // -----------------------------------------------------------------------

   template<typename OsModel,
            typename Radio,
            typename Timer,
            typename Debug,
            typename Rand>
   void
   MqttSn<OsModel, Radio, Timer, Debug, Rand>::
#if defined(CONTIKI) || defined(ISENSE) || defined(SHAWN)
   on_receive( node_id_t from, size_t size, block_data_t* data, ExData const &ex )
#else
   on_receive( node_id_t from, size_t size, block_data_t* data )
#endif
   {
#if defined(CONTIKI) || defined(ISENSE) || defined(SHAWN)
       DEBUG( "\nMessage from %x RSSI: %d", from, ex.get_rssi() );
#else
       DEBUG( "\nMessage from %x", from );
#endif
       MqttSnHeader<OsModel, Radio> &message = *reinterpret_cast<MqttSnHeader<OsModel, Radio>*>(data);

       switch( message.type() )
       {
           case MqttSnHeader<OsModel, Radio>::ADVERTISE:
           {
               MqttSnAdvertise<OsModel, Radio> &advertise_message = *reinterpret_cast<MqttSnAdvertise<OsModel, Radio>*>(data);
#if defined(CONTIKI) || defined(ISENSE) || defined(SHAWN)
               recv_advertise( advertise_message.gw_id(), from, advertise_message.duration(), ex.get_rssi() );
#else
               recv_advertise( advertise_message.gw_id(), from, advertise_message.duration(), 0 );
#endif
               break;
           }

           case MqttSnHeader<OsModel, Radio>::SEARCHGW:
           {
               recv_search_gw( from );
               break;
           }

           case MqttSnHeader<OsModel, Radio>::GWINFO:
           {
               MqttSnGwInfo<OsModel, Radio> &gwinfo_message = *reinterpret_cast<MqttSnGwInfo<OsModel, Radio>*>(data);
#if defined(CONTIKI) || defined(ISENSE) || defined(SHAWN)
               recv_gw_info( gwinfo_message.gw_id(), from, ex.get_rssi() );
#else
               recv_gw_info( gwinfo_message.gw_id(), from, 0 );
#endif
               break;
           }

           case MqttSnHeader<OsModel, Radio>::CONNACK:
           {
               MqttSnConAck<OsModel, Radio> &conack_message = *reinterpret_cast<MqttSnConAck<OsModel, Radio>*>(data);
               recv_conn_ack( conack_message.return_code() );
               break;
           }

           case MqttSnHeader<OsModel, Radio>::WILLTOPICREQ:
           {
               recv_will_topic_req();
               break;
           }

           case MqttSnHeader<OsModel, Radio>::WILLMSGREQ:
           {
               recv_will_msg_req();
               break;
           }

           case MqttSnHeader<OsModel, Radio>::REGISTER:
           {
                MqttSnRegister<OsModel, Radio> &register_message = *reinterpret_cast<MqttSnRegister<OsModel, Radio>*>(data);
                recv_register( register_message.topic_id(), register_message.topic_name() );
                break;
           }

           case MqttSnHeader<OsModel, Radio>::REGACK:
           {
               MqttSnRegAck<OsModel, Radio> &regack_message = *reinterpret_cast<MqttSnRegAck<OsModel, Radio>*>(data);
               recv_reg_ack( regack_message.return_code(), regack_message.topic_id(), regack_message.msg_id() );
               break;
           }

           case MqttSnHeader<OsModel, Radio>::PUBLISH:
           {
               MqttSnPublish<OsModel, Radio> &publish_message = *reinterpret_cast<MqttSnPublish<OsModel, Radio>*>(data);
               recv_publish( publish_message.topic_id(), publish_message.data() );
               break;
           }

           case MqttSnHeader<OsModel, Radio>::PUBACK:
           {
               MqttSnPubAck<OsModel, Radio> &puback_message = *reinterpret_cast<MqttSnPubAck<OsModel, Radio>*>(data);
               recv_pub_ack( puback_message.return_code(), puback_message.msg_id() );
               break;
           }

           case MqttSnHeader<OsModel, Radio>::SUBACK:
           {
               MqttSnSubAck<OsModel, Radio> &suback_message = *reinterpret_cast<MqttSnSubAck<OsModel, Radio>*>(data);
               recv_sub_ack( suback_message.return_code(), suback_message.topic_id(), suback_message.msg_id() );
               break;
           }

           case MqttSnHeader<OsModel, Radio>::UNSUBACK:
           {
               MqttSnUnsubAck<OsModel, Radio> &unsuback_message = *reinterpret_cast<MqttSnUnsubAck<OsModel, Radio>*>(data);
               recv_unsub_ack( unsuback_message.msg_id() );
               break;
           }

           case MqttSnHeader<OsModel, Radio>::PINGREQ:
           {
               recv_pingreq();
               break;
           }

           case MqttSnHeader<OsModel, Radio>::PINGRESP:
           {
               recv_pingresp();
               break;
           }

           case MqttSnHeader<OsModel, Radio>::DISCONNECT:
           {
               recv_disconnect();
               break;
           }

           case MqttSnHeader<OsModel, Radio>::WILLTOPICUPD:
           {
               break;
           }

           case MqttSnHeader<OsModel, Radio>::WILLTOPICRESP:
           {
               break;
           }

           case MqttSnHeader<OsModel, Radio>::WILLMSGUPD:
           {
               break;
           }

           case MqttSnHeader<OsModel, Radio>::WILLMSGRESP:
           {
               break;
           }

           default:
           {
               DEBUG( "ERROR: Unrecognized message Value: %x", message.type() );
               break;
           }
       }
   }

   // -----------------------------------------------------------------------

   template<typename OsModel,
            typename Radio,
            typename Timer,
            typename Debug,
            typename Rand>
   void
   MqttSn<OsModel, Radio, Timer, Debug, Rand>::
   recv_advertise( uint8_t gw_id, node_id_t gateway_address, uint16_t duration, int16_t rssi )
   {
       // if gateway is not known
       if ( false == is_this_gateway( gw_id ) )
       {
           // find empty connection in gateways_ array
           uint8_t gateway_index = 0;
           while ( 0 != gateways_[gateway_index].gateway_id )
           {
               ++gateway_index;
           }

           // if gateways_ array is not full
           if ( !( GATEWAY_MAX <= gateway_index ) )
           {
               DEBUG("recv ADVERTISE | Received new gateway id: %d", gw_id );

               //assign values from message to connection info
               gateways_[gateway_index].gateway_id = gw_id;
               gateways_[gateway_index].gateway_address = gateway_address;
               gateways_[gateway_index].next_advertise = duration;
               gateways_[gateway_index].signal_str = rssi;
               gateways_[gateway_index].is_monitored = true;

               for ( uint8_t i = 0; i <= gateway_index; ++i )
               {
                   DEBUG( "----------------------------------------------" );
                   DEBUG( "Gateway id: %d", gateways_[i].gateway_id );
                   DEBUG( "Gateway address: %x", gateways_[i].gateway_address );
                   DEBUG( "Duration:   %d", gateways_[i].next_advertise );
                   DEBUG( "RSSI:       %d", gateways_[i].signal_str );
               }

               monitor_gateway( gateway_index );

               set_active_gateway();


               if ( false == is_connected() )
               {
                   send_connect();
               }
           }
           else
           {
               DEBUG("recv ADVERTISE | Gateway limit reached - gateway rejected");
           }
       }

       // if ADVERTISE is from known gateway
       else
       {
           for ( uint8_t i = 0; i < GATEWAY_MAX; ++i)
           {
               if ( gw_id == gateways_[i].gateway_id )
               {
                   if ( false == gateways_[i].is_monitored )
                   {
                       gateways_[i].is_monitored = true;
                       monitor_gateway( i );
                   }
                   else
                   {
                       DEBUG( "recv ADVERTISE | GW: %d | Counter of missed ADVERTISE set to 0", gw_id );
                       gateways_[active_gateway_].missed_adv = 0;
                       gateways_[active_gateway_].next_advertise = duration;
                   }
               }
           }
       }
   }

   // -----------------------------------------------------------------------

   template<typename OsModel,
            typename Radio,
            typename Timer,
            typename Debug,
            typename Rand>
   void
   MqttSn<OsModel, Radio, Timer, Debug, Rand>::
   recv_search_gw( node_id_t from )
   {
       DEBUG( "recv SEARCHGW | From: %x", from );

       //check if any gateway id is known
       if ( true == is_any_gateway() )
       {
           // send GWINFO to a client searching for a gateway
           MqttSnGwInfo<OsModel, Radio> gw_info_message;
           gw_info_message.set_gw_id( gateways_[active_gateway_].gateway_id );
           gw_info_message.set_gw_address( gateways_[active_gateway_].gateway_address );

           DEBUG( "recv SEARCHGW | Send GWINFO to %x", from );
           DEBUG( "recv SEARCHGW | Send gateway address: %x", gateways_[active_gateway_].gateway_address );

           radio_->send( from, sizeof( gw_info_message ), ( block_data_t* )&gw_info_message );

       }
       else
       {
           DEBUG( "recv SEARCHGW | No gateway is known | Prevent broadcast storm" );
           recv_search_gw_ = true;
       }
   }

   // -----------------------------------------------------------------------

   template<typename OsModel,
            typename Radio,
            typename Timer,
            typename Debug,
            typename Rand>
   void
   MqttSn<OsModel, Radio, Timer, Debug, Rand>::
   recv_gw_info( uint8_t gw_id, node_id_t gateway_address, int16_t rssi )
   {
       // if gateway is not known
       if ( false == is_this_gateway( gw_id ) )
       {
           // find empty connection in gateways_ array
           uint8_t gateway_index = 0;
           while ( 0 != gateways_[gateway_index].gateway_id )
           {
               ++gateway_index;
           }

           // if gateways_ array is not full
           if ( !(GATEWAY_MAX <= gateway_index) )
           {
               DEBUG("recv GWINFO | Gateway id: %d", gw_id);

               //assign values from message to connection info
               gateways_[gateway_index].gateway_id = gw_id;
               gateways_[gateway_index].gateway_address = gateway_address;
               gateways_[gateway_index].signal_str = rssi;

               for ( uint8_t i = 0; i <= gateway_index; ++i )
               {
                   DEBUG( "----------------------------------------------" );
                   DEBUG( "Gateway id: %d", gateways_[i].gateway_id );
                   DEBUG( "Gateway address: %x", gateways_[i].gateway_address );
                   DEBUG( "Duration:   %d", gateways_[i].next_advertise );
                   DEBUG( "RSSI:       %d", gateways_[i].signal_str );
               }


               set_active_gateway();

               if ( false == is_connected() )
               {
                   send_connect();
               }
           }
       }
       else
       {
           DEBUG( "recv GWINFO | Gateway is already known" );
       }
   }

   // -----------------------------------------------------------------------

   template<typename OsModel,
            typename Radio,
            typename Timer,
            typename Debug,
            typename Rand>
   void
   MqttSn<OsModel, Radio, Timer, Debug, Rand>::
   recv_conn_ack( uint8_t return_code )
   {
       if ( ACCEPTED == return_code )
       {
           gateways_[active_gateway_].is_connected = true;
           DEBUG("recv CONNACK | Connection with gateway %d established", gateways_[active_gateway_].gateway_id );

           keep_alive();
       }
       else if ( REJECTED_CONGESTION == return_code )
       {
           DEBUG( "recv CONNACK | Rejected congestion: resent after %d", static_cast<uint16_t>( T_WAIT ) );
           timer_->template set_timer<self_type, &self_type::send_connect_callback>( static_cast<uint16_t>( T_WAIT * 1000), this, 0 );
       }
       else
       {
           DEBUG("recv CONNACK | Connection error - Return code: %d", return_code);
       }
   }

   // -----------------------------------------------------------------------

   template<typename OsModel,
            typename Radio,
            typename Timer,
            typename Debug,
            typename Rand>
   void
   MqttSn<OsModel, Radio, Timer, Debug, Rand>::
   recv_will_topic_req()
   {
       if ( false == will_topic_.is_empty() )
       {
           DEBUG( "recv WILTOPICREQ | send WILLTOPIC");
           send_will_topic();
       }
       else
       {
           DEBUG( "recv WILLTOPICREQ | ERROR | WILLTOPIC is not set");
       }
   }

   // -----------------------------------------------------------------------

   template<typename OsModel,
            typename Radio,
            typename Timer,
            typename Debug,
            typename Rand>
   void
   MqttSn<OsModel, Radio, Timer, Debug, Rand>::
   recv_will_msg_req()
   {
       if ( false == will_msg_.is_empty() )
       {
           DEBUG( "recv WILLMSGREQ | send WILLMSG");
           send_will_msg();
       }
       else
       {
           DEBUG( "recv WILLMSGREQ | ERROR | WILLMSG is not set");
       }
   }

   // -----------------------------------------------------------------------

   template<typename OsModel,
            typename Radio,
            typename Timer,
            typename Debug,
            typename Rand>
   void
   MqttSn<OsModel, Radio, Timer, Debug, Rand>::
   recv_register(uint16_t topic_id, TopicNameString topic_name)
   {
       DEBUG( "recv REGISTER" );
       DEBUG( "Topic id: %d", topic_id );
       DEBUG( "Topic name: %s", topic_name.c_str() );

       if ( true == subscribed_topics_.is_topic( topic_id ) )
       {
           subscribed_topics_.set_topic_id_by_name( topic_id, topic_name );
       }
       else
       {
           subscribed_topics_.set_topic_name( topic_name );
           subscribed_topics_.set_topic_id( topic_id );
           subscribed_topics_.set_acknowledged( true );
       }
   }

   // -----------------------------------------------------------------------

   template<typename OsModel,
            typename Radio,
            typename Timer,
            typename Debug,
            typename Rand>
   void
   MqttSn<OsModel, Radio, Timer, Debug, Rand>::
   recv_reg_ack( uint8_t return_code, uint16_t topic_id, uint16_t msg_id )
   {
       if ( REG_MSG_ID == msg_id )
       {
           register_in_progress_ = false;

           if ( ACCEPTED == return_code )
           {
               DEBUG("recv REGACK | Topic registered | Topic id %d", topic_id);

               registered_topics_.set_topic_id( topic_id );

               DEBUG( "recv REGACK | TOPICS: ");
               for ( uint8_t i = 0; i <= registered_topics_.size(); ++i )
               {
                   DEBUG( "----------------------------------------------" );
                   DEBUG( "Topic id: %d", registered_topics_.topic_id( i ) );
                   DEBUG( "Topic name: %s", registered_topics_.topic_name( i ).c_str() );
                   DEBUG( "----------------------------------------------" );
               }

               registered_topics_.set_acknowledged( true );
           }
           else if( REJECTED_CONGESTION == return_code )
           {
               //if rejected by congestion wait T_WAIT and resend
               DEBUG( "recv REGACK | Rejected congestion: resent after %d", static_cast<uint16_t>( T_WAIT ) );
               timer_->template set_timer<self_type, &self_type::send_register_callback>( static_cast<uint16_t>( T_WAIT * 1000 * 60), this, 0 );
           }
           else
           {
               DEBUG( "recv REGACK | Error" );
           }
       }
       else
       {
           DEBUG( "recv REGACK | ERROR | Wrong message id: %d Expected: %d", msg_id, REG_MSG_ID);
       }
   }

   // -----------------------------------------------------------------------

   template<typename OsModel,
            typename Radio,
            typename Timer,
            typename Debug,
            typename Rand>
   void
   MqttSn<OsModel, Radio, Timer, Debug, Rand>::
   recv_publish( uint16_t topic_id, block_data_t* data )
   {
       DEBUG( "recv PUBLISH Topic id: %d", topic_id );
       TopicNameString topic_name;
       topic_name = subscribed_topics_.topic_name_from_id( topic_id );
       bool topic_delegate_called = false;

       if( 0 != topic_delegates_[0].topic_publish_delegate_ )
       {
           for ( uint8_t i = 0; i < SUB_TOPIC_MAX; ++i )
           {
               if ( topic_delegates_[i].topic_name == topic_name )
               {
                   topic_delegate_called = true;
                   topic_delegates_[i].topic_publish_delegate_( topic_name.c_str(), data );
                   break;
               }
           }
       }
       else if ( global_publish_delegate_ && ( false == topic_delegate_called ) )
       {
           global_publish_delegate_( topic_name.c_str(), data );
       }

       else
       {
           if ( (false == (topic_name == "") ) )
           {
               DEBUG( "recv PUBLISH | Topic name: %s Data: %s", topic_name.c_str(), data );

           }
           else
           {
               DEBUG( "recv PUBLISH | ERROR | Topic is not subscribed %s", topic_name.c_str() );
           }
       }
   }

   // -----------------------------------------------------------------------

   template<typename OsModel,
            typename Radio,
            typename Timer,
            typename Debug,
            typename Rand>
   void
   MqttSn<OsModel, Radio, Timer, Debug, Rand>::
   recv_pub_ack( uint8_t return_code, uint16_t msg_id )
   {
       if ( PUB_MSG_ID == msg_id )
       {
           publishing_in_progress_ = false;
           if ( ACCEPTED == return_code )
           {
               DEBUG( "recv PUBACK | ACCEPTED: Message delivered" );
           }
           else if ( REJECTED_CONGESTION == return_code )
           {
               //if rejected by congestion wait T_WAIT and resend
               DEBUG( "recv PUBACK | Rejected congestion: resent after %d", static_cast<uint16_t>( T_WAIT ) );
               timer_->template set_timer<self_type, &self_type::send_publish_callback>( static_cast<uint16_t>( T_WAIT ), this, 0 );
           }
           else
           {
               DEBUG( "recv PUBACK | Unrecognized error: %d", return_code );
           }
       }
       else
       {
           DEBUG( "recv PUBACK | ERROR | Wrong message id: %d Expected: %d", msg_id, PUB_MSG_ID);
       }

   }

   // -----------------------------------------------------------------------

   template<typename OsModel,
            typename Radio,
            typename Timer,
            typename Debug,
            typename Rand>
   void
   MqttSn<OsModel, Radio, Timer, Debug, Rand>::
   recv_sub_ack( uint8_t return_code, uint16_t topic_id, uint16_t msg_id )
   {
       if ( SUB_MSG_ID == msg_id )
       {
           if ( ACCEPTED == return_code )
           {
               subscribed_topics_.set_topic_id( topic_id );
               subscription_in_progress_ = false;

               subscribed_topics_.set_acknowledged( true );

               DEBUG( "recv SUBACK | TOPICS: ");
               for ( uint8_t i = 0; i < subscribed_topics_.size(); ++i )
               {
                   DEBUG( "----------------------------------------------" );
                   DEBUG( "Topic id: %d", subscribed_topics_.topic_id( i ) );
                   DEBUG( "Topic name: %s", subscribed_topics_.topic_name( i ).c_str() );
                   DEBUG( "----------------------------------------------" );
               }
           }
           else if ( REJECTED_CONGESTION == return_code )
           {
               //if rejected by congestion wait T_WAIT and resend
               DEBUG( "recv SUBACK | Rejected congestion: resend after T_WAIT" );
               timer_->template set_timer<self_type, &self_type::send_subscribe_callback>( T_WAIT, this, 0 );
           }
           else
           {
               DEBUG( "recv SUBACK | Unrecognized error: %d", return_code );
           }
       }
       else
       {
           DEBUG( "recv SUBACK | ERROR | Wrong message id: %d Expected: %d", msg_id, SUB_MSG_ID);
       }
   }

   // -----------------------------------------------------------------------

   template<typename OsModel,
            typename Radio,
            typename Timer,
            typename Debug,
            typename Rand>
   void
   MqttSn<OsModel, Radio, Timer, Debug, Rand>::
   recv_unsub_ack( uint16_t msg_id )
   {
       if ( UNSUB_MSG_ID == msg_id )
       {
           DEBUG( "recv UNSUBACK | Topic %s unsubscribed", subscribed_topics_.topic_name( subscribed_topics_.to_delete_index() ).c_str() );
           subscribed_topics_.delete_topic();
       }
       else
       {
           DEBUG( "recv UNSUBACK | ERROR | Wrong message id: %d Expected: %d", msg_id, UNSUB_MSG_ID);
       }
   }

   // -----------------------------------------------------------------------

   template<typename OsModel,
            typename Radio,
            typename Timer,
            typename Debug,
            typename Rand>
   void
   MqttSn<OsModel, Radio, Timer, Debug, Rand>::
   recv_pingresp()
   {
       //reset keep_alive_counter_
       DEBUG( "recv PINGRESP | Keep alive counter = 0" );
       keep_alive_counter_ = 0;
   }

   // -----------------------------------------------------------------------

   template<typename OsModel,
            typename Radio,
            typename Timer,
            typename Debug,
            typename Rand>
   void
   MqttSn<OsModel, Radio, Timer, Debug, Rand>::
   recv_pingreq()
   {
       MqttSnPingReq<OsModel, Radio> pingreq_message;
       radio_->send( gateways_[active_gateway_].gateway_address, sizeof( pingreq_message ), ( block_data_t* )&pingreq_message );
   }

   // -----------------------------------------------------------------------

   template<typename OsModel,
            typename Radio,
            typename Timer,
            typename Debug,
            typename Rand>
   void
   MqttSn<OsModel, Radio, Timer, Debug, Rand>::
   recv_disconnect()
   {
       gateways_[active_gateway_].is_connected = false;
       DEBUG("recv DISCONNECT | Disconnected");
   }

   // -----------------------------------------------------------------------

   template<typename OsModel,
            typename Radio,
            typename Timer,
            typename Debug,
            typename Rand>
   void
   MqttSn<OsModel, Radio, Timer, Debug, Rand>::
   recv_will_topic_resp()
   {
   }

   // -----------------------------------------------------------------------

   template<typename OsModel,
            typename Radio,
            typename Timer,
            typename Debug,
            typename Rand>
   void
   MqttSn<OsModel, Radio, Timer, Debug, Rand>::
   recv_will_msg_resp()
   {
   }

   // -----------------------------------------------------------------------

   template<typename OsModel,
            typename Radio,
            typename Timer,
            typename Debug,
            typename Rand>
   void
   MqttSn<OsModel, Radio, Timer, Debug, Rand>::
   send_search_gw()
   {
       // if no gateway is known
       if ( false == is_any_gateway() )
       {
           // check if search_gw has been received from another client
           if ( true == recv_search_gw_ )
           {
               // behave like SEARCH_GW was sent by itself
               DEBUG("send SEARCHGW | send by another client");
               recv_search_gw_ = false;

           }
           else
           {
               MqttSnSearchGw<OsModel, Radio> search_gw_message;
               radio_->send( Radio::BROADCAST_ADDRESS, sizeof( search_gw_message ), ( block_data_t* )&search_gw_message);
               DEBUG("send SEARCHGW | message broadcasted");
           }

           // repeat after interval_search_gw
           timer_->template set_timer<self_type, &self_type::send_search_gw_callback>( interval_search_gw_, this, 0 );

           // extend search_gw interval
           interval_search_gw_ *= 2;
       }
   }

   // -----------------------------------------------------------------------

   template<typename OsModel,
            typename Radio,
            typename Timer,
            typename Debug,
            typename Rand>
   void
   MqttSn<OsModel, Radio, Timer, Debug, Rand>::
   send_connect()
   {
       if ( is_any_gateway() )
       {
           MqttSnConnect<OsModel, Radio> connect_message;
           char client_id[25];
           sprintf( client_id, "%x", radio_->id() );
           DEBUG( "send CONNECT | Active gateway index: %d", active_gateway_);
           DEBUG( "send CONNECT | Client id: %s | Gateway address: %x", client_id, gateways_[active_gateway_].gateway_address);
           connect_message.set_client_id( client_id );
           connect_message.set_duration( T_DURATION );
           connect_message.set_flags( MqttSnHeader<OsModel, Radio>::QOS | MqttSnHeader<OsModel, Radio>::CLEAN_SESSION );
           if ( ( false == will_topic_.is_empty() ) && ( false == will_msg_.is_empty() ) )
           {
               DEBUG( "send CONNECT | Set WILL flag" );
               connect_message.set_flags( MqttSnHeader<OsModel, Radio>::WILL );
           }

           radio_->send( gateways_[active_gateway_].gateway_address, sizeof( connect_message ), ( block_data_t* )&connect_message );
       }
       else
       {
           DEBUG( "send CONNECT | No gateway in known" );
           send_search_gw();
       }
   }

   // -----------------------------------------------------------------------

   template<typename OsModel,
            typename Radio,
            typename Timer,
            typename Debug,
            typename Rand>
   void
   MqttSn<OsModel, Radio, Timer, Debug, Rand>::
   send_will_topic()
   {
       DEBUG( "send WILLTOPIC | WillTopic: %s", will_topic_.c_str() );
       MqttSnWillTopic<OsModel, Radio> will_topic_message;
       will_topic_message.set_flags( MqttSnHeader<OsModel, Radio>::QOS );
       will_topic_message.set_will_topic( will_topic_ );
       radio_->send( gateways_[active_gateway_].gateway_address, sizeof( will_topic_message ), ( block_data_t* )&will_topic_message );
   }

   // -----------------------------------------------------------------------

   template<typename OsModel,
            typename Radio,
            typename Timer,
            typename Debug,
            typename Rand>
   void
   MqttSn<OsModel, Radio, Timer, Debug, Rand>::
   send_will_msg()
   {
       DEBUG( "send WILLMSG | WillMsg: %s", will_msg_.c_str() );
       MqttSnWillMsg<OsModel, Radio> will_msg_message;
       will_msg_message.set_will_msg( will_msg_ );
       radio_->send( gateways_[active_gateway_].gateway_address, sizeof( will_msg_message ), ( block_data_t* )&will_msg_message );
   }

   // -----------------------------------------------------------------------

   template<typename OsModel,
            typename Radio,
            typename Timer,
            typename Debug,
            typename Rand>
   void
   MqttSn<OsModel, Radio, Timer, Debug, Rand>::
   send_will_topic_update( TopicNameString will_topic )
   {
       if ( true == is_connected() )
       {
           MqttSnWillTopic<OsModel, Radio> will_topic_message;
           will_topic_message.set_to_update();
           will_topic_message.set_flags( MqttSnHeader<OsModel, Radio>::QOS );
           will_topic_message.set_will_topic( will_topic );
           DEBUG( "send WILLTOPICUPD | WillTopic: %s", will_topic.c_str() );
           radio_->send( gateways_[active_gateway_].gateway_address, sizeof( will_topic_message ), ( block_data_t* )&will_topic_message );
       }
       else
       {
           DEBUG( "send WILLTOPICUPD | ERROR | Client is not connected" );
           send_connect();
       }
   }

   // -----------------------------------------------------------------------

   template<typename OsModel,
            typename Radio,
            typename Timer,
            typename Debug,
            typename Rand>
   void
   MqttSn<OsModel, Radio, Timer, Debug, Rand>::
   send_will_msg_update( DataString will_data )
   {
       if ( true == is_connected() )
       {
           MqttSnWillMsg<OsModel, Radio> will_msg_message;
           will_msg_message.set_to_update();
           will_msg_message.set_will_msg( will_data );
           DEBUG( "send WILLMSGUPD | WillData: %s", will_data.c_str() );
           radio_->send( gateways_[active_gateway_].gateway_address, sizeof( will_msg_message ), ( block_data_t* )&will_msg_message );
       }
       else
       {
           DEBUG( "send WILLMSGUPD | ERROR | Client is not connected" );
           send_connect();
       }
   }

   // -----------------------------------------------------------------------

   template<typename OsModel,
            typename Radio,
            typename Timer,
            typename Debug,
            typename Rand>
   void
   MqttSn<OsModel, Radio, Timer, Debug, Rand>::
   send_delete_will()
   {
       if ( true == is_connected() )
       {
           MqttSnWillMsg<OsModel, Radio> will_msg_message;
           DEBUG(" WILLMSG | Delete will data" );
           radio_->send( gateways_[active_gateway_].gateway_address, sizeof( will_msg_message ), ( block_data_t* )&will_msg_message );
       }
       else
       {
           DEBUG("send WILLMSG | ERROR | Client is not connected" );
           send_connect();
       }
   }

   // -----------------------------------------------------------------------

   template<typename OsModel,
            typename Radio,
            typename Timer,
            typename Debug,
            typename Rand>
   void
   MqttSn<OsModel, Radio, Timer, Debug, Rand>::
   send_register( TopicNameString topic_name )
   {
       if ( true == is_connected() )
       {
           if ( false == register_in_progress_ )
           {
               MqttSnRegister<OsModel, Radio> register_message;
               register_message.set_msg_id( REG_MSG_ID );
               register_message.set_topic_name( topic_name );
               registered_topics_.set_topic_name ( topic_name );
               register_in_progress_ = true;

               DEBUG("send REGISTER | Topic name: %s", registered_topics_.topic_name(0).c_str() );
               radio_->send( gateways_[active_gateway_].gateway_address, sizeof( register_message ), ( block_data_t* )&register_message );
           }
           else
           {
               DEBUG("send REGISTER | Previous registration in progress - rejected");
           }
       }
       else
       {
           DEBUG("send REGISTER | ERROR | Client is not connected" );
           send_connect();
       }

   }

   // -----------------------------------------------------------------------

   template<typename OsModel,
            typename Radio,
            typename Timer,
            typename Debug,
            typename Rand>
   void
   MqttSn<OsModel, Radio, Timer, Debug, Rand>::
   send_publish(TopicNameString topic_name, block_data_t *data, size_t length ,bool duplicate = false )
   {
       if ( true == is_connected() )
       {
           if ( false == publishing_in_progress_ )
           {
               uint16_t topic_id = 0;
               topic_id = registered_topics_.topic_id_from_name( topic_name );

               if ( 0 != topic_id )
               {
                   MqttSnPublish<OsModel, Radio> publish_message;
                   //if first time
                   publish_message.set_flags( MqttSnHeader<OsModel, Radio>::QOS );
                   publish_message.set_msg_id( PUB_MSG_ID );
                   publish_message.set_topic_id( topic_id );
                   publish_message.set_data( data, length );

                   publishing_in_progress_ = true;
                   publish_topic_id_buffer_ = topic_name;

                   for( uint8_t i = 0; i < length; ++i)
                   {
                       publish_data_buffer_[i] = data[i];
                   }

                   //DEBUG( "send PUBLISH | Message: %s", data );

                   if ( true == duplicate)
                   {
                       DEBUG( "send UBLISH | Duplicate" );
                       publish_message.set_flags( MqttSnHeader<OsModel, Radio>::DUP );
                   }

                   DEBUG( "send PUBLISH | Message length: %d", length);
                   radio_->send( gateways_[active_gateway_].gateway_address,
                                 length + MqttSnHeader<OsModel, Radio>::PUBLISH_NON_PAYLOAD/*sizeof( publish_message )*/,
                                 ( block_data_t* )&publish_message );
               }
               else
               {
                   DEBUG( "send PUBLISH | ERROR | Topic id is not registered" );
               }
           }
           else
           {
               DEBUG( "send PUBLISH| Previous publish in progress - rejected");
           }
       }
       else
       {
           DEBUG( "send PUBLISH | ERROR | Client is not connected" );
           send_connect();
       }
   }

   // -----------------------------------------------------------------------

   template<typename OsModel,
            typename Radio,
            typename Timer,
            typename Debug,
            typename Rand>
   void
   MqttSn<OsModel, Radio, Timer, Debug, Rand>::
   send_subscribe( TopicNameString topic_name, bool duplicate = false )
   {
       if ( true == is_connected() )
       {
           if ( false == subscription_in_progress_ )
           {
               MqttSnSubscribe<OsModel, Radio> subscribe_message;
               subscribe_message.set_topic_name( topic_name );
               subscribe_message.set_msg_id( SUB_MSG_ID ); //to identify the corresponding SUBACK message
               subscribed_topics_.set_topic_name( topic_name );

               if ( true == duplicate)
               {
                   subscribe_message.set_flags( MqttSnHeader<OsModel, Radio>::DUP );
               }

               radio_->send( gateways_[active_gateway_].gateway_address, sizeof(subscribe_message), (block_data_t*)&subscribe_message );
               subscription_in_progress_ = true;

               DEBUG( "send SUBSCRIBE | Topic name: %s", subscribe_message.topic_name().c_str() );
           }
           else
           {
               DEBUG( "send SUBSCRIBE| ERROR | Previous subscription in progress - rejected");
           }
       }
       else
       {
           DEBUG( "send SUBSCRIBE | ERROR | Client is not connected" );
           send_connect();
       }
   }

   // -----------------------------------------------------------------------

   template<typename OsModel,
               typename Radio,
               typename Timer,
               typename Debug,
               typename Rand>
      void
      MqttSn<OsModel, Radio, Timer, Debug, Rand>::
      send_unsubscribe( TopicNameString topic_name, bool duplicate = false )
      {
          if ( true == is_connected() )
          {
              if ( false == subscription_in_progress_ )
              {
                  bool is_topic = subscribed_topics_.prepare_to_delete( topic_name );
                  if ( true == is_topic )
                  {
                      MqttSnUnsubscribe<OsModel, Radio> unsubscribe_message;
                      unsubscribe_message.set_topic_name( topic_name );
                      unsubscribe_message.set_msg_id( UNSUB_MSG_ID );

                      if ( true == duplicate)
                      {
                          unsubscribe_message.set_flags( MqttSnHeader<OsModel, Radio>::DUP );
                      }

                      radio_->send( gateways_[active_gateway_].gateway_address, sizeof( unsubscribe_message ), (block_data_t*)&unsubscribe_message );

                      subscription_in_progress_ = true;

                      DEBUG( "send UNSUBSCRIBE | Topic name: %s", topic_name.c_str() );
                  }
                  else
                  {
                      DEBUG( "send UNSUBSCIRBE | Topic name: %s is not subscribed", topic_name.c_str() );
                  }
              }
              else
              {
                  DEBUG( "send UNSUBSCRIBE| ERROR | Previous subscription in progress - rejected");
              }
          }
          else
          {
              DEBUG( "send UNSUBSCRIBE | ERROR | Client is not connected");
          }
      }

   // -----------------------------------------------------------------------

   template<typename OsModel,
            typename Radio,
            typename Timer,
            typename Debug,
            typename Rand>
   void
   MqttSn<OsModel, Radio, Timer, Debug, Rand>::
   send_pingreq()
   {
       MqttSnPingReq<OsModel, Radio> pingreq_message;

       DEBUG( "send PINGREQ" );
       radio_->send( gateways_[active_gateway_].gateway_address, sizeof( pingreq_message ), ( block_data_t* )&pingreq_message );
   }

   // -----------------------------------------------------------------------

   template<typename OsModel,
            typename Radio,
            typename Timer,
            typename Debug,
            typename Rand>
   void
   MqttSn<OsModel, Radio, Timer, Debug, Rand>::
   send_disconnect()
   {
       if ( true == is_connected() )
       {
           MqttSnDisconnect<OsModel, Radio> disconnect_message;

           DEBUG( "send DISCONNECT" );
           gateways_[active_gateway_].is_connected = false;
           radio_->send( gateways_[active_gateway_].gateway_address, sizeof( disconnect_message ), ( block_data_t* )&disconnect_message );
       }
       else
       {
           DEBUG( "send DISCONNECT | ERROR | Client is not connected" );
       }
   }

   // -----------------------------------------------------------------------

   template<typename OsModel,
            typename Radio,
            typename Timer,
            typename Debug,
            typename Rand>
   void
   MqttSn<OsModel, Radio, Timer, Debug, Rand>::
   set_will_connect( TopicNameString will_topic, DataString will_msg )
   {
       will_topic_ = will_topic;
       will_msg_ = will_msg;
   }

   // -----------------------------------------------------------------------

   template<typename OsModel,
            typename Radio,
            typename Timer,
            typename Debug,
            typename Rand>
   void
   MqttSn<OsModel, Radio, Timer, Debug, Rand>::
   set_active_gateway()
   {
       for( uint8_t i = 0; i < GATEWAY_MAX; ++i)
       {
           if ( i == active_gateway_ )
           {
               continue;
           }
           if ( 0 != gateways_[active_gateway_].signal_str )
           {
               if ( gateways_[i].signal_str > gateways_[active_gateway_].signal_str  )
               {
                   DEBUG( "INFO | Current gateway %x", gateways_[active_gateway_].gateway_address);
                   send_disconnect();
                   DEBUG( "INFO | Switch to gateway %x", gateways_[i].gateway_address);
                   active_gateway_ = i;
               }
            }
            if ( 0 == gateways_[active_gateway_].gateway_id )
            {
               if ( 0 != gateways_[i].gateway_id )
               {
                   DEBUG( "INFO | Switch to gateway %x", gateways_[i].gateway_address);
                   active_gateway_ = i;
                   break;
               }
            }
       }
   }

   // -----------------------------------------------------------------------

   template<typename OsModel,
            typename Radio,
            typename Timer,
            typename Debug,
            typename Rand>
   void
   MqttSn<OsModel, Radio, Timer, Debug, Rand>::
   monitor_gateway( uint8_t index )
   {
       if ( true == gateways_[index].is_monitored )
       {
           DEBUG( "Gateway monitor | ID: %d | Missed ADVERTISE: %d", gateways_[index].gateway_id, gateways_[index].missed_adv );

           timer_->template set_timer<self_type, &self_type::monitor_gateway_callback>( gateways_[index].next_advertise * 1000, this, reinterpret_cast<void*>(index) );

           //increment counter of missed ADVERTISE | it will be set to 0 if ADVERTISE will be received
           gateways_[index].missed_adv++;

           //if number of missed ADVERTISE is equal to N_ADV assume that gateway is no longer active
           if ( (N_ADV + 1) == gateways_[index].missed_adv )
           {
               send_disconnect();
               delete_gateway( index );
               set_active_gateway();
               DEBUG( "Monitor gateway | No ADVERTISE - Gateway deleted" );
           }
       }
   }

   // -----------------------------------------------------------------------

   template<typename OsModel,
            typename Radio,
            typename Timer,
            typename Debug,
            typename Rand>
   void
   MqttSn<OsModel, Radio, Timer, Debug, Rand>::
   keep_alive()
   {
       if ( true == is_connected() )
       {
           DEBUG( "Keep alive| Monitoring connection with gateway: %d", gateways_[active_gateway_].gateway_id );

           timer_->template set_timer<self_type, &self_type::keep_alive_callback>( T_DURATION * 1000, this, 0 );

           if ( ( N_ADV+1 ) == keep_alive_counter_ )
           {
               gateways_[active_gateway_].is_connected = false;
               DEBUG( "Keep alive | Missed pingresp - gateway not active");
           }
           keep_alive_counter_++;
           DEBUG( "Keep alive| Missed PINGRESP: %d", keep_alive_counter_ );

           send_pingreq();
       }
       else
       {
           send_disconnect();
           delete_gateway( active_gateway_ );
           set_active_gateway();
           DEBUG( "Keep alive | ERROR | Stopped - not connected" );
       }
   }

   // -----------------------------------------------------------------------

   template<typename OsModel,
            typename Radio,
            typename Timer,
            typename Debug,
            typename Rand>
   bool
   MqttSn<OsModel, Radio, Timer, Debug, Rand>::
   is_any_gateway()
   {
       bool is_any_gateway = false;

       for( uint8_t i = 0; i < GATEWAY_MAX; ++i )
       {
           if ( 0 != gateways_[i].gateway_id )
           {
               is_any_gateway = true;
               break;
           }
       }

       return is_any_gateway;
   }

   // -----------------------------------------------------------------------

   template<typename OsModel,
            typename Radio,
            typename Timer,
            typename Debug,
            typename Rand>
   bool
   MqttSn<OsModel, Radio, Timer, Debug, Rand>::
   is_this_gateway( uint8_t gw_id )
   {
       bool is_this_gateway = false;

       for( uint8_t i = 0; i < GATEWAY_MAX; ++i )
       {
           if ( gateways_[i].gateway_id == gw_id )
           {
               is_this_gateway = true;
               break;
           }
       }

       return is_this_gateway;
   }

   // -----------------------------------------------------------------------

   template<typename OsModel,
            typename Radio,
            typename Timer,
            typename Debug,
            typename Rand>
   bool
   MqttSn<OsModel, Radio, Timer, Debug, Rand>::
   is_connected()
   {
      return gateways_[active_gateway_].is_connected;
   }

   // -----------------------------------------------------------------------

   template<typename OsModel,
            typename Radio,
            typename Timer,
            typename Debug,
            typename Rand>
   void
   MqttSn<OsModel, Radio, Timer, Debug, Rand>::
   delete_gateway( uint8_t index )
   {
      gateways_[index].gateway_id = 0;
      gateways_[index].gateway_address = 0;
      gateways_[index].is_connected = false;
      gateways_[index].is_monitored = false;
      gateways_[index].missed_adv = 0;
      gateways_[index].signal_str = 0;
   }

   // -----------------------------------------------------------------------

   template<typename OsModel,
            typename Radio,
            typename Timer,
            typename Debug,
            typename Rand>
   void
   MqttSn<OsModel, Radio, Timer, Debug, Rand>::
   monitor_gateway_callback( void* index )
   {
      uint8_t ui8_index = long(index);
      monitor_gateway( ui8_index );
   }

   // -----------------------------------------------------------------------

   template<typename OsModel,
            typename Radio,
            typename Timer,
            typename Debug,
            typename Rand>
   void
   MqttSn<OsModel, Radio, Timer, Debug, Rand>::
   keep_alive_callback( void* )
   {
       keep_alive();
   }

   // -----------------------------------------------------------------------

   template<typename OsModel,
            typename Radio,
            typename Timer,
            typename Debug,
            typename Rand>
   void
   MqttSn<OsModel, Radio, Timer, Debug, Rand>::
   send_search_gw_callback( void* )
   {
      send_search_gw();
   }

   // -----------------------------------------------------------------------

   template<typename OsModel,
            typename Radio,
            typename Timer,
            typename Debug,
            typename Rand>
   void
   MqttSn<OsModel, Radio, Timer, Debug, Rand>::
   send_connect_callback( void* )
   {
      send_connect();
   }

   // -----------------------------------------------------------------------

   template<typename OsModel,
            typename Radio,
            typename Timer,
            typename Debug,
            typename Rand>
   void
   MqttSn<OsModel, Radio, Timer, Debug, Rand>::
   send_register_callback( void* )
   {
       send_register( registered_topics_.topic_name( subscribed_topics_.size() ) );
   }

   // -----------------------------------------------------------------------

   template<typename OsModel,
            typename Radio,
            typename Timer,
            typename Debug,
            typename Rand>
   void
   MqttSn<OsModel, Radio, Timer, Debug, Rand>::
   send_publish_callback( void* )
   {
       send_publish( publish_topic_id_buffer_, publish_data_buffer_, sizeof( publish_data_buffer_ ) );
   }

   // -----------------------------------------------------------------------

   template<typename OsModel,
            typename Radio,
            typename Timer,
            typename Debug,
            typename Rand>
   void
   MqttSn<OsModel, Radio, Timer, Debug, Rand>::
   send_subscribe_callback( void* )
   {
       send_subscribe( subscribed_topics_.topic_name( subscribed_topics_.size() ), true );
   }

   // -----------------------------------------------------------------------

   template<typename OsModel,
            typename Radio,
            typename Timer,
            typename Debug,
            typename Rand>
   template <class T,
            void ( T::*TMethod )( const char*,
                                  typename MqttSn<OsModel, Radio, Timer, Debug, Rand>::block_data_t* )>
   int
   MqttSn<OsModel, Radio, Timer, Debug, Rand>::
   reg_recv_publish( T* obj_pnt )
   {
       if ( !global_publish_delegate_ )
       {
           global_publish_delegate_ = publish_delegate_t::template from_method<T, TMethod>( obj_pnt );
           return SUCCESS;
       }
       return ERR_UNSPEC;
   }

   // -----------------------------------------------------------------------

   template<typename OsModel,
            typename Radio,
            typename Timer,
            typename Debug,
            typename Rand>
   template <class T,
            void ( T::*TMethod )( const char*,
                                  typename MqttSn<OsModel, Radio, Timer, Debug, Rand>::block_data_t* )>
   int
   MqttSn<OsModel, Radio, Timer, Debug, Rand>::
   reg_recv_publish_topic( T* obj_pnt, TopicNameString topic_name )
   {
       for ( uint8_t i = 0; i < SUB_TOPIC_MAX; ++i )
       {
           if ( !topic_delegates_[i].topic_publish_delegate_ )
           {
               DEBUG(" Save topic delegate ");
               topic_delegates_[i].topic_name = topic_name;
               topic_delegates_[i].topic_publish_delegate_ = publish_delegate_t::template from_method<T, TMethod>( obj_pnt );
               return SUCCESS;
           }
       }
       return ERR_UNSPEC;
   }

   // -----------------------------------------------------------------------

   template<typename OsModel,
            typename Radio,
            typename Timer,
            typename Debug,
            typename Rand>
   int
   MqttSn<OsModel, Radio, Timer, Debug, Rand>::
   unreg_recv_publish()
   {
       if ( global_publish_delegate_ )
       {
           global_publish_delegate_ = publish_delegate_t();
           return SUCCESS;
       }
       return ERR_UNSPEC;
   }

   // -----------------------------------------------------------------------

   template<typename OsModel,
            typename Radio,
            typename Timer,
            typename Debug,
            typename Rand>
   int
   MqttSn<OsModel, Radio, Timer, Debug, Rand>::
   unreg_recv_publish_topic( TopicNameString topic_name )
   {
       for ( uint8_t i = 0; i < SUB_TOPIC_MAX; ++i )
       {
           if ( topic_delegates_[i].topic_name == topic_name )
           {
               topic_delegates_[i].topic_publish_delegate_ = publish_delegate_t();
               return SUCCESS;
           }
       }
       return ERR_UNSPEC;
   }

   // -----------------------------------------------------------------------

   template<typename OsModel,
            typename Radio,
            typename Timer,
            typename Debug,
            typename Rand>
   int
   MqttSn<OsModel, Radio, Timer, Debug, Rand>::
   unreg_all_recv_publish_topics()
   {
       for ( uint8_t i = 0; i < SUB_TOPIC_MAX; ++i )
       {
           if ( topic_delegates_[i].topic_publish_delegate_ )
           {
               topic_delegates_[i].topic_publish_delegate_ = publish_delegate_t();
           }
       }
   }
}

#endif /* __ALGORITHMS_MQTTSN_H__ */

