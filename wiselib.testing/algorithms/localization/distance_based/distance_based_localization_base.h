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
#ifndef __ALGORITHMS_LOCALIZATION_DISTANCE_BASED_H
#define __ALGORITHMS_LOCALIZATION_DISTANCE_BASED_H

#include "algorithms/localization/distance_based/neighborhood/localization_neighbor_info.h"
#include "algorithms/localization/distance_based/neighborhood/localization_neighborhood.h"
#include "algorithms/localization/distance_based/neighborhood/localization_local_coordinate_system.h"
#include "algorithms/localization/distance_based/util/localization_defutils.h"
#include "algorithms/localization/distance_based/math/vec.h"

namespace wiselib
{

   /// Three-phase localization
   /** There are anchorbased and anchorless algorithms. Anchorbased means, that
    *  unknown nodes, which do not know their position, need some nodes in the
    *  topology, which know their position. Anchorless algorithms build a
    *  relative coordinate system without the need of position aware nodes.
    *  Generally you should not mix these algorithms because potentially
    *  there will be no result.
    *
    *  Localization is cut in three phases depending on using anchors or not.
    *
    *  Anchorbased: At first, unknown nodes try to
    *  estimate their distance to the anchors. Second, with the resulting
    *  information the position is calculated. Third, an optional one, the
    *  estimated position is calibrated, e.g. with help of neighbors
    *  information.
    *
    *  Anchorless: At first, nodes try to get information about their
    *  neighborhood and build a local coordinate system. Second, they will
    *  build a global, or rather network, coordinate system. Third phase
    *  is an optional one where the positions and coordinate systems are
    *  calibrated.
    *
    *  The algorithms are not designed for mobility primary, but there is a
    *  chance to do so. In general each phase is processed one time before
    *  the processor deactivate itself. Therefor you could give an rollback
    *  number, which rerun the phases after the given number of rounds.
    *  Moreover there is an rollback_limit which limit the number of
    *  maximal rollbacks.
    *
    *  The algorithms are implemented in so called
    *  \ref LocalizationModule "modules", one designed for each algorithm in
    *  each phase. All algorithms and calculations work on the same
    *  \ref LocalizationNeighborhood "neighborhood".
    */
   template<typename OsModel_P,
            typename Radio_P,
            typename Timer_P,
            typename SharedData_P,
            typename DistanceModule_P,
            typename PositionModule_P,
            typename RefinementModule_P,
            typename Arithmatic_P ,
            typename Debug_P = typename OsModel_P::Debug>
   class DistanceBasedLocalization
   {

   public:

      typedef OsModel_P OsModel;
      typedef DistanceModule_P DistanceModule;
      typedef PositionModule_P PositionModule;
      typedef RefinementModule_P RefinementModule;
      typedef Radio_P Radio;
      typedef Timer_P Timer;
      typedef Debug_P Debug;
      typedef SharedData_P SharedData;
      typedef Arithmatic_P Arithmatic;

      typedef Vec<Arithmatic_P> position_t;

      typedef DistanceBasedLocalization<OsModel, Radio, Timer, SharedData, DistanceModule,
                  PositionModule, RefinementModule,Arithmatic ,Debug> self_type;

      typedef typename Radio::size_t size_t;
      typedef typename Radio::node_id_t node_id_t;
      typedef typename Radio::block_data_t block_data_t;
      // --------------------------------------------------------------------
      enum ErrorCodes
      {
         SUCCESS = OsModel::SUCCESS,
         ERR_UNSPEC = OsModel::ERR_UNSPEC,
         ERR_NOTIMPL = OsModel::ERR_NOTIMPL
      };
      // --------------------------------------------------------------------
      ///@name construction
      ///@{
      ///
      DistanceBasedLocalization();
      ///@}

      int init( Radio& radio, Timer& timer, Debug & debug,
                 SharedData& shared_data, DistanceModule& dist_module,
                 PositionModule& pos_module, RefinementModule& ref_module );
      int init( void );
      int destruct( void );

      /** At first, this method checks, if the message should gone lost.
       *  If not, message is forwarded to
       *  \ref LocalizationModule "distance and refinement module".
       *  \return \c true, if message is processed, \c false, if not.
       *  \sa shawn::Processor::process_message()
       */
      void receive( node_id_t from, size_t len, block_data_t *data );
      /** TODO: modules had been renewed!
       *  At first, this method executes the work method of both
       *  \ref LocalizationModule "distance and refinement module".
       *  Second, if the distance phase is finished, this method tries to
       *  estimate the node's position.
       *  \sa shawn::Processor::work()
       */
      void work( void *data );
      ///@}
      // --------------------------------------------------------------------
      SharedData& shared_data( void )
      { return shared_data_; }
      // --------------------------------------------------------------------
      void set_anchor( bool anchor )
      {
         shared_data_.set_anchor( anchor );
      }
      // --------------------------------------------------------------------
      void set_confidence( Arithmatic confidence )
      {
         shared_data_.set_confidence( confidence );
      }
      // --------------------------------------------------------------------
      void set_position( const Vec<Arithmatic_P>& position )
      {
         shared_data_.set_position( position );
      }
      // --------------------------------------------------------------------
      void set_active( bool active )
      {
         active_ = active;
      }
      // --------------------------------------------------------------------
      bool active( void )
      {
         return active_;
      }

      void inline roll_back(void){
    	  phase_  = distance;
    	  dist_module_->rollback();
    	  pos_module_->rollback();
    	  ref_module_->rollback();
    	  active_=true;
    	  //timer_->template set_timer<self_type, &self_type::work>( work_period_, this, 0 );

      }


private:
      enum LocalizationPhase { distance, position, refinement };
      LocalizationPhase phase_;

      bool check_residue_;
      int idle_time_;
      int idle_shutdown_time_;

      int rollback_iteration_;
      int rollback_period_;
      int rollback_cnt_;
      int rollback_limit_;
      int work_period_;

      bool active_;

      typename Radio::self_pointer_t radio_;
      typename Timer::self_pointer_t timer_;
      typename Debug::self_pointer_t debug_;

      SharedData *shared_data_;

      DistanceModule *dist_module_;
      PositionModule *pos_module_;
      RefinementModule *ref_module_;
   };
   // -----------------------------------------------------------------------
   template<typename OsModel_P,
            typename Radio_P,
            typename Timer_P,
            typename SharedData_P,
            typename DistanceModule_P,
            typename PositionModule_P,
            typename RefinementModule_P,
            typename Arithmatic_P,
            typename Debug_P>
   DistanceBasedLocalization<OsModel_P, Radio_P, Timer_P, SharedData_P, DistanceModule_P, PositionModule_P, RefinementModule_P,Arithmatic_P ,Debug_P>::
   DistanceBasedLocalization()
      : phase_               ( distance ),
         check_residue_       ( true ),
         idle_time_           ( 5 ),
         idle_shutdown_time_  ( 0 ),
         rollback_iteration_  ( 0 ),
         rollback_period_     ( INT_MAX ),
         rollback_cnt_        ( 0 ),
         rollback_limit_      ( 0 ),
         //min_max
         work_period_         ( 1000 ),
         //work_period_         ( 1800 ),
         active_              ( true )
   {}
   // -----------------------------------------------------------------------
   template<typename OsModel_P,
            typename Radio_P,
            typename Timer_P,
            typename SharedData_P,
            typename DistanceModule_P,
            typename PositionModule_P,
            typename RefinementModule_P,
            typename Arithmatic_P,
            typename Debug_P>
   int
   DistanceBasedLocalization<OsModel_P, Radio_P, Timer_P, SharedData_P, DistanceModule_P, PositionModule_P, RefinementModule_P,Arithmatic_P ,Debug_P>::
   init( Radio& radio, Timer& timer, Debug & debug,
         SharedData& shared_data, DistanceModule& dist_module,
         PositionModule& pos_module, RefinementModule& ref_module )
   {
      radio_ = &radio;
      timer_ = &timer;
      debug_ = &debug;
      shared_data_ = &shared_data;
      dist_module_ = &dist_module;
      pos_module_ = &pos_module;
      ref_module_ = &ref_module;

      shared_data_->neighborhood().set_source( radio_->id() );

      dist_module_->set_shared_data( *shared_data_ );
      dist_module_->rollback();
      pos_module_->set_shared_data( *shared_data_ );
      pos_module_->rollback();
      ref_module_->set_shared_data( *shared_data_ );
      ref_module_->rollback();

      radio_->enable_radio();




      radio_->template reg_recv_callback<self_type, &self_type::receive>( this );
      timer_->template set_timer<self_type, &self_type::work>( work_period_, this, 0 );

      return SUCCESS;
   }
   // -----------------------------------------------------------------------
   template<typename OsModel_P,
            typename Radio_P,
            typename Timer_P,
            typename SharedData_P,
            typename DistanceModule_P,
            typename PositionModule_P,
            typename RefinementModule_P,
            typename Arithmatic_P,
            typename Debug_P>
   int
   DistanceBasedLocalization<OsModel_P, Radio_P, Timer_P, SharedData_P, DistanceModule_P, PositionModule_P, RefinementModule_P,Arithmatic_P, Debug_P>::
   init( void )
   {
      return ERR_NOTIMPL;
   }
   // -----------------------------------------------------------------------
   template<typename OsModel_P,
            typename Radio_P,
            typename Timer_P,
            typename SharedData_P,
            typename DistanceModule_P,
            typename PositionModule_P,
            typename RefinementModule_P,
            typename Arithmatic_P,
            typename Debug_P>
   int
   DistanceBasedLocalization<OsModel_P, Radio_P, Timer_P, SharedData_P, DistanceModule_P, PositionModule_P, RefinementModule_P,Arithmatic_P, Debug_P>::
   destruct( void )
   {
      return ERR_NOTIMPL;
   }
   // -----------------------------------------------------------------------
   template<typename OsModel_P,
            typename Radio_P,
            typename Timer_P,
            typename SharedData_P,
            typename DistanceModule_P,
            typename PositionModule_P,
            typename RefinementModule_P,
            typename Arithmatic_P,
            typename Debug_P>
   void
   DistanceBasedLocalization<OsModel_P, Radio_P, Timer_P, SharedData_P, DistanceModule_P, PositionModule_P, RefinementModule_P,Arithmatic_P, Debug_P>::
   receive( node_id_t from, size_t len, block_data_t *data )
   {
      dist_module_->receive( from, len, data );
      pos_module_->receive( from, len, data );
      ref_module_->receive( from, len, data );
   }
   // -----------------------------------------------------------------------
   template<typename OsModel_P,
            typename Radio_P,
            typename Timer_P,
            typename SharedData_P,
            typename DistanceModule_P,
            typename PositionModule_P,
            typename RefinementModule_P,
            typename Arithmatic_P,
            typename Debug_P>
   void
   DistanceBasedLocalization<OsModel_P, Radio_P, Timer_P, SharedData_P, DistanceModule_P, PositionModule_P, RefinementModule_P, Arithmatic_P, Debug_P>::
   work( void *data )
   {
      rollback_iteration_++;

      if ( phase_ == distance )
      {
         dist_module_->work();

         if ( dist_module_->finished() )
            phase_ = position;
      }

      if ( phase_ == position )
      {
         pos_module_->work();

         if ( pos_module_->finished() )
            phase_ = refinement;
      }

      if ( phase_ == refinement )
         ref_module_->work();

      if ( rollback_iteration_ == rollback_period_ )
      {
         rollback_iteration_ = 0;
         rollback_cnt_++;

         phase_ = distance;
         dist_module_->rollback();
         pos_module_->rollback();
         ref_module_->rollback();
      }

      if ( rollback_limit_ != 0 && rollback_cnt_ == rollback_limit_ )
         set_active( false );

      if ( rollback_limit_ == 0 && dist_module_->finished() && pos_module_->finished() && ref_module_->finished() )
         set_active( false );

      timer_->template set_timer<self_type, &self_type::work>( work_period_, this, 0 );
   }

}// namespace wiselib
#endif
