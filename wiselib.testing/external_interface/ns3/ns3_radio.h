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
#ifndef CONNECTOR_NS3_RADIOMODEL_H
#define CONNECTOR_NS3_RADIOMODEL_H

#include "external_interface/ns3/ns3_types.h"

namespace wiselib
{

   /** \brief Ns3 Implementation of \ref radio_concept "Radio concept".
    *  \ingroup radio_concept
    *  \ingroup shawn_facets
    *
    * Ns3 implementation of the \ref radio_concept "Radio concept" ...
    *
    * @tparam OsModel_P Has to implement @ref os_concept "Os concept".
    */
   template<typename OsModel_P>
   class Ns3RadioModel
   {
   public:
      typedef OsModel_P OsModel;

      typedef Ns3RadioModel<OsModel> self_type;
      typedef self_type* self_pointer_t;

      typedef WiselibExtIface::node_id_t node_id_t;
      typedef WiselibExtIface::block_data_t block_data_t;
      typedef WiselibExtIface::size_t size_t;
      typedef WiselibExtIface::message_id_t message_id_t;

      // extended data facet support
      typedef ExtendedDataClass ExtendedData;  // extended data class in NS-3
 
      // tx power facet support
      typedef TxPowerClass TxPower;
      // --------------------------------------------------------------------
      enum ErrorCodes
      {
         SUCCESS = OsModel::SUCCESS,
         ERR_UNSPEC = OsModel::ERR_UNSPEC,
         ERR_NOTIMPL = OsModel::ERR_NOTIMPL
      };
      // --------------------------------------------------------------------
      enum SpecialNodeIds {
         BROADCAST_ADDRESS = 0xffffffffU, ///< All nodes in communication rnage
         NULL_NODE_ID      = -1      ///< Unknown/No node id
      };
      // --------------------------------------------------------------------
      enum Restrictions {
         MAX_MESSAGE_LENGTH = 0xff   ///< Maximal number of bytes in payload
      };
      // --------------------------------------------------------------------
      Ns3RadioModel( Ns3Os& os )
         : os_(os),
           node_id_(0)
      {}
      // --------------------------------------------------------------------
      int send( node_id_t id, size_t len, block_data_t *data )
      {
         os().proc.SendWiselibMessage( id, len, data, node_id_);
         return SUCCESS;
      };
      // --------------------------------------------------------------------
      int enable_radio()
      {
        node_id_ = os().proc.EnableRadio (); 
        return SUCCESS; 
      }
      // --------------------------------------------------------------------
      int disable_radio()
      { return SUCCESS; }
      // --------------------------------------------------------------------
      node_id_t id()
      {
         return node_id_;
      }
      // --------------------------------------------------------------------
      template<class T, void (T::*TMethod)(node_id_t, size_t, block_data_t*)>
      int reg_recv_callback( T *obj_pnt )
      {
         if (os().proc.template RegRecvCallback<T, TMethod>( obj_pnt, node_id_))
            return SUCCESS;

         return ERR_UNSPEC;
      }
      // --------------------------------------------------------------------
      // extended data radio facet
      template<class T, void (T::*TMethod)(node_id_t, size_t, block_data_t*, ExtendedData*)>
      int reg_recv_callback( T *obj_pnt )
      {
         if (os().proc.template RegExtendedDataRecvCallback<T, TMethod>( obj_pnt, node_id_))
            return SUCCESS;

         return ERR_UNSPEC;
      }
     
      // tx power radio facet
      void set_power (TxPower &p)
      {
        os().proc.SetTxPower (p, node_id_);
      }
 
      TxPower power ()
      {
        return os().proc.GetTxPower (node_id_);
      }     
      // --------------------------------------------------------------------
      int unreg_recv_callback( int idx )
      { return ERR_NOTIMPL; }

   private:
      Ns3Os& os()
      { return os_; }
      // --------------------------------------------------------------------
      Ns3Os& os_;

      node_id_t node_id_; // the id of the node of radio located
   };
}

#endif
