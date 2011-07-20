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
#ifndef __ALGORITHMS_LOCALIZATION_DISTANCE_BASED_LOCALIZATION_ITER_LATERATION_MODULE_H
#define __ALGORITHMS_LOCALIZATION_DISTANCE_BASED_LOCALIZATION_ITER_LATERATION_MODULE_H

#include "algorithms/localization/distance_based/modules/localization_module.h"
#include "algorithms/localization/distance_based/modules/refinement/localization_iter_lateration_messages.h"
#include "algorithms/localization/distance_based/util/localization_defutils.h"

namespace wiselib
{

   /// Module implementing refinement by iterative lateration
   /** This module implements refinement by iterative lateration. Idea is to
    *  take into account all neighbors, respectively their distances and
    *  positions, and start lateration with this information. After that the
    *  new position is broadcasted, so that neighbors are able to start
    *  lateration again with this new data, and so on.
    *
    *  The process finished, either after a given number of steps, or, if the
    *  position update becomes very small.
    *
    *  Main problem of this procedere are nodes with inaccurate information.
    *  To filter out most of these cases, there are different methods.
    *
    *  At first, there is the use of confidences. This means, that every node
    *  is associated with a special confidence level between 0 and 1. Unknowns
    *  start with a low confidence like 0.1, anchors with a high one like 1.
    *  These values are used in the
    *  \ref localization::est_pos_lateration "lateration" by a weighted least
    *  squares approach. Instead of solving \f$ Ax = b\f$,
    *  \f$ \omega Ax = \omega b\f$ is solved, where \f$\omega\f$ is the vector
    *  of confidences. After a successful lateration a node sets its
    *  confidence to the average of its neighbors confidences. This will, in
    *  general, increase the confidence level.
    *
    *  Second issue is filtering out ill-connected nodes.\n
    *  To take part in refinement phase, a node has to be \em sound. This
    *  means, that there are \em independent references to at least three/four
    *  anchors. That is, information about anchors has to be get from
    *  different neighbors. Moreover, if a node becomes sound, it sends this
    *  out. The neighbors again receiving this information, add the sound node
    *  to an own list, so that an unsound node is able to become sound, if
    *  the size of intersection between sound and references becomes greater
    *  or equal three/four.
    *
    *  Third, the twin neighbors should be filtered out. This means, that one
    *  neighbor, that is very close to another, is set to be a twin and is
    *  ignored in further computation. The twin check has to be done in every
    *  iteration, because in most cases their position change.
    *
    *  At least, there is the
    *  \ref localization::check_residue "residuen check" to check the validity
    *  of the new position. To avoid being trapped in a local minimum, there
    *  is a given chance to accept a failed check anyway. If this happens, the
    *  confidence of affected node is reduced by 50 percent.
    */
   template<typename OsModel_P,
            typename Radio_P,
            typename Distance_P,
            typename Debug_P,
            typename SharedData_P>
   class LocalizationIterLaterationModule
      : public LocalizationModule<OsModel_P, Radio_P, SharedData_P>
   {

   public:
      typedef OsModel_P OsModel;
      typedef Radio_P Radio;
      typedef Distance_P Distance;
      typedef Debug_P Debug;
      typedef SharedData_P SharedData;

      typedef LocalizationIterLaterationModule<OsModel, Radio, Distance, Debug, SharedData> self_type;
      typedef LocalizationModule<OsModel, Radio, SharedData> base_type;

      typedef typename Radio::size_t size_t;
      typedef typename Radio::node_id_t node_id_t;
      typedef typename Radio::block_data_t block_data_t;

      typedef LocalizationIterLaterationMessage<OsModel, Radio> IterLaterationMessage;
      typedef LocalizationIterLaterationSoundMessage<OsModel, Radio> IterLaterationSoundMessage;

      typedef typename SharedData::NeighborInfoList NeighborInfoList;
      typedef typename SharedData::Neighborhood::NeighborhoodIterator NeighborhoodIterator;

      typedef typename SharedData::Neighborhood Neighborhood;

      ///@name construction / destruction
      ///@{
      ///
      LocalizationIterLaterationModule();
      ///
      ~LocalizationIterLaterationModule();
      ///@}


      ///@name standard methods startup/simulation steps
      ///@{
      /** Handling of Iter-Lateration-Messages.
       *
       *  \sa LocalizationModule::process_message()
       */
      void receive( node_id_t from, size_t len, block_data_t *data );
      /** Check, whether state can be set to finished or not. Moreover, send
       *  initial messages and check on first pass, whether node is sound or
       *  not.
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

      void init( Radio& radio, Debug& debug, SharedData& shared_data, Distance& distance )
      {
         radio_ = &radio;
         distance_ = &distance;
         debug_ = &debug;
         this->set_shared_data( shared_data );
      }

      ///@name Parameter Configuration
      ///@{
      /** Maximum number of iterations in refinement phase. Default 5.
       */
      void set_iteration_limit( int iteration_limit )
      { iteration_limit_ = iteration_limit; }
      /** Minimum of confident neighbors to start refinement. Default 5.
       */
      void set_min_confident_nbrs( int min_confident_nbrs )
      { min_confident_nbrs_ = min_confident_nbrs; }
      /** Percentage of communication range. This parameter is taken to
       *  decide, whether a neighbor is twin or not. Default 0.1.
       */
      void set_twin_measure( double twin_measure )
      { twin_measure_ = twin_measure; }
      /** Percentage of communication range. decides, if new position is very
       *  close to old one, whether the refinement phase is finished or not
       *  after position update. Default 0.001.
       */
      void set_abort_pos_update( double abort_pos_update )
      { abort_pos_update_ = abort_pos_update; }
      /** If residuen check of lateration fails, there is a given chance,
       *  namely res_acceptance, to accept the result anyway. This happens, to
       *  avoid being trapped in some local mimina. Default 0.1.
       */
      void set_res_acceptance( double res_acceptance )
      { res_acceptance_ = res_acceptance; }
      ///@}

   protected:

      ///@name processing iterative lateration messages
      ///@{
      /** This method processes sound messages. Message source is added to
       *  sound nodes and, if necessary, the sound check is started again.
       *
       *  \sa LocalizationIterLaterationSoundMessage
       */
      bool process_iter_lateration_sound_message( node_id_t from, size_t len, block_data_t *data );
      /** This method processes normal messages. The received information is
       *  updated in own neighborhood.
       *
       *  \sa LocalizationIterLaterationMessage
       */
      bool process_iter_lateration_message( node_id_t from, size_t len, block_data_t *data );
      ///@}


      ///@name main refinement step
      ///@{
      /** This method executes the iterative lateration step. There are passed
       *  all checks mentioned in the \em Detailed \em Description above.
       */
      void iter_lateration_step( void );
      ///@}


   private:

      enum MessageIds
      {
         ITER_LATERATION_MESSAGE = 209,
         ITER_LATERATION_SOUND_MESSAGE = 210
      };

      enum IterLaterationState
      {
         il_init,
         il_work,
         il_finished
      };

      IterLaterationState state_;
      int iteration_limit_;
      int iteration_cnt_;
      int min_confident_nbrs_;
      double twin_measure_;
      double abort_pos_update_;
      double res_acceptance_;
      bool sound_;
      Distance* distance_;
      Radio* radio_;
      Debug* debug_;
   };
   // ----------------------------------------------------------------------
   // ----------------------------------------------------------------------
   // ----------------------------------------------------------------------
   template<typename OsModel_P,
            typename Radio_P,
            typename Distance_P,
            typename Debug_P,
            typename SharedData_P>
   LocalizationIterLaterationModule<OsModel_P, Radio_P, Distance_P, Debug_P, SharedData_P>::
   LocalizationIterLaterationModule()
      : state_               ( il_init ),
         iteration_limit_    ( 5 ),
         iteration_cnt_      ( 0 ),
         min_confident_nbrs_ ( 5 ),
         twin_measure_       ( 0.1 ),
         abort_pos_update_   ( 0.001 ),
         res_acceptance_     ( 0.1 ),
         sound_              ( false )
   {}
   // ----------------------------------------------------------------------
   template<typename OsModel_P,
            typename Radio_P,
            typename Distance_P,
            typename Debug_P,
            typename SharedData_P>
   LocalizationIterLaterationModule<OsModel_P, Radio_P, Distance_P, Debug_P, SharedData_P>::
   ~LocalizationIterLaterationModule()
   {}
   // ----------------------------------------------------------------------
   template<typename OsModel_P,
            typename Radio_P,
            typename Distance_P,
            typename Debug_P,
            typename SharedData_P>
   void
   LocalizationIterLaterationModule<OsModel_P, Radio_P, Distance_P, Debug_P, SharedData_P>::
   receive( node_id_t from, size_t len, block_data_t *data )
   {
      switch ( data[0] )
      {
         case ITER_LATERATION_MESSAGE:
            process_iter_lateration_message( from, len, data );
            break;
         case ITER_LATERATION_SOUND_MESSAGE:
            process_iter_lateration_sound_message( from, len, data );
            break;
      }
   }
   // ----------------------------------------------------------------------
   template<typename OsModel_P,
            typename Radio_P,
            typename Distance_P,
            typename Debug_P,
            typename SharedData_P>
   void
   LocalizationIterLaterationModule<OsModel_P, Radio_P, Distance_P, Debug_P, SharedData_P>::
   work( void )
   {
      if ( state_ == il_finished )
         return;

      // do initial stuff;
      //    if anchor, send 'init-message' and set state to 'finished';
      //    if unknown, check whether the node is sound or not. if sound,
      //       send messages, if not, clear estimated position.
      if ( state_ == il_init )
      {
         if ( this->shared_data().is_anchor() )
         {
            IterLaterationMessage message;
            message.set_msg_id( ITER_LATERATION_MESSAGE );
            message.set_confidence( this->shared_data().confidence() );
            radio_->send( Radio::BROADCAST_ADDRESS, message.buffer_size(), (block_data_t*)&message );

            state_ = il_finished;
         }
         else
         {
            sound_ = this->neighborhood().is_sound();

            if ( sound_ && this->shared_data().position() != UNKNOWN_POSITION )
            {
               IterLaterationMessage message;
               message.set_msg_id( ITER_LATERATION_MESSAGE );
               message.set_confidence( this->shared_data().confidence() );
               radio_->send( Radio::BROADCAST_ADDRESS, message.buffer_size(), (block_data_t*)&message );

               IterLaterationSoundMessage sound_message;
               sound_message.set_msg_id( ITER_LATERATION_SOUND_MESSAGE );
               radio_->send(  Radio::BROADCAST_ADDRESS, sound_message.buffer_size(), (block_data_t*)&sound_message );
            }
            else
            {
               this->shared_data().set_confidence( 0.0 );
               this->shared_data().set_position( UNKNOWN_POSITION );
            }

            state_ = il_work;
         }
      }

      if ( state_ == il_work )
         iter_lateration_step();
   }
   // ----------------------------------------------------------------------
   template<typename OsModel_P,
            typename Radio_P,
            typename Distance_P,
            typename Debug_P,
            typename SharedData_P>
   bool
   LocalizationIterLaterationModule<OsModel_P, Radio_P, Distance_P, Debug_P, SharedData_P>::
   process_iter_lateration_sound_message( node_id_t from, size_t len, block_data_t *data )
   {
      if ( this->shared_data().is_anchor() || state_ == il_finished )
         return true;

      this->neighborhood().add_sound( from );

      if ( !sound_ )
      {
         if ( (sound_ = this->neighborhood().is_sound()) )
         {
            IterLaterationSoundMessage message;
            message.set_msg_id( ITER_LATERATION_SOUND_MESSAGE );
            radio_->send(  Radio::BROADCAST_ADDRESS, message.buffer_size(), (block_data_t*)&message );
         }
      }

      return true;
   }
   // ----------------------------------------------------------------------
   template<typename OsModel_P,
            typename Radio_P,
            typename Distance_P,
            typename Debug_P,
            typename SharedData_P>
   bool
   LocalizationIterLaterationModule<OsModel_P, Radio_P, Distance_P, Debug_P, SharedData_P>::
   process_iter_lateration_message( node_id_t from, size_t len, block_data_t *data )
   {
      if ( this->shared_data().is_anchor() || state_ == il_finished )
         return true;

      IterLaterationMessage* msg = (IterLaterationMessage*)data;
      double distance = distance_->distance( from );

      this->neighborhood().update_neighbor( from, distance );
      NeighborhoodIterator it = this->neighborhood().find( from );
      it->second->set_pos( msg->position() );
      it->second->set_confidence( msg->confidence() );

      return true;
   }
   // ----------------------------------------------------------------------
   template<typename OsModel_P,
            typename Radio_P,
            typename Distance_P,
            typename Debug_P,
            typename SharedData_P>
   void
   LocalizationIterLaterationModule<OsModel_P, Radio_P, Distance_P, Debug_P, SharedData_P>::
   iter_lateration_step( void )
   {
      if ( state_ == il_finished || !sound_ )
         return;

      if ( iteration_cnt_ >= iteration_limit_ )
         state_ = il_finished;
      ++iteration_cnt_;

      this->neighborhood().reassign_twins( twin_measure_ * this->shared_data().communication_range() );

     if ( this->neighborhood().confident_neighbor_cnt() < min_confident_nbrs_ )
         return;

      Vec est_pos;
      NeighborInfoList neighbors;
      collect_neighbors<OsModel, Neighborhood, NeighborInfoList>( this->neighborhood(), lat_confident, neighbors );

     // try to update position. if lateration fails, confidence is set to 0,
      // else position is updated and confidence is set to average of all
      // neighbor confidences
      if ( est_pos_lateration<OsModel, NeighborInfoList>( neighbors, est_pos, lat_confident, false ) &&
            est_pos_lateration<OsModel, NeighborInfoList>( neighbors, est_pos, lat_confident, true ) )
      {
         this->shared_data().set_confidence( this->neighborhood().avg_neighbor_confidence() );

         // check validity of new position. if check fails, there is a chance
         // of 'res_acceptance_', which is normally set to 0.1, to accept
         // the position anyway. this is done to avoid being trapped in a
         // local minimum. moreover, if the bad position is accepted, the
         // confidence is reduced by 50%.
         if ( !check_residue<OsModel, NeighborInfoList>( neighbors, est_pos, lat_confident, this->shared_data().communication_range() ) )
         {
            // TODO: add random variables to Wiselib!
//             if ( res_acceptance_ > uniform_random_0i_1i() )
//                set_confidence( observer().confidence() / 2 );
//             else
            {
               if ( this->shared_data().confidence() == 0.0 )
                  return;
               this->shared_data().set_confidence( 0.0 );
               this->shared_data().set_position( UNKNOWN_POSITION );

               IterLaterationMessage message;
               message.set_msg_id( ITER_LATERATION_MESSAGE );
               message.set_confidence( this->shared_data().confidence() );
               radio_->send(  Radio::BROADCAST_ADDRESS, message.buffer_size(), (block_data_t*)&message );

               return;
            }
         }

         // check, whether the new position is very close to the old one or
         // not. if so, refinement is finished.
         if ( this->shared_data().position() != UNKNOWN_POSITION )
         {
            double pos_diff = euclidean_distance( est_pos, this->shared_data().position() );

            if ( pos_diff < abort_pos_update_ * this->shared_data().communication_range() )
               state_ = il_finished;
         }

         this->shared_data().set_position( est_pos );
      }
      else
      {
         if ( this->shared_data().confidence() == 0.0 )
            return;
         this->shared_data().set_confidence( 0.0 );
         this->shared_data().set_position( UNKNOWN_POSITION );
      }

      IterLaterationMessage message;
      message.set_msg_id( ITER_LATERATION_MESSAGE );
      message.set_confidence( this->shared_data().confidence() );
      radio_->send( Radio::BROADCAST_ADDRESS, message.buffer_size(), (block_data_t*)&message );
   }
   // ----------------------------------------------------------------------
   template<typename OsModel_P,
            typename Radio_P,
            typename Distance_P,
            typename Debug_P,
            typename SharedData_P>
   bool
   LocalizationIterLaterationModule<OsModel_P, Radio_P, Distance_P, Debug_P, SharedData_P>::
   finished( void )
   {
      return state_ == il_finished;
   }
    // ----------------------------------------------------------------------
    template<typename OsModel_P,
            typename Radio_P,
            typename Distance_P,
            typename Debug_P,
            typename SharedData_P>
   void
   LocalizationIterLaterationModule<OsModel_P, Radio_P, Distance_P, Debug_P, SharedData_P>::
   rollback( void )
   {
#ifdef LOCALIZATION_DISTANCEBASED_ITER_LATERATION_DEBUG
      debug_->debug( "iter lateration rollback" );
#endif
      state_ = il_init;
      iteration_cnt_ = 0;
      sound_ = false;
   }

}// namespace wiselib
#endif
