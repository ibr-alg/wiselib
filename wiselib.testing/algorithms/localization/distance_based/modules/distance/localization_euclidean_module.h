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
#ifndef __ALGORITHMS_LOCALIZATION_DISTANCE_BASED_LOCALIZATION_EUCLIEDEAN_MODULE_H
#define __ALGORITHMS_LOCALIZATION_DISTANCE_BASED_LOCALIZATION_EUCLIEDEAN_MODULE_H

#include "algorithms/localization/distance_based/modules/localization_module.h"
#include "algorithms/localization/distance_based/modules/distance/localization_euclidean_messages.h"
#include "algorithms/localization/distance_based/util/localization_defutils.h"
#include "algorithms/localization/distance_based/math/localization_triangulation.h"
#include "algorithms/localization/distance_based/math/localization_statistic.h"
#include "config_testing.h"


namespace wiselib
{

   ///@name euclidean module parameters
   ///@{
   const char* EUCL_COL_CHECK_STD[] = { "lax", "strict", "one" };
   const char* EUCL_COL_CHECK_NV[] = { "lax", "strict", "one" };
   const char* EUCL_COL_CHECK_CN[] = { "lax", "strict", "one" };
   const char* EUCL_ALGO[] = { "normal", "opt" };
   const char* EUCL_VOTE[] = { "nv", "cn", "nvcn", "cnnv" };
   ///@}


   /// Module implementing euclidean distance estimation
   /** This module implements euclidean distance estimation. Idea is to
    *  compute the real distances to anchors.
    *
    *  If unknown receives a message from two neighbors that know their
    *  distance to an anchor and each other, unknown is able to get two
    *  possible distances to the anchor via trilateration. One of these
    *  distances is right, the other is wrong. To decide, which of these
    *  distances is right, there are two different methods, named
    *  'neighbor vote' and 'common neighbor'.
    *
    *  The first, 'neighbor vote', needs at least one more neighbor, that has
    *  a distance to the anchor and one of the first mentioned neighbors. Now
    *  the trilateration is done a second time, resulting again in two
    *  distances. Then you take the distances of first and second pair, which
    *  are nearest to each other.
    *
    *  Second method, 'common neighbor', needs one more neighbor that has a
    *  distance to the anchor and both of first mentioned neighbors. Basic
    *  geometric reasoning leads to the right solution.
    */
   template<typename OsModel_P,
            typename Radio_P,
            typename Clock_P,
            typename Distance_P,
            typename Debug_P,
            typename SharedData_P,
            typename DistancePair_P>
   class LocalizationEuclideanModule
      : public LocalizationModule<OsModel_P, Radio_P, SharedData_P>
   {

   public:
      typedef OsModel_P OsModel;
      typedef Radio_P Radio;
      typedef Clock_P Clock;
      typedef Distance_P Distance;
      typedef Debug_P Debug;
      typedef SharedData_P SharedData;
      typedef DistancePair_P DistancePair;

      typedef LocalizationSumDistModule<OsModel, Radio, Clock, Distance, Debug, SharedData> self_type;
      typedef LocalizationModule<OsModel, Radio, SharedData> base_type;

      typedef typename Radio::size_t size_t;
      typedef typename Radio::node_id_t node_id_t;
      typedef typename Radio::block_data_t block_data_t;

      typedef typename Clock_P::time_t time_t;

      typedef typename SharedData::DistanceMap DistanceMap;
      typedef typename SharedData::NodeList NodeList;
      typedef typename NodeList::iterator NodeListIterator;

      typedef LocalizationEuclideanInitMessage<OsModel, Radio> EuclideanInitMessage;
      typedef LocalizationEuclideanAnchorMessage<OsModel, Radio> EuclideanAnchorMessage;
      typedef LocalizationEuclideanNeighborMessage<OsModel, Radio, DistanceMap> EuclideanNeighborMessage;

      typedef LocalizationStatistic<OsModel> Statistics;

      typedef typename SharedData::Neighborhood::NeighborhoodIterator NeighborhoodIterator;

      enum EuclideanCollinearCheckStd
      {
         eu_cc_std_lax,
         eu_cc_std_strict,
         eu_cc_std_none
      };

      enum EuclideanCollinearCheckNV
      {
         eu_cc_nv_lax,
         eu_cc_nv_strict,
         eu_cc_nv_none
      };

      enum EuclideanCollinearCheckCN
      {
         eu_cc_cn_lax,
         eu_cc_cn_strict,
         eu_cc_cn_none
      };

      enum EuclideanAlgo
      {
         eu_algo_normal,
         eu_algo_opt
      };

      enum EuclideanVote
      {
         eu_vote_nv,
         eu_vote_cn,
         eu_vote_nvcn,
         eu_vote_cnnv
      };

      ///@name construction / destruction
      ///@{
      ///
      LocalizationEuclideanModule();
      ///
      ~LocalizationEuclideanModule();
      ///@}


      ///@name standard methods startup/simulation steps
      ///@{
      /** Handling of Euclidean-Messages.
       *
       *  \sa LocalizationModule::process_message()
       */
      void receive( node_id_t from, size_t len, block_data_t *data );
      /** Check, whether state can be set to finished or not. Moreover, send
       *  initial messages.
       *
       *  \sa LocalizationModule::work()
       */
      void work( void );
      ///@}


      ///@name module status info
      ///@{
      /** \return \c true, if module is finished. \c false otherwise
       *  \sa LocalizationModule::finished()
       */
      bool finished( void );
      ///@}

      void rollback( void );


      void init( Radio& radio, Clock& clock, Debug& debug, SharedData& shared_data, Distance& distance ) {
         radio_ = &radio;
         clock_ = &clock;
         debug_ = &debug;
         this->set_shared_data( shared_data );
         distance_ = &distance;
      }

   protected:

      ///@name processing euclidean messages
      ///@{
      /** This method processes initial messages. Source of message is added
       *  to neighborhood.
       *
       *  \sa LocalizationEuclideanInitMessage
       */
      bool process_euclidean_init_message( node_id_t from, size_t len, block_data_t *data );
      /** This method broadcasts the neighborhood generated by the initial
       *  messages.
       *
       *  \sa LocalizationEuclideanNeighborMessage
       */
      void broadcast_neighborhood( void );
      /** This method processes neighborhood messages. The neighbors of
       *  message source are added to neighborhood.
       *
       *  \sa LocalizationEuclideanNeighborMessage
       */
      bool process_euclidean_neighbor_message( node_id_t from, size_t len, block_data_t *data );
      /** This method processes anchor messages. The message tells, that a
       *  neighbor got a distance to some anchor. New information is added
       *  to neighborhood and the module tries to estimate/compute distance
       *  to mentioned anchor.
       *
       *  \sa LocalizationEuclideanAnchorMessage, execute_euclidean()
       */
      bool process_euclidean_anchor_message( node_id_t from, size_t len, block_data_t *data );
      /** This method tries to estimate/compute a new anchor distance.
       *
       *  \param Node given anchor
       *  \sa find_anchor_distance(), find_anchor_distance_opt()
       */
      void execute_euclidean( node_id_t anchor );
      ///@}


      ///@name work on neighborhood
      ///@{
      /** This method searches for two valid neighbors, that know their
       *  distance to given anchor and each other. If found, it tries to apply
       *  as well mentioned 'neighbor vote' as 'common neighbor'.
       *
       *  Per parameters you can tell, if just one or both methods are used.
       *
       *  If wanted solution is reached, the search ends.
       *
       *  \param Node given anchor
       *  \sa find_anchor_distance_opt()
       */
      double find_anchor_distance( node_id_t anchor );
      /** This method searches for two valid neighbors, that know their
       *  distance to given anchor and each other. If found, it tries to apply
       *  as well mentioned 'neighbor vote' as 'common neighbor'.
       *
       *  Unlike find_anchor_distance(), it just searches a 'common neighbor'
       *  for both vote methods. Because of some problems with collinear nodes
       *  it takes the 'fewest collinear' solution.
       *
       *  \param Node given anchor
       *  \sa find_anchor_distance()
       */
      double find_anchor_distance_opt( node_id_t anchor );
      /** There are an anchor and two neighbors given. This method searches
       *  for all neighbors, that either are neighbors of first or second, and
       *  have a link to self and anchor.
       *
       *  \param Node given anchor
       *  \param Node first neighbor
       *  \param Node second neighbor
       *  \return NodeList above mentioned neighbors
       *  \sa find_common_neighbor_neighbors(),
       *    find_common_neighbor_neighbors_opt()
       */
      NodeList find_unique_neighbor_neighbors( node_id_t anchor, node_id_t n1, node_id_t n2 );
      /** There are an anchor and two neighbors given. This method searches
       *  for all neighbors, that are neighbors of first and second, and
       *  have a link to self and anchor.
       *
       *  \param Node given anchor
       *  \param Node first neighbor
       *  \param Node second neighbor
       *  \return NodeList above mentioned neighbors
       *  \sa find_unique_neighbor_neighbors(),
       *    find_common_neighbor_neighbors_opt()
       */
      NodeList find_common_neighbor_neighbors( node_id_t anchor, node_id_t n1, node_id_t n2 );
      /** There are an anchor and two neighbors given. This method searches
       *  for all neighbors, that are neighbors of first and second, and
       *  have a link to self and anchor.
       *
       *  Because of conflicts with collinear neighbors, the result is the
       *  'fewest collinear' neighbor pair.
       *
       *  \param Node given anchor
       *  \param Node first neighbor
       *  \param Node second neighbor
       *  \return NodeList above mentioned neighbor
       *  \sa find_common_neighbor_neighbors(),
       *    find_unique_neighbor_neighbors()
       */
      NodeList find_common_neighbor_neighbors_opt( node_id_t, node_id_t, node_id_t, double& );
      /** This method decides, which of given feasible solutions is the right
       *  one. The solutions are a result of trilateration with distances
       *  between self, two neighbors and an anchor.
       *
       *  The neighbor vote applies the same procedure with given \em third
       *  \em neighbors and each of both first mentioned neighbors. This
       *  results in at least two different localization::DistancePairs. Now
       *  the results are split into two parts. One with the distances, which
       *  are nearest to each other, and one with the others.\n
       *  The mean of first mentioned part should be the right solution.
       *
       *  Moreover, there are two checks to decide, whether the solution is
       *  valid or not. At first, if the nodes are collinear, it is hard to
       *  select the right alternative, because the difference in distances in
       *  both parts is very low. These cases are filtered out by the
       *  requirement, that the standard deviation in one part must be at most
       *  1/3rd of the standard deviation of the other part.\n
       *  Second, if there is one neighbor with incorrect information, this
       *  could result in two wrong votes. This case is filtered out by the
       *  requirement, that the standard deviation of selected part is at most
       *  5% of mean.
       *
       *  \param Node given anchor
       *  \param Node first neighbor
       *  \param Node second neighbor
       *  \param localization::DistancePair two feasible solutions
       *  \param localization::NodeList list of \em third \em neighbors
       *  \return Best fit distance of given DistancePair. If there is no
       *    decision or a check fails, method returns -1
       *  \sa common_neighbor()
       */
      double neighbor_vote(
            node_id_t, node_id_t, node_id_t,
            DistancePair&, NodeList& );
      /** This method decides, which of given feasible solutions is the right
       *  one. The solutions are a result of trilateration with distances
       *  between self, two neighbors and an anchor.
       *
       *  A third neighbor is taken, that has distance to both mentioned
       *  neighbors, self and anchor. Basic geometric reasoning leads to the
       *  right solution.
       *
       *  \param Node given anchor
       *  \param Node first neighbor
       *  \param Node second neighbor
       *  \param localization::DistancePair two feasible solutions
       *  \param localization::NodeList list of \em third \em neighbors
       *  \return Best fit distance of given DistancePair. If there is no
       *    decision, method returns -1
       *  \sa neighbor_vote()
       */
      double common_neighbor(
            node_id_t, node_id_t, node_id_t,
            DistancePair&, NodeList& );
      ///@}


      ///@name parametrization
      ///@{
      /** Read the given parameters, which have been set via simulator
       *  commands or configuration file.
       */
      void set_collinear_check_std( EuclideanCollinearCheckStd check )
      { cc_std_ = check; }
      void set_collinear_check_nv( EuclideanCollinearCheckNV check )
      { cc_nv_ = check; }
      void set_collinear_check_cn( EuclideanCollinearCheckCN check )
      { cc_cn_ = check; }
      void set_euclidean_algo( EuclideanAlgo algo )
      { algo_ = algo; }
      void set_euclidean_algo( EuclideanVote vote )
      { vote_ = vote; }
      ///@}


   private:

      enum MessagesIds
      {
         EUCLIDEAN_INIT_MESSAGE = 203,
         EUCLIDEAN_ANCHOR_MESSAGE = 204,
         EUCLIDEAN_NEIGHBOR_MESSAGE = 205
      };

      enum EuclideanState
      {
         eu_init,
         eu_wait,
         eu_broadcast,
         eu_work,
         eu_finished
      };

      EuclideanState state_;
      EuclideanCollinearCheckStd cc_std_;
      EuclideanCollinearCheckNV cc_nv_;
      EuclideanCollinearCheckCN cc_cn_;
      EuclideanAlgo algo_;
      EuclideanVote vote_;

      time_t last_useful_msg_;
      double col_measure_;

      Radio* radio_;
      Clock* clock_;
      Debug* debug_;
      Distance* distance_;
   };
   // ----------------------------------------------------------------------
   // ----------------------------------------------------------------------
   // ----------------------------------------------------------------------
   template<typename OsModel_P,
            typename Radio_P,
            typename Clock_P,
            typename Distance_P,
            typename Debug_P,
            typename SharedData_P,
            typename DistancePair_P>
   LocalizationEuclideanModule<OsModel_P, Radio_P, Clock_P, Distance_P, Debug_P, SharedData_P, DistancePair_P>::
   LocalizationEuclideanModule()
      : state_            ( eu_init ),
         cc_std_          ( eu_cc_std_lax ),
         cc_nv_           ( eu_cc_nv_lax ),
         cc_cn_           ( eu_cc_cn_strict ),
         algo_            ( eu_algo_opt ),
         vote_            ( eu_vote_nvcn ),
         last_useful_msg_ ( 0 ),
         col_measure_     ( 0 )
   {}
   // ----------------------------------------------------------------------
   template<typename OsModel_P,
            typename Radio_P,
            typename Clock_P,
            typename Distance_P,
            typename Debug_P,
            typename SharedData_P,
            typename DistancePair_P>
   LocalizationEuclideanModule<OsModel_P, Radio_P, Clock_P, Distance_P, Debug_P, SharedData_P, DistancePair_P>::
   ~LocalizationEuclideanModule()
   {}
   // ----------------------------------------------------------------------
   template<typename OsModel_P,
            typename Radio_P,
            typename Clock_P,
            typename Distance_P,
            typename Debug_P,
            typename SharedData_P,
            typename DistancePair_P>
   void
   LocalizationEuclideanModule<OsModel_P, Radio_P, Clock_P, Distance_P, Debug_P, SharedData_P, DistancePair_P>::
   receive( node_id_t from, size_t len, block_data_t *data )
   {
      switch ( data[0] )
      {
         case EUCLIDEAN_INIT_MESSAGE:
            process_euclidean_init_message( from, len, data );
            break;
         case EUCLIDEAN_ANCHOR_MESSAGE:
            process_euclidean_anchor_message( from, len, data );
            break;
         case EUCLIDEAN_NEIGHBOR_MESSAGE:
            process_euclidean_neighbor_message( from, len, data );
            break;
      }
   }
   // ----------------------------------------------------------------------
   template<typename OsModel_P,
            typename Radio_P,
            typename Clock_P,
            typename Distance_P,
            typename Debug_P,
            typename SharedData_P,
            typename DistancePair_P>
   void
   LocalizationEuclideanModule<OsModel_P, Radio_P, Clock_P, Distance_P, Debug_P, SharedData_P, DistancePair_P>::
   work( void )
   {
      // send initial messages
      if ( state_ == eu_init )
      {
         if ( this->shared_data().is_anchor() )
         {
            EuclideanInitMessage message;
            message.set_msg_id( EUCLIDEAN_INIT_MESSAGE );
            message.set_anchor( true );
            message.set_source_position( this->shared_data().position() );
            radio_->send(  Radio::BROADCAST_ADDRESS,
                         message.buffer_size(), (block_data_t*)&message );
         }
         else
         {
            EuclideanInitMessage message;
            message.set_msg_id( EUCLIDEAN_INIT_MESSAGE );
            message.set_anchor( false );
            radio_->send( Radio::BROADCAST_ADDRESS,
                         message.buffer_size(), (block_data_t*)&message );
         }

         state_ = eu_wait;
      }

      // after idle-time passed, initial messages of neighbors had already been
      // received and state is set to 'broadcast'.
      if ( state_ == eu_wait &&
               clock_->time() - last_useful_msg_ >
                  this->shared_data().idle_time() )
         state_ = eu_broadcast;

      // broadcast collected information
      if ( state_ == eu_broadcast )
         broadcast_neighborhood();
   }
   // ----------------------------------------------------------------------
   template<typename OsModel_P,
            typename Radio_P,
            typename Clock_P,
            typename Distance_P,
            typename Debug_P,
            typename SharedData_P,
            typename DistancePair_P>
   bool
   LocalizationEuclideanModule<OsModel_P, Radio_P, Clock_P, Distance_P, Debug_P, SharedData_P, DistancePair_P>::
   finished( void )
   {
      return (state_ == eu_finished);
   }
   // ----------------------------------------------------------------------
   template<typename OsModel_P,
            typename Radio_P,
            typename Clock_P,
            typename Distance_P,
            typename Debug_P,
            typename SharedData_P,
            typename DistancePair_P>
   void
   LocalizationEuclideanModule<OsModel_P, Radio_P, Clock_P, Distance_P, Debug_P, SharedData_P, DistancePair_P>::
   rollback( void )
   {
      state_ = eu_init;
      last_useful_msg_ = clock_->time();
      col_measure_ = 0;
   }
   // ----------------------------------------------------------------------
   template<typename OsModel_P,
            typename Radio_P,
            typename Clock_P,
            typename Distance_P,
            typename Debug_P,
            typename SharedData_P,
            typename DistancePair_P>
   bool
   LocalizationEuclideanModule<OsModel_P, Radio_P, Clock_P, Distance_P, Debug_P, SharedData_P, DistancePair_P>::
   process_euclidean_init_message( node_id_t from, size_t len, block_data_t* data )
   {
      EuclideanInitMessage* msg = (EuclideanInitMessage*)data;
      Vec source_pos = msg->source_position();
      double distance = distance_->distance( from );
      if ( distance == UNKNOWN_DISTANCE )
         return false;

      last_useful_msg_ = clock_->time();

      // add info of received message to own neighborhood
      if ( msg->anchor() )
         this->neighborhood().update_anchor( from, source_pos, distance );
      else
         this->neighborhood().update_neighbor( from, distance );

     //BugFix: One-hop to anchor
     if ( this->neighborhood().valid_anchor_cnt() >= (int)this->shared_data().floodlimit() )
        state_ = eu_finished;

      return true;
   }
   // ----------------------------------------------------------------------
   template<typename OsModel_P,
            typename Radio_P,
            typename Clock_P,
            typename Distance_P,
            typename Debug_P,
            typename SharedData_P,
            typename DistancePair_P>
   void
   LocalizationEuclideanModule<OsModel_P, Radio_P, Clock_P, Distance_P, Debug_P, SharedData_P, DistancePair_P>::
   broadcast_neighborhood( void )
   {
      // send info about own neighborhood
      EuclideanNeighborMessage message;
      message.set_msg_id( EUCLIDEAN_NEIGHBOR_MESSAGE );
      // FIXME: isn't there a better way of doing this? problem here is
      //        without having const, and passing dmaps per value, not
      //        reference, where even copy constructor is called!
      DistanceMap dmap = this->neighborhood().neighbor_distance_map();
      message.set_neighbors( dmap );
      radio_->send( Radio::BROADCAST_ADDRESS, message.buffer_size(), (block_data_t*)&message );
      state_ = eu_work;
   }
   // ----------------------------------------------------------------------
   template<typename OsModel_P,
            typename Radio_P,
            typename Clock_P,
            typename Distance_P,
            typename Debug_P,
            typename SharedData_P,
            typename DistancePair_P>
   bool
   LocalizationEuclideanModule<OsModel_P, Radio_P, Clock_P, Distance_P, Debug_P, SharedData_P, DistancePair_P>::
   process_euclidean_neighbor_message( node_id_t from, size_t len, block_data_t* data )
   {
      EuclideanNeighborMessage* msg = (EuclideanNeighborMessage*)data;

      // set neighborhood of received node
      // FIXME: isn't there a better way of doing this? problem here is
      //        without having const, and passing dmaps per value, not
      //        reference, where even copy constructor is called!
      DistanceMap dmap = msg->neighbors();
      this->neighborhood().update_nneighbors( from, dmap );

      // if source is valid anchor, send anchor-message
      NeighborhoodIterator it = this->neighborhood().find( from );
      if ( it == this->neighborhood().end_neighborhood() )
         return true;

      if ( it->second->is_anchor() && it->second->is_valid() )
      {
         EuclideanAnchorMessage message;
         message.set_msg_id( EUCLIDEAN_ANCHOR_MESSAGE );
         message.set_anchor( it->second->node() );
         message.set_anchor_position( it->second->pos() );
         message.set_distance( it->second->distance() );
         radio_->send( Radio::BROADCAST_ADDRESS,
                      message.buffer_size(), (block_data_t*)&message );
      }

      return true;
   }
   // ----------------------------------------------------------------------
   template<typename OsModel_P,
            typename Radio_P,
            typename Clock_P,
            typename Distance_P,
            typename Debug_P,
            typename SharedData_P,
            typename DistancePair_P>
   bool
   LocalizationEuclideanModule<OsModel_P, Radio_P, Clock_P, Distance_P, Debug_P, SharedData_P, DistancePair_P>::
   process_euclidean_anchor_message( node_id_t from, size_t len, block_data_t* data )
   {
      if ( state_ == eu_finished )
            return true;

      EuclideanAnchorMessage* msg = (EuclideanAnchorMessage*)data;

      node_id_t anchor = msg->anchor();
      Vec anchor_pos = msg->anchor_position();

      // if anchor receives message about another anchor, the real distance
      // is calculated and sent as new anchor-message
      if ( this->shared_data().is_anchor() )
      {
         if ( this->neighborhood().find( anchor ) == this->neighborhood().end_neighborhood() )
            this->neighborhood().update_anchor( anchor, anchor_pos );
         else
            return true;

         double distance = euclidean_distance( anchor_pos, this->shared_data().position() );
         EuclideanAnchorMessage message;
         message.set_msg_id( EUCLIDEAN_ANCHOR_MESSAGE );
         message.set_anchor( anchor );
         message.set_anchor_position( anchor_pos );
         message.set_distance( distance );
         radio_->send(  Radio::BROADCAST_ADDRESS,
                      message.buffer_size(), (block_data_t*)&message );

         return true;
      }

      this->neighborhood().update_anchor( anchor, anchor_pos );
      this->neighborhood().update_nneighbor( anchor, from, msg->distance() );

      execute_euclidean( anchor );

      return true;
   }
   // ----------------------------------------------------------------------
   template<typename OsModel_P,
            typename Radio_P,
            typename Clock_P,
            typename Distance_P,
            typename Debug_P,
            typename SharedData_P,
            typename DistancePair_P>
   void
   LocalizationEuclideanModule<OsModel_P, Radio_P, Clock_P, Distance_P, Debug_P, SharedData_P, DistancePair_P>::
   execute_euclidean( node_id_t anchor )
   {
      NeighborhoodIterator it = this->neighborhood().find( anchor );
      //In case that we have not yet estimated a distance to this anchor
      if ( !it->second->is_valid() )
      {
         double distance = -1;

         switch ( algo_ )
         {
            case eu_algo_normal:
               distance = find_anchor_distance( it->second->node() );
               break;

            case eu_algo_opt:
               distance = find_anchor_distance_opt( it->second->node() );
               break;
         }

         if ( distance == -1 )
         {
#ifdef LOCALIZATION_DISTANCEBASED_EUCLIDEAN_DEBUG
            debug_->debug(  "Distance to anchor %d is -1\n", anchor );
#endif
            return;
         }
         else
         {
#ifdef LOCALIZATION_DISTANCEBASED_EUCLIDEAN_DEBUG
            debug_->debug(  "Distance to anchor %d is %f\n", anchor, distance );
#endif
         }

         it->second->set_distance( distance );
         EuclideanAnchorMessage message;
         message.set_msg_id( EUCLIDEAN_ANCHOR_MESSAGE );
         message.set_anchor( it->second->node() );
         message.set_anchor_position( it->second->pos() );
         message.set_distance( it->second->distance() );
         radio_->send(  Radio::BROADCAST_ADDRESS,
                      message.buffer_size(), (block_data_t*)&message );

         // if floodlimit reached, finished
         if ( this->neighborhood().valid_anchor_cnt() >= (int)this->shared_data().floodlimit() )
            state_ = eu_finished;

      }
   }
   // ----------------------------------------------------------------------
   template<typename OsModel_P,
            typename Radio_P,
            typename Clock_P,
            typename Distance_P,
            typename Debug_P,
            typename SharedData_P,
            typename DistancePair_P>
   double
   LocalizationEuclideanModule<OsModel_P, Radio_P, Clock_P, Distance_P, Debug_P, SharedData_P, DistancePair_P>::
   find_anchor_distance( node_id_t anchor )
   {
      double distance = -1;
      bool leave = false;

      for ( NeighborhoodIterator it1 = this->neighborhood().begin_neighborhood(); it1 != this->neighborhood().end_neighborhood(); ++it1 )
      {
         for ( NeighborhoodIterator it2 = it1; it2 != this->neighborhood().end_neighborhood(); ++it2 )
         {
            if ( it1 == it2 )
               continue;

            // check, that all needed distances exist
            if ( !this->neighborhood().has_valid_neighbor( it1->first ) ||
                  !this->neighborhood().has_valid_neighbor( it2->first ) ||
                  !this->neighborhood().has_valid_nneighbor( it1->first, it2->first ) ||
                  !this->neighborhood().has_valid_nneighbor( it1->first, anchor ) ||
                  !this->neighborhood().has_valid_nneighbor( it2->first, anchor ) )
               continue;

            double self_n1 = this->neighborhood().neighbor_distance( it1->first );
            double self_n2 = this->neighborhood().neighbor_distance( it2->first );
            double n1_n2 = this->neighborhood().nneighbor_distance( it1->first, it2->first );
            double n1_anchor = this->neighborhood().nneighbor_distance( it1->first, anchor );
            double n2_anchor = this->neighborhood().nneighbor_distance( it2->first, anchor );

            // check collinearity
            switch ( cc_std_ )
            {
               case eu_cc_std_strict:
                  if ( is_collinear( self_n1, self_n2, n1_n2, col_measure_ ) ||
                        is_collinear( n1_anchor, n2_anchor, n1_n2, col_measure_ ) )
                     continue;
                  break;

               case eu_cc_std_lax:
                  if ( is_collinear( self_n1, self_n2, n1_n2, col_measure_ ) &&
                        is_collinear( n1_anchor, n2_anchor, n1_n2, col_measure_ ) )
                     continue;
                  break;

               case eu_cc_std_none:
                  ;
            }

            DistancePair dp = trilateration_distance<OsModel, DistancePair>( self_n1, self_n2, n1_n2, n1_anchor, n2_anchor );
            if ( dp.first == -1 )
               continue;

            NodeList nl_nv = find_unique_neighbor_neighbors( anchor, it1->first, it2->first );
            NodeList nl_cn = find_common_neighbor_neighbors( anchor, it1->first, it2->first );
            double dist_nv = neighbor_vote( anchor, it1->first, it2->first, dp, nl_nv );
            double dist_cn = common_neighbor( anchor, it1->first, it2->first, dp, nl_cn );

            if ( dist_nv == -1 && dist_cn == -1 )
               continue;

            switch ( vote_)
            {
               case eu_vote_nv:
               {
                  if ( dist_nv != -1 ) distance = dist_nv;
                  else continue;

                  break;
               }
               case eu_vote_cn:
               {
                  if ( dist_cn != -1 ) distance = dist_cn;
                  else continue;

                  break;
               }
               case eu_vote_nvcn:
               {
                  if ( dist_cn != -1 ) distance = dist_cn;
                  if ( dist_nv != -1 ) distance = dist_nv;
                  else continue;

                  break;
               }
               case eu_vote_cnnv:
               {
                  if ( dist_nv != -1 ) distance = dist_nv;
                  if ( dist_cn != -1 ) distance = dist_cn;
                  else continue;

                  break;
               }
            }// switch vote_

            leave = true;
            break;
         }// for it2

         if ( leave ) break;
      }// for it1

      return distance;
   }
   // ----------------------------------------------------------------------
   template<typename OsModel_P,
            typename Radio_P,
            typename Clock_P,
            typename Distance_P,
            typename Debug_P,
            typename SharedData_P,
            typename DistancePair_P>
   double
   LocalizationEuclideanModule<OsModel_P, Radio_P, Clock_P, Distance_P, Debug_P, SharedData_P, DistancePair_P>::
   find_anchor_distance_opt( node_id_t anchor )
   {
      double distance = -1;
      double best_nv = -1;
      double best_cn = -1;
      double max_measure_nv = DBL_MIN;
      double max_measure_cn = DBL_MIN;

      for ( NeighborhoodIterator
               it1 = this->neighborhood().begin_neighborhood();
               it1 != this->neighborhood().end_neighborhood();
               ++it1 )
      {
         for ( NeighborhoodIterator
                  it2 = it1;
                  it2 != this->neighborhood().end_neighborhood();
                  ++it2 )
         {
            if ( it1 == it2 )
               continue;

            // check, that all needed distance estimations to neighboring nodes exist
            if ( !this->neighborhood().has_valid_neighbor( it1->first ) ||
                  !this->neighborhood().has_valid_neighbor( it2->first ) ||
                  !this->neighborhood().has_valid_nneighbor( it1->first, it2->first ) ||
                  !this->neighborhood().has_valid_nneighbor( it1->first, anchor ) ||
                  !this->neighborhood().has_valid_nneighbor( it2->first, anchor ) )
               continue;

            double self_n1 = this->neighborhood().neighbor_distance( it1->first );
            double self_n2 = this->neighborhood().neighbor_distance( it2->first );
            double n1_anchor = this->neighborhood().nneighbor_distance( it1->first, anchor );
            double n2_anchor = this->neighborhood().nneighbor_distance( it2->first, anchor );
            double n1_n2 = this->neighborhood().nneighbor_distance( it1->first, it2->first );

            // check collinearity
            switch ( cc_std_ )
            {
               case eu_cc_std_strict:
                  if ( is_collinear( self_n1, self_n2, n1_n2, col_measure_ ) ||
                        is_collinear( n1_anchor, n2_anchor, n1_n2, col_measure_ ) )
                     continue;
                  break;

               case eu_cc_std_lax:
                  if ( is_collinear( self_n1, self_n2, n1_n2, col_measure_ ) &&
                        is_collinear( n1_anchor, n2_anchor, n1_n2, col_measure_ ) )
                     continue;
                  break;

               case eu_cc_std_none:
                  ;
            }

            DistancePair dp = trilateration_distance<OsModel, DistancePair>( self_n1, self_n2, n1_n2, n1_anchor, n2_anchor );

            if ( dp.first > -1.00000000001 && dp.first < -0.999999999999 )
            {
               continue;
            }

            double measure;
            NodeList nl = find_common_neighbor_neighbors_opt( anchor, it1->first, it2->first, measure );

            double dist_nv = neighbor_vote( anchor, it1->first, it2->first, dp, nl );
            double dist_cn = common_neighbor( anchor, it1->first, it2->first, dp, nl );

            if ( dist_cn > -1.000000001 && dist_cn < -0.9999999999 && max_measure_cn < measure && measure > 0 )
            {
               max_measure_cn = measure;
               best_cn = dist_cn;
            }

            if ( dist_nv > -1.000000001 && dist_nv < -0.9999999999 && max_measure_nv < measure && measure > 0 )
            {
               max_measure_nv = measure;
               best_nv = dist_nv;
            }
         }// for it2
      }// for it1

      switch ( vote_)
      {
         case eu_vote_nv:
         {
            if ( best_nv > -1.000000001 && best_nv < -0.9999999999 ) distance = best_nv;
            break;
         }
         case eu_vote_cn:
         {
            if ( best_cn > -1.000000001 && best_cn < -0.9999999999 ) distance = best_cn;
            break;
         }
         case eu_vote_nvcn:
         {
            if ( best_cn > -1.000000001 && best_cn < -0.9999999999 ) distance = best_cn;
            if (best_nv > -1.000000001 && best_nv < -0.9999999999 ) distance = best_nv;
            break;
         }
         case eu_vote_cnnv:
         {
            if ( best_nv > -1.0000000001 && best_nv < -0.9999999999 -1 ) distance = best_nv;
            if ( best_cn > -1.0000000001 && best_cn < -0.9999999999 ) distance = best_cn;
            break;
         }
      }// switch vote_

      return distance;
   }
   // ----------------------------------------------------------------------
   template<typename OsModel_P,
            typename Radio_P,
            typename Clock_P,
            typename Distance_P,
            typename Debug_P,
            typename SharedData_P,
            typename DistancePair_P>
   typename LocalizationEuclideanModule<OsModel_P, Radio_P, Clock_P, Distance_P, Debug_P, SharedData_P, DistancePair_P>::NodeList
   LocalizationEuclideanModule<OsModel_P, Radio_P, Clock_P, Distance_P, Debug_P, SharedData_P, DistancePair_P>::
   find_unique_neighbor_neighbors( node_id_t anchor, node_id_t n1, node_id_t n2 )
   {
      NodeList temp;

      for ( NeighborhoodIterator
               it = this->neighborhood().begin_neighborhood();
               it != this->neighborhood().end_neighborhood();
               ++it )
      {
         // node is not n1 or n2, and connected to self and anchor
         if ( it->first == n1 || it->first == n2 || it->first == anchor ||
               !this->neighborhood().has_valid_neighbor( it->first ) ||
               !this->neighborhood().has_valid_nneighbor( it->first, anchor ) )
            continue;

         // node is connected to n1 and n2
         if ( !this->neighborhood().has_valid_nneighbor( n1, it->first ) &&
               !this->neighborhood().has_valid_nneighbor( n2, it->first ) )
            continue;

         temp.push_back( it->first );
      }

      return temp;
   }
   // ----------------------------------------------------------------------
   template<typename OsModel_P,
            typename Radio_P,
            typename Clock_P,
            typename Distance_P,
            typename Debug_P,
            typename SharedData_P,
            typename DistancePair_P>
   typename LocalizationEuclideanModule<OsModel_P, Radio_P, Clock_P, Distance_P, Debug_P, SharedData_P, DistancePair_P>::NodeList
   LocalizationEuclideanModule<OsModel_P, Radio_P, Clock_P, Distance_P, Debug_P, SharedData_P, DistancePair_P>::
   find_common_neighbor_neighbors( node_id_t anchor, node_id_t n1, node_id_t n2 )
   {
      NodeList temp;

      for ( NeighborhoodIterator
               it = this->neighborhood().begin_neighborhood();
               it != this->neighborhood().end_neighborhood();
               ++it )
      {
         // node is not n1 or n2, and connected to self and anchor
         if ( it->first == n1 || it->first == n2 || it->first == anchor ||
               !this->neighborhood().has_valid_neighbor( it->first ) ||
               !this->neighborhood().has_valid_nneighbor( it->first, anchor ) )
            continue;

         // node is connected to n1 and n2
         if ( !this->neighborhood().has_valid_nneighbor( n1, it->first ) ||
               !this->neighborhood().has_valid_nneighbor( n2, it->first ) )
            continue;

         temp.push_back( it->first );
      }

      return temp;
   }
   // ----------------------------------------------------------------------
   template<typename OsModel_P,
            typename Radio_P,
            typename Clock_P,
            typename Distance_P,
            typename Debug_P,
            typename SharedData_P,
            typename DistancePair_P>
   typename LocalizationEuclideanModule<OsModel_P, Radio_P, Clock_P, Distance_P, Debug_P, SharedData_P, DistancePair_P>::NodeList
   LocalizationEuclideanModule<OsModel_P, Radio_P, Clock_P, Distance_P, Debug_P, SharedData_P, DistancePair_P>::
   find_common_neighbor_neighbors_opt( node_id_t anchor, node_id_t n1,node_id_t n2, double& col_measure )
   {
      NodeList temp;
      col_measure = DBL_MIN;

      for ( NeighborhoodIterator
               it = this->neighborhood().begin_neighborhood();
               it != this->neighborhood().end_neighborhood();
               ++it )
      {
         // node is not n1 or n2, and connected to self and anchor
         if ( it->first == n1 || it->first == n2 || it->first == anchor ||
               !this->neighborhood().has_valid_neighbor( it->first ) ||
               !this->neighborhood().has_valid_nneighbor( it->first, anchor ) )
            continue;

         // node is connected to n1 and n2
         if ( !this->neighborhood().has_valid_nneighbor( n1, it->first ) ||
               !this->neighborhood().has_valid_nneighbor( n2, it->first ) )
            continue;

         double n1_n2 = this->neighborhood().nneighbor_distance( n1, n2 );
         double n1_n3 = this->neighborhood().nneighbor_distance( n1, it->first );
         double n2_n3 = this->neighborhood().nneighbor_distance( n2, it->first );

         double tmp_measure = collinear_measure( n1_n2, n1_n3, n2_n3 );

         if ( tmp_measure > col_measure )
         {
            col_measure = tmp_measure;

            temp.clear();
            temp.push_back( it->first );
         }
      }

      return temp;
   }
   // ----------------------------------------------------------------------
   template<typename OsModel_P,
            typename Radio_P,
            typename Clock_P,
            typename Distance_P,
            typename Debug_P,
            typename SharedData_P,
            typename DistancePair_P>
   double
   LocalizationEuclideanModule<OsModel_P, Radio_P, Clock_P, Distance_P, Debug_P, SharedData_P, DistancePair_P>::
   neighbor_vote( node_id_t anchor, node_id_t n1, node_id_t n2, DistancePair& dp1, NodeList& nl )
   {
      if ( nl.empty() )
      {
         return -1;
      }

      // temporary workaround. update to 2D/3D should follow soon, so euclidean
      // will exclusively work with NodeLists.
      NodeList nl_refs;
      nl_refs.push_back( n1 );
      nl_refs.push_back( n2 );

      Statistics stat1, stat2;
      stat1 += dp1.first;
      stat2 += dp1.second;

      for ( NodeListIterator it = nl.begin(); it != nl.end(); ++it )
      {
         node_id_t n3 = *it;

         for ( NodeListIterator
                  it_refs = nl_refs.begin();
                  it_refs != nl_refs.end();
                  ++it_refs )
         {

            node_id_t n1_2 = *it_refs;
            if ( !this->neighborhood().has_nneighbor( n1_2, n3 ) )
               continue;

            double self_n12 = this->neighborhood().neighbor_distance( n1_2 );
            double self_n3 = this->neighborhood().neighbor_distance( n3 );
            double n12_n3 = this->neighborhood().nneighbor_distance( n1_2, n3 );
            double n12_anchor = this->neighborhood().nneighbor_distance( n1_2, anchor );
            double n3_anchor = this->neighborhood().nneighbor_distance( n3, anchor );

            // check collinearity
            switch ( cc_nv_ )
            {
               case eu_cc_nv_strict:
                  if ( is_collinear( self_n12, self_n3, n12_n3, col_measure_ ) ||
                        is_collinear( n12_n3, n12_anchor, n3_anchor, col_measure_ ) )
                     continue;
                  break;

               case eu_cc_nv_lax:
                  if ( is_collinear( self_n12, self_n3, n12_n3, col_measure_ ) &&
                        is_collinear( n12_n3, n12_anchor, n3_anchor, col_measure_ ) )
                     continue;
                  break;

               case eu_cc_nv_none:
                  ;
            }

            DistancePair dp2 = trilateration_distance<OsModel, DistancePair>( self_n12, self_n3, n12_n3, n12_anchor, n3_anchor );
            if ( dp2.first > -1.00000000001 && dp2.first < -0.999999999999)
               continue;

            enum ChooseDist { d11, d12, d21, d22 } ch_dist = d11;
            double dev_sel = fabs( stat1.mean() - dp2.first );

            if ( fabs( stat1.mean() - dp2.second ) < dev_sel )
            {
               dev_sel = fabs( stat1.mean() - dp2.second );
               ch_dist = d12;
            }
            if ( fabs( stat2.mean() - dp2.first ) < dev_sel )
            {
               dev_sel = fabs( stat2.mean() - dp2.first );
               ch_dist = d21;
            }
            if ( fabs( stat2.mean() - dp2.second ) < dev_sel )
            {
               dev_sel = fabs( stat2.mean() - dp2.second );
               ch_dist = d22;
            }

            switch ( ch_dist )
            {
               case d11:
               case d22:
               {
                  stat1 += dp2.first;
                  stat2 += dp2.second;
                  continue;
               }
               case d12:
               case d21:
               {
                  stat1 += dp2.second;
                  stat2 += dp2.first;
                  continue;
               }
            }// switch
         }// for NodeList refs
      }// for NodeList nl

#ifdef LOCALIZATION_DISTANCEBASED_EUCLIDEAN_DEBUG
//       double real = euclidean_distance( anchor.real_position(), node().real_position() );
//       DEBUG( owner().logger(),
//          "Stats: "
//             << std::endl
//             << stat1 << ";;; " << stat2
//             << std::endl
//             << "Real: " << real << ": "
//             << std::endl
//             << stat1.mean() << "; " << stat1.std_dev() << " <= " << stat1.mean()*0.05
//             << " | "
//             << stat2.mean() << "; " << stat2.std_dev() << " <= " << stat2.mean()*0.05
//             << std::endl );
#endif

      if ( stat1.std_dev() > stat1.mean()*0.05 && stat2.std_dev() > stat2.mean()*0.05 )
         return -1;

      if ( stat1.std_dev()*3 < stat2.std_dev() )
         return stat1.mean();
      else if ( stat2.std_dev()*3 < stat1.std_dev() )
         return stat2.mean();

      return -1;
   }
   // ----------------------------------------------------------------------
   template<typename OsModel_P,
            typename Radio_P,
            typename Clock_P,
            typename Distance_P,
            typename Debug_P,
            typename SharedData_P,
            typename DistancePair_P>
   double
   LocalizationEuclideanModule<OsModel_P, Radio_P, Clock_P, Distance_P, Debug_P, SharedData_P, DistancePair_P>::
   common_neighbor( node_id_t anchor, node_id_t n1, node_id_t n2, DistancePair& dp, NodeList& nl )
   {
      if ( nl.empty() ) return -1;

      for ( NodeListIterator
               it = nl.begin();
               it != nl.end();
               ++it )
      {
         node_id_t n3 = *it;

         double self_n1 = this->neighborhood().neighbor_distance( n1 );
         double self_n2 = this->neighborhood().neighbor_distance( n2 );
         double self_n3 = this->neighborhood().neighbor_distance( n3 );
         double n1_n2, n1_anchor, n2_anchor, n3_n1, n3_n2;
         if ( this->neighborhood().has_valid_nneighbor( n1, n2 ) &&
               this->neighborhood().has_valid_nneighbor( anchor, n1 ) &&
               this->neighborhood().has_valid_nneighbor( anchor, n2 ) &&
               this->neighborhood().has_valid_nneighbor( n3, n1 ) &&
               this->neighborhood().has_valid_nneighbor( n3, n2 ) )
         {
            n1_n2 = this->neighborhood().nneighbor_distance( n1, n2 );
            n1_anchor = this->neighborhood().nneighbor_distance( anchor, n1 );
            n2_anchor = this->neighborhood().nneighbor_distance( anchor, n2 );
            n3_n1 = this->neighborhood().nneighbor_distance( n3, n1 );
            n3_n2 = this->neighborhood().nneighbor_distance( n3, n2 );
         }
         else
            continue;

         // check collinearity
         switch ( cc_cn_ )
         {
            case eu_cc_cn_strict:
               if ( is_collinear( self_n1, self_n2, n1_n2, col_measure_ ) ||
                     is_collinear( n1_n2, n3_n1, n3_n2, col_measure_ ) ||
                     is_collinear( n1_n2, n1_anchor, n2_anchor, col_measure_ ) )
                  continue;
               break;

            case eu_cc_cn_lax:
               if ( is_collinear( self_n1, self_n2, n1_n2, col_measure_ ) &&
                     is_collinear( n1_n2, n3_n1, n3_n2, col_measure_ ) &&
                     is_collinear( n1_n2, n1_anchor, n2_anchor, col_measure_ ) )
                  continue;
               break;

            case eu_cc_cn_none:
               ;
         }

         // distancepair self-n3
         DistancePair dp_sn3 = trilateration_distance<OsModel, DistancePair>(
                                 self_n1, self_n2, n1_n2, n3_n1, n3_n2 );
         if ( dp_sn3.first == -1 )
            continue;

         int side_n3 = 0;
         if ( fabs( self_n3 - dp_sn3.first ) < fabs( self_n3 - dp_sn3.second ) )
            side_n3 = 1;
         else
            side_n3 = -1;

         // distancepair n3-anchor
         DistancePair dp_n3a = trilateration_distance<OsModel, DistancePair>(
                                 n3_n1, n3_n2, n1_n2, n1_anchor, n2_anchor );
         if ( dp_n3a.first == -1 )
            continue;

         int side_a = 0;
         double n3_a = this->neighborhood().find( n3 )->second->neighbor_distance( anchor );
         if ( fabs( n3_a - dp_n3a.first ) < fabs( n3_a - dp_n3a.second ) )
            side_a = 1;
         else
            side_a = -1;

#ifdef LOCALIZATION_DISTANCEBASED_EUCLIDEAN_DEBUG
//          double real_dist = euclidean_distance( node().real_position(), anchor.real_position() );
//          DEBUG( owner().logger(),
//             real_dist << ": " << dp.first << "; " << dp.second
//                << " :: " << side_n3 * side_a
//                << std::endl
//                << self_n3 << ": " << dp_sn3.first << "; " << dp_sn3.second
//                << std::endl
//                << n3_a    << ": " << dp_n3a.first << "; " << dp_n3a.second
//                << std::endl );
#endif

         if ( side_n3 == 1 && side_a == 1 )
            return dp.first;
         else if ( side_n3 * side_a == -1 || ( side_n3 == -1 && side_a == -1 ) )
            return dp.second;
      }// for

      return -1;
   }

}// namespace wiselib
#endif
