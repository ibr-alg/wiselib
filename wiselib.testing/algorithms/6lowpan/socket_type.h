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
#ifndef __ALGORITHMS_6LOWPAN_SOCKET_TYPE_H__
#define __ALGORITHMS_6LOWPAN_SOCKET_TYPE_H__

#include "algorithms/6lowpan/ipv6_address.h"

namespace wiselib
{
	template<typename Radio_P>
	class LoWPANSocket
	{
	public:
		typedef Radio_P Radio;
		typedef typename Radio::node_id_t node_id_t;
		
		// -----------------------------------------------------------------
		LoWPANSocket()
			: local_port( 0 ),
			remote_port( 0 ),
			remote_host( Radio::NULL_NODE_ID ),
			callback_id( -1 )
			{}

		// -----------------------------------------------------------------

		LoWPANSocket(uint16_t local_port, uint16_t remote_port, node_id_t remote_host, int callback_id)
			: local_port( local_port ),
			remote_port( remote_port ),
			remote_host( remote_host ),
			callback_id( callback_id )
			{}
			
		/**
		* Local Port
		*/
		uint16_t local_port;
		/**
		* Remote Port
		*/
		uint16_t remote_port;
		/**
		* IPv6 Address of the rmoote host
		*/
		node_id_t remote_host;
		/**
		* Callback ID of the user application
		*/
		int callback_id;	  
	};

}
#endif
