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
#ifndef __ALGORITHMS_LOCALIZATION_DISTANCE_BASED_LCS_HELPERS_H
#define __ALGORITHMS_LOCALIZATION_DISTANCE_BASED_LCS_HELPERS_H

#include "algorithms/localization/distance_based/math/vec.h"
#include "algorithms/localization/distance_based/util/localization_defutils.h"
#include <float.h>

namespace wiselib
{

   /** This method computes relative coordinates of two nodes given
    *  the distances from source to the two nodes and distance
    *  between latter. The coordinates are relative to the source position
    *  (0,0,0).
    *
    *  Till now, there is just 2D supported.
    *
    *  \param Arithmatic_P distance source - first node p
    *  \param Arithmatic_P distance source - second node q
    *  \param Arithmatic_P distance first node p - second node q
    *  \param Vec<Arithmatic_P>& coordinate of first node p
    *  \param Vec<Arithmatic_P>& coordinate of second node q
    *  \result \c true, if computation succeedes. \c false otherwise.
    */
template <typename Arithmatic_P >
   static bool compute_basis_coords(
         Arithmatic_P, Arithmatic_P, Arithmatic_P,
         Vec<Arithmatic_P>&, Vec<Arithmatic_P>& );
   /** This method computes the relative coordinate of a node j
    *  depending on the distances to two others p and q.
    *
    *  Up to now, there is just 2D supported.
    *
    *  \param Arithmatic_P distance source - first node p
    *  \param Arithmatic_P distance source - second node q
    *  \param Arithmatic_P distance source - third node j
    *  \param Arithmatic_P distance first node p - third node j
    *  \param Arithmatic_P distance second node q - third node j
    *  \param Arithmatic_P distance first node p - second node q
    *  \param Vec<Arithmatic_P>& coordinate of the node j
    *  \param Arithmatic_P epsilon
    *  \result \c true, if computation succeedes. \c false otherwise.
    */
template <typename Arithmatic_P >
   static bool compute_rel_coord_triangulisation(
         Arithmatic_P, Arithmatic_P, Arithmatic_P, Arithmatic_P, Arithmatic_P, Arithmatic_P,
         Vec<Arithmatic_P>&, Arithmatic_P );
   // -----------------------------------------------------------------------
   // -----------------------------------------------------------------------
   // -----------------------------------------------------------------------
template <typename Arithmatic_P >
   bool
   compute_basis_coords(
         Arithmatic_P src_p, Arithmatic_P src_q, Arithmatic_P p_q,
         Vec<Arithmatic_P>& pos_p, Vec<Arithmatic_P>& pos_q )
   {
      if ( p_q == UNKNOWN_DISTANCE || p_q == 0 ||
            src_p == UNKNOWN_DISTANCE || src_p == 0 ||
            src_q == UNKNOWN_DISTANCE || src_q == 0 )
         return false;

      Arithmatic_P gamma = acos( ( src_q*src_q + src_p*src_p - p_q*p_q ) / ( 2 * src_p * src_q ) );

      Arithmatic_P pos_x_q = src_q*cos(gamma);
      Arithmatic_P pos_y_q = src_q*sin(gamma);

      if ( isnan( pos_x_q ) || isnan( pos_y_q ) )
         return false;

      pos_p = Vec<Arithmatic_P>( src_p, 0.0, 0.0 );
      pos_q = Vec<Arithmatic_P>( pos_x_q, pos_y_q, 0.0 );

      return true;
   }
   // ----------------------------------------------------------------------
template <typename Arithmatic_P >
   bool
   compute_rel_coord_triangulisation(
         Arithmatic_P src_p, Arithmatic_P src_q, Arithmatic_P src_j, Arithmatic_P p_j, Arithmatic_P q_j, Arithmatic_P p_q,
         Vec<Arithmatic_P>& pos_j, Arithmatic_P epsilon )
   {
      if ( src_j == UNKNOWN_DISTANCE || src_j == 0 ||
            p_j == UNKNOWN_DISTANCE || p_j == 0 ||
            q_j == UNKNOWN_DISTANCE || q_j == 0 )
         return false;

      Arithmatic_P alpha_j =
         acos( ( src_j*src_j + src_p*src_p - p_j*p_j ) / ( 2 * src_j * src_p )  );
      Arithmatic_P beta_j =
         acos( ( src_j*src_j + src_q*src_q - q_j*q_j ) / ( 2 * src_j * src_q )  );

      Arithmatic_P gamma = acos( ( src_q*src_q + src_p*src_p - p_q*p_q ) / ( 2 * src_p * src_q ) );

      Arithmatic_P pos_x_j = src_j * cos( alpha_j );
      Arithmatic_P pos_y_j = 0.0;
      if ( fabs( fabs( alpha_j - gamma ) - beta_j ) <= epsilon )
         pos_y_j = src_j * sin( alpha_j );
      else
         pos_y_j = -src_j * sin( alpha_j );

      if ( isnan( pos_x_j ) || isnan( pos_y_j ) )
         return false;

      pos_j = Vec<Arithmatic_P>( pos_x_j, pos_y_j, 0.0 );

      return true;
   }

}// namespace wiselib
#endif
