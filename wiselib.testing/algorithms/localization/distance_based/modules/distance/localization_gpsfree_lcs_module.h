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
#ifndef __ALGORITHMS_LOCALIZATION_DISTANCE_BASED_LOCALIZATION_GPSFREE_LCS_MODULE_H
#define __ALGORITHMS_LOCALIZATION_DISTANCE_BASED_LOCALIZATION_GPSFREE_LCS_MODULE_H

#include "algorithms/localization/distance_based/modules/localization_module.h"
#include "algorithms/localization/distance_based/modules/distance/localization_gpsfree_lcs_messages.h"
#include "algorithms/localization/distance_based/util/localization_defutils.h"


namespace wiselib
{

   /// Module that implements GPS-free Local Coordinate System building
   /** This module implements the algorithm to get a local coordinate system
    *  just like described in <em>GPS-free positioning in moblie ad hoc
    *  networks</em> by Capkun, Hamdi, Hubeaux.
    *
    *  At first nodes build their 2-hop neighborhood by sending an initial
    *  message, receive initial messages from neighbors and send again a
    *  message contending constructed neighborhood to surrounding nodes.
    *
    *  After that nodes build their local coordinate system. [...]
    *  <hr>
    *
    *  <b>Comment by the author</b>
    *  This is not a full implementation of the above mentioned paper. Up to
    *  now there are two parts missing: On the one hand (just a little thing)
    *  the computation per triangulation of unknown nodes (the stuff with the
    *  node k, which is not neighbor of p and q and should be computed by
    *  using the position of node i and two other nodes for which the
    *  positions are already obtained) has not be done. On the other hand
    *  (the big part) the whole stuff with the Location Reference Group is
    *  still missing.
    *
    *  I think, that these parts are useless unless the algorithm has been
    *  optimized in two ways. First the nearly unrelevant one: By choosing
    *  nodes p and q to maximize the number of regular nodes in the LVS
    *  (see LocalizationGpsFreeLcsModule<OsModel_P, Radio_P, Clock_P, Distance_P, Debug_P, SharedData_P>::build_lcs()), the
    *  "up-to-now-solution" is a very unfortunaletly one, because the
    *  computation overhead is really big. Second (in my opinion the main
    *  argument against using this algorithm unless there appears a better
    *  solution) there is a problem with the triangulisation in the paper
    *  (see formula (3), deciding, on which side of the x-axis a node is
    *  located) and errors in distance measurement. The author mentioned, that
    *  the two angles can not be equal in practice and that some differences
    *  should be tolerated. Unfortunaletly I hadn't found any <em>epsilon</em>
    *  leading to satisfactorily solutions. Maybe it is an approach to look
    *  at the papers of K. Langendoen and D. Niculesco and their handling
    *  of the similar "euclidean-problem".
    */
   template<typename OsModel_P,
            typename Radio_P,
            typename Clock_P,
            typename Distance_P,
            typename Debug_P,
            typename SharedData_P>
   class LocalizationGpsFreeLcsModule
      : public LocalizationModule<OsModel_P, Radio_P, SharedData_P>
   {

   public:
      typedef OsModel_P OsModel;
      typedef Radio_P Radio;
      typedef Clock_P Clock;
      typedef Distance_P Distance;
      typedef Debug_P Debug;
      typedef SharedData_P SharedData;

      typedef LocalizationGpsFreeLcsModule<OsModel, Radio, Clock, Distance, Debug, SharedData> self_type;
      typedef LocalizationModule<OsModel, Radio, SharedData> base_type;

      typedef typename Radio::size_t size_t;
      typedef typename Radio::node_id_t node_id_t;
      typedef typename Radio::block_data_t block_data_t;

      typedef typename Clock_P::time_t time_t;

      typedef typename SharedData::DistanceMap DistanceMap;
      typedef typename SharedData::LocalCoordinateSystem LocalCoordinateSystem;
      typedef typename SharedData::Neighborhood::NeighborhoodIterator NeighborhoodIterator;

      typedef LocalizationGpsFreeLcsInitMessage<OsModel, Radio> GpsFreeLcsInitMessage;
      typedef LocalizationGpsFreeLcsNeighborMessage<OsModel, Radio, DistanceMap> GpsFreeLcsNeighborMessage;

      ///@name construction / destruction
      ///@{
      ///
      LocalizationGpsFreeLcsModule();
      ///
      ~LocalizationGpsFreeLcsModule();
      ///@}


      ///@name standard methods startup/simulation steps
      ///@{
      /** Handling of GPS-free-LCS-Messages.
       *
       *  \sa LocalizationModule::process_message()
       */
      void receive( node_id_t from, size_t len, block_data_t *data );
      /** Check, whether state can be set to finished or not. Moreover, send
       *  initial messages and build local coordinate system.
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

      void init( Radio& radio, Clock& clock, Debug& debug, SharedData& shared_data, Distance& distance )
      {
         radio_ = &radio;
         clock_ = &clock;
         debug_ = &debug;
         this->set_shared_data( shared_data );
         distance_ = &distance;
      }

   protected:

      ///@name processing gpsfree lcs messages
      ///@{
      /** This method processes initial messages. Source of message is added
       *  to neighborhood.
       *
       *  \sa LocalizationEuclideanInitMessage
       */
      bool process_gpsfree_lcs_init_message( node_id_t from, size_t len, block_data_t *data );
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
      bool process_gpsfree_lcs_neighbor_message( node_id_t from, size_t len, block_data_t *data );
      ///@}

      ///@name work
      ///@{
      /** Build a local coordinate system just like described in the paper.
       */
      void build_lcs( void );
      ///@}

      ///@name initial methods
      ///@{
      /** Read the given parameters, which have been set via simulator
       *  commands or configuration file.
       */
      void set_lcs_epsilon( double epsilon )
      { this->local_coord_sys_w().set_epsilon( epsilon ); }
      ///@}


   private:

      enum MessageIds
      {
         GPSFREE_LCS_INIT_MESSAGE = 206,
         GPSFREE_LCS_NEIGHBOR_MESSAGE = 207
      };

      enum GPSfreeLCSState
      {
         gflcs_init,
         gflcs_wait_bc,
         gflcs_broadcast,
         gflcs_wait_w,
         gflcs_work,
         gflcs_finished
      };


      GPSfreeLCSState state_;
      time_t last_useful_msg_;

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
            typename SharedData_P>
   LocalizationGpsFreeLcsModule<OsModel_P, Radio_P, Clock_P, Distance_P, Debug_P, SharedData_P>::
   LocalizationGpsFreeLcsModule()
      : state_            ( gflcs_init ),
         last_useful_msg_ ( 0 )
   {}
   // ----------------------------------------------------------------------
   template<typename OsModel_P,
            typename Radio_P,
            typename Clock_P,
            typename Distance_P,
            typename Debug_P,
            typename SharedData_P>
   LocalizationGpsFreeLcsModule<OsModel_P, Radio_P, Clock_P, Distance_P, Debug_P, SharedData_P>::
   ~LocalizationGpsFreeLcsModule()
   {}
   // ----------------------------------------------------------------------
   template<typename OsModel_P,
            typename Radio_P,
            typename Clock_P,
            typename Distance_P,
            typename Debug_P,
            typename SharedData_P>
   void
   LocalizationGpsFreeLcsModule<OsModel_P, Radio_P, Clock_P, Distance_P, Debug_P, SharedData_P>::
   receive( node_id_t from, size_t len, block_data_t *data )
   {
      switch ( data[0] )
      {
         case GPSFREE_LCS_INIT_MESSAGE:
            process_gpsfree_lcs_init_message( from, len, data );
            break;
         case GPSFREE_LCS_NEIGHBOR_MESSAGE:
            process_gpsfree_lcs_neighbor_message( from, len, data );
            break;
      }
   }
   // ----------------------------------------------------------------------
   template<typename OsModel_P,
            typename Radio_P,
            typename Clock_P,
            typename Distance_P,
            typename Debug_P,
            typename SharedData_P>
   void
   LocalizationGpsFreeLcsModule<OsModel_P, Radio_P, Clock_P, Distance_P, Debug_P, SharedData_P>::
   work( void )
   {
      // send initial messages
      if ( state_ == gflcs_init )
      {
         GpsFreeLcsInitMessage message;
         message.set_msg_id( GPSFREE_LCS_INIT_MESSAGE );
         message.set_position( this->shared_data().position() );
         radio_->send( Radio::BROADCAST_ADDRESS, message.buffer_size(), (block_data_t*)&message );

         state_ = gflcs_wait_bc;
      }

      // after idle-time passed, initial messages of neighbors had already been
      // received and state is set to 'broadcast'.
      if ( state_ == gflcs_wait_bc && clock_->time() - last_useful_msg_ > this->shared_data().idle_time() )
      {
         state_ = gflcs_broadcast;
         last_useful_msg_ = clock_->time();
      }

      // broadcast collected information
      if ( state_ == gflcs_broadcast )
      {
         broadcast_neighborhood();
         state_ = gflcs_wait_w;
      }

      // after idle-time passed, neighborhood messages of neighbors had already been
      // received and state is set to 'work'.
      if ( state_ == gflcs_wait_w && clock_->time() - last_useful_msg_ > this->shared_data().idle_time() )
         state_ = gflcs_work;

      if ( state_ == gflcs_work )
      {
         build_lcs();
         state_ = gflcs_finished;
      }

      // maybe shutdown processor
      if ( clock_->time() - last_useful_msg_ > this->shared_data().idle_time() )
      {
         state_ = gflcs_finished;
      }
   }
   // ----------------------------------------------------------------------
   template<typename OsModel_P,
            typename Radio_P,
            typename Clock_P,
            typename Distance_P,
            typename Debug_P,
            typename SharedData_P>
   bool
   LocalizationGpsFreeLcsModule<OsModel_P, Radio_P, Clock_P, Distance_P, Debug_P, SharedData_P>::
   finished( void )
   {
      return state_ == gflcs_finished;
   }
   // ----------------------------------------------------------------------
   template<typename OsModel_P,
            typename Radio_P,
            typename Clock_P,
            typename Distance_P,
            typename Debug_P,
            typename SharedData_P>
   void
   LocalizationGpsFreeLcsModule<OsModel_P, Radio_P, Clock_P, Distance_P, Debug_P, SharedData_P>::
   rollback( void )
   {
      state_  = gflcs_init;
      last_useful_msg_ = clock_->time();
   }

   // ----------------------------------------------------------------------
   template<typename OsModel_P,
            typename Radio_P,
            typename Clock_P,
            typename Distance_P,
            typename Debug_P,
            typename SharedData_P>
   bool
   LocalizationGpsFreeLcsModule<OsModel_P, Radio_P, Clock_P, Distance_P, Debug_P, SharedData_P>::
   process_gpsfree_lcs_init_message( node_id_t from, size_t len, block_data_t* data )
   {
      GpsFreeLcsInitMessage* msg = (GpsFreeLcsInitMessage*)data;
      Vec source_pos = msg->position();
      double distance = distance_->distance( from );

      last_useful_msg_ = clock_->time();

      this->neighborhood().update_neighbor( from, distance );

      return true;
   }
   // ----------------------------------------------------------------------
   template<typename OsModel_P,
            typename Radio_P,
            typename Clock_P,
            typename Distance_P,
            typename Debug_P,
            typename SharedData_P>
   void
   LocalizationGpsFreeLcsModule<OsModel_P, Radio_P, Clock_P, Distance_P, Debug_P, SharedData_P>::
   broadcast_neighborhood( void )
   {
      // send info about own neighborhood
      GpsFreeLcsNeighborMessage message;
      message.set_msg_id( GPSFREE_LCS_NEIGHBOR_MESSAGE );
      DistanceMap dmap = this->neighborhood().neighbor_distance_map();
      message.set_neighbors( dmap );
      radio_->send( Radio::BROADCAST_ADDRESS, message.buffer_size(), (block_data_t*)&message );
   }
   // ----------------------------------------------------------------------
   template<typename OsModel_P,
            typename Radio_P,
            typename Clock_P,
            typename Distance_P,
            typename Debug_P,
            typename SharedData_P>
   bool
   LocalizationGpsFreeLcsModule<OsModel_P, Radio_P, Clock_P, Distance_P, Debug_P, SharedData_P>::
   process_gpsfree_lcs_neighbor_message( node_id_t from, size_t len, block_data_t* data )
   {
      GpsFreeLcsNeighborMessage* msg = (GpsFreeLcsNeighborMessage*)data;
      // set neighborhood of received node
      this->neighborhood().update_nneighbors( from, msg->neighbors() );

      return true;
   }
   // ----------------------------------------------------------------------
   template<typename OsModel_P,
            typename Radio_P,
            typename Clock_P,
            typename Distance_P,
            typename Debug_P,
            typename SharedData_P>
   void
   LocalizationGpsFreeLcsModule<OsModel_P, Radio_P, Clock_P, Distance_P, Debug_P, SharedData_P>::
   build_lcs( void )
   {
      for ( NeighborhoodIterator
               it1 = this->neighborhood().begin_neighborhood();
               it1 != this->neighborhood().end_neighborhood();
               ++it1 )
         for ( NeighborhoodIterator
                  it2 = it1;
                  it2 != this->neighborhood().end_neighborhood();
                  ++it2 )
         {
            if ( it1 == it2 )
               continue;

            LocalCoordinateSystem lcs;
            lcs.update_basic_nodes( it1->first, it2->first, this->neighborhood() );

            for ( NeighborhoodIterator
                     it3 = this->neighborhood().begin_neighborhood();
                     it3 != this->neighborhood().end_neighborhood();
                     ++it3 )
            {
               if ( it3 == it1 || it3 == it2 )
                  continue;

               lcs.update_node( it3->first );
            }

            if (  lcs.is_valid() &&
                  ( !this->local_coord_sys().is_valid() ||
                      this->local_coord_sys().size() < lcs.size() ) )
               this->local_coord_sys() = lcs;
         }

      this->local_coord_sys().set_src_node( radio_->id() );
   }

}// namespace wiselib
#endif
