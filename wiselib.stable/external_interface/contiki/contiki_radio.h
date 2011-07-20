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
#ifndef CONNECTOR_CONTIKI_RADIOMODEL_H
#define CONNECTOR_CONTIKI_RADIOMODEL_H

#include "external_interface/contiki/contiki_types.h"
#include "util/delegates/delegate.hpp"
#include "util/serialization/simple_types.h"
#include <string.h>
extern "C" {
#include "contiki.h"
#include "net/rime.h"
}

namespace wiselib
{

   typedef delegate3<void, uint16_t, uint8_t, uint8_t*> contiki_radio_delegate_t;
   // -----------------------------------------------------------------------
   typedef void (*contiki_radio_fp)( struct abc_conn *c );
   extern contiki_radio_fp contiki_internal_radio_callback;
   extern abc_conn contiki_radio_conn;
   // -----------------------------------------------------------------------
   void init_contiki_radio( void );
   int contiki_radio_add_receiver( contiki_radio_delegate_t& delegate );
   void contiki_radio_del_receiver( int idx );
   void contiki_notify_receivers( struct abc_conn *c );
   // -----------------------------------------------------------------------
   // -----------------------------------------------------------------------
   // -----------------------------------------------------------------------
   /** \brief Contiki Implementation of \ref radio_concept "Radio concept".
    *  \ingroup radio_concept
    *  \ingroup contiki_facets
    *
    * Contiki implementation of the \ref radio_concept "Radio concept" ...
    *
    * @tparam OsModel_P Has to implement @ref os_concept "Os concept".
    */
   template<typename OsModel_P>
   class ContikiRadio
   {
   public:
      typedef OsModel_P OsModel;

      typedef ContikiRadio<OsModel> self_type;
      typedef self_type* self_pointer_t;

		/// From \ref radio_concept "Radio concept"
      typedef uint16_t node_id_t;
      typedef uint8_t  block_data_t;
      typedef uint8_t  size_t;
      typedef uint8_t  message_id_t;

      typedef contiki_radio_delegate_t radio_delegate_t;
      // --------------------------------------------------------------------
      enum ErrorCodes
      {
         SUCCESS = OsModel::SUCCESS,
         ERR_UNSPEC = OsModel::ERR_UNSPEC
      };
      // --------------------------------------------------------------------
      enum SpecialNodeIds {
         BROADCAST_ADDRESS = 0xffff, ///< All nodes in communication range
         NULL_NODE_ID      = 0       ///< Unknown/No node id
      };
      // --------------------------------------------------------------------
      enum Restrictions {
         MAX_MESSAGE_LENGTH = PACKETBUF_SIZE ///< Maximal number of bytes in payload
      };
      // --------------------------------------------------------------------
      void init()
      {
         init_contiki_radio();
      }
      // --------------------------------------------------------------------
      int send( node_id_t id, size_t len, block_data_t *data );
      // --------------------------------------------------------------------
      int enable_radio()
      {
         contiki_internal_radio_callback = contiki_notify_receivers;
         return SUCCESS;
      }
      // --------------------------------------------------------------------
      int disable_radio()
      {
         contiki_internal_radio_callback = 0;
         return SUCCESS;
      }
      // --------------------------------------------------------------------
      node_id_t id()
      {
         uint16_t addr = rimeaddr_node_addr.u8[0] |
            (rimeaddr_node_addr.u8[1] << 8);
         return addr;
      }
      // --------------------------------------------------------------------
      template<class T, void (T::*TMethod)(node_id_t, size_t, block_data_t*)>
      int reg_recv_callback( T *obj_pnt );
      // --------------------------------------------------------------------
      int unreg_recv_callback( int idx )
      {
         contiki_radio_del_receiver( idx );
         return SUCCESS;
      }
   };
   // --------------------------------------------------------------------
   // -----------------------------------------------------------------------
   // -----------------------------------------------------------------------
   template<typename OsModel_P>
   int
   ContikiRadio<OsModel_P>::
   send( node_id_t dest, size_t len, block_data_t *data )
   {
      uint8_t buf[PACKETBUF_SIZE];

      // wirte own id and destination in first 4 bytes of buffer
      uint16_t addr = id();
      write<OsModel, block_data_t, node_id_t>( buf, addr );
      write<OsModel, block_data_t, node_id_t>( buf + sizeof(node_id_t), dest );
      // write payload
      memcpy( buf + 2*sizeof(node_id_t), data, len );

      packetbuf_clear();
      packetbuf_copyfrom( buf, len + 2*sizeof(node_id_t) );
      abc_send( &contiki_radio_conn );
      return SUCCESS;
   }
   // -----------------------------------------------------------------------
   template<typename OsModel_P>
   template<class T,
            void (T::*TMethod)( typename ContikiRadio<OsModel_P>::node_id_t,
                                typename ContikiRadio<OsModel_P>::size_t,
                                typename ContikiRadio<OsModel_P>::block_data_t*)>
   int
   ContikiRadio<OsModel_P>::
   reg_recv_callback( T *obj_pnt )
   {
      contiki_radio_delegate_t delegate =
         contiki_radio_delegate_t::from_method<T, TMethod>( obj_pnt );
      return contiki_radio_add_receiver( delegate );
   }
}

#endif
