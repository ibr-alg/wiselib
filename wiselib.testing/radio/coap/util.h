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

#ifndef UTIL_H_
#define UTIL_H_

namespace wiselib {

enum path_compare
{
	EQUAL,
	LHS_IS_SUBRESOURCE,
	RHS_IS_SUBRESOURCE,
	NOT_EQUAL
};

template<typename string_t>
int path_cmp(const string_t &lhs, const string_t &rhs)
{
	for( size_t i = 0; ; ++i )
	{
		if( i == lhs.length() )
		{
			if( i == rhs.length() )
				return EQUAL;
			else if( rhs[i] == '/' )
				return RHS_IS_SUBRESOURCE;
			else
				return NOT_EQUAL;
		}
		if( i == rhs.length() )
		{
			if( lhs[i] == '/' )
				return LHS_IS_SUBRESOURCE;
			else
				return NOT_EQUAL;
		}

		if( lhs[i] != rhs[i] )
			return NOT_EQUAL;
	}
}

}



#endif /* UTIL_H_ */
