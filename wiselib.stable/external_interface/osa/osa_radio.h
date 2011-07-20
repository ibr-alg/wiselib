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
#ifndef CONNECTOR_OSA_RADIOMODEL_H
#define CONNECTOR_OSA_RADIOMODEL_H

#include "external_interface/osa/osa_types.h"
#include "util/delegates/delegate.hpp"
#include <string.h>
extern "C" {
#include "opencom.h"
#include "oc_bindinglistener.h"
#include "interfaces/radio/iradio_int.h"
#include "interfaces/hardware/ihardware_control.h"
}

namespace wiselib
{

   typedef delegate3<void, TNodeID_Int, uint8_t, uint8_t*> osa_radio_delegate_t;

   /** \brief OSA Implementation of \ref radio_concept "Radio concept".
    *  \ingroup radio_concept
    *  \ingroup osa_facets
    *
    * OSA implementation of the \ref radio_concept "Radio concept" ...
    *
    * @tparam OsModel_P Has to implement @ref os_concept "Os concept".
    */
   template<typename OsModel_P>
   class OsaRadioModel
   {
   public:
      typedef OsModel_P OsModel;

      typedef OsaRadioModel<OsModel> self_type;
      typedef self_type* self_pointer_t;

      typedef TNodeID_Int node_id_t;
      typedef uint8_t     block_data_t;
      typedef uint8_t     size_t;
      typedef uint8_t     message_id_t;

      typedef osa_radio_delegate_t radio_delegate_t;
      // --------------------------------------------------------------------
      enum ErrorCodes
      {
         SUCCESS = OsModel::SUCCESS,
         ERR_UNSPEC = OsModel::ERR_UNSPEC,
         ERR_NOTIMPL = OsModel::ERR_NOTIMPL
      };
      // --------------------------------------------------------------------
      enum SpecialNodeIds {
         BROADCAST_ADDRESS = BROADCAST_INT, ///< All nodes in communication rnage
         NULL_NODE_ID      = -1             ///< Unknown/No node id
      };
      // --------------------------------------------------------------------
      enum Restrictions {
         MAX_MESSAGE_LENGTH = 116           ///< Maximal number of bytes in payload
      };
      // --------------------------------------------------------------------
      int send( node_id_t id, size_t len, block_data_t *data )
      {
         CALL(RECPS -> radio -> send, id, len, data);
         return SUCCESS;
      }
      // --------------------------------------------------------------------
      int enable_radio()
      {
         CALL( RECPS -> ihc -> enable );
         return SUCCESS;
      }
      // --------------------------------------------------------------------
      int disable_radio()
      {
         CALL( RECPS -> ihc -> disable );
         return SUCCESS;
      }
      // --------------------------------------------------------------------
      node_id_t id()
      {
         return CALL( RECPS -> radio -> get_id );
      }
      // --------------------------------------------------------------------
      template<class T, void (T::*TMethod)(node_id_t, size_t, block_data_t*)>
      int reg_recv_callback(T *obj_pnt )
      {
         return ERR_NOTIMPL;
      }
      // --------------------------------------------------------------------
      int unreg_recv_callback(int idx )
      {
         return ERR_NOTIMPL;
      }
   };
}

#endif
