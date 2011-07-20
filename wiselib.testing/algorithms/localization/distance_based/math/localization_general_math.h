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
#ifndef __ALGORITHMS_LOCALIZATION_DISTANCE_BASED_MATH_GENERAL_MATH_H
#define __ALGORITHMS_LOCALIZATION_DISTANCE_BASED_MATH_GENERAL_MATH_H

#include "algorithms/localization/distance_based/util/localization_defutils.h"
#include "algorithms/localization/distance_based/math/vec.h"
#include <math.h>
#include <limits.h>

namespace wiselib
{

   /** To decide, whether a triangle is collinear or not, the sum of the two
    *  smallest sides minus the largest side is formed. To take account
    *  ranging errors, additionally the largest side is multiplied by
    *  1 plus a threshold, which is given as the fourth parameter.
    *
    *  Resulting formula looks like following:
    *  \f[ a + b - ( 1 + threshold ) * c \f]
    *
    *  The greater the resulting measure is, the fewer collinear the
    *  triangle is.
    *
    * \param Arithmatic_P length of side of a triangle
    * \param Arithmatic_P length of side of a triangle
    * \param Arithmatic_P length of side of a triangle
    * \param Arithmatic_P above mentioned threshold
    * \return Measure of collinearity of the three given distances
    *  with respect to the given error
    * \sa is_collinear()
    */
template <typename Arithmatic_P>
   static Arithmatic_P collinear_measure( Arithmatic_P, Arithmatic_P, Arithmatic_P, Arithmatic_P = 0 );
   /** This methode forms the collinear_measure() and decides, whether
    *  the triangle is collinear or not.
    *
    * \param Arithmatic_P length of side of a triangle
    * \param Arithmatic_P length of side of a triangle
    * \param Arithmatic_P length of side of a triangle
    * \param Arithmatic_P above mentioned threshold
    * \return \c true, if collinear_measure() is less or equal 0.
    * \c false otherwise.
    * \sa collinear_measure()
    */
template <typename Arithmatic_P>
   static bool is_collinear ( Arithmatic_P, Arithmatic_P, Arithmatic_P, Arithmatic_P = 0 );
   /** This method rotates a given coordinate by a given radius.
    *
    *  \param Arithmatic_P angle
    *  \param shawn::Vec<Arithmatic_P>& coordinate to be rotated.
    *  \param shawn::Vec<Arithmatic_P>& coordinate to rotate around (std: (0,0,0)).
    */
template <typename Arithmatic_P>
   static void rotate_2D( Arithmatic_P, Vec<Arithmatic_P>&, const Vec<Arithmatic_P>& = Vec<Arithmatic_P>( 0.0, 0.0, 0.0 ) );
   /** This method computes the angle of a vector in respect to the x-axis.
    *
    *  \param shawn::Vec<Arithmatic_P>& Vector
    *  \result angle
    */
template <typename Arithmatic_P>
   static Arithmatic_P angle_vec( const Vec<Arithmatic_P>& );
   /** This method sets the angle between 0 and 2*PI.
    *
    *  \param doublke angle
    *  \result angle
    */
template <typename Arithmatic_P>
   static Arithmatic_P normalize_angle( Arithmatic_P );
   // -----------------------------------------------------------------------
   // -----------------------------------------------------------------------
   // -----------------------------------------------------------------------
template <typename Arithmatic_P>
   Arithmatic_P
   collinear_measure( Arithmatic_P a, Arithmatic_P b, Arithmatic_P c, Arithmatic_P error )
   {
      if ( a <= c && b <= c )
      {
         return a + b - ( 1 + error ) * c;
      }
      else if ( a <= b && c <= b )
      {
         return a + c - ( 1 + error ) * b;
      }
      else if ( b <= a && c <= a )
      {
         return b + c - ( 1 + error ) * a;
      }

      return DBL_MIN;
   }
   // ----------------------------------------------------------------------
template <typename Arithmatic_P>
   inline bool
   is_collinear( Arithmatic_P a, Arithmatic_P b, Arithmatic_P c, Arithmatic_P error )
   {
      return collinear_measure( a, b, c, error ) <= 0;
   }
   // ----------------------------------------------------------------------
template <typename Arithmatic_P>
   void
   rotate_2D( Arithmatic_P angle, Vec<Arithmatic_P>& coord, const Vec<Arithmatic_P>& origin )
   {
      Arithmatic_P cos_angle = cos( angle );
      Arithmatic_P sin_angle = sin( angle );

      coord = Vec<Arithmatic_P>(
         ((coord.x() - origin.x()) * cos_angle) - ((coord.y() - origin.y()) * sin_angle) + origin.x(),
         ((coord.y() - origin.y()) * cos_angle) + ((coord.x() - origin.x()) * sin_angle) + origin.y(),
         coord.z() );
   }
   // ----------------------------------------------------------------------
template <typename Arithmatic_P>
   Arithmatic_P
   angle_vec( const Vec<Arithmatic_P>& vec )
   {
      if ( vec.x() == 0 && vec.y() >= 0 )
         return 0.5 * M_PI;
      else if ( vec.x() == 0 && vec.y() < 0 )
         return 1.5 * M_PI;

      if ( vec.y() == 0 && vec.x() >= 0 )
         return 0;
      else if ( vec.y() == 0 && vec.x() < 0 )
         return M_PI;

      if ( vec.y() >= 0 )
         return acos( vec.x() / vec.euclidean_norm() );
      else
         return 2.0*M_PI - acos( vec.x() / vec.euclidean_norm() );
   }
   // ----------------------------------------------------------------------
template <typename Arithmatic_P >
   Arithmatic_P
   normalize_angle( Arithmatic_P angle )
   {
      while ( angle < 0 ) angle += 2 * M_PI;
      while ( angle >= (2 * M_PI) ) angle -= (2 * M_PI);

      return angle;
   }


}// namespace wiselib
#endif
