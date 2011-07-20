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
#ifndef __ALGORITHMS_LOCALIZATION_DISTANCE_BASED_NEIGHBORHOOD_LCS_H
#define __ALGORITHMS_LOCALIZATION_DISTANCE_BASED_NEIGHBORHOOD_LCS_H

#include "algorithms/localization/distance_based/math/vec.h"
#include "algorithms/localization/distance_based/math/localization_lcs_helpers.h"
#include "algorithms/localization/distance_based/math/localization_general_math.h"
#include "algorithms/localization/distance_based/math/localization_triangulation.h"
#include "algorithms/localization/distance_based/util/localization_defutils.h"
#include "algorithms/localization/distance_based/neighborhood/localization_neighborhood.h"


namespace wiselib
{

   /// Local Coordinate System
   /** Class representing a single Local Coordinate System
    *  This class represents a single LCS such as described in the paper by
    *  Capkun, Hamdi and Hubaux, <em>GPS-free positioning in mobile ad-hoc
    *  networks</em>.
    *
    *
    */
   template<typename OsModel_P,
            typename node_id_t_P,
            typename Neighborhood_P,
            typename LocationMap_P,
            typename Arithmatic_P >
   class LocalizationLocalCoordinateSystem
   {

   public:
      typedef OsModel_P OsModel;
      typedef node_id_t_P node_id_t;
      typedef Neighborhood_P Neighborhood;
      typedef LocationMap_P LocationMap;
      typedef Arithmatic_P Arithmatic;
      typedef typename LocationMap::iterator LocationMapIterator;

      typedef LocalizationLocalCoordinateSystem<OsModel, node_id_t, Neighborhood, LocationMap, Arithmatic> self_type;

      /// Container for data of correction of LCS
      struct CorrectionData
      {
         Arithmatic correction_angle;
         bool mirror;
         Vec<Arithmatic_P> pos;
      };

      ///@name construction / destruction
      ///@{
      ///
      LocalizationLocalCoordinateSystem();
      ///
      LocalizationLocalCoordinateSystem( const self_type& );
      ///
      ~LocalizationLocalCoordinateSystem();
      ///@}

      ///@name nodes
      ///@{
      /**
       */
      void update_basic_nodes( node_id_t, node_id_t, Neighborhood& );
      /** Add a node to the current LCS, if there can be a position computed.
       */
      void update_node( node_id_t );
      /** Set src position and adapt all positions of neighbors in LCS.
       */
      void set_src_node( node_id_t );
      /** Set src position and adapt all positions of neighbors in LCS.
       */
      void set_position( Vec<Arithmatic_P>& );
      ///@}

      ///@name information
      ///@{
      /** \result Used epsilon
       */
      Arithmatic epsilon( void );
      /** Set epsilon
       */
      void set_epsilon( Arithmatic );
      /** \result /c true, if LCS is valid.
       */
      bool is_valid( void );
      /** \return number of nodes of current LCS (include the basic nodes p
       *    and q)
       */
      int size( void );
      /** \result Src-Node.
       */
      node_id_t src_node( void );
      /** \param Node neighbor to search for
       *
       *  \result \true if neighbor exists
       */
      bool has_neighbor( node_id_t );
      /** \result Position of given node.
       */
      const Vec<Arithmatic_P>& node_position( node_id_t );
      /** \result Position of src node.
       */
      Vec<Arithmatic_P>& src_position( void );
      ///@}

      ///@name special methods on LCS
      ///@{
      /** Perform the correction of LCS based on given CorrectionData.
       */
      void perform_correction( CorrectionData& );
      /** Rotate all coordinates by given angle.
       */
      void rotate( Arithmatic );
      /** Mirror coordinate system on x-axis.
       */
      void mirror_x( void );
      /** Mirror coordinate system on y-axis.
       */
      void mirror_y( void );
      /** Correct LCS in respect to the given one. At first it is turned by a
       *  computed correction angle and then, if necessary, mirrored by x-
       *  and/or y-axis.
       *
       *  \param LocalizationLocalCoordinateSystem LCS
       *  \return \c true, if computation succeeds.
       */
      bool correct_lcs( self_type&, CorrectionData& );
      /** Correct LCS to the direction of the real global coordinate system.
       *  Therefor the real positions of self and the basic nodes p and q are
       *  taken.
       *
       *  \return \c true, if computation succeeds.
       */
      bool correct_lcs_to_real_ncs( CorrectionData& );
      ///@}

      ///@name internal methods
      ///@{
      /** Delete all internal data.
       */
      void clear( void );
      ///@}

   private:

      node_id_t p_, q_, src_node_;
      Neighborhood* neighborhood_;
      Vec<Arithmatic_P> src_pos_;

      LocationMap locations_;

      bool valid_;

      Arithmatic epsilon_;
   };
   // ----------------------------------------------------------------------
   // ----------------------------------------------------------------------
   // ----------------------------------------------------------------------
   template<typename OsModel_P,
            typename node_id_t_P,
            typename Neighborhood_P,
            typename LocationMap_P,
            typename Arithmatic_P>
   LocalizationLocalCoordinateSystem<OsModel_P, node_id_t_P, Neighborhood_P, LocationMap_P, Arithmatic_P>::
   LocalizationLocalCoordinateSystem()
      : src_pos_  ( Vec<Arithmatic_P>( 0.0, 0.0, 0.0 ) ),
         valid_   ( false ),
         epsilon_ ( 0.0001 )
   {}
   // ----------------------------------------------------------------------
   template<typename OsModel_P,
            typename node_id_t_P,
            typename Neighborhood_P,
            typename LocationMap_P,
            typename Arithmatic_P>
   LocalizationLocalCoordinateSystem<OsModel_P, node_id_t_P, Neighborhood_P, LocationMap_P, Arithmatic_P>::
   LocalizationLocalCoordinateSystem( const LocalizationLocalCoordinateSystem& llcs )
      : p_             ( llcs.p_ ),
         q_            ( llcs.q_ ),
         src_node_     ( llcs.src_node_ ),
         neighborhood_ ( llcs.neighborhood_ ),
         src_pos_      ( llcs.src_pos_ ),
         locations_    ( llcs.locations_ ),
         valid_        ( llcs.valid_ ),
         epsilon_      ( llcs.epsilon_ )
   {}
   // ----------------------------------------------------------------------
   template<typename OsModel_P,
            typename node_id_t_P,
            typename Neighborhood_P,
            typename LocationMap_P,
            typename Arithmatic_P>
   LocalizationLocalCoordinateSystem<OsModel_P, node_id_t_P, Neighborhood_P, LocationMap_P, Arithmatic_P>::
   ~LocalizationLocalCoordinateSystem()
   {}
   // ----------------------------------------------------------------------
   template<typename OsModel_P,
            typename node_id_t_P,
            typename Neighborhood_P,
            typename LocationMap_P,
            typename Arithmatic_P>
   void
   LocalizationLocalCoordinateSystem<OsModel_P, node_id_t_P, Neighborhood_P, LocationMap_P, Arithmatic_P>::
   update_basic_nodes( node_id_t p, node_id_t q, Neighborhood& nbrh )
   {
      p_ = p;
      q_ = q;
      neighborhood_ = &nbrh;

      Arithmatic src_p = neighborhood_->neighbor_distance( p_ );
      Arithmatic src_q = neighborhood_->neighbor_distance( q_ );
      Arithmatic p_q = neighborhood_->nneighbor_distance( p_, q_ );

      Vec<Arithmatic_P> pos_p, pos_q;

      if ( !compute_basis_coords( src_p, src_q, p_q, pos_p, pos_q ) )
      {
         if ( !valid_ )
            clear();
         return;
      }

      locations_[p_] = pos_p;
      locations_[q_] = pos_q;

      valid_ = true;
   }
   // ----------------------------------------------------------------------
   template<typename OsModel_P,
            typename node_id_t_P,
            typename Neighborhood_P,
            typename LocationMap_P,
            typename Arithmatic_P>
   void
   LocalizationLocalCoordinateSystem<OsModel_P, node_id_t_P, Neighborhood_P, LocationMap_P, Arithmatic_P>::
   update_node( node_id_t j )
   {
      if ( !valid_ )
         return;

      Arithmatic src_p = neighborhood_->neighbor_distance( p_ );
      Arithmatic src_q = neighborhood_->neighbor_distance( q_ );
      Arithmatic src_j = neighborhood_->neighbor_distance( j );
      Arithmatic p_j = neighborhood_->nneighbor_distance( p_, j );
      Arithmatic q_j = neighborhood_->nneighbor_distance( q_, j );
      Arithmatic p_q = neighborhood_->nneighbor_distance( p_, q_ );

      Vec<Arithmatic_P> pos_j;

      if ( !compute_rel_coord_triangulisation( src_p, src_q, src_j, p_j, q_j, p_q, pos_j, epsilon_ ) )
         return;

      locations_[j] = pos_j;
   }
   // ----------------------------------------------------------------------
   template<typename OsModel_P,
            typename node_id_t_P,
            typename Neighborhood_P,
            typename LocationMap_P,
            typename Arithmatic_P>
   void
   LocalizationLocalCoordinateSystem<OsModel_P, node_id_t_P, Neighborhood_P, LocationMap_P, Arithmatic_P>::
   set_src_node( node_id_t src_node )
   {
      src_node_ = src_node;
   }
   // ----------------------------------------------------------------------
   template<typename OsModel_P,
            typename node_id_t_P,
            typename Neighborhood_P,
            typename LocationMap_P,
            typename Arithmatic_P>
   void
   LocalizationLocalCoordinateSystem<OsModel_P, node_id_t_P, Neighborhood_P, LocationMap_P, Arithmatic_P>::
   set_position( Vec<Arithmatic_P>& new_src_pos )
   {
      for ( LocationMapIterator
               it = locations_.begin();
               it != locations_.end();
               ++it )
         it->second += new_src_pos - src_pos_;

      src_pos_ = new_src_pos;
   }
   // ----------------------------------------------------------------------
   template<typename OsModel_P,
            typename node_id_t_P,
            typename Neighborhood_P,
            typename LocationMap_P,
            typename Arithmatic_P>
   typename LocalizationLocalCoordinateSystem<OsModel_P, node_id_t_P, Neighborhood_P, LocationMap_P, Arithmatic_P>::Arithmatic
   LocalizationLocalCoordinateSystem<OsModel_P, node_id_t_P, Neighborhood_P, LocationMap_P, Arithmatic_P>::
   epsilon( void )
   {
      return epsilon_;
   }
   // ----------------------------------------------------------------------
   template<typename OsModel_P,
            typename node_id_t_P,
            typename Neighborhood_P,
            typename LocationMap_P,
            typename Arithmatic_P>
   void
   LocalizationLocalCoordinateSystem<OsModel_P, node_id_t_P, Neighborhood_P, LocationMap_P, Arithmatic_P>::
   set_epsilon( Arithmatic epsilon )
   {
      epsilon_ = epsilon;
   }
   // ----------------------------------------------------------------------
   template<typename OsModel_P,
            typename node_id_t_P,
            typename Neighborhood_P,
            typename LocationMap_P,
            typename Arithmatic_P>
   bool
   LocalizationLocalCoordinateSystem<OsModel_P, node_id_t_P, Neighborhood_P, LocationMap_P, Arithmatic_P>::
   is_valid( void )
   {
      return valid_;
   }
   // ----------------------------------------------------------------------
   template<typename OsModel_P,
            typename node_id_t_P,
            typename Neighborhood_P,
            typename LocationMap_P,
            typename Arithmatic_P>
   int
   LocalizationLocalCoordinateSystem<OsModel_P, node_id_t_P, Neighborhood_P, LocationMap_P, Arithmatic_P>::
   size( void )
   {
      return locations_.size();
   }
   // ----------------------------------------------------------------------
   template<typename OsModel_P,
            typename node_id_t_P,
            typename Neighborhood_P,
            typename LocationMap_P,
            typename Arithmatic_P>
   typename LocalizationLocalCoordinateSystem<OsModel_P, node_id_t_P, Neighborhood_P, LocationMap_P, Arithmatic_P>::node_id_t
   LocalizationLocalCoordinateSystem<OsModel_P, node_id_t_P, Neighborhood_P, LocationMap_P, Arithmatic_P>::
   src_node( void )
   {
      return src_node_;
   }
   // ----------------------------------------------------------------------
   template<typename OsModel_P,
            typename node_id_t_P,
            typename Neighborhood_P,
            typename LocationMap_P,
            typename Arithmatic_P>
   bool
   LocalizationLocalCoordinateSystem<OsModel_P, node_id_t_P, Neighborhood_P, LocationMap_P, Arithmatic_P>::
   has_neighbor( node_id_t node )
   {
      return ( locations_.find( node ) != locations_.end() );
   }
   // ----------------------------------------------------------------------
   template<typename OsModel_P,
            typename node_id_t_P,
            typename Neighborhood_P,
            typename LocationMap_P,
            typename Arithmatic_P>
   const Vec<Arithmatic_P>&
   LocalizationLocalCoordinateSystem<OsModel_P, node_id_t_P, Neighborhood_P, LocationMap_P, Arithmatic_P>::
   node_position( node_id_t node )
   {
      if ( locations_.find( node ) != locations_.end() )
         return locations_.find( node )->second;

      return UNKNOWN_POSITION;
   }
   // ----------------------------------------------------------------------
   template<typename OsModel_P,
            typename node_id_t_P,
            typename Neighborhood_P,
            typename LocationMap_P,
            typename Arithmatic_P>
   Vec<Arithmatic_P>&
   LocalizationLocalCoordinateSystem<OsModel_P, node_id_t_P, Neighborhood_P, LocationMap_P, Arithmatic_P>::
   src_position( void )
   {
      return src_pos_;
   }
   // ----------------------------------------------------------------------
   template<typename OsModel_P,
            typename node_id_t_P,
            typename Neighborhood_P,
            typename LocationMap_P,
            typename Arithmatic_P>
   void
   LocalizationLocalCoordinateSystem<OsModel_P, node_id_t_P, Neighborhood_P, LocationMap_P, Arithmatic_P>::
   perform_correction( CorrectionData& cd )
   {
      rotate( cd.correction_angle );
      if ( cd.mirror )
         mirror_x();
      set_position( cd.pos );
   }
   // ----------------------------------------------------------------------
   template<typename OsModel_P,
            typename node_id_t_P,
            typename Neighborhood_P,
            typename LocationMap_P,
            typename Arithmatic_P>
   void
   LocalizationLocalCoordinateSystem<OsModel_P, node_id_t_P, Neighborhood_P, LocationMap_P, Arithmatic_P>::
   rotate( Arithmatic angle )
   {
      for ( LocationMapIterator
               it = locations_.begin();
               it != locations_.end();
               ++it )
         rotate_2D( angle, it->second, src_pos_ );
   }
   // ----------------------------------------------------------------------
   template<typename OsModel_P,
            typename node_id_t_P,
            typename Neighborhood_P,
            typename LocationMap_P,
            typename Arithmatic_P>
   void
   LocalizationLocalCoordinateSystem<OsModel_P, node_id_t_P, Neighborhood_P, LocationMap_P, Arithmatic_P>::
   mirror_x( void )
   {
      for ( LocationMapIterator
               it = locations_.begin();
               it != locations_.end();
               ++it )
         it->second = Vec<Arithmatic_P>( -it->second.x(), it->second.y(), it->second.z() );
   }
   // ----------------------------------------------------------------------
   template<typename OsModel_P,
            typename node_id_t_P,
            typename Neighborhood_P,
            typename LocationMap_P,
            typename Arithmatic_P>
   void
   LocalizationLocalCoordinateSystem<OsModel_P, node_id_t_P, Neighborhood_P, LocationMap_P, Arithmatic_P>::
   mirror_y( void )
   {
      for ( LocationMapIterator
               it = locations_.begin();
               it != locations_.end();
               ++it )
         it->second = Vec<Arithmatic_P>( it->second.x(), -it->second.y(), it->second.z() );
   }
   // ----------------------------------------------------------------------
   template<typename OsModel_P,
            typename node_id_t_P,
            typename Neighborhood_P,
            typename LocationMap_P,
            typename Arithmatic_P>
   bool
   LocalizationLocalCoordinateSystem<OsModel_P, node_id_t_P, Neighborhood_P, LocationMap_P, Arithmatic_P>::
   correct_lcs( LocalizationLocalCoordinateSystem& lcs, CorrectionData& cd )
   {
      if ( !is_valid() || !lcs.is_valid() ||
            lcs.node_position( src_node() ) == UNKNOWN_POSITION ||
            node_position( lcs.src_node() ) == UNKNOWN_POSITION )
         return false;

      for ( LocationMapIterator
               it = locations_.begin();
               it != locations_.end();
               ++it )
         if ( lcs.has_neighbor( it->first ) )
         {
            int bas_node = lcs.src_node();
            int ref_node = it->first;
            int me = src_node();

            if ( lcs.node_position( ref_node ) == UNKNOWN_POSITION ||
                  node_position( ref_node ) == UNKNOWN_POSITION ||
                  bas_node == ref_node || me == ref_node )
               continue;

            Arithmatic angle_bas_me = angle_vec( lcs.node_position( me ) - lcs.src_position() );
            Arithmatic angle_bas_ref = angle_vec( lcs.node_position( ref_node ) - lcs.src_position() );
            Arithmatic angle_me_bas = angle_vec( node_position( bas_node ) - src_position() );
            Arithmatic angle_me_ref = angle_vec( node_position( ref_node ) - src_position() );

            Arithmatic me_bas = angle_me_ref - angle_me_bas;
            Arithmatic bas_me = angle_bas_ref - angle_bas_me;

            me_bas = normalize_angle( me_bas );
            bas_me = normalize_angle( bas_me );

            if ( ( bas_me < M_PI && me_bas < M_PI ) ||
               ( bas_me > M_PI && me_bas > M_PI ) )
            {
               Arithmatic correction_angle = angle_bas_me + angle_me_bas;
               cd.correction_angle = -correction_angle;
               cd.mirror = true;
            }
            else // if ( ( bas_me < M_PI && me_bas > M_PI ) ||
                 //       ( bas_me > M_PI && me_bas < M_PI ) )
            {
               Arithmatic correction_angle = angle_bas_me - angle_me_bas + M_PI;
               cd.correction_angle = correction_angle;
               cd.mirror = false;
            }

            cd.correction_angle = normalize_angle( cd.correction_angle );
            cd.pos = lcs.node_position( src_node() );

            return true;
         }

      return false;
   }
   // ----------------------------------------------------------------------
   template<typename OsModel_P,
            typename node_id_t_P,
            typename Neighborhood_P,
            typename LocationMap_P,
            typename Arithmatic_P>
   bool
   LocalizationLocalCoordinateSystem<OsModel_P, node_id_t_P, Neighborhood_P, LocationMap_P, Arithmatic_P>::
   correct_lcs_to_real_ncs( CorrectionData& cd )
   {
      if ( !is_valid() ) return false;

// TODO: This does only work for Shawn - but is quite useful when debugging
//       (visualizing) the algorithm. however, somehow there is a possibility
//       needed to get the real positions of other nodes in the network :/
//
//       int bas_node = q_;
//       int ref_node = p_;
//       int me = src_node();
// 
//       Arithmatic angle_bas_me = angle_vec( me->real_position() - bas_node->real_position() );
//       Arithmatic angle_bas_ref = angle_vec( ref_node->real_position() - bas_node->real_position() );
//       Arithmatic angle_me_bas = angle_vec( node_position( bas_node ) - src_position() );
//       Arithmatic angle_me_ref = angle_vec( node_position( ref_node ) - src_position() );
// 
//       Arithmatic me_bas = angle_me_ref - angle_me_bas;
//       Arithmatic bas_me = angle_bas_ref - angle_bas_me;
// 
//       me_bas = normalize_angle( me_bas );
//       bas_me = normalize_angle( bas_me );
// 
//       if ( ( bas_me < M_PI && me_bas < M_PI ) ||
//             ( bas_me > M_PI && me_bas > M_PI ) )
//       {
//          Arithmatic correction_angle = angle_bas_me + angle_me_bas;
//          cd.correction_angle = -correction_angle;
//          cd.mirror = true;
//       }
//       else // if ( ( bas_me < M_PI && me_bas > M_PI ) ||
//            //       ( bas_me > M_PI && me_bas < M_PI ) )
//       {
//          Arithmatic correction_angle = angle_bas_me - angle_me_bas + M_PI;
//          cd.correction_angle = correction_angle;
//          cd.mirror = false;
//       }
// 
//       cd.correction_angle = normalize_angle( cd.correction_angle );
//       cd.pos = me->real_position();

      return true;
   }
   // ----------------------------------------------------------------------
   template<typename OsModel_P,
            typename node_id_t_P,
            typename Neighborhood_P,
            typename LocationMap_P,
            typename Arithmatic_P>
   void
   LocalizationLocalCoordinateSystem<OsModel_P, node_id_t_P, Neighborhood_P, LocationMap_P, Arithmatic_P>::
   clear( void )
   {
      p_ = NULL;
      q_ = NULL;
      neighborhood_ = NULL;

      src_pos_ = Vec<Arithmatic_P>( 0.0, 0.0, 0.0 );
      locations_.clear();
      valid_ = false;
   }

}// namespace wiselib
#endif
