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
#ifndef CONNECTOR_SHAWN_RADIOMODEL_H
#define CONNECTOR_SHAWN_RADIOMODEL_H
 
#include "external_interface/shawn/shawn_types.h"
#include "util/pstl/vector_static.h"
 
namespace wiselib
{
 
   /** \brief Shawn Implementation of \ref radio_concept "Radio concept".
    *  \ingroup radio_concept
    *  \ingroup shawn_facets
    *
    * Shawn implementation of the \ref radio_concept "Radio concept" ...
    *
    * @tparam OsModel_P Has to implement @ref os_concept "Os concept".
    */
   template<typename OsModel_P>
   class ShawnRadioModel
   {
   public:

      typedef OsModel_P OsModel;
 
      typedef ShawnRadioModel<OsModel> self_type;
      typedef self_type* self_pointer_t;
 
      typedef ExtIfaceProcessor::node_id_t node_id_t;
      typedef ExtIfaceProcessor::block_data_t block_data_t;
      typedef ExtIfaceProcessor::size_t size_t;
      typedef ExtIfaceProcessor::message_id_t message_id_t;
      typedef struct s
      {
             ShawnOs* object;
             int flag;
             int ID;
             s()
             {
                 flag=0;
             }
      }z;
      
      typedef delegate3<void, int, long, unsigned char*> radio_delegate_t;
      // --------------------------------------------------------------------
      enum ErrorCodes
      {
         SUCCESS = OsModel::SUCCESS,
         ERR_UNSPEC = OsModel::ERR_UNSPEC,
         ERR_NOTIMPL = OsModel::ERR_NOTIMPL
      };
      // --------------------------------------------------------------------
      enum SpecialNodeIds {
         BROADCAST_ADDRESS = 0xffff, ///< All nodes in communication rnage
         NULL_NODE_ID      = -1      ///< Unknown/No node id
      };
      // --------------------------------------------------------------------
      enum Restrictions {
         MAX_MESSAGE_LENGTH = 0xff   ///< Maximal number of bytes in payload
      };
      // --------------------------------------------------------------------
      enum MAX_INTERNAL_RECEIVERS {
         MAX_INTERNAL_RECEIVERS=40   ///Maximum number of recievers
      };
      // --------------------------------------------------------------------    
      ShawnRadioModel( ShawnOs& os )
         : os_(os)
      {}
      // --------------------------------------------------------------------
      int send( node_id_t id, size_t len, block_data_t *data )
      {
         os().proc->send_wiselib_message( id, len, data );
         return SUCCESS;
      };
      // --------------------------------------------------------------------
      int enable_radio()
      { return SUCCESS; }
      // --------------------------------------------------------------------
      int disable_radio()
      { return SUCCESS; }
      // --------------------------------------------------------------------
      node_id_t id()
      {
         return os().proc->id();
      }
      // --------------------------------------------------------------------
      template<class T, void (T::*TMethod)(node_id_t, size_t, block_data_t*)>
      int reg_recv_callback( T *obj_pnt )
      {            
    if (os().proc->template reg_recv_callback<T, TMethod>( obj_pnt ))
         return SUCCESS;
 
         return ERR_UNSPEC;
      }
      // --------------------------------------------------------------------
      int unreg_recv_callback(int idx)
      {
          return ERR_NOTIMPL;
      }

      int register_radio(struct s& callback_struct)
      {
	    receivers.ID.push_back(callback_struct.ID);
            receivers.object.push_back(callback_struct.object);
            callback_struct.flag=1;

	    return SUCCESS;
      }
      
      int unregister_radio(struct s& callback_struct)
      { 
	if(receivers.erase(receivers.ID.find(callback_struct.ID)))
	  return SUCCESS;

	else
	  return ERR_UNSPEC;
	   
      }

   private:
      ShawnOs& os()
      { return os_; }
      // --------------------------------------------------------------------
      ShawnOs& os_;
      vector_static <OsModel,s,MAX_INTERNAL_RECEIVERS> receivers;
   };
}
 
#endif
