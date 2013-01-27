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

namespace wiselib
{
	#ifndef __POSITION3D___
	#define __POSITION3D___
	template<	typename Os_P,
				typename Radio_P,
				typename CoordinatesNumber_P,
				typename Debug_P>
	class Position3DType
	{
	public:
		typedef Os_P Os;
		typedef Radio_P Radio;
		typedef CoordinatesNumber_P CoordinatesNumber;
		typedef Debug_P Debug;
		typedef typename Radio::block_data_t block_data_t;
		typedef typename Radio::size_t size_t;
		typedef Position3DType<Os, Radio, CoordinatesNumber, Debug> self_type;
		Position3DType()
		{}
		Position3DType( CoordinatesNumber _x, CoordinatesNumber _y, CoordinatesNumber _z)
		{
			x = _x;
			y = _y;
			z = _z;
		}
		Position3DType( const self_type& _p )
		{
			*this = _p;
		}
		inline block_data_t* serialize( block_data_t* _buff, size_t _offset = 0 )
		{
			size_t X_POS = 0;
			size_t Y_POS = X_POS + sizeof( CoordinatesNumber );
			size_t Z_POS = Y_POS + sizeof( CoordinatesNumber );
			write<Os, block_data_t, CoordinatesNumber>( _buff + X_POS + _offset, x );
			write<Os, block_data_t, CoordinatesNumber>( _buff + Y_POS + _offset, y );
			write<Os, block_data_t, CoordinatesNumber>( _buff + Z_POS + _offset, z );
			return _buff;
		}
		inline void de_serialize( block_data_t* _buff, size_t _offset = 0 )
		{
			size_t X_POS = 0;
			size_t Y_POS = X_POS + sizeof( CoordinatesNumber );
			size_t Z_POS = Y_POS + sizeof( CoordinatesNumber );
			x = read<Os, block_data_t, CoordinatesNumber>( _buff + X_POS + _offset );
			y = read<Os, block_data_t, CoordinatesNumber>( _buff + Y_POS + _offset );
			z = read<Os, block_data_t, CoordinatesNumber>( _buff + Z_POS + _offset );
		}
		inline size_t serial_size()
		{
			size_t X_POS = 0;
			size_t Y_POS = X_POS + sizeof( CoordinatesNumber );
			size_t Z_POS = Y_POS + sizeof( CoordinatesNumber );
			return Z_POS + sizeof( CoordinatesNumber );
		}
		inline self_type& operator=( const self_type& _p )
		{
			x = _p.x;
			y = _p.y;
			z = _p.z;
			return *this;
		}
		inline void set_x( const CoordinatesNumber& _x)
		{
			x = _x;
		}
		inline void set_y( const CoordinatesNumber& _y)
		{
			y = _y;
		}
		inline void set_z( const CoordinatesNumber& _z)
		{
			z = _z;
		}
		inline void set_all(const CoordinatesNumber& _x, const CoordinatesNumber& _y, const CoordinatesNumber& _z)
		{
			x = _x;
			y = _y;
			z = _z;
		}
		inline CoordinatesNumber get_x()
		{
			return x;
		}
		inline CoordinatesNumber get_y()
		{
			return y;
		}
		inline CoordinatesNumber get_z()
		{
			return z;
		}
		inline CoordinatesNumber distsq( const self_type& _p )
		{
			const register CoordinatesNumber inc_x = x - _p.x;
			const register CoordinatesNumber inc_y = y - _p.y;
			const register CoordinatesNumber inc_z = z - _p.z;
			return inc_x*inc_x + inc_y*inc_y + inc_z*inc_z;
		}
		inline void print( Debug& debug, Radio& radio )
		{
			debug.debug("Position3D (size %i) : ( %d, %d, %d )", sizeof(x) + sizeof(y) + sizeof(z), x, y, z);
		}
	private:
		CoordinatesNumber x, y, z;
	};
	#endif

	#ifndef __POSITION2D___
	#define __POSITION2D___
	template<	typename Os_P,
				typename Radio_P,
				typename CoordinatesNumber_P,
				typename Debug_P>
	class Position2DType
	{
	public:
		typedef Os_P Os;
		typedef Radio_P Radio;
		typedef CoordinatesNumber_P CoordinatesNumber;
		typedef Debug_P Debug;
		typedef typename Radio_P::block_data_t block_data_t;
		typedef typename Radio_P::size_t size_t;
		typedef Position2DType<Os, Radio, CoordinatesNumber, Debug> self_type;
		Position2DType()
		{}
		Position2DType( const CoordinatesNumber& _x, const CoordinatesNumber _y, const CoordinatesNumber _z = 1)
		{
			x = _x;
			y = _y;
		}
		Position2DType( const self_type& _p )
		{
			*this = _p;
		}
		inline block_data_t* serialize( block_data_t* _buff, size_t _offset = 0 )
		{
			uint8_t X_POS = 0;
			uint8_t Y_POS = X_POS + sizeof( CoordinatesNumber );
			write<Os, block_data_t, CoordinatesNumber>( _buff + X_POS + _offset, x );
			write<Os, block_data_t, CoordinatesNumber>( _buff + Y_POS + _offset, y );
			return _buff;
		}
		inline void de_serialize( block_data_t* _buff, size_t _offset = 0 )
		{
			uint8_t X_POS = 0;
			uint8_t Y_POS = X_POS + sizeof( CoordinatesNumber );
			x = read<Os, block_data_t, CoordinatesNumber>( _buff + X_POS + _offset );
			y = read<Os, block_data_t, CoordinatesNumber>( _buff + Y_POS + _offset );
		}
		inline size_t serial_size()
		{
			uint8_t X_POS = 0;
			uint8_t Y_POS = X_POS + sizeof( CoordinatesNumber );
			return Y_POS + sizeof( CoordinatesNumber );
		}
		inline self_type& operator=( const self_type& _p )
		{
			x = _p.x;
			y = _p.y;
			return *this;
		}
		inline void set_x( const CoordinatesNumber& _x)
		{
			x = _x;
		}
		inline void set_y( const CoordinatesNumber& _y)
		{
			y = _y;
		}
		inline void set_z( const CoordinatesNumber& _z = 1 )
		{
		}
		inline void set_all( const CoordinatesNumber& _x, const CoordinatesNumber& _y, const CoordinatesNumber& _z = 1 )
		{
			x = _x;
			y = _y;
		}
		inline CoordinatesNumber get_x()
		{
			return x;
		}
		inline CoordinatesNumber get_y()
		{
			return y;
		}
		inline CoordinatesNumber get_z()
		{
			return 1;
		}
		inline CoordinatesNumber distsq( const self_type& _p )
		{
			const register CoordinatesNumber inc_x = x - _p.x;
			const register CoordinatesNumber inc_y = y - _p.y;
			return inc_x*inc_x + inc_y*inc_y;
		}
		inline void print( Debug& _debug, Radio& _radio )
		{
			_debug.debug("Position2D (size %i) : ( %d, %d )\n", sizeof(x) + sizeof(y), x, y );
		}
	private:
		CoordinatesNumber x, y;
	};
	#endif
}
