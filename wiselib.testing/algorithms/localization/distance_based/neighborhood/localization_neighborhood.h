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
#ifndef __ALGORITHMS_LOCALIZATION_DISTANCE_BASED_NEIGHBORHOOD_NEIGHBORHOOD_H
#define __ALGORITHMS_LOCALIZATION_DISTANCE_BASED_NEIGHBORHOOD_NEIGHBORHOOD_H

#include "algorithms/localization/distance_based/neighborhood/localization_neighbor_info.h"
#include "algorithms/localization/distance_based/math/localization_statistic.h"
#include "algorithms/localization/distance_based/math/vec.h"
#include "algorithms/localization/distance_based/util/localization_defutils.h"
#include "util/pstl/map_static_vector.h"


namespace wiselib
{

   /// Basic neighborhood operations
   /** This class combines \ref LocalizationNeighborInfo "neighbors" to a
    *  neighborhood. In addition to basic methods such like adding an anchor
    *  or neighbor, there are operations on the neighborhood like counting
    *  anchors or something alike.
    *
    *  At first, there are the above mentioned basic methods like
    *  adding/updating anchors or neighbors, methods for searching certain
    *  anchors/neighbors in the neighborhood, same with distances, and simply
    *  methods which return iterators.\n
    *  To add more information to a neighbor, you have to direct access the
    *  LocalizationNeighborInfo via find_w() and resulting iterator.
    *
    *  Second there is a chance to decide, whether the neighborhood is sound
    *  or not. This means, that the received information is unique. The
    *  neighborhood is sound, if there are at least three/four valid anchors,
    *  for which the data is received by different neighbors. Moreover, e.g.
    *  the LocalizationIterLaterationModule does this, you can add so called
    *  sound neighbors, so that the intersection of unique anchors and sound
    *  nodes has to be greater or equal than three/four.\n
    *  The sound nodes are stored in separate set
    *
    *  At last there are operations on the neighborhood. On the one hand
    *  the varying like reassign_twins(), on the other hand just \em const
    *  ones like valid_anchor_cnt() or confident_neighbor_cnt().
    *
    */
   template<typename OsModel_P,
            typename node_id_t_P,
            typename NeighborInfo_P,
            typename NeighborInfoMap_P,
            typename Arithmatic_P >
   class LocalizationNeighborhood
   {

   public:
      typedef OsModel_P OsModel;
      typedef node_id_t_P node_id_t;

      typedef NeighborInfo_P NeighborInfo;
      typedef typename NeighborInfo::NodeSet NodeSet;
      typedef typename NeighborInfo::DistanceMap DistanceMap;

      typedef NeighborInfoMap_P NeighborInfoMap;
      typedef typename NeighborInfoMap::iterator NeighborhoodIterator;

      typedef Arithmatic_P Arithmatic;

      ///@name construction / destruction
      ///@{
      ///
      LocalizationNeighborhood();
      ///
      ~LocalizationNeighborhood();
      ///@}


      ///@name owner info
      ///@{
      /** Set the node this neighborhood belongs to.
       *
       *  \param Node belonging Node
       */
      void set_source( node_id_t );
      /** \return Node this neighborhood belongs to
       */
      node_id_t source( void );
      ///@}


      ///@name add/update nodes to neighborhood
      ///@{
      /** Update an anchor with given data. If the anchor does not exist, it
       *  is created and inserted in the neighborhood.
       *
       *  \param Node anchor to insert/update
       *  \param Vec position of anchor
       *  \param Arithmatic distance to anchor. if distance is not known, the
       *    default value is localization::UNKNOWN_DISTANCE
       */
      void update_anchor( node_id_t, Vec<Arithmatic>, Arithmatic = UNKNOWN_DISTANCE );
      /** Update a neighbor with given data. If the neighbor does not exist,
       *  it is created and inserted in the neighborhood.
       *
       *  \param Node neighbor to insert/update
       *  \param Arithmatic distance to neighbor
       */
      void update_neighbor( node_id_t, Arithmatic );
      /** Update a neighbor of a neighbor with given data, so that the
       *  neighbor's neighborhood is filled/updated. If both Nodes exists
       *  in source neighborhood, the update happens in both ways.
       *
       *  \param Node neighbor to update
       *  \param Node neighbor of above mentioned neighbor
       *  \param Arithmatic distance between Nodes
       *  \sa update_nneighbors()
       */
      void update_nneighbor( node_id_t, node_id_t, Arithmatic );
      /** Update neighbors of a neighbor with given data, so that the
       *  neighbor's neighborhood is filled/updated.
       *
       *  \param Node neighbor to update
       *  \param localization::DistanceMap neighbors of above mentioned
       *    neighbor
       *  \sa update_nneighbor()
       */
      void update_nneighbors( node_id_t, DistanceMap& );
      ///@}


      ///@name reference nodes and sound-check
      ///@{
      /** Set a reference node of a neighbor. Old one(s) will be deleted
       *  and new one set.
       *
       *  \param Node neighbor to update
       *  \param Node reference node
       *  \sa update_ref_node(), update_ref_nodes()
       */
      void set_ref_node( node_id_t, node_id_t );
      /** Set a reference node of a neighbor. Old one(s) will be deleted
       *  and new one set.
       *
       *  Till now this method has the same functionality as set_ref_node().
       *  Maybe later there will be a check, if the update results in a better
       *  \ref is_sound() "sound-check" following of this it is decided,
       *  whether the update happens or not.
       *
       *  \param Node neighbor to update
       *  \param Node reference node
       *  \sa set_ref_node(), update_ref_nodes()
       */
      void update_ref_node( node_id_t, node_id_t );
      /** Set reference nodes of a neighbor. Old one(s) will be deleted
       *  and new one(s) set.
       *
       *  \param Node neighbor to update
       *  \param Node reference nodes
       *  \sa set_ref_node(), update_ref_node()
       */
      template<typename NodeList_P>
      void update_ref_nodes( node_id_t node, const NodeList_P& nl )
      {
         NeighborhoodIterator it = find_w( node );
         if ( it == end_neighborhood() )
            return;

         it->second->clear_ref_nodes();
         it->second->ref_nodes_w().insert( nl.begin(), nl.end() );
      }
      /** \return Set of all reference nodes
       */
      NodeSet ref_nodes( void );
      /** Add sound neighbor. The sound nodes are stored in separate set.
       */
      void add_sound( node_id_t );
      /** To decide, whether the neighborhood is sound or not, the number of
       *  intersection of reference and \ref add_sound() "sound" nodes is
       *  taken. If this number is greater or eual than three/four,
       *  neighborhood is sound.
       *
       *  \return \c true, if neighborhood is sound. \c false otherwise
       */
      bool is_sound( void );
      ///@}


      ///@name information about neighborhood
      ///@{
      /** \param Node anchor to search for
       *
       *  \return \true if neighbor exists and is an anchor
       */
      bool has_anchor( node_id_t );
      /** \param Node neighbor to search for
       *
       *  \return \true if neighbor exists
       *  \sa has_valid_neighbor()
       */
      bool has_neighbor( node_id_t );
      /** \param Node source neighbor
       *  \param Node neighbor of neighbor to search for
       *
       *  \return \true if neighbor of neighbor exists
       *  \sa has_valid_nneighbor()
       */
      bool has_nneighbor( node_id_t, node_id_t );
      /** This method checks, whether a neighbor exits or not, and is valid.
       *  Unlike LocalizationNeighborInfo::is_valid \em valid in this case
       *  just means, that there exists a distance to this neighbor.
       *
       *  \param Node neighbor to search for
       *
       *  \return \true if neighbor exists and is valid
       *  \sa has_neighbor()
       */
      bool has_valid_neighbor( node_id_t );
      /** This method checks, whether a neighbor of neighbor exits or not, and
       *  is valid. Unlike LocalizationNeighborInfo::is_valid \em valid in
       *  this case just means, that there exists a distance of given neighbor
       *  to his neighbor.
       *
       *  \param Node source neighbor
       *  \param Node neighbor of neighbor to search for
       *
       *  \return \true if neighbor of neighbor exists and is valid
       *  \sa has_nneighbor()
       */
      bool has_valid_nneighbor( node_id_t, node_id_t );
      ///@}


      ///@name distance information
      ///@{
      /** \param Node neighbor to search for
       *
       *  \return Distance to given neighbor
       */
      Arithmatic neighbor_distance( node_id_t );
      /** Returns the distance between given neighbors. If both distances are
       *  known, the mean of both is returned. If none is known,
       *  UNKNOWN_DISTANCE is returned.
       *
       *  \param Node source neighbor
       *  \param Node neighbor of neighbor to search for
       *
       *  \return Distance from given neighbor to its neighbor
       */
      Arithmatic nneighbor_distance( node_id_t, node_id_t );
      /** \return localization::DistanceMap of complete neighborhood
       */
      DistanceMap neighbor_distance_map( void );
      ///@}


      ///@name special information
      ///@{
      /** This method count the number of valid anchors. Valid means, that
       *  there is a distance to this anchor known and it has a position.
       *
       *  \return Number of valid anchors
       *  \sa anchor_cnt(), LocalizationNeighborInfo::is_valid()
       */
      int valid_anchor_cnt( void );
      /** This method count the number of anchors, whether they are valid or
       *  not.
       *
       *  \return Number of anchors
       *  \sa valid_anchor_cnt(), LocalizationNeighborInfo::is_valid()
       */
      int anchor_cnt();
      /** This method count the number of confident neighbors. Confident
       *  means, that there is a position and distance known, the neighbor is
       *  not a twin and has a confidence != 0.
       *
       *  \return Number of confident neighbors
       *  \sa valid_anchor_cnt(), LocalizationNeighborInfo::is_confident()
       */
      int confident_neighbor_cnt( void );
      /** This method computes the average confidence of all confident
       *  neighbors. Confident means, that there is a position and distance
       *  known, the neighbor is not a twin and has a confidence != 0.
       *
       *  \return Average confidence of all confident neighbors
       *  \sa confident_neighbor_cnt(),
       *    LocalizationNeighborInfo::is_confident()
       */
      Arithmatic avg_neighbor_confidence( void );
      ///@}


      ///@name work on the neighborhood
      ///@{
      /** This method passes all pairs of neighbors and decide, whether they
       *  are twins or not on the basis of given measure, and set the second
       *  neighbor to a twin in positive cases.\n
       *  The result is, that there is only one neighbor on one position in
       *  the neighborhood, which is not a twin.
       *
       *  \param Arithmatic Measure to decide, whether a neighbor is near enough
       *    to be a twin
       */
      void reassign_twins( Arithmatic );
      ///@}


      ///@name iterators and general functions on neighboorhood
      ///@{
      /** \return writable iterator to begin of neighborhood
       */
      inline NeighborhoodIterator begin_neighborhood( void )
      { return neighborhood_.begin(); }
      /** \return writable iterator to end of neighborhood
       */
      inline NeighborhoodIterator end_neighborhood( void )
      { return neighborhood_.end(); }
      /** \return size of neighborhood
       */
      inline size_t size( void )
      { return neighborhood_.size(); }
      /** \param Node neighbor to search for
       *
       *  \return writable iterator to given neighbor
       */
      inline NeighborhoodIterator find( node_id_t node )
      { return neighborhood_.find( node ); }
      ///@}

      inline NeighborInfoMap get_neighborhood(void )
      { return neighborhood_; }

   private:
      NeighborInfoMap neighborhood_;
      NodeSet sounds_;

      int source_;
   };
   // ----------------------------------------------------------------------
   // ----------------------------------------------------------------------
   // ----------------------------------------------------------------------
   template<typename OsModel_P,
            typename node_id_t_P,
            typename NeighborInfo_P,
            typename NeighborInfoMap_P,
            typename Arithmatic_P>
   LocalizationNeighborhood<OsModel_P, node_id_t_P, NeighborInfo_P, NeighborInfoMap_P,Arithmatic_P>::
   LocalizationNeighborhood()
   {}
   // ----------------------------------------------------------------------
   template<typename OsModel_P,
            typename node_id_t_P,
            typename NeighborInfo_P,
            typename NeighborInfoMap_P,
            typename Arithmatic_P
            >
   LocalizationNeighborhood<OsModel_P, node_id_t_P, NeighborInfo_P, NeighborInfoMap_P,Arithmatic_P>::
   ~LocalizationNeighborhood()
   {}
   // ----------------------------------------------------------------------
   template<typename OsModel_P,
            typename node_id_t_P,
            typename NeighborInfo_P,
            typename NeighborInfoMap_P,
            typename Arithmatic_P>
   void
   LocalizationNeighborhood<OsModel_P, node_id_t_P, NeighborInfo_P, NeighborInfoMap_P,Arithmatic_P>::
   set_source( node_id_t source )
   {
      source_ = source;
   }
   // ----------------------------------------------------------------------
   template<typename OsModel_P,
            typename node_id_t_P,
            typename NeighborInfo_P,
            typename NeighborInfoMap_P,
            typename Arithmatic_P>
   node_id_t_P
   LocalizationNeighborhood<OsModel_P, node_id_t_P, NeighborInfo_P, NeighborInfoMap_P, Arithmatic_P>::
   source( void )
   {
      return source_;
   }
   // ----------------------------------------------------------------------
   template<typename OsModel_P,
            typename node_id_t_P,
            typename NeighborInfo_P,
            typename NeighborInfoMap_P,
            typename Arithmatic_P>
   void
   LocalizationNeighborhood<OsModel_P, node_id_t_P, NeighborInfo_P, NeighborInfoMap_P, Arithmatic_P>::
   update_anchor( node_id_t node, Vec<Arithmatic> pos, Arithmatic distance )
   {
      NeighborhoodIterator it = find( node );
      if ( it != end_neighborhood() )
      {
         it->second.set_pos( pos );
         if ( distance != UNKNOWN_DISTANCE )
            it->second.set_distance( distance );
         it->second.set_anchor( true );
         return;
      }

      // TODO: NO NEW!!!
//       NeighborInfo *nih = new NeighborInfo( node, true );
//       nih->set_pos( pos );
//       if ( distance != UNKNOWN_DISTANCE )
//          nih->set_distance( distance );
//          neighborhood_[node] = nih;

//       NeighborInfo nih2 =  NeighborInfo( node, true );
//       temp_[node] = nih2;
//       neighborhood_[node] = &(temp_[index_]);
//       index_++;

      neighborhood_[node].set_node(node);
      neighborhood_[node].set_anchor(true);
      neighborhood_[node].set_pos( pos );
      if ( distance != UNKNOWN_DISTANCE )
         neighborhood_[node].set_distance( distance );
   }
   // ----------------------------------------------------------------------
   template<typename OsModel_P,
            typename node_id_t_P,
            typename NeighborInfo_P,
            typename NeighborInfoMap_P,
            typename Arithmatic_P>
   void
   LocalizationNeighborhood<OsModel_P, node_id_t_P, NeighborInfo_P, NeighborInfoMap_P, Arithmatic_P>::
   update_neighbor( node_id_t node, Arithmatic distance )
   {
      NeighborhoodIterator it = find( node );
      if ( it != end_neighborhood() )
      {
         it->second->set_distance( distance );
         return;
      }

      // TODO: NO NEW!!!
//       NeighborInfo *nih = new NeighborInfo( node, false );
//       nih->set_distance( distance );
//       neighborhood_[node] = nih;

//       NeighborInfo nih2 =  NeighborInfo( node, false );
//       temp_[node] = nih2;
//             neighborhood_[node] = &(temp_[index_]);
//             index_++;

      neighborhood_[node].set_node(node);
      neighborhood_[node].set_anchor(false);
      neighborhood_[node].set_distance( distance );
   }
   // ----------------------------------------------------------------------
   template<typename OsModel_P,
            typename node_id_t_P,
            typename NeighborInfo_P,
            typename NeighborInfoMap_P,
            typename Arithmatic_P>
   void
   LocalizationNeighborhood<OsModel_P, node_id_t_P, NeighborInfo_P, NeighborInfoMap_P, Arithmatic_P>::
   update_nneighbor( node_id_t node, node_id_t neighbor, Arithmatic distance )
   {
      if ( node == neighbor )
         return;

      NeighborhoodIterator it;

      if ( ( it = find( node ) ) != end_neighborhood() )
         it->second->update_neighbor( neighbor, distance );

      if ( ( it = find( neighbor ) ) != end_neighborhood() )
         it->second->update_neighbor( node, distance );
   }
   // ----------------------------------------------------------------------
   template<typename OsModel_P,
            typename node_id_t_P,
            typename NeighborInfo_P,
            typename NeighborInfoMap_P,
            typename Arithmatic_P>
   void
   LocalizationNeighborhood<OsModel_P, node_id_t_P, NeighborInfo_P, NeighborInfoMap_P, Arithmatic_P>::
   update_nneighbors( node_id_t node, DistanceMap& dm )
   {
      NeighborhoodIterator it = find( node );
      if ( it == end_neighborhood() )
         return;

      it->second->update_neighbors( dm );
   }
   // ----------------------------------------------------------------------
   template<typename OsModel_P,
            typename node_id_t_P,
            typename NeighborInfo_P,
            typename NeighborInfoMap_P,
            typename Arithmatic_P>
   void
   LocalizationNeighborhood<OsModel_P, node_id_t_P, NeighborInfo_P, NeighborInfoMap_P,Arithmatic_P>::
   set_ref_node( node_id_t node, node_id_t ref )
   {
      NeighborhoodIterator it = find( node );
      if ( it == end_neighborhood() )
         return;

      it->second.clear_ref_nodes();
      it->second.add_ref_node( ref );
   }
   // ----------------------------------------------------------------------
   template<typename OsModel_P,
            typename node_id_t_P,
            typename NeighborInfo_P,
            typename NeighborInfoMap_P,
            typename Arithmatic_P>
   void
   LocalizationNeighborhood<OsModel_P, node_id_t_P, NeighborInfo_P, NeighborInfoMap_P,Arithmatic_P>::
   update_ref_node( node_id_t node, node_id_t ref )
   {
      NeighborhoodIterator it = find( node );
      if ( it == end_neighborhood() )
         return;

//FIXME: check, if 'set of ref-nodes' is getting better/greater by adding the node.
//  -> Too much computing???
      it->second.clear_ref_nodes();
      it->second.add_ref_node( ref );
   }
   // ----------------------------------------------------------------------
   template<typename OsModel_P,
            typename node_id_t_P,
            typename NeighborInfo_P,
            typename NeighborInfoMap_P,
            typename Arithmatic_P>
   typename LocalizationNeighborhood<OsModel_P, node_id_t_P, NeighborInfo_P, NeighborInfoMap_P,Arithmatic_P>::NodeSet
   LocalizationNeighborhood<OsModel_P, node_id_t_P, NeighborInfo_P, NeighborInfoMap_P,Arithmatic_P>::
   ref_nodes( void )
   {
      NodeSet temp;

      for ( NeighborhoodIterator
               it = begin_neighborhood();
               it != end_neighborhood();
               ++it )
         if ( it->second->is_anchor() && it->second->is_valid() )
         {
            NodeSet references = it->second->ref_nodes();
            for ( typename NodeSet::iterator
                     refit = references.begin();
                     refit != references.end();
                     ++refit )
               temp.insert( *refit );
         }

      return temp;
   }
   // ----------------------------------------------------------------------
   template<typename OsModel_P,
            typename node_id_t_P,
            typename NeighborInfo_P,
            typename NeighborInfoMap_P,
            typename Arithmatic_P>
   void
   LocalizationNeighborhood<OsModel_P, node_id_t_P, NeighborInfo_P, NeighborInfoMap_P,Arithmatic_P>::
   add_sound( node_id_t node )
   {
      sounds_.insert( node );
   }
   // ----------------------------------------------------------------------
   template<typename OsModel_P,
            typename node_id_t_P,
            typename NeighborInfo_P,
            typename NeighborInfoMap_P,
            typename Arithmatic_P>
   bool
   LocalizationNeighborhood<OsModel_P, node_id_t_P, NeighborInfo_P, NeighborInfoMap_P, Arithmatic_P>::
   is_sound( void )
   {
      NodeSet temp = ref_nodes();

      for ( typename NodeSet::iterator
               sit = sounds_.begin();
               sit != sounds_.end();
               ++sit )
         temp.insert( *sit );

      // if there is no info about ref-nodes of the anchors, ignore this
      // check return true anyway
      if ( anchor_cnt() > 0 && temp.size() == 0 )
         return true;

//TODO: Dimension anpassen !!!!
      return (int)temp.size() >= 3;
   }
   // ----------------------------------------------------------------------
   template<typename OsModel_P,
            typename node_id_t_P,
            typename NeighborInfo_P,
            typename NeighborInfoMap_P,
            typename Arithmatic_P>
   bool
   LocalizationNeighborhood<OsModel_P, node_id_t_P, NeighborInfo_P, NeighborInfoMap_P, Arithmatic_P>::
   has_anchor( node_id_t node )
   {
      NeighborhoodIterator it = neighborhood_.find( node );

      return ( it != end_neighborhood() && it->second->is_anchor() );
   }
   // ----------------------------------------------------------------------
   template<typename OsModel_P,
            typename node_id_t_P,
            typename NeighborInfo_P,
            typename NeighborInfoMap_P,
            typename Arithmatic_P>
   bool
   LocalizationNeighborhood<OsModel_P, node_id_t_P, NeighborInfo_P, NeighborInfoMap_P, Arithmatic_P>::
   has_neighbor( node_id_t node )
   {
      return ( neighborhood_.find( node ) != end_neighborhood() );
   }
   // ----------------------------------------------------------------------
   template<typename OsModel_P,
            typename node_id_t_P,
            typename NeighborInfo_P,
            typename NeighborInfoMap_P,
            typename Arithmatic_P>
   bool
   LocalizationNeighborhood<OsModel_P, node_id_t_P, NeighborInfo_P, NeighborInfoMap_P, Arithmatic_P>::
   has_nneighbor( node_id_t node, node_id_t neighbor )
   {
      if ( node == neighbor )
         return false;

      NeighborhoodIterator it = neighborhood_.find( node );
      if ( it == end_neighborhood() )
         return false;

      return it->second->has_neighbor( neighbor );
   }
   // ----------------------------------------------------------------------
   template<typename OsModel_P,
            typename node_id_t_P,
            typename NeighborInfo_P,
            typename NeighborInfoMap_P,
            typename Arithmatic_P>
   bool
   LocalizationNeighborhood<OsModel_P, node_id_t_P, NeighborInfo_P, NeighborInfoMap_P, Arithmatic_P>::
   has_valid_neighbor( node_id_t node )
   {
      NeighborhoodIterator it = neighborhood_.find( node );
      if ( it == end_neighborhood() ) return false;

      return ( it->second->has_distance() );
   }
   // ----------------------------------------------------------------------
   template<typename OsModel_P,
            typename node_id_t_P,
            typename NeighborInfo_P,
            typename NeighborInfoMap_P,
            typename Arithmatic_P>
   bool
   LocalizationNeighborhood<OsModel_P, node_id_t_P, NeighborInfo_P, NeighborInfoMap_P, Arithmatic_P>::
   has_valid_nneighbor( node_id_t node, node_id_t neighbor )
   {
      if ( node == neighbor )
         return false;

      NeighborhoodIterator it = neighborhood_.find( node );
      if ( it == end_neighborhood() ) return false;

      return ( it->second->has_neighbor( neighbor ) &&
                  it->second->has_neighbor_distance( neighbor ) );
   }
   // ----------------------------------------------------------------------
   template<typename OsModel_P,
            typename node_id_t_P,
            typename NeighborInfo_P,
            typename NeighborInfoMap_P,
            typename Arithmatic_P>
   typename LocalizationNeighborhood<OsModel_P, node_id_t_P, NeighborInfo_P, NeighborInfoMap_P, Arithmatic_P>::Arithmatic
   LocalizationNeighborhood<OsModel_P, node_id_t_P, NeighborInfo_P, NeighborInfoMap_P, Arithmatic_P>::
   neighbor_distance( node_id_t node )
   {
      NeighborhoodIterator it = neighborhood_.find( node );
      if ( it == end_neighborhood() )
         return UNKNOWN_DISTANCE;

      return ( it->second->distance() );
   }
   // ----------------------------------------------------------------------
   template<typename OsModel_P,
            typename node_id_t_P,
            typename NeighborInfo_P,
            typename NeighborInfoMap_P,
            typename Arithmatic_P>
   typename LocalizationNeighborhood<OsModel_P, node_id_t_P, NeighborInfo_P, NeighborInfoMap_P, Arithmatic_P>::Arithmatic
   LocalizationNeighborhood<OsModel_P, node_id_t_P, NeighborInfo_P, NeighborInfoMap_P, Arithmatic_P>::
   nneighbor_distance( node_id_t node, node_id_t neighbor )
   {
      if ( node == neighbor )
         return 0;

      NeighborhoodIterator it1 = neighborhood_.find( node );
      NeighborhoodIterator it2 = neighborhood_.find( neighbor );

      Arithmatic sum = 0.0;

      if ( it1 != end_neighborhood() &&
            it1->second->has_neighbor_distance( neighbor ) )
         sum += it1->second->neighbor_distance( neighbor );
      else
         return UNKNOWN_DISTANCE;

      if ( it2 != end_neighborhood() &&
            it2->second->has_neighbor_distance( node ) )
         sum += it2->second->neighbor_distance( node );
      else
         return UNKNOWN_DISTANCE;

      return sum / 2;
#if 0
      if ( node == neighbor )
         return 0;

      NeighborhoodIterator it1 = neighborhood_.find( node );
      NeighborhoodIterator it2 = neighborhood_.find( neighbor );

      LocalizationStatistic<OsModel> stat;

      if ( it1 != end_neighborhood() &&
            it1->second->has_neighbor_distance( neighbor ) )
         stat += it1->second->neighbor_distance( neighbor );

      if ( it2 != end_neighborhood() &&
            it2->second->has_neighbor_distance( node ) )
         stat += it2->second->neighbor_distance( node );

      if ( stat.size() == 0 )
         return UNKNOWN_DISTANCE;

      return ( stat.mean() );
#endif
return 0;
   }
   // ----------------------------------------------------------------------
   template<typename OsModel_P,
            typename node_id_t_P,
            typename NeighborInfo_P,
            typename NeighborInfoMap_P,
            typename Arithmatic_P>
   typename LocalizationNeighborhood<OsModel_P, node_id_t_P, NeighborInfo_P, NeighborInfoMap_P, Arithmatic_P>::DistanceMap
   LocalizationNeighborhood<OsModel_P, node_id_t_P, NeighborInfo_P, NeighborInfoMap_P, Arithmatic_P>::
   neighbor_distance_map( void )
   {
      DistanceMap temp;

      for ( NeighborhoodIterator
               it = begin_neighborhood();
               it != end_neighborhood();
               ++it )
         temp[it->second->node()] = it->second->distance();

      return temp;
   }
   // ----------------------------------------------------------------------
   template<typename OsModel_P,
            typename node_id_t_P,
            typename NeighborInfo_P,
            typename NeighborInfoMap_P,
            typename Arithmatic_P>
   int
   LocalizationNeighborhood<OsModel_P, node_id_t_P, NeighborInfo_P, NeighborInfoMap_P, Arithmatic_P>::
   valid_anchor_cnt( void )
   {
      int cnt = 0;

      for ( NeighborhoodIterator
               it = begin_neighborhood();
               it != end_neighborhood();
               ++it )
         if ( it->second->is_anchor() && it->second->is_valid() )
            ++cnt;

      return cnt;
   }
   // ----------------------------------------------------------------------
   template<typename OsModel_P,
            typename node_id_t_P,
            typename NeighborInfo_P,
            typename NeighborInfoMap_P,
            typename Arithmatic_P>
   int
   LocalizationNeighborhood<OsModel_P, node_id_t_P, NeighborInfo_P, NeighborInfoMap_P, Arithmatic_P>::
   anchor_cnt()
   {
      int cnt = 0;

      for ( NeighborhoodIterator
               it = begin_neighborhood();
               it != end_neighborhood();
               ++it )
         if ( it->second.is_anchor() )
            ++cnt;

      return cnt;
   }
   // ----------------------------------------------------------------------
   template<typename OsModel_P,
            typename node_id_t_P,
            typename NeighborInfo_P,
            typename NeighborInfoMap_P,
            typename Arithmatic_P>
   int
   LocalizationNeighborhood<OsModel_P, node_id_t_P, NeighborInfo_P, NeighborInfoMap_P, Arithmatic_P>::
   confident_neighbor_cnt( void )
   {
      int cnt = 0;

      for ( NeighborhoodIterator
               it = begin_neighborhood();
               it != end_neighborhood();
               ++it )
         if ( it->second->is_confident() )
            ++cnt;

      return cnt;
   }
   // ----------------------------------------------------------------------
   template<typename OsModel_P,
            typename node_id_t_P,
            typename NeighborInfo_P,
            typename NeighborInfoMap_P,
            typename Arithmatic_P>
   typename LocalizationNeighborhood<OsModel_P, node_id_t_P, NeighborInfo_P, NeighborInfoMap_P, Arithmatic_P>::Arithmatic
   LocalizationNeighborhood<OsModel_P, node_id_t_P, NeighborInfo_P, NeighborInfoMap_P, Arithmatic_P>::
   avg_neighbor_confidence( void )
   {
      Arithmatic conf = 0;
      int count = 0;

      for ( NeighborhoodIterator
               it = begin_neighborhood();
               it != end_neighborhood();
               ++it )
         if ( it->second->is_confident() )
         {
            ++count;
            conf += it->second->confidence();
         }

      return ( conf / count );
   }
   // ----------------------------------------------------------------------
   template<typename OsModel_P,
            typename node_id_t_P,
            typename NeighborInfo_P,
            typename NeighborInfoMap_P,
            typename Arithmatic_P>
   void
   LocalizationNeighborhood<OsModel_P, node_id_t_P, NeighborInfo_P, NeighborInfoMap_P, Arithmatic_P>::
   reassign_twins( Arithmatic twin_measure )
   {
      // main task of this cycle is to unset all twins in the neighborhood.
      // additionally it is checked, whether the neighbor is a twin of the
      // source node or not.
      for ( NeighborhoodIterator
               it = begin_neighborhood();
               it != end_neighborhood();
               ++it )
// TODO!!
//          if ( euclidean_distance(
//                   source().est_position(), it->second->node().est_position() )
//                <= twin_measure )
//             it->second->set_twin( true );
//          else
//             it->second->set_twin( false );

      // this cycle sorts out all neighbors that are twins to each other, so
      // that after this check there is only one neighbor on one position.
      for ( NeighborhoodIterator
               it1 = begin_neighborhood();
               it1 != end_neighborhood();
               ++it1 )
         for ( NeighborhoodIterator
                  it2 = it1;
                  it2 != end_neighborhood();
                  ++it2 )
         {
            if ( it1 == it2 )
               continue;

            if ( !it1->second->has_pos() || !it2->second->has_pos() )
               continue;

            if ( euclidean_distance( it1->second->pos(), it2->second->pos() )
                  <= twin_measure )
               it2->second->set_twin( true );
         }
   }

}// namespace wiselib
#endif
