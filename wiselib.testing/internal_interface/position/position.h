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
#ifndef __INTERNAL_INTERFACE_POSITION___
#define __INTERNAL_INTERFACE_POSITION___

#include <math.h> // NOTE: required for sqrt
#include "util/serialization/serialization.h"
namespace wiselib
{

	template<typename Float_P,typename block_data_P, typename OsModel_P>
	struct PositionType
	{
		typedef Float_P Float;
		typedef block_data_P block_data;
		typedef OsModel_P OsModel;

		Float x, y, z;

		PositionType()
		{}

		PositionType( const Float& a, const Float& b, const Float& c )
		: x  ( a ),
		  y ( b ),
		  z ( c )
		{}

		PositionType( block_data* buff )
		{
			get_Position3D_from_buffer( buff );
		}

		enum Position_Positions
		{
			X_POS = 0,
			Y_POS = X_POS + sizeof(Float),
			Z_POS = Y_POS + sizeof(Float)
		};

		inline block_data* set_buffer_from_Position2D( block_data* buff, size_t offset = 0)
		{
			write<OsModel, block_data, Float>( buff + X_POS + offset, x );
			write<OsModel, block_data, Float>( buff + Y_POS + offset, y );
			return buff;
		}

		inline block_data* set_buffer_from_Position3D( block_data* buff, size_t offset = 0 )
		{
			write<OsModel, block_data, Float>( buff + X_POS + offset, x );
			write<OsModel, block_data, Float>( buff + Y_POS + offset, y );
			write<OsModel, block_data, Float>( buff + Z_POS + offset, z );
			return buff;
		}

		inline void get_Position2D_from_buffer( block_data* buff, size_t offset = 0 )
		{
			x = read<OsModel, block_data, Float>( buff + X_POS + offset );
			y = read<OsModel, block_data, Float>( buff + Y_POS + offset );
		}

		inline void get_Position3D_from_buffer( block_data* buff, size_t offset = 0 )
		{
			x = read<OsModel, block_data, Float>( buff + X_POS + offset );
			y = read<OsModel, block_data, Float>( buff + Y_POS + offset );
			z = read<OsModel, block_data, Float>( buff + Z_POS + offset );
		}

		inline size_t  get_buffer_size_2D(){return 2*sizeof( Float );}

		inline size_t  get_buffer_size_3D(){return 3*sizeof( Float );}

	};

	template<typename Float, typename block_data, typename Os>
	Float dist(const PositionType<Float, block_data, Os> p1, const PositionType<Float, block_data, Os> p2)
	{
		const register Float inc_x = p1.x - p2.x;
		const register Float inc_y = p1.y - p2.y;
		const register Float inc_z = p1.z - p2.z;
		return sqrt( inc_x*inc_x + inc_y*inc_y + inc_z*inc_z );
	}

	// NOTE: square of euclidean distance is not a distance!
	template<typename Float, typename block_data, typename Os>
	Float distsq(const PositionType<Float, block_data, Os> p1, const PositionType<Float, block_data, Os> p2)
	{
		const register Float inc_x = p1.x - p2.x;
		const register Float inc_y = p1.y - p2.y;
		const register Float inc_z = p1.z - p2.z;
		return inc_x*inc_x + inc_y*inc_y + inc_z*inc_z;
	}

}
#endif
