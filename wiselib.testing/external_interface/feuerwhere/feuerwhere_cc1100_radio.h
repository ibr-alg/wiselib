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
#ifndef CONNECTOR_FEUERWHERE_CC1100_RADIOMODEL_H
#define CONNECTOR_FEUERWHERE_CC1100_RADIOMODEL_H

#include "external_interface/feuerwhere/feuerwhere_types.h"
#include "util/delegates/delegate.hpp"
#include <string.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
//#include <iostream>
//#include <sstream>


extern "C" {
#include "board.h"
#include "cc1100.h"
//#include "cc1100-interface.h" already included in cc1100.h
}

enum SpecialNodeIds {
  WISELIB_PROTOCOL = 2,
  WISELIB_PRIORITY = 3
};

namespace wiselib
{

   typedef delegate3<void, uint8_t, uint8_t, char*> feuerwhere_radio_delegate_t;
   typedef feuerwhere_radio_delegate_t radio_delegate_t;
   // -----------------------------------------------------------------------
   void init_feuerwhere_cc1100_radio( void );
   int feuerwhere_radio_add_receiver( feuerwhere_radio_delegate_t& delegate );
   void feuerwhere_radio_del_receiver( int idx );
   void feuerwhere_notify_receivers( struct abc_conn *c );
   // -----------------------------------------------------------------------
   // -----------------------------------------------------------------------
   // -----------------------------------------------------------------------
   /** \brief Feuerwhere Implementation of \ref radio_concept "Radio concept"
    *  \ingroup radio_concept
    *
    * Feuerwhere implementation of the \ref radio_concept "Radio concept" ...
    */
   template<typename OsModel_P>
   class FeuerwhereRadioModel
   {
   public:
      typedef OsModel_P OsModel;

      typedef FeuerwhereRadioModel<OsModel> self_type;
      typedef self_type* self_pointer_t;

      typedef uint8_t node_id_t;
      typedef char  block_data_t;
      typedef uint8_t  size_t;
      typedef uint8_t  message_id_t;
      //static void protocol_handler(void* msg, int msg_size, packet_info_t* packet_info);
      // --------------------------------------------------------------------
      enum ErrorCodes
      {
	/*
         SUCCESS = OsModel::SUCCESS,
	 	 ERROR_MNF=CMD_ERROR,
         ERR_UNSPEC = OsModel::ERR_UNSPEC,
         ERR_NOTIMPL = OsModel::ERR_NOTIMPL
	*/

         SUCCESS = OsModel::SUCCESS,
         ERR_UNSPEC = OsModel::ERR_UNSPEC,
         ERR_NOTIMPL = OsModel::ERR_NOTIMPL

      };
      // --------------------------------------------------------------------
      ///BROADCAST_ADDRESS = CC1100_BROADCAST_ADDRESS, ///< All nodes in communication range
      enum SpecialNodeIds {
         BROADCAST_ADDRESS = 0x00, ///< All nodes in communication range
         NULL_NODE_ID      = -1    ///< Unknown/No node id
      };
      // --------------------------------------------------------------------
      enum Restrictions {
         // max is 58
         MAX_MESSAGE_LENGTH = 57 ///< Maximal number of bytes in payload
      };
      // --------------------------------------------------------------------
      int send( node_id_t id, size_t len, block_data_t *data )
      {
    	  printf("------- sending to %u: %s (%u chars)\n", id, data, len);
    	  //int res = cc1100_send_csmaca(id, WISELIB_PROTOCOL, PRIORITY_DATA, (char*)data, len);
    	  int res = cc1100_send_csmaca(id, WISELIB_PROTOCOL, PRIORITY_DATA, data, len);

    	  if (res > 0) {
    		  printf("needed %u retransmissions\r\n", rflags.RTC);
    		  return SUCCESS;
    	  } else {
    		  if (res == RADIO_CS_TIMEOUT) {
    			  printf("CS timeout reached, air was never free!\r\n");
    		  }
    		  return ERR_UNSPEC;
    	  }
      }
      // --------------------------------------------------------------------
      int enable_radio()
      {
        //return ERR_NOTIMPL;
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
	 //radio_address_t cc1100_get_address(void)
	 //{
	//	return radio_address;
	 //}
//TODO DONE 
//       return radio_cc1100->get_address();
         //return ERR_NOTIMPL;
    	 // printf ("The node id is %d\n", cc1100_get_address());
    	  return cc1100_get_address();
      }
      // --------------------------------------------------------------------
      template<class T, void (T::*TMethod)(node_id_t, size_t, block_data_t*)>
      int reg_recv_callback( T *obj_pnt );
      // --------------------------------------------------------------------
      int unreg_recv_callback( int idx )
      {
         feuerwhere_radio_del_receiver( idx );
         return SUCCESS;
      }
   };
   // -----------------------------------------------------------------------
   // -----------------------------------------------------------------------
   // -----------------------------------------------------------------------
   template<typename OsModel_P>
   template<class T,
            void (T::*TMethod)(
                      typename FeuerwhereRadioModel<OsModel_P>::node_id_t,
                      typename FeuerwhereRadioModel<OsModel_P>::size_t,
                      typename FeuerwhereRadioModel<OsModel_P>::block_data_t*)>
   int
   FeuerwhereRadioModel<OsModel_P>::
   reg_recv_callback( T *obj_pnt )
   {
      feuerwhere_radio_delegate_t delegate =
    		  feuerwhere_radio_delegate_t::from_method<T, TMethod>( obj_pnt );
      return feuerwhere_radio_add_receiver( delegate );
   }
}

#endif
