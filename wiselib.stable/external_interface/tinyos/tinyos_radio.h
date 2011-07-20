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
#ifndef EXTERNAL_INTERFACE_TINYOS_RADIOMODEL_H
#define EXTERNAL_INTERFACE_TINYOS_RADIOMODEL_H

extern "C" {
#include "external_interface/tinyos/tinyos_wiselib_glue.h"
}
#include "external_interface/tinyos/tinyos_types.h"
#include "util/delegates/delegate.hpp"

namespace wiselib
{

   namespace tinyos
   {
      class ExtendedData
      {
         public:
            ExtendedData(){}

            uint16_t link_metric() const
            { return link_metric_; };

            void set_link_metric( uint16_t lm )
            { link_metric_ = lm; };

         private:
            uint16_t link_metric_;
      };
      // --------------------------------------------------------------------
      typedef delegate3<void, uint16_t, uint8_t, uint8_t*> radio_delegate_t;
      typedef delegate4<void, uint16_t, uint8_t, uint8_t*, const ExtendedData&> extended_radio_delegate_t;
      // --------------------------------------------------------------------
      void tinyos_init_wiselib_radio( void );
      int tinyos_radio_add_receiver( tinyos::radio_delegate_t& delegate );
      int tinyos_radio_add_extended_receiver( tinyos::extended_radio_delegate_t& delegate );
      bool tinyos_radio_del_receiver( int idx );
   }
   // -----------------------------------------------------------------------
   /** \brief TinyOs Implementation of \ref radio_concept "Radio concept".
    *  \ingroup radio_concept
    *  \ingroup extdata_radio_concept
    *  \ingroup tinyos_facets
    *
    * TinyOS implementation of the \ref radio_concept "Radio concept" ...
    *
    * @tparam OsModel_P Has to implement @ref os_concept "Os concept".
    */
   template<typename OsModel_P>
   class TinyOsRadioModel
   {
   public:
      typedef OsModel_P OsModel;

      typedef TinyOsRadioModel<OsModel> self_type;
      typedef self_type* self_pointer_t;

      typedef uint16_t node_id_t;
      typedef uint8_t  block_data_t;
      typedef uint8_t  size_t;
      typedef uint8_t  message_id_t;

      typedef tinyos::ExtendedData ExtendedData;
      typedef tinyos::radio_delegate_t radio_delegate_t;
      typedef tinyos::extended_radio_delegate_t extended_radio_delegate_t;
      // --------------------------------------------------------------------
      enum ErrorCodes
      {
         SUCCESS = OsModel::SUCCESS,
         ERR_UNSPEC = OsModel::ERR_UNSPEC,
         ERR_NOTIMPL = OsModel::ERR_NOTIMPL
      };
      // --------------------------------------------------------------------
      enum SpecialNodeIds {
         BROADCAST_ADDRESS = 0xffff, ///< All nodes in communication range
         NULL_NODE_ID      = 0       ///< Unknown/No node id
      };
      // --------------------------------------------------------------------
      enum Restrictions {
         MAX_MESSAGE_LENGTH = 28  ///< Maximal number of bytes in payload
      };
      // --------------------------------------------------------------------
		/** \todo: tinyos_send_message returns more error codes - use them!
		 */
      int send( node_id_t id, size_t len, block_data_t *data )
      {
         if ( tinyos_send_message( id, len, data ) == TINYOS_RADIO_SUCCESS )
            return SUCCESS;

         // TODO: tinyos_send_message returns more error codes - use them!
         return ERR_UNSPEC;
      }
      // --------------------------------------------------------------------
      int enable_radio()
      {
         return SUCCESS;
      }
      // --------------------------------------------------------------------
      int disable_radio()
      {
         return SUCCESS;
      }
      // --------------------------------------------------------------------
      node_id_t id()
      {
         return tinyos_get_nodeid();
      }
      // --------------------------------------------------------------------
      template<class T, void (T::*TMethod)(node_id_t, size_t, block_data_t*)>
      int reg_recv_callback( T *obj_pnt )
      {
         radio_delegate_t delegate =
               radio_delegate_t::from_method<T, TMethod>( obj_pnt );
         return tinyos::tinyos_radio_add_receiver( delegate );
      }
      // --------------------------------------------------------------------
      template<class T, void (T::*TMethod)(node_id_t, size_t, block_data_t*,const ExtendedData&)>
      int reg_recv_callback( T *obj_pnt )
      {
         extended_radio_delegate_t delegate =
               extended_radio_delegate_t::from_method<T, TMethod>( obj_pnt );
         return tinyos::tinyos_radio_add_extended_receiver( delegate );
      }
      // --------------------------------------------------------------------
      template<class T, void (T::*TMethod)(node_id_t, size_t, block_data_t*)>
      int unreg_recv_callback( int idx )
      {
         if (tinyos::tinyos_radio_del_receiver( idx ))
            return SUCCESS;
         else
            return ERR_UNSPEC;
      }
   };


}

#endif
