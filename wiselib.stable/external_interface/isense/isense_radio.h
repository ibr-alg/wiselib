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
#ifndef CONNECTOR_ISENSE_RADIOMODEL_H
#define CONNECTOR_ISENSE_RADIOMODEL_H

#include "external_interface/isense/isense_os.h"
#include "external_interface/isense/isense_types.h"
#include "util/delegates/delegate.hpp"
#include <isense/os.h>
#include <isense/radio.h>
#include <isense/dispatcher.h>

namespace wiselib
{

   /** \brief iSense Implementation of \ref radio_concept "Radio concept".
    *  \ingroup radio_concept
    *  \ingroup isense_facets
    *
    * iSense implementation of the \ref radio_concept "Radio concept" ...
    *
    * @tparam OsModel_P Has to implement @ref os_concept "Os concept".
    */
   template<typename OsModel_P>
   class iSenseRadioModel
      : public isense::Receiver
   {
   public:
      typedef OsModel_P OsModel;

      typedef iSenseRadioModel<OsModel> self_type;
      typedef self_type* self_pointer_t;

      typedef uint16 node_id_t;
      typedef uint8  block_data_t;
      typedef uint8  size_t;
      typedef uint8  message_id_t;
      // --------------------------------------------------------------------
      typedef delegate3<void, node_id_t, size_t, block_data_t*> isense_radio_delegate_t;
      typedef isense_radio_delegate_t radio_delegate_t;
      // --------------------------------------------------------------------
      enum ErrorCodes
      {
         SUCCESS = OsModel::SUCCESS,
         ERR_UNSPEC = OsModel::ERR_UNSPEC
      };
      // --------------------------------------------------------------------
      enum { MAX_INTERNAL_RECEIVERS = 10 };
      // --------------------------------------------------------------------
      enum SpecialNodeIds {
         BROADCAST_ADDRESS = 0xffff, ///< All nodes in communication range
         NULL_NODE_ID      = 0       ///< Unknown/No node id
      };
      // --------------------------------------------------------------------
      enum Restrictions {
         MAX_MESSAGE_LENGTH = 116    ///< Maximal number of bytes in payload
      };
      // --------------------------------------------------------------------
      iSenseRadioModel( isense::Os& os )
         : os_(os)
      {
         os_.dispatcher().add_receiver( this );
      }
      // --------------------------------------------------------------------
      virtual ~iSenseRadioModel()
      {}
      // --------------------------------------------------------------------
      int
      send( node_id_t id, size_t len, block_data_t *data )
      {
         os_.radio().send( id, len, data, 0, 0 );
         return SUCCESS;
      }
      // --------------------------------------------------------------------
      int enable_radio()
      {
         os_.radio().enable();
         return SUCCESS;
      }
      // --------------------------------------------------------------------
      int disable_radio()
      {
#ifndef ISENSE_ENABLE_REMOVE_RECEIVERS
   #warning ISENSE_ENABLE_REMOVE_RECEIVERS is not enabled. Add to be able to remove receiers.
#endif
         os_.radio().disable();
         return SUCCESS;
      }
      // --------------------------------------------------------------------
      node_id_t id()
      {
         return os_.id();
      }
      // --------------------------------------------------------------------
      template<class T, void (T::*TMethod)(node_id_t, size_t, block_data_t*)>
      int reg_recv_callback(T *obj_pnt )
      {
         int idx = get_free_receiver_item();
         if ( idx < 0 )
            return -1;

         isense_radio_callbacks_[idx] =
            isense_radio_delegate_t::template from_method<T, TMethod>( obj_pnt );

         return idx;
      }
      // --------------------------------------------------------------------
      void unreg_recv_callback(int idx )
      {
         isense_radio_callbacks_[idx] = isense_radio_delegate_t();
         //return SUCCESS;
      }
      // --------------------------------------------------------------------
#ifdef ISENSE_RADIO_ADDR_TYPE
         virtual void receive( uint8 len, const uint8 *buf,
            ISENSE_RADIO_ADDR_TYPE src_addr, ISENSE_RADIO_ADDR_TYPE dest_addr,
            uint16 signal_strength, uint16 signal_quality, uint8 sequence_no,
            uint8 interface, isense::Time rx_time )
#else
         virtual void receive( uint8 len, const uint8 *buf,
                               uint16 src_addr, uint16 dest_addr,
                               uint16 lqi, uint8 seq_no, uint8 interface )
#endif
      {
         for ( int i = 0; i < MAX_INTERNAL_RECEIVERS; i++ )
         {
            if ( isense_radio_callbacks_[i] )
               isense_radio_callbacks_[i]( src_addr, len, const_cast<uint8*>(buf) );
         }
      }

   private:
      int get_free_receiver_item()
      {
         for ( int i = 0; i < MAX_INTERNAL_RECEIVERS; i++ )
         {
            if ( !isense_radio_callbacks_[i] )
               return i;
         }
         return -1;
      }
      // --------------------------------------------------------------------
      int find_receiver_item( void *obj_ptr )
      {
         for ( int i = 0; i < MAX_INTERNAL_RECEIVERS; i++ )
         {
            if ( isense_radio_callbacks_[i].obj_ptr() == obj_ptr )
               return i;
         }
         return -1;
      }
      // --------------------------------------------------------------------
      // --------------------------------------------------------------------
      // --------------------------------------------------------------------
      isense::Os& os_;
      isense_radio_delegate_t isense_radio_callbacks_[MAX_INTERNAL_RECEIVERS];
   };
}

#endif
