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
#ifndef CONNECTOR_CONTIKI_EXTDATA_RADIOMODEL_H
#define CONNECTOR_CONTIKI_EXTDATA_RADIOMODEL_H

#include "external_interface/contiki/contiki_types.h"
#include "util/delegates/delegate.hpp"
#include "util/serialization/simple_types.h"
#include "util/base_classes/extended_radio_base.h"
#include "util/base_classes/base_extended_data.h"
#include <string.h>
extern "C" {
#include "contiki.h"
#include "net/rime.h"
}

namespace wiselib
{

   namespace contiki
   {
      extern abc_conn contiki_extdata_radio_conn;

      typedef delegate1<void, struct abc_conn*> contiki_extended_receive_delegate_t;
      void contiki_register_receive( contiki_extended_receive_delegate_t& delegate );

      void init_contiki_extdata_radio( void );
   }
   // -----------------------------------------------------------------------
   // -----------------------------------------------------------------------
   // -----------------------------------------------------------------------
   /** \brief Contiki Implementation of \ref radio_concept "Radio concept"
    *  \ingroup radio_concept
    *
    * Contiki implementation of the \ref radio_concept "Radio concept" ...
    */
   template<typename OsModel_P>
   class ContikiExtendedDataRadioModel
      : public ExtendedRadioBase<OsModel_P, uint16_t, uint8_t, uint8_t>
   {
   public:

      typedef OsModel_P OsModel;
      typedef BaseExtendedData<OsModel> ExtendedData;
      typedef uint16_t node_id_t;
      typedef uint8_t  block_data_t;
      typedef uint8_t  size_t;
      typedef uint8_t  message_id_t;

      typedef ContikiExtendedDataRadioModel<OsModel> self_type;
      typedef self_type* self_pointer_t;

      typedef delegate3<void, uint16_t, uint8_t, uint8_t*> radio_delegate_t;
      typedef delegate4<void, uint16_t, uint8_t, uint8_t*, const ExtendedData&> extended_radio_delegate_t;
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
         contiki::init_contiki_extdata_radio();
      }
      // --------------------------------------------------------------------
      int enable_radio()
      {
         contiki::contiki_extended_receive_delegate_t d =
            contiki::contiki_extended_receive_delegate_t::from_method<self_type, &self_type::receive>( this );
         contiki::contiki_register_receive( d );
         return SUCCESS;
      }
      // --------------------------------------------------------------------
      int disable_radio()
      {
         contiki::contiki_register_receive( contiki::contiki_extended_receive_delegate_t() );
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
      int send( node_id_t dest, size_t len, block_data_t *data )
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
         abc_send( &contiki::contiki_extdata_radio_conn );
         return SUCCESS;
      }
      // --------------------------------------------------------------------
      void receive( struct abc_conn* )
      {
         uint8_t buffer[PACKETBUF_SIZE];
         int len = packetbuf_copyto( buffer );

         uint16_t addr = rimeaddr_node_addr.u8[0] | (rimeaddr_node_addr.u8[1] << 8);

         uint16_t src = read<OsModel, block_data_t, node_id_t>( buffer );
         uint16_t dst = read<OsModel, block_data_t, node_id_t>( buffer + 2 );
//          printf( "RCVD: src=%u, dst=%u, myaddr=%u of len=%u\n", src, dst, addr, len );

         if ( dst == addr || dst == BROADCAST_ADDRESS )
         {
            uint16_t  signal_strength = (uint16_t) packetbuf_attr(PACKETBUF_ATTR_LINK_QUALITY);
            ExtendedData ex;
            ex.set_link_metric( 255 - signal_strength );

            this->notify_receivers( src, len - 4, buffer + 4 );
            this->notify_receivers( src, len - 4, buffer + 4, ex );
         }
      }
   };

}

#endif
