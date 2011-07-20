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
#ifndef __GEOMETRY_H__
#define __GEOMETRY_H__

#define _USE_MATH_DEFINES
#include <cmath>
#include <cstdlib>

using namespace std;
namespace wiselib
{
	template<typename NodePosition_P>
	class Geometry
	{
	public:
		typedef NodePosition_P NodePosition;
		typedef typename NodePosition::Float Number;
		
		Geometry() {}
		// --------------------------------------------------------------------
		~Geometry() {}
		// --------------------------------------------------------------------
		inline NodePosition trilateration(Number x1, Number y1, Number R1, Number x2, Number y2, Number R2, Number x3, Number y3, Number R3)
		{
			Number x_one, x_two, y_one, y_two;
			poly_points(x1, y1, R1, x2, y2, R2, x_one, y_one, x_two, y_two);
			if ( fabs((R3*R3) - ((x3-x_one)*(x3-x_one) + (y3-y_one)*(y3-y_one))) < fabs((R3*R3) - ((x3-x_two)*(x3-x_two) + (y3-y_two)*(y3-y_two))) )
			{
				return NodePosition(x_one, y_one, 0);
			}
			else
			{
				return NodePosition(x_two, y_two, 0);
			}
		}
		// --------------------------------------------------------------------
		inline void poly_points(Number x1, Number y1, Number R1, Number x2, Number y2, Number R2, Number& x_one, Number& y_one, Number& x_two, Number& y_two)
		{
			Number ALPHA, BETA, a,b,c,d;
			if ( x1 == x2 )
			{
				ALPHA = BETA = (y1*y1 - y2*y2 - R1*R1 + R2*R2)/(2*(y1-y2));
				a = 1;
				b = (-2*x1);
				c = (x1*x1 + y1*y1 + ALPHA*ALPHA - 2*y1*ALPHA - R1*R1);
				d = b*b -4*a*c;
				y_one = ALPHA;
				y_two = ALPHA;
				x_one = (-b-sqrt(d))/(2*a);
				x_two = (-b+sqrt(d))/(2*a);
			}
			else
			{
				ALPHA = (x1*x1 - x2*x2 + y1*y1 - y2*y2 -R1*R1 + R2*R2)/(2*(x1-x2));
				BETA = - (y1-y2)/(x1-x2);
				a = (BETA*BETA+1);
				b = (2*ALPHA*BETA -2*x1*BETA -2*y1);
				c = (x1*x1 + ALPHA*ALPHA - 2*x1*ALPHA + y1*y1 - R1*R1);
				d = b*b - 4*a*c;
				y_one = (-b-sqrt(d))/(2*a);
				y_two = (-b+sqrt(d))/(2*a);
				x_one = ALPHA + BETA*y_one;
				x_two = ALPHA + BETA*y_two;
			}
		}
		// --------------------------------------------------------------------
	};

}
#endif
