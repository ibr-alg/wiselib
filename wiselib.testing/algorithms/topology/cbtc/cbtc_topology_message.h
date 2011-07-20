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
#ifndef __ALGORITHMS_TOPOLOGY_CBTC_TOPOLOGY_MESSAGE_H__
#define __ALGORITHMS_TOPOLOGY_CBTC_TOPOLOGY_MESSAGE_H__

#include "util/serialization/simple_types.h"

namespace wiselib
{

	template<typename OsModel_P,
			typename Radio_P>
	class CbtcTopologyMessage
	{
		public:
		
		typedef OsModel_P OsModel;
		typedef Radio_P Radio;
		typedef typename Radio::block_data_t block_data_t;
		typedef typename Radio::node_id_t node_id_t;
		
		uint8_t const static HELLO_SIZE = sizeof(uint8_t) + sizeof(int) + sizeof(double) * 2;
		uint8_t const static ACK_SIZE = sizeof(uint8_t) + sizeof(int) + sizeof(double) * 2;
		uint8_t const static ASYMMETRIC_SIZE = sizeof(uint8_t);
		uint8_t const static NDP_SIZE = sizeof(uint8_t) + sizeof(int) + sizeof(double) * 2;

		// --------------------------------------------------------------------
		inline CbtcTopologyMessage( uint8_t id );

		// --------------------------------------------------------------------
		inline uint8_t msg_id()
		{ return read<OsModel, block_data_t, uint8_t>( buffer ); }

		// --------------------------------------------------------------------
		inline void set_msg_id( uint8_t id )
		{ write<OsModel, block_data_t, uint8_t>( buffer, id ); }
      
		// --------------------------------------------------------------------
		inline int power()
		{ return (int)read<OsModel, block_data_t, int>( buffer + POWER_POS); }
		
		// --------------------------------------------------------------------
		inline void set_power( int power )
		{ write<OsModel, block_data_t, int>(buffer + POWER_POS, power); }
		
		// --------------------------------------------------------------------
		inline double position_x()
		{ return read<OsModel, block_data_t, double>(buffer + POSITIONX_POS);}
		
		// --------------------------------------------------------------------
		inline double position_y()
		{ return read<OsModel, block_data_t, double>(buffer + POSITIONY_POS);}
		
		// --------------------------------------------------------------------
		inline void set_position( double x, double y )
		{ 
			write<OsModel, block_data_t, double>(buffer + POSITIONX_POS, x);
			write<OsModel, block_data_t, double>(buffer + POSITIONY_POS, y);
		}

		private:

		enum data_positions
		{
			MSG_ID_POS  = 0,
			POWER_POS = MSG_ID_POS + sizeof(uint8_t),
			POSITIONX_POS = (POWER_POS + sizeof(int)),
			POSITIONY_POS = (POSITIONX_POS + sizeof(double))
		};

		uint8_t buffer[sizeof(uint8_t) + sizeof(int) + sizeof(double) * 2];
	};
	
	// -----------------------------------------------------------------------
	// -----------------------------------------------------------------------
	// -----------------------------------------------------------------------
	template<typename OsModel_P, typename Radio_P>
	CbtcTopologyMessage<OsModel_P, Radio_P>::
	CbtcTopologyMessage( uint8_t id )
	{
		set_msg_id( id );
	}
}
#endif
