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
#ifndef __ALGORITHMS_LOCALIZATION_DISTANCE_BASED_MATH_TRIANGULATION_H
#define __ALGORITHMS_LOCALIZATION_DISTANCE_BASED_MATH_TRIANGULATION_H

#include "algorithms/localization/distance_based/math/vec.h"
#include "algorithms/localization/distance_based/math/localization_simple_matrix.h"
#include "algorithms/localization/distance_based/neighborhood/localization_neighborhood.h"
#include "algorithms/localization/distance_based/util/localization_defutils.h"
#include "util/pstl/algorithm.h"
#include "util/pstl/pair.h"
#include <float.h>

//#include <isense/platforms/jennic/jennic_os.h>

// TODO: Remove macros!!!
#define SQR(n) (n*n)

namespace wiselib
{

   enum LaterationType
   {
      lat_anchors,  ///< only valid anchors
      lat_confident ///< only confident neighbors
   };

   /** Position estimation with distance to anchors and their positions.
    *  Main idea is set a bounding box, defined by position and distance,
    *  around each anchor. The estimated position is the center of the
    *  resulting intersection box.
    *
    *  \param NeighborInfoList Neighbors, for which above mentioned
    *    bounding boxes are builded
    *  \param Vec New, estimated position
    *  \result \c true, if estimation succeeded. \c false otherwise
    *  \sa est_pos_lateration()
    */
   template<typename OsModel_P,
            typename NeighborInfoList,
            typename Arithmatic_P>
   bool est_pos_min_max( const NeighborInfoList&, Vec<Arithmatic_P>& );
   /** Position estimation with distance to anchors and their positions.
    *  This method uses lateration for position estimation. Main idea is
    *  to solve a system of equations, using here a least squares approach.
    *
    *  Cause of different requirements, there is a chance to use standard
    *  or weighted least squares approach.
    *
    *  \param NeighborInfoList Neighbors, for which above mentioned
    *    least squares approach is started
    *  \param Vec New, estimated position
    *  \param LaterationType Type of lateration to decide, whether using
    *    standard or weighted least squares approach
    *  \param bool if \c true, given position is used in least squares
    *    approach
    *  \result \c true, if estimation succeeded. \c false otherwise
    *  \sa est_pos_min_max(), collect_neighbors(), LaterationType
    */
   template<typename OsModel_P,
            typename NeighborInfoList,
            typename Arithmatic_P>
   bool est_pos_lateration(
         const NeighborInfoList&,
         Vec<Arithmatic_P>&,
         const LaterationType&,
         bool );
   /** This Method checks, whether an estimated position is accepted or
    *  not. It uses the distance residues between given ones and estimated.
    *  Formula looks like following:
    *  \f[residue = \frac{\sum^n_{i=1}\sqrt{(x_i-x)^2+(y_i-y)^2}-d_i}{n}\f]
    *
    *  \param NeighborInfoList Neighbors, for which sanity check is started
    *  \param Vec estimated position
    *  \param LaterationType Type of lateration to decide, whether use
    *    weights in calculation or not
    *  \param Arithmatic_P Simulations communication range
    *  \result \c true, if estimated position is accepted. \c false
    *    otherwise
    *  \sa est_pos_lateration(), est_pos_min_max(), collect_neighbors(),
    *    LaterationType
    */
   template<typename OsModel_P,
            typename NeighborInfoList,
            typename Arithmatic_P>
   bool check_residue(
         const NeighborInfoList&,
         const Vec<Arithmatic_P>&,
         const LaterationType&,
         Arithmatic_P );
   /** This method calculates the distance between two nodes using only
    *  distance information with trilateration.
    *
    *  There are two triangles known. At first, the triangle source-n1-n2,
    *  where as well the distance from source to n1 and n2 as the distance
    *  from n1 to n2 is known. Second, there is the triangle
    *  n1-n2-destination with same distance information.
    *
    *  Result is a choice of two valid distances.
    *
    *  \param Arithmatic_P distance source-n1
    *  \param Arithmatic_P distance source-n2
    *  \param Arithmatic_P distance n1-n2
    *  \param Arithmatic_P distance n1-destination
    *  \param Arithmatic_P distance n2-destination
    *  \result The two resulting distances
    */
   template<typename OsModel_P, typename DistancePair_P, typename Arithmatic_P>
   DistancePair_P trilateration_distance(
         Arithmatic_P, Arithmatic_P, Arithmatic_P, Arithmatic_P, Arithmatic_P );
   /** This method collects different types of neighbors in a
    *  NeighborInfoList used in est_pos_lateration() and check_residue().
    *  The type of neighbors is given by the lateration type.
    *
    *  \param LocalizationNeighborhood Used neighborhood
    *  \param LaterationType Type of resulting neighbors, e.g. anchors or
    *    confident neighbors
    *  \param NeighborInfoList Resulting list of neighbors
    *  \sa est_pos_lateration(), check_residue(), LaterationType
    */
   template<typename OsModel_P,
            typename Neighborhood,
            typename NeighborInfoList,
            typename Arithmatic_P>
   void collect_neighbors(
         Neighborhood&,
         const LaterationType&,
         NeighborInfoList& );
   // -----------------------------------------------------------------------
   // -----------------------------------------------------------------------
   // -----------------------------------------------------------------------
   template<typename OsModel_P,
            typename NeighborInfoList,
            typename Arithmatic_P>
   bool
   est_pos_min_max( const NeighborInfoList& neighbors, Vec<Arithmatic_P>& pos )
   {
      typedef typename NeighborInfoList::iterator NeighborInfoListIterator;

      if ( neighbors.empty() ) return false;

      wiselib::pair<Vec<Arithmatic_P>, Vec<Arithmatic_P> > intersection;
      NeighborInfoListIterator it = neighbors.begin();

      intersection.first = Vec<Arithmatic_P>(
         (*it)->pos().x() - (*it)->distance(),
         (*it)->pos().y() - (*it)->distance() );
      intersection.second = Vec<Arithmatic_P>(
         (*it)->pos().x() + (*it)->distance(),
         (*it)->pos().y() + (*it)->distance() );

      for ( ++it; it != neighbors.end(); ++it )
      {
         intersection.first = Vec<Arithmatic_P>(
            wiselib::max( intersection.first.x(),
                  (*it)->pos().x() - (*it)->distance() ),
            wiselib::max( intersection.first.y(),
                  (*it)->pos().y() - (*it)->distance() ) );
         intersection.second = Vec<Arithmatic_P>(
            wiselib::min( intersection.second.x(),
                  (*it)->pos().x() + (*it)->distance() ),
            wiselib::min( intersection.second.y(),
                  (*it)->pos().y() + (*it)->distance() ) );
      }

      pos = Vec<Arithmatic_P>(
         ( intersection.first.x() + intersection.second.x() ) / 2,
         ( intersection.first.y() + intersection.second.y() ) / 2 );

      return true;
   }
   // ----------------------------------------------------------------------
   template<typename OsModel_P,
            typename NeighborInfoList,
            typename Arithmatic_P>
   bool
   est_pos_lateration(
         const NeighborInfoList& neighbors,
         Vec<Arithmatic_P>& pos,
         const LaterationType& lat_type,
         bool use_pos )
   {


      typedef typename NeighborInfoList::iterator NeighborInfoListIterator;


      int nbr_size = neighbors.size();
      if ( nbr_size < 3 ) return false;



      typedef SimpleMatrix<OsModel_P, Arithmatic_P> Matrix;
      Matrix m_a;
      Matrix m_b;
      Matrix m_x;



      NeighborInfoListIterator it = neighbors.begin();




      Arithmatic_P x_1, y_1, d_1;
      if ( use_pos )
      {
         m_a = Matrix( nbr_size, 2 );
         m_b = Matrix( nbr_size, 1 );
         x_1 = pos.x();
         y_1 = pos.y();
         d_1 = 0;
      }
      else
      {
         m_a = Matrix( nbr_size - 1, 2 );
         m_b = Matrix( nbr_size - 1, 1 );
         x_1 = (*it)->pos().x();
         y_1 = (*it)->pos().y();
         d_1 = (*it)->distance();
         ++it;
      }

     /* if(isense::JennicOs::jennic_os->id()==0x9999)
                  	  isense::JennicOs::jennic_os->debug("befgin it %f %f %f",x_1,y_1,d_1);*/

      int row = 0;
      for ( ; it != neighbors.end(); ++it )
      {
         Arithmatic_P confidence = (*it)->confidence();

             /*  if(isense::JennicOs::jennic_os->id()==0x9999)
             	  isense::JennicOs::jennic_os->debug("it %f %f %f %f",(*it)->pos().x(),(*it)->pos().y(),(*it)->distance(),(*it)->confidence());*/

         if ( lat_type == lat_anchors ) confidence = 1;


         m_a(row,0) = 2 * ( (*it)->pos().x() - x_1 ) * confidence;
         m_a(row,1) = 2 * ( (*it)->pos().y() - y_1 ) * confidence;

       m_b(row,0) =
            ( SQR( (*it)->pos().x() ) - SQR( x_1 )
               + SQR( (*it)->pos().y() ) - SQR( y_1 )
               + SQR( d_1 )
               - SQR( (*it)->distance() ) )
            * confidence;
//    std::cout << "distanz von Knoten"<<  <<"gesch�tzt: " << (*
/*     m_b(row,0) = 
          ( SQR( (*it)->pos().x() -  x_1 )
               + SQR( (*it)->pos().y() - y_1 )
               + SQR( d_1 )
               - SQR( (*it)->distance() ) ) *  0.5;
*/         row++;
      }


//
//     if (
//    		  (( m_a.transposed() * m_a ).det() < 0.0001 )&&
//    		  (( m_a.transposed() * m_a ).det() > -0.0001 )
//    	){
//    	  /*if(isense::JennicOs::jennic_os->id()==0x9999)
//    		  isense::JennicOs::jennic_os->debug("before second transport");*/
//    	  return false;
//      }else
//      {
//    	 /* if(isense::JennicOs::jennic_os->id()==0x9999)
//    	     		  isense::JennicOs::jennic_os->debug("det %f",( m_a.transposed() * m_a ).det());*/
//    	 // return true;
//      }



Matrix tmp = m_a.transposed();
tmp*=m_a;

Arithmatic_P det = tmp.det();




if( (det < 0.0001 )&&(det > -0.0001 ))
{
	return false;
}




m_x = tmp.inverse();



tmp = m_a.transposed();






tmp*=m_b;




m_x*=tmp;



     // solve Ax = b
 /*     m_x = ( m_a.transposed() * m_a ).inverse()
         * ( m_a.transposed() * m_b );*/

/*      if(isense::JennicOs::jennic_os->id()==0x9999)
    	  isense::JennicOs::jennic_os->debug("after second transport");
      return true;*/

      pos = Vec<Arithmatic_P>( m_x(0,0), m_x(1,0));

      return true;
   }
   // ----------------------------------------------------------------------
   template<typename OsModel_P,
            typename NeighborInfoList,
            typename Arithmatic_P>
   bool
   check_residue(
         const NeighborInfoList& neighbors,
         const Vec<Arithmatic_P>& est_pos,
         const LaterationType& lat_type,
         Arithmatic_P comm_range )
   {
      typedef typename NeighborInfoList::iterator NeighborInfoListIterator;

      int nbr_size = neighbors.size();
      if ( nbr_size == 0 ) return false;

      Arithmatic_P residue = 0;
      Arithmatic_P conf_sum = 0;

      for ( NeighborInfoListIterator
               it = neighbors.begin();
               it != neighbors.end();
               ++it )
      {
         Arithmatic_P confidence = (*it)->confidence();
         if ( lat_type == lat_anchors ) confidence = 1;

         conf_sum += confidence;
         residue +=
            ( sqrt(
                  SQR( est_pos.x() - (*it)->pos().x() ) +
                  SQR( est_pos.y() - (*it)->pos().y() ) )
               - (*it)->distance() )
            * confidence;
      }

      residue /= conf_sum;

      if ( fabs( residue ) > comm_range )
         return false;
      else
         return true;
   }
   // ----------------------------------------------------------------------
   template<typename OsModel_P, typename DistancePair_P, typename Arithmatic_P>
   DistancePair_P
   trilateration_distance(
         Arithmatic_P s_n1,
         Arithmatic_P s_n2,
         Arithmatic_P n1_n2,
         Arithmatic_P n1_anchor,
         Arithmatic_P n2_anchor )
   {
      typedef DistancePair_P DistancePair;
      typedef Arithmatic_P Arithmatic;

      // trilaterate triangle self-n1-n2
      Arithmatic_P self_x = ( SQR(s_n1) - SQR(s_n2) + SQR(n1_n2) ) / ( 2*n1_n2 );
      Arithmatic_P tmp = SQR(s_n1) - SQR(self_x);
      if ( tmp < -0.1 )
         return DistancePair( -1, -1 );

      Arithmatic_P self_y = sqrt( fabs(tmp) );

      // trilaterate triangle anchor-n1-n2
      Arithmatic_P anchor_x = ( SQR(n1_anchor) - SQR(n2_anchor) + SQR(n1_n2) ) / ( 2*n1_n2 );
      tmp = SQR(n1_anchor) - SQR(anchor_x);
      if ( tmp < -0.1 )
         return DistancePair( -1, -1 );
      Arithmatic_P anchor_y = sqrt( fabs(tmp) );

      // distance between self and anchor
      Arithmatic_P d_x = self_x - anchor_x;
      Arithmatic_P d_y1 = self_y - anchor_y;
      Arithmatic_P d_y2 = self_y + anchor_y;

      Arithmatic_P dist1 = sqrt( SQR(d_x) + SQR(d_y1) );
      Arithmatic_P dist2 = sqrt( SQR(d_x) + SQR(d_y2) );

      return DistancePair( dist1, dist2 );
   }
   // ----------------------------------------------------------------------
   template<typename OsModel_P,
            typename Neighborhood,
            typename NeighborInfoList,
            typename Arithmatic_P>
   void
   collect_neighbors(
         Neighborhood& nbrhood,
         const LaterationType& lat_type,
         NeighborInfoList& nbrs )
   {
      typedef typename Neighborhood::NeighborhoodIterator NeighborhoodIterator;

      for ( NeighborhoodIterator
               it = nbrhood.begin_neighborhood();
               it != nbrhood.end_neighborhood();
               ++it )
         if ( ( lat_type == lat_anchors && it->second.is_anchor() && it->second.is_valid() ) ||
               ( lat_type == lat_confident && it->second.is_confident() ) )
            nbrs.push_back( &it->second );
   }

}// namespace wiselib
#endif
