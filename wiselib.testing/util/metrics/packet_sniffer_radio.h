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
#ifndef __UTIL_METRICS_PACKET_SNIFFER_RADIO_H
#define __UTIL_METRICS_PACKET_SNIFFER_RADIO_H

#include "util/delegates/delegate.hpp"
#include "util/serialization/simple_types.h"
#include "util/pstl/map_static_vector.h"


namespace wiselib {

   /** \brief Implementation of \ref radio_concept "Radio Concept" that
   *     outputs packet information about al packages it receives.
   *  \ingroup radio_concept
   *
   */
   template<typename OsModel_P,
            typename Statistics_Map_P,
            typename Radio_P = typename OsModel_P::Radio,
            typename Debug_P = typename OsModel_P::Debug>
   class PacketSnifferRadioModel {
   public:
      typedef OsModel_P OsModel;
      typedef Radio_P Radio;
      typedef Debug_P Debug;
      typedef Statistics_Map_P Statistics_map;
      typedef PacketSnifferRadioModel <OsModel, Statistics_map, Radio, Debug> self_type;
      typedef self_type* self_pointer_t;

      typedef typename Radio::node_id_t node_id_t;
      typedef typename Radio::size_t size_t;
      typedef typename Radio::block_data_t block_data_t;
      typedef typename Radio::message_id_t message_id_t;

      typedef typename Radio::radio_delegate_t radio_delegate_t;
      // --------------------------------------------------------------------
      enum SpecialNodeIds {
         BROADCAST_ADDRESS = Radio::BROADCAST_ADDRESS, ///< All nodes in communication range
         NULL_NODE_ID      = Radio::NULL_NODE_ID       ///< Unknown/No node id
      };
      // --------------------------------------------------------------------
      enum Restrictions {
         MAX_MESSAGE_LENGTH = Radio::MAX_MESSAGE_LENGTH
      };
      // --------------------------------------------------------------------
      void
      send(node_id_t id, size_t len, block_data_t *data)
      {

         radio().send( id, len, data );
      }
      // --------------------------------------------------------------------
      void init( Radio& radio,Debug& debug,message_id_t sniffer_filter )
      {
         radio_ = &radio;
         debug_ = &debug;
         sniffer_filter_ = sniffer_filter;
         sniffer_filter_set_ = true;
      }
      // --------------------------------------------------------------------
      void init( Radio& radio,Debug& debug)
      {
         radio_ = &radio;
         debug_ = &debug;
         remove_filter();
      }
      // --------------------------------------------------------------------
      void destruct()
      {}
      // --------------------------------------------------------------------
      void enable_radio()
      {
         callback_id_ = radio().template reg_recv_callback<self_type, &self_type::receive_radio_message>(this);
         radio().enable_radio();
      }
      // --------------------------------------------------------------------
      void disable_radio()
      {
         radio().unreg_recv_callback( callback_id_ );
         radio().disable_radio();
      }
      // --------------------------------------------------------------------
      node_id_t id()
      {
         return radio().id();
      }
      // --------------------------------------------------------------------
      template<class T, void (T::*TMethod)(node_id_t, size_t, block_data_t*)>
      int reg_recv_callback( T *obj_pnt )
      {
         callback_ = radio_delegate_t::template from_method<T, TMethod>( obj_pnt );
         return -1;
      }
      // --------------------------------------------------------------------
      void unreg_recv_callback( int idx )
      {
         radio().unreg_recv_callback( idx );
      }
      // --------------------------------------------------------------------
      void
            receive_radio_message(
                typename Radio::node_id_t source,
                typename Radio::size_t length,
                typename Radio::block_data_t *buf)
      {
          message_id_t message_id = read<OsModel, block_data_t, message_id_t>( buf );
          if(!sniffer_filter_set_ || message_id == sniffer_filter_)
          {
             if ( stat_map.find(source) == stat_map.end() )
               stat_map[source] = 0;
            stat_map[source]++;
            debug().debug("SNIFFER: Node %d received message from node %d of type %d received %d messages from this node\n",
            id(),source, message_id, stat_map[source]);
          }
          if(callback_)
              callback_( source, length, buf );
      }
      // --------------------------------------------------------------------
      void set_sniffer_filter(message_id_t type)
      {
          sniffer_filter_ = type;
          sniffer_filter_set_  = true;
      }
      // --------------------------------------------------------------------
      void remove_filter()
      {
          sniffer_filter_set_  = false;
      }
   private:
      Radio& radio()
      { return *radio_; }

      Debug& debug()
      { return *debug_; }

      Debug* debug_;

      Radio *radio_;

      message_id_t sniffer_filter_;
      
      bool sniffer_filter_set_;

      Statistics_map stat_map;

      radio_delegate_t callback_;

      int callback_id_;

   };

}

#endif
