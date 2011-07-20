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
#ifndef __ALGORITHMS_LOCALIZATION_DISTANCE_BASED_MATH_VEC_H
#define __ALGORITHMS_LOCALIZATION_DISTANCE_BASED_MATH_VEC_H

#include <math.h>

namespace wiselib
{

    template<typename Arithmatic_P = double>
   class Vec
   {
//       static const Arithmatic epsilon;

   public:
    	typedef Arithmatic_P Arithmatic;
    	typedef Vec<Arithmatic> self_type;
      // --------------------------------------------------------------------
      inline Vec( const self_type& p )
         : x_(p.x()), y_(p.y()), z_(p.z())
      {}
      inline Vec( Arithmatic x=0.0, Arithmatic y=0.0, Arithmatic z=0.0 )
         : x_(x), y_(y), z_(z)
      {}
      // --------------------------------------------------------------------
      inline Arithmatic x( void ) const
      { return x_; }

      inline Arithmatic y( void ) const
      { return y_; }

      inline Arithmatic z( void ) const
      { return z_; }
      // --------------------------------------------------------------------
      inline self_type operator - ( const self_type& p ) const
      { return Vec( x()-p.x(), y()-p.y(), z()-p.z() ); }
      inline self_type operator + ( const self_type& p ) const
      { return self_type( x()+p.x(), y()+p.y(), z()+p.z() ); }

      inline self_type& operator -= ( const self_type& p )
      { return *this = (*this-p); }
      inline self_type& operator += ( const self_type& p )
      { return *this = (*this+p); }

      inline self_type operator * ( Arithmatic f ) const
      { return self_type( f*x(), f*y(), f*z() ); }
      inline self_type operator / ( Arithmatic f ) const
      { return self_type( x()/f, y()/f, z()/f ); }

      inline self_type& operator *= ( Arithmatic f )
      { return *this = (*this*f); }
      inline self_type& operator /= ( Arithmatic f )
      { return *this = (*this/f); }
      // --------------------------------------------------------------------
      inline bool operator == ( const self_type& p ) const
      { return (*this - p).euclidean_norm()<=.00000000001; }

      inline bool operator != ( const self_type& p ) const
      { return !(*this == p); }

      inline self_type& operator= ( const self_type& p )
      { x_=p.x(); y_=p.y(); z_=p.z(); return *this; }

      inline Arithmatic operator * ( const self_type& p ) const
      { return x()*p.x() + y()*p.y() + z()*p.z() ; }
      // --------------------------------------------------------------------
      Arithmatic euclidean_norm( void )
         const
      { return sqrt( (x_*x_) + (y_*y_) + (z_*z_) ); }

 /*     static Arithmatic euclidean_distance( const self_type& p1,
                                   const self_type& p2 );
      static self_type cross_product( const self_type& p1, const self_type& p2 );
*/

     static inline Arithmatic

         euclidean_distance( const self_type& p1,
                             const self_type& p2 )
         { return (p1-p2).euclidean_norm(); }
         // -----------------------------------------------------------------------

         static inline self_type

         cross_product( const self_type& p1, const self_type& p2 )
         { return self_type( p1.y()*p2.z() - p1.z()*p2.y(), p1.z()*p2.x() - p1.x()*p2.z(), p1.x()*p2.y() - p1.y()*p2.x() ); }

   private:
      Arithmatic x_, y_, z_;
   };
   // -----------------------------------------------------------------------

/*   template<typename Arithmatic_P>

   typename Vec<Arithmatic_P>::Arithmatic
		   Vec<Arithmatic_P>::
   euclidean_distance( const self_type& p1,
                       const self_type& p2 )
   { return (p1-p2).euclidean_norm(); }
   // -----------------------------------------------------------------------
    template<typename Arithmatic_P>
   typename Vec<Arithmatic_P>::self_type
   Vec<Arithmatic_P>::
   cross_product( const self_type& p1, const self_type& p2 )
   { return Vec<Arithmatic_P>::self_type( p1.y()*p2.z() - p1.z()*p2.y(), p1.z()*p2.x() - p1.x()*p2.z(), p1.x()*p2.y() - p1.y()*p2.x() ); }*/
   // -----------------------------------------------------------------------
//    const Arithmatic Vec::epsilon = .00000000001;
}

#endif
