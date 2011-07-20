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
#ifndef CONNECTOR_TRISOS_RADIOMODEL_H
#define CONNECTOR_TRISOS_RADIOMODEL_H

#include "external_interface/trisos/trisos_types.h"
#include "util/delegates/delegate.hpp"
#include "util/serialization/simple_types.h"
#include <string.h>
extern "C" {
#include "src/sys/sys.h"
#include "src/hw/rf/rf.h"
}

namespace wiselib
{

   typedef delegate3<void, uint16_t, uint8_t, uint8_t*> trisos_radio_delegate_t;
   // -----------------------------------------------------------------------
   typedef void (*trisos_radio_fp)( uint8_t data_length, uint8_t *data );
   extern trisos_radio_fp trisos_internal_radio_callback;
   // -----------------------------------------------------------------------
   void init_trisos_radio( void );
   int  trisos_radio_add_receiver( trisos_radio_delegate_t& delegate );
   void trisos_radio_del_receiver( int idx );
   void trisos_notify_receivers( uint8_t, uint8_t* );
   // -----------------------------------------------------------------------
   // -----------------------------------------------------------------------
   // -----------------------------------------------------------------------
   /** \brief TriSOS Implementation of \ref radio_concept "Radio concept"
    *  \ingroup radio_concept
    *  \ingroup trisos_facets
    *
    * TriSOS implementation of the \ref radio_concept "Radio concept" ...
    */
	template<typename OsModel_P>
	class TriSOSRadio
	{
	public:
		typedef OsModel_P OsModel;

		typedef TriSOSRadio<OsModel> self_type;
		typedef self_type* self_pointer_t;

		typedef uint16_t node_id_t;
		typedef uint8_t  block_data_t;
		typedef uint8_t  size_t;
		typedef uint8_t  message_id_t;

		typedef trisos_radio_delegate_t radio_delegate_t;
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
			MAX_MESSAGE_LENGTH = 255 ///< Maximal number of bytes in payload
		};
		// --------------------------------------------------------------------
		void init()
		{
			init_trisos_radio();
		}
		// --------------------------------------------------------------------
		int send( node_id_t id, size_t len, block_data_t *data );
		// --------------------------------------------------------------------
		int enable_radio()
		{
			// TODO: Also HW enable
			trisos_internal_radio_callback = trisos_notify_receivers;
			return SUCCESS;
		}
		// --------------------------------------------------------------------
		int disable_radio()
		{
			// TODO: Also HW disable
			trisos_internal_radio_callback = 0;
			return SUCCESS;
		}
		// --------------------------------------------------------------------
		node_id_t id()
		{
			uint16_t addr = sys_get_id();
			return addr;
		}
		// --------------------------------------------------------------------
		template<class T, void (T::*TMethod)(node_id_t, size_t, block_data_t*)>
		int reg_recv_callback( T *obj_pnt );
		// --------------------------------------------------------------------
		int unreg_recv_callback( int idx )
		{
			trisos_radio_del_receiver( idx );
			return SUCCESS;
		}
	};
	// --------------------------------------------------------------------
	// -----------------------------------------------------------------------
	// -----------------------------------------------------------------------
	template<typename OsModel_P>
	int
	TriSOSRadio<OsModel_P>::
	send( node_id_t dest, size_t len, block_data_t *data )
	{
		if( !trisos_internal_radio_callback || 		// Radio enabled?
			len <= 0 ) 								// Parameter sanity check						
		{
			return ERR_UNSPEC;
		}

		uint8_t buf[MAX_MESSAGE_LENGTH];

		// wirte own id and destination in first 4 bytes of buffer
		uint16_t addr = id();
		write<OsModel, block_data_t, node_id_t>( buf, addr );
		write<OsModel, block_data_t, node_id_t>( buf + sizeof(node_id_t), dest );
		// write payload
		memcpy( buf + 2*sizeof(node_id_t), data, len );

		rf_state_type state = rf_send_data( 2 * sizeof(node_id_t) + len, buf );

		if( state == rf_SUCCESS )
		{
			return SUCCESS;
		}
		else
		{
			return ERR_UNSPEC;
		}
	}
	// -----------------------------------------------------------------------
	template<typename OsModel_P>
	template<class T,
	        void (T::*TMethod)( typename TriSOSRadio<OsModel_P>::node_id_t,
	                            typename TriSOSRadio<OsModel_P>::size_t,
	                            typename TriSOSRadio<OsModel_P>::block_data_t*)>
	int
	TriSOSRadio<OsModel_P>::
	reg_recv_callback( T *obj_pnt )
	{
		trisos_radio_delegate_t delegate =
			trisos_radio_delegate_t::from_method<T, TMethod>( obj_pnt );
		return trisos_radio_add_receiver( delegate );
	}
}

#endif
