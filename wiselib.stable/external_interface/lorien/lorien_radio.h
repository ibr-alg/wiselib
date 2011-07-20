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
#ifndef CONNECTOR_LORIEN_RADIOMODEL_H
#define CONNECTOR_LORIEN_RADIOMODEL_H

#include "external_interface/lorien/lorien_types.h"
#include "util/delegates/delegate.hpp"
#include "util/pstl/vector_static.h"
#include <string.h>
extern "C" {
#include "lorien.h"
#include "interfaces/networking/imx_radio.h"
#include "interfaces/hardware/ihw_control.h"
}

namespace wiselib
{

   typedef delegate3<void, TNodeID_Int, uint8_t, uint8_t*> lorien_radio_delegate_t;

   /** \brief Lorien Implementation of \ref radio_concept "Radio concept".
    *  \ingroup radio_concept
    *  \ingroup lorien_facets
    *
    * Lorien implementation of the \ref radio_concept "Radio concept" ...
    *
    * @tparam OsModel_P Has to implement @ref os_concept "Os concept".
    */
   template<typename OsModel_P>
   class LorienRadioModel
   {
   public:
      typedef OsModel_P OsModel;

      typedef LorienRadioModel<OsModel> self_type;
      typedef self_type* self_pointer_t;

      typedef TNodeID_Int node_id_t;
      typedef uint8_t     block_data_t;
      typedef uint8_t     size_t;
      typedef uint8_t     message_id_t;

      typedef lorien_radio_delegate_t radio_delegate_t;

      typedef vector_static<OsModel, radio_delegate_t, 10> CallbackVector;
      typedef typename CallbackVector::iterator CallbackVectorIterator;
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
         MAX_MESSAGE_LENGTH = 100           ///< Maximal number of bytes in payload
      };
      // -----------------------------------------------------------------
      void init( Component *comp )
      {
         comp_ = comp;
         ((LXState*) comp_ -> state) -> lorien_radio = this;

         if ( callbacks_.empty() )
            callbacks_.assign( 10, radio_delegate_t() );
      }
      // --------------------------------------------------------------------
      int send( node_id_t id, size_t len, block_data_t *data )
      {
      	((IMXRadio*) ((LXState*) comp_ -> state) -> radio -> userRecp) -> sendTo(((LXState*) comp_ -> state) -> radio -> ifHostComponent, data, len, id, WISELIB_PORT);
         return SUCCESS;
      }
      // --------------------------------------------------------------------
      int enable_radio()
      {
         ((IHWControl*) ((LXState*) comp_ -> state) -> hardwareControl -> userRecp) -> enable(((LXState*) comp_ -> state) -> hardwareControl -> ifHostComponent);
         return SUCCESS;
      }
      // --------------------------------------------------------------------
      int disable_radio()
      {
         ((IHWControl*) ((LXState*) comp_ -> state) -> hardwareControl -> userRecp) -> disable(((LXState*) comp_ -> state) -> hardwareControl -> ifHostComponent);
         return SUCCESS;
      }
      // --------------------------------------------------------------------
      node_id_t id()
      {
         return ((IMXRadio*) ((LXState*) comp_ -> state) -> radio -> userRecp) -> getID(((LXState*) comp_ -> state) -> radio -> ifHostComponent);
      }
      // --------------------------------------------------------------------
      template<class T, void (T::*TMethod)(node_id_t, size_t, block_data_t*)>
      int reg_recv_callback(T *obj_pnt )
      {
         for ( unsigned int i = 0; i < callbacks_.size(); ++i )
         {
            if ( callbacks_.at(i) == radio_delegate_t() )
            {
               callbacks_.at(i) = radio_delegate_t::template from_method<T, TMethod>( obj_pnt );
               return i;
            }
         }

         return -1;
      }
      // --------------------------------------------------------------------
      int unreg_recv_callback(int idx )
      {
         callbacks_.at(idx) = radio_delegate_t();
         return SUCCESS;
      }
      // --------------------------------------------------------------------
      void received( unsigned char *data, size_t len, unsigned int from )
      {
         for ( CallbackVectorIterator
                  it = callbacks_.begin();
                  it != callbacks_.end();
                  ++it )
         {
            if ( *it != radio_delegate_t() )
               (*it)( from, len, data );
         }
      }

   private:
      Component *comp_;
      CallbackVector callbacks_;
   };

}

#endif
