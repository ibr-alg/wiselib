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
#ifndef CONNECTOR_SCW_RADIOMODEL_H
#define CONNECTOR_SCW_RADIOMODEL_H

#include "external_interface/scw/scw_types.h"
#include "util/delegates/delegate.hpp"
extern "C" {
#include <ScatterWeb.System.h>
#include <ScatterWeb.Net.h>
}

namespace wiselib
{
   typedef delegate3<void, netaddr_t, uint8_t, uint8_t*> scw_radio_delegate_t;
   // --------------------------------------------------------------------------
   void scw_radio_init( void );
   void scw_radio_add_receiver( scw_radio_delegate_t receiver );
   void scw_radio_del_receiver( scw_radio_delegate_t receiver );
   // --------------------------------------------------------------------------
   // --------------------------------------------------------------------------
   // --------------------------------------------------------------------------
   /**
    * \brief Scatterweb^2 Implementation of \ref radio_concept "Radio concept".
    * \ingroup radio_concept
    * \ingroup scw_facets
    *
    * Scatterweb^2 implementation of the \ref radio_concept "Radio concept" ...
    *
    * @tparam OsModel_P Has to implement @ref os_concept "Os concept".
    */
   template<typename OsModel_P>
   class ScwRadioModel
   {
      public:
         typedef OsModel_P OsModel;

         typedef ScwRadioModel<OsModel> self_type;
         typedef self_type* self_pointer_t;

         typedef netaddr_t node_id_t;
         typedef uint8_t block_data_t;
         typedef uint8_t size_t;
         typedef uint8_t message_id_t;

         typedef scw_radio_delegate_t radio_delegate_t;
         // --------------------------------------------------------------------
         enum ErrorCodes
         {
            SUCCESS = OsModel::SUCCESS,
            ERR_UNSPEC = OsModel::ERR_UNSPEC,
            ERR_NOTIMPL = OsModel::ERR_NOTIMPL
         };
         // --------------------------------------------------------------------
         enum SpecialNodeIds {
            BROADCAST_ADDRESS = 0xff, ///< All nodes in communication range
            NULL_NODE_ID      = 0     ///< Unknown/No node id
         };
         // --------------------------------------------------------------------
         enum Restrictions {
            MAX_MESSAGE_LENGTH = 100  ///< Maximal number of bytes in payload
         };
         // --------------------------------------------------------------------
         int send( node_id_t id, size_t len, block_data_t *data );
         // --------------------------------------------------------------------
         int enable_radio()
         {
            Net_on();
            return SUCCESS;
         }
         // --------------------------------------------------------------------
         int disable_radio()
         {
            Net_off();
            return SUCCESS;
         }
         // --------------------------------------------------------------------
         node_id_t id()
         {
            return Configuration.id;
         }
         // --------------------------------------------------------------------
         template<class T, void (T::*TMethod)(node_id_t, size_t, block_data_t*)>
         int reg_recv_callback( T *obj_pnt )
         {
            scw_radio_delegate_t delegate =
               scw_radio_delegate_t::from_method<T, TMethod>( obj_pnt );
            scw_radio_add_receiver( delegate );

            return 0;
         }
         // --------------------------------------------------------------------
			/** \todo: Implement unreg_recv_callback
			 */
         int unreg_recv_callback( int idx )
         {
#warning ScwRadio TODO: Implement unreg_recv_callback!
            return ERR_NOTIMPL;
         }
   };
   // --------------------------------------------------------------------
   template<typename OsModel_P>
   int
   ScwRadioModel<OsModel_P>::
   send( node_id_t id, size_t len, block_data_t *data )
   {
      netpacket_send_args_t npsargs;
      // fill structure with default values
      Net_sendArgsInit( &npsargs );

      // fill network header custom values
      npsargs.netheader.to = BROADCAST_ADDRESS;    // receiver's address
      npsargs.netheader.type = USERDEFINED_PACKET; // application protocol
      npsargs.netheader.flags = 0;                 // network layer config flags (CRC

      netpacket_send_buffer buf;
      buf.size = len;
      buf.buffer = data;
      npsargs.payload[0] = buf;

      // queue packet for sending
      Net_send( &npsargs );

      return SUCCESS;
   }
}

#endif
