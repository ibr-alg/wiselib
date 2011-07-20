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
#ifndef __ALGORITHMS_LOCALIZATION_DISTANCE_BASED_NEIGHBORHOOD_NEIGHBOR_INFO_H
#define __ALGORITHMS_LOCALIZATION_DISTANCE_BASED_NEIGHBORHOOD_NEIGHBOR_INFO_H

#include "algorithms/localization/distance_based/util/localization_defutils.h"
#include <limits.h>


namespace wiselib
{

   /// Store information about a single neighbor
   /** This class stores information about a single neighbor which is again
    *  collected in LocalizationNeighborhood.
    *
    *  In addition to storing general information about position, hops,
    *  distance and if it is a anchor or not, there is a lot of extra data
    *  collected.
    *
    *  At first, there is information about confidence and twin. Confidence is
    *  used for a weighted least squares approach. A twin is defined as a node
    *  which is (or thinks it is) nearly on the same position as the neighbor
    *  itself. Both parameters are used, e.g., in
    *  LocalizationIterLaterationModule.
    *
    *  Moreover the data of neighborhood is collected. This means, that there
    *  is a chance to store a two-hop neighborhood.
    *
    *  At least there are reference nodes. These are defined as nodes
    *  neighbors get their information about anchors from, and used by
    *  LocalizationNeighborhood to decide whether a node is
    *  \ref LocalizationNeighborhood::is_sound "sound" or not.
    */
   template<typename OsModel_P,
            typename node_id_t_P,
            typename NodeSet_P,
            typename DistanceMap_P,
            typename Arithmatic_P
            >
   class LocalizationNeighborInfo
   {

   public:
      typedef OsModel_P OsModel;
      typedef node_id_t_P node_id_t;

      typedef NodeSet_P NodeSet;
      typedef DistanceMap_P DistanceMap;

      typedef Arithmatic_P Arithmatic;

      typedef typename DistanceMap::iterator NeighborIterator;

      ///@name construction / destruction
      ///@{
      /** Constructor of new neighbor info.
       *
       *  \param Node Neighbor
       *  \param bool \c true, if anchor. \c false otherwise
       */
      LocalizationNeighborInfo( node_id_t, bool );
      LocalizationNeighborInfo(  );
      ///
      ~LocalizationNeighborInfo();
      ///@}


      LocalizationNeighborInfo( const LocalizationNeighborInfo& rhs )
      {
         node_ = rhs.node_;
         pos_ = rhs.pos_;
         hops_ = rhs.hops_;
         distance_ = rhs.distance_;
         confidence_ = rhs.confidence_;

         is_anchor_ = rhs.is_anchor_;
         is_twin_ = rhs.is_twin_;

         has_pos_ = rhs.has_pos_;

         reference_points_ = rhs.reference_points_;
         DistanceMap tmp = rhs.neighbors_;
         neighbors_ = tmp;
      }

      LocalizationNeighborInfo& operator=(const LocalizationNeighborInfo &rhs)
      {
         node_ = rhs.node_;
         pos_ = rhs.pos_;
         hops_ = rhs.hops_;
         distance_ = rhs.distance_;
         confidence_ = rhs.confidence_;

         is_anchor_ = rhs.is_anchor_;
         is_twin_ = rhs.is_twin_;

         has_pos_ = rhs.has_pos_;

         reference_points_ = rhs.reference_points_;
         DistanceMap tmp = rhs.neighbors_;
         neighbors_ = tmp;

         return *this;
      }


      ///@name update/insert neighborhood
      ///@{
      /** Update neighborhood of this neighbor. If neighbor exists, it is just
       *  updated, else inserted.
       *
       *  \param Node neighbor to insert/update
       *  \param Arithmatic distance to this neighbor
       *  \sa update_neighbors()
       */
      void update_neighbor( node_id_t, Arithmatic );
      /** Update neighborhood of this neighbor. If a neighbor exists, it is
       *  just updated, else inserted.
       *
       *  \param DistanceMap neighbors and distances to insert/update
       *  \sa update_neighbor()
       */
      void update_neighbors( DistanceMap& );
      ///@}


      ///@name reference nodes
      ///@{
      /** Add a reference node.
       *
       *  \param Node reference node
       *  \sa LocalizationNeighborhood::is_sound
       */
      void add_ref_node( node_id_t );
      /** Clear reference nodes.
       *
       *  \sa LocalizationNeighborhood::is_sound
       */
      void clear_ref_nodes( void );
      /** \return set of reference nodes
       *  \sa LocalizationNeighborhood::is_sound
       */
      const NodeSet& ref_nodes( void )
         const;
      /** \return writable set of reference nodes
       *  \sa LocalizationNeighborhood::is_sound
       */
      NodeSet& ref_nodes_w( void );
      ///@}


      ///@name set data
      ///@{
      /** Set position.
       *
       *  \param Vec new position
       */
      void set_pos( const Vec<Arithmatic>& );
      /** Clear position.
       */
      void clear_pos();
      /** Set hop count.
       *
       *  \param int hop count
       */
      void set_hops( int );
      /** Set distance.
       *
       *  \param Arithmatic distance
       */
      void set_distance( Arithmatic );
      /** Set confidence.
       *
       *  \param Arithmatic confidence
       */
      void set_confidence( Arithmatic );
      /** Set twin property.
       *
       *  \param bool if it is twin or not
       */
      void set_twin( bool );
      /** Set anchor property.
       *
       *  \param bool if it is anchor or not
       */
      void set_anchor( bool );
      ///@}

      void set_node( node_id_t );


      ///@name special work on data
      ///@{
      /** This method converts hop count into distance by multiplying the
       *  hop count by given average hop distance.
       *
       *  \param Arithmatic average hop distance
       */
      void convert_hops( Arithmatic );
      ///@}


      ///@name get data
      ///@{
      /** \return Node id of this neighbor
       */
      node_id_t node( void );
      /** \return Position of this neighbor
       */
      Vec<Arithmatic>& pos( void );
      /** This method returns the hop count. If there is no hop count known,
       *  it returns localization::UNKNOWN_HOP_CNT
       *
       *  \return Hop count of this neighbor
       */
      int hops( void );
      /** \return Distance to this neighbor
       */
      Arithmatic distance( void );
      /** \return Confidence of this neighbor
       */
      Arithmatic confidence( void );
      ///@}


      ///@name get special data
      ///@{
      /** \return \c true, if this neighbor is an anchor. \c false otherwise
       */
      bool is_anchor( void );
      /** A valid neighbor has a position and distance.
       *
       *  \return \c true, if this neighbor is valid. \c false otherwise
       */
      bool is_valid( void );
      /** \return \c true, if this neighbor is a twin. \c false otherwise
       */
      bool is_twin( void );
      /** \return \c true, if this neighbor has a position. \c false otherwise
       */
      bool has_pos( void );
      /** \return \c true, if there is distance to this neighbor known.
       *    \c false otherwise
       */
      bool has_distance( void );
      /** A confident neighbor has a position and distance. Moreover it is not
       *  a twin and has a confidence != 0.
       *
       *  \return \c true, if this neighbor is confident. \c false otherwise
       */
      bool is_confident( void );
      ///@}


      ///@name neighbor information
      ///@{
      /** \return \ref localization::DistanceMap "DistanceMap" of neighborhood
       */
      DistanceMap& neighbors( void );
      /** \param Node Node for which is searched in neighborhood
       *  \return \c true, if given neighbor is part of neighborhood. \c false
       *    otherwise
       */
      bool has_neighbor( node_id_t );
      /** \param Node Node for which is searched in neighborhood
       *  \return \c true, if there is distance for given neighbor known.
       *    \c false otherwise
       */
      bool has_neighbor_distance( node_id_t );
      /** This method returns the distance to given neighbor. If there is no
       *  distance known, method returns localization::UNKNOWN_DISTANCE.
       *
       *  \param Node Node for which is searched in neighborhood
       *  \return distance to given neighbor
       */
      Arithmatic neighbor_distance( node_id_t );
      ///@}


      ///@name iterators on neighbors
      ///@{
      /** \return writable iterator to begin of neighbors
       */
      NeighborIterator begin_neighbors( void )
      { return neighbors_.begin(); }
      /** \return writable iterator to end of neighbors
       */
      NeighborIterator end_neighbors( void )
      { return neighbors_.end(); }
      ///@}

   private:

      node_id_t node_;
      Vec<Arithmatic> pos_;
      int hops_;
      Arithmatic distance_;
      Arithmatic confidence_;

      bool is_anchor_;
      bool is_twin_;

      bool has_pos_;

      NodeSet reference_points_;
      DistanceMap neighbors_;
   };
   // ----------------------------------------------------------------------
   // ----------------------------------------------------------------------
   // ----------------------------------------------------------------------
   template<typename OsModel_P,
            typename node_id_t_P,
            typename NodeSet_P,
            typename DistanceMap_P,
            typename Arithmatic_P>
   LocalizationNeighborInfo<OsModel_P, node_id_t_P, NodeSet_P, DistanceMap_P,Arithmatic_P>::
   LocalizationNeighborInfo( node_id_t node, bool is_anchor )
      : node_        ( node ),
         hops_       ( UNKNOWN_HOP_CNT ),
         distance_   ( UNKNOWN_DISTANCE ),
         confidence_ ( 0 ),
         is_anchor_  ( is_anchor ),
         is_twin_    ( false ),
         has_pos_    ( false )
   {}
   // ----------------------------------------------------------------------
   template<typename OsModel_P,
            typename node_id_t_P,
            typename NodeSet_P,
            typename DistanceMap_P,
            typename Arithmatic_P>
   LocalizationNeighborInfo<OsModel_P, node_id_t_P, NodeSet_P, DistanceMap_P,Arithmatic_P>::
   LocalizationNeighborInfo(  )
      : node_        ( 0 ),
         hops_       ( UNKNOWN_HOP_CNT ),
         distance_   ( UNKNOWN_DISTANCE ),
         confidence_ ( 0 ),
         is_anchor_  ( false ),
         is_twin_    ( false ),
         has_pos_    ( false )
   {}
   // ----------------------------------------------------------------------
   template<typename OsModel_P,
            typename node_id_t_P,
            typename NodeSet_P,
            typename DistanceMap_P,
            typename Arithmatic_P>
   LocalizationNeighborInfo<OsModel_P, node_id_t_P, NodeSet_P, DistanceMap_P, Arithmatic_P>::
   ~LocalizationNeighborInfo()
   {}
   // ----------------------------------------------------------------------
   template<typename OsModel_P,
            typename node_id_t_P,
            typename NodeSet_P,
            typename DistanceMap_P,
            typename Arithmatic_P>
   void
   LocalizationNeighborInfo<OsModel_P, node_id_t_P, NodeSet_P, DistanceMap_P, Arithmatic_P>::
   update_neighbor( node_id_t node, Arithmatic distance )
   {
      neighbors_[node] = distance;
   }
   // ----------------------------------------------------------------------
   template<typename OsModel_P,
            typename node_id_t_P,
            typename NodeSet_P,
            typename DistanceMap_P,
            typename Arithmatic_P>
   void
   LocalizationNeighborInfo<OsModel_P, node_id_t_P, NodeSet_P, DistanceMap_P, Arithmatic_P>::
   update_neighbors( DistanceMap& dm )
   {
      for ( NeighborIterator
               it = dm.begin();
               it != dm.end();
               ++it )
         neighbors_[it->first] = it->second;
   }
   // ----------------------------------------------------------------------
   template<typename OsModel_P,
            typename node_id_t_P,
            typename NodeSet_P,
            typename DistanceMap_P,
            typename Arithmatic_P>
   void
   LocalizationNeighborInfo<OsModel_P, node_id_t_P, NodeSet_P, DistanceMap_P, Arithmatic_P>::
   add_ref_node( node_id_t node )
   {
      reference_points_.insert( node );
   }
   // ----------------------------------------------------------------------
   template<typename OsModel_P,
            typename node_id_t_P,
            typename NodeSet_P,
            typename DistanceMap_P,
            typename Arithmatic_P>
   void
   LocalizationNeighborInfo<OsModel_P, node_id_t_P, NodeSet_P, DistanceMap_P, Arithmatic_P>::
   clear_ref_nodes( void )
   {
      reference_points_.clear();
   }
   // ----------------------------------------------------------------------
   template<typename OsModel_P,
            typename node_id_t_P,
            typename NodeSet_P,
            typename DistanceMap_P,
            typename Arithmatic_P>
   const NodeSet_P&
   LocalizationNeighborInfo<OsModel_P, node_id_t_P, NodeSet_P, DistanceMap_P, Arithmatic_P>::
   ref_nodes( void )
      const
   {
      return reference_points_;
   }
   // ----------------------------------------------------------------------
   template<typename OsModel_P,
            typename node_id_t_P,
            typename NodeSet_P,
            typename DistanceMap_P,
            typename Arithmatic_P>
   NodeSet_P&
   LocalizationNeighborInfo<OsModel_P, node_id_t_P, NodeSet_P, DistanceMap_P, Arithmatic_P>::
   ref_nodes_w( void )
   {
      return reference_points_;
   }
   // ----------------------------------------------------------------------
   template<typename OsModel_P,
            typename node_id_t_P,
            typename NodeSet_P,
            typename DistanceMap_P,
            typename Arithmatic_P>
   void
   LocalizationNeighborInfo<OsModel_P, node_id_t_P, NodeSet_P, DistanceMap_P, Arithmatic_P>::
   set_pos( const wiselib::Vec<Arithmatic_P>& pos )
   {
      pos_ = pos;
      has_pos_ = true;
   }
   // ----------------------------------------------------------------------
   template<typename OsModel_P,
            typename node_id_t_P,
            typename NodeSet_P,
            typename DistanceMap_P,
            typename Arithmatic_P>
   void
   LocalizationNeighborInfo<OsModel_P, node_id_t_P, NodeSet_P, DistanceMap_P, Arithmatic_P>::
   clear_pos()
   {
      pos_ = Vec<Arithmatic_P>(0,0);
      has_pos_ = false;
   }
   // ----------------------------------------------------------------------
   template<typename OsModel_P,
            typename node_id_t_P,
            typename NodeSet_P,
            typename DistanceMap_P,
            typename Arithmatic_P>
   void
   LocalizationNeighborInfo<OsModel_P, node_id_t_P, NodeSet_P, DistanceMap_P, Arithmatic_P>::
   set_hops( int hops )
   {
      hops_ = hops;
   }
   // ----------------------------------------------------------------------
   template<typename OsModel_P,
            typename node_id_t_P,
            typename NodeSet_P,
            typename DistanceMap_P,
            typename Arithmatic_P>
   void
   LocalizationNeighborInfo<OsModel_P, node_id_t_P, NodeSet_P, DistanceMap_P, Arithmatic_P>::
   set_distance( Arithmatic distance )
   {
      distance_ = distance;
   }
   // ----------------------------------------------------------------------
   template<typename OsModel_P,
            typename node_id_t_P,
            typename NodeSet_P,
            typename DistanceMap_P,
            typename Arithmatic_P>
   void
   LocalizationNeighborInfo<OsModel_P, node_id_t_P, NodeSet_P, DistanceMap_P, Arithmatic_P>::
   set_confidence( Arithmatic confidence )
   {
      confidence_ = confidence;
   }
   // ----------------------------------------------------------------------
   template<typename OsModel_P,
            typename node_id_t_P,
            typename NodeSet_P,
            typename DistanceMap_P,
            typename Arithmatic_P>
   void
   LocalizationNeighborInfo<OsModel_P, node_id_t_P, NodeSet_P, DistanceMap_P, Arithmatic_P>::
   set_twin( bool twin )
   {
      is_twin_ = twin;
   }
   // ----------------------------------------------------------------------
   template<typename OsModel_P,
            typename node_id_t_P,
            typename NodeSet_P,
            typename DistanceMap_P,
            typename Arithmatic_P>
   void
   LocalizationNeighborInfo<OsModel_P, node_id_t_P, NodeSet_P, DistanceMap_P, Arithmatic_P>::
   set_anchor( bool anchor )
   {
      is_anchor_ = anchor;
   }
   // ----------------------------------------------------------------------
   template<typename OsModel_P,
            typename node_id_t_P,
            typename NodeSet_P,
            typename DistanceMap_P,
            typename Arithmatic_P>
   void
   LocalizationNeighborInfo<OsModel_P, node_id_t_P, NodeSet_P, DistanceMap_P, Arithmatic_P>::
   set_node( node_id_t node )
   {
      node_ = node;
   }
   // ----------------------------------------------------------------------
   template<typename OsModel_P,
            typename node_id_t_P,
            typename NodeSet_P,
            typename DistanceMap_P,
            typename Arithmatic_P>
   void
   LocalizationNeighborInfo<OsModel_P, node_id_t_P, NodeSet_P, DistanceMap_P,Arithmatic_P>::
   convert_hops( Arithmatic hop_dist )
   {
      if ( hops_ != UNKNOWN_HOP_CNT )
         distance_ = hops_ * hop_dist;
   }
   // ----------------------------------------------------------------------
   template<typename OsModel_P,
            typename node_id_t_P,
            typename NodeSet_P,
            typename DistanceMap_P,
            typename Arithmatic_P>
   node_id_t_P
   LocalizationNeighborInfo<OsModel_P, node_id_t_P, NodeSet_P, DistanceMap_P,Arithmatic_P>::
   node( void )
   {
      return node_;
   }
   // ----------------------------------------------------------------------
   template<typename OsModel_P,
            typename node_id_t_P,
            typename NodeSet_P,
            typename DistanceMap_P,
            typename Arithmatic_P>
   Vec<Arithmatic_P>&
   LocalizationNeighborInfo<OsModel_P, node_id_t_P, NodeSet_P, DistanceMap_P, Arithmatic_P>::
   pos( void )
   {
      return pos_;
   }
   // ----------------------------------------------------------------------
   template<typename OsModel_P,
            typename node_id_t_P,
            typename NodeSet_P,
            typename DistanceMap_P,
            typename Arithmatic_P>
   int
   LocalizationNeighborInfo<OsModel_P, node_id_t_P, NodeSet_P, DistanceMap_P, Arithmatic_P>::
   hops( void )
   {
      return hops_;
   }
   // ----------------------------------------------------------------------
   template<typename OsModel_P,
            typename node_id_t_P,
            typename NodeSet_P,
            typename DistanceMap_P,
            typename Arithmatic_P>
   typename LocalizationNeighborInfo<OsModel_P, node_id_t_P, NodeSet_P, DistanceMap_P, Arithmatic_P>::Arithmatic
   LocalizationNeighborInfo<OsModel_P, node_id_t_P, NodeSet_P, DistanceMap_P, Arithmatic_P>::
   distance( void )
   {
      return distance_;
   }
   // ----------------------------------------------------------------------
   template<typename OsModel_P,
            typename node_id_t_P,
            typename NodeSet_P,
            typename DistanceMap_P,
            typename Arithmatic_P>
   typename LocalizationNeighborInfo<OsModel_P, node_id_t_P, NodeSet_P, DistanceMap_P, Arithmatic_P>::Arithmatic
   LocalizationNeighborInfo<OsModel_P, node_id_t_P, NodeSet_P, DistanceMap_P, Arithmatic_P>::
   confidence( void )
   {
      return confidence_;
   }
   // ----------------------------------------------------------------------
   template<typename OsModel_P,
            typename node_id_t_P,
            typename NodeSet_P,
            typename DistanceMap_P,
            typename Arithmatic_P>
   bool
   LocalizationNeighborInfo<OsModel_P, node_id_t_P, NodeSet_P, DistanceMap_P, Arithmatic_P>::
   is_anchor( void )
   {
      return is_anchor_;
   }
   // ----------------------------------------------------------------------
   template<typename OsModel_P,
            typename node_id_t_P,
            typename NodeSet_P,
            typename DistanceMap_P,
            typename Arithmatic_P>
   bool
   LocalizationNeighborInfo<OsModel_P, node_id_t_P, NodeSet_P, DistanceMap_P, Arithmatic_P>::
   is_valid( void )
   {
      return ( has_pos() && has_distance() );
   }
   // ----------------------------------------------------------------------
   template<typename OsModel_P,
            typename node_id_t_P,
            typename NodeSet_P,
            typename DistanceMap_P,
            typename Arithmatic_P>
   bool
   LocalizationNeighborInfo<OsModel_P, node_id_t_P, NodeSet_P, DistanceMap_P,Arithmatic_P>::
   is_twin( void )
   {
      return is_twin_;
   }
   // ----------------------------------------------------------------------
   template<typename OsModel_P,
            typename node_id_t_P,
            typename NodeSet_P,
            typename DistanceMap_P,
            typename Arithmatic_P>
   bool
   LocalizationNeighborInfo<OsModel_P, node_id_t_P, NodeSet_P, DistanceMap_P,Arithmatic_P>::
   has_pos( void )
   {
      return has_pos_;
   }
   // ----------------------------------------------------------------------
   template<typename OsModel_P,
            typename node_id_t_P,
            typename NodeSet_P,
            typename DistanceMap_P,
            typename Arithmatic_P>
   bool
   LocalizationNeighborInfo<OsModel_P, node_id_t_P, NodeSet_P, DistanceMap_P, Arithmatic_P>::
   has_distance( void )
   {
      return distance_ != UNKNOWN_DISTANCE;
   }
   // ----------------------------------------------------------------------
   template<typename OsModel_P,
            typename node_id_t_P,
            typename NodeSet_P,
            typename DistanceMap_P,
            typename Arithmatic_P>
   bool
   LocalizationNeighborInfo<OsModel_P, node_id_t_P, NodeSet_P, DistanceMap_P,Arithmatic_P>::
   is_confident( void )
   {
      return confidence() != 0 && !is_twin() && has_pos() && has_distance();
   }
   // ----------------------------------------------------------------------
   template<typename OsModel_P,
            typename node_id_t_P,
            typename NodeSet_P,
            typename DistanceMap_P,
            typename Arithmatic_P>
   DistanceMap_P&
   LocalizationNeighborInfo<OsModel_P, node_id_t_P, NodeSet_P, DistanceMap_P,Arithmatic_P>::
   neighbors( void )
   {
      return neighbors_;
   }
   // ----------------------------------------------------------------------
   template<typename OsModel_P,
            typename node_id_t_P,
            typename NodeSet_P,
            typename DistanceMap_P,
            typename Arithmatic_P>
   bool
   LocalizationNeighborInfo<OsModel_P, node_id_t_P, NodeSet_P, DistanceMap_P, Arithmatic_P>::
   has_neighbor( node_id_t node )
   {
      return neighbors_.find( node ) != neighbors_.end();
   }
   // ----------------------------------------------------------------------
   template<typename OsModel_P,
            typename node_id_t_P,
            typename NodeSet_P,
            typename DistanceMap_P,
            typename Arithmatic_P>
   bool
   LocalizationNeighborInfo<OsModel_P, node_id_t_P, NodeSet_P, DistanceMap_P, Arithmatic_P>::
   has_neighbor_distance( node_id_t node )
   {
      return neighbor_distance( node ) != UNKNOWN_DISTANCE;
   }
   // ----------------------------------------------------------------------
   template<typename OsModel_P,
            typename node_id_t_P,
            typename NodeSet_P,
            typename DistanceMap_P,
            typename Arithmatic_P>
   typename LocalizationNeighborInfo<OsModel_P, node_id_t_P, NodeSet_P, DistanceMap_P, Arithmatic_P>::Arithmatic
   LocalizationNeighborInfo<OsModel_P, node_id_t_P, NodeSet_P, DistanceMap_P, Arithmatic_P>::
   neighbor_distance( node_id_t node )
   {
      NeighborIterator it = neighbors_.find( node );

      if ( it != neighbors_.end() )
         return it->second;
      else
         return UNKNOWN_DISTANCE;
   }

}// namespace wiselib
#endif
