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

#ifndef __UTIL_IOS_NODE_API_TESTBED_RADIO_MODEL_H
#define __UTIL_IOS_NODE_API_TESTBED_RADIO_MODEL_H

#define TESTBED_RADIO_DEBUG

#include "util/ios_node_api/ios_link_message.h"
#include "util/wisebed_node_api/command_types.h"

#include "util/delegates/delegate.hpp"
#include "util/serialization/simple_types.h"
#include "util/pstl/vector_static.h"
#include "util/pstl/pair.h"
#include <stdint.h>

namespace wiselib
{

   /** \brief iOs Link Radio Implementation of \ref radio_concept "Radio Concept"
    *  \ingroup radio_concept
    *
    *  iOs Link Radio implementation of the \ref radio_concept "Radio concept" ...
    */
   template<typename OsModel_P,
            typename Radio_P,
            typename Uart_P,
            typename Debug_P = typename OsModel_P::Debug,
            int MAX_LINKS = 10>
   class IOsLinkRadioModel
      : public RadioBase<OsModel_P, Radio_P::node_id_t, Radio_P::size_t, Radio_P::BlockData_P>
   {
   public:
      typedef OsModel_P OsModel;
      typedef typename OsModel::Os Os;
      typedef Radio_P Radio;
      typedef Uart_P Uart;
      typedef Debug_P Debug;
      typedef IOsLinkRadioModel<OsModel, Radio, Uart, Debug, MAX_LINKS> self_type;
      typedef self_type* self_pointer_t;

      typedef IOsLinkMessage<OsModel, Radio> Message;

      typedef typename Radio::node_id_t node_id_t;
      typedef typename Radio::size_t size_t;
      typedef typename Radio::block_data_t block_data_t;
      typedef typename Radio::message_id_t message_id_t;

      typedef typename Radio::radio_delegate_t radio_delegate_t;
      // --------------------------------------------------------------------
      enum SpecialNodeIds
      {
         BROADCAST_ADDRESS = Radio::BROADCAST_ADDRESS, ///< All nodes in communication range
         NULL_NODE_ID      = Radio::NULL_NODE_ID       ///< Unknown/No node id
      };
      // --------------------------------------------------------------------
      enum Restrictions
      {
         MAX_MESSAGE_LENGTH = Radio::MAX_MESSAGE_LENGTH ///< Maximal number of bytes in payload
      };
      // --------------------------------------------------------------------
      void init( Radio& radio, Uart& uart, Debug& debug )
      {
         radio_ = &radio;
         uart_ = &uart;
         debug_ = &debug;

#ifdef TESTBED_RADIO_DEBUG
          debug_->debug( "Node%i: Init", id() );
#endif
      }
      // --------------------------------------------------------------------
      void destruct()
      {}
      // --------------------------------------------------------------------
      void send( node_id_t to, size_t len, block_data_t *buf )
      {
#ifdef TESTBED_RADIO_DEBUG
          debug().debug( "TESTBED: Sent to %d",to);
#endif
          // send message over physical radio
          radio().send( to, len, buf );

          // send message over uart
          // isense_com_uart.h von PLOT auf CUSTOM_OUT geaendert
          Message message;
          message.set_command_type( IOS_LINK_MESSAGE );
          message.set_destination( to );
          message.set_source( id() );
          message.set_payload( len, buf );

          uart().write( message.buffer_size(), (uint8_t*)(&message) );

      };
      // --------------------------------------------------------------------
      void
      receive_message( node_id_t node_id, size_t len, block_data_t *buf )
      {
#ifdef TESTBED_RADIO_DEBUG
         debug().debug("TESTBED: Received from %d (len %d)", node_id, len);
#endif
         this->notify_receivers( node_id, len, buf );
      }
      // --------------------------------------------------------------------
      void rcv_uart_packet( size_t len, block_data_t* data )
      {
#ifdef TESTBED_RADIO_DEBUG
         debug().debug( "TESTBED: Received over UART" );
#endif
    	 
         switch (*data)
         {
            case IOS_LINK_MESSAGE:
            {
               Message *msg = (Message*)data;

               // receive a message over uart and forward to internal receive_message
               if ( msg->destination() == radio().id() ||
                     msg->destination() == Radio::BROADCAST_ADDRESS ) {
                  
                  receive_message( msg->source(), msg->payload_length(), msg->payload() );
               }
               
               /*
               // uart message isn't for me or broadcast! send over radio
               if ( msg->destination() != radio().id() ||
                     msg->destination() == Radio::BROADCAST_ADDRESS ) {
                  
                  send( msg->destination(), msg->payload_length(), msg->payload() );
               }
               */

               break;
            }
            default:
#ifdef TESTBED_RADIO_DEBUG
         debug().debug( "TESTBED: unknown UART Message" );
#endif
               break;
         }
         
      }
      // --------------------------------------------------------------------
      void enable_radio()
      {
#ifdef TESTBED_RADIO_DEBUG
         debug().debug( "TESTBED: Enable radio" );
#endif

         //testbed_node_id_ = radio().id();

         radio().enable_radio();
         radio().template reg_recv_callback<self_type, &self_type::receive_message>(this);
         uart().enable_serial_comm();
         uart().template reg_read_callback<self_type, &self_type::rcv_uart_packet>(this);
      }
      // --------------------------------------------------------------------
      void disable_radio()
      {
         uart().disable_serial_comm();
         radio().disable_radio();
      }
      // --------------------------------------------------------------------
      node_id_t id()
      {
         return radio().id();
      }

   private:
      Radio& radio()
      { return *radio_; }

      Uart& uart()
      { return *uart_; }

      Debug& debug()
      { return *debug_; }

      Radio* radio_;
      Uart* uart_;
      Debug* debug_;
   };
}

#endif
