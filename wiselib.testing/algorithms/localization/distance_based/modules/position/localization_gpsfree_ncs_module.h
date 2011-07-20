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
#ifndef __ALGORITHMS_LOCALIZATION_DISTANCE_BASED_LOCALIZATION_GPSFREE_NCS_MODULE_H
#define __ALGORITHMS_LOCALIZATION_DISTANCE_BASED_LOCALIZATION_GPSFREE_NCS_MODULE_H

#include "algorithms/localization/distance_based/modules/localization_module.h"
#include "algorithms/localization/distance_based/modules/position/localization_gpsfree_ncs_messages.h"
#include "algorithms/localization/distance_based/util/localization_defutils.h"


namespace wiselib
{

   /// Module implementing GPS-free Network Coordinate System building
   /** ...
    */
   template<typename OsModel_P,
            typename Radio_P,
            typename Clock_P,
            typename Debug_P,
            typename SharedData_P>
   class LocalizationGpsFreeNcsModule
      : public LocalizationModule<OsModel_P, Radio_P, SharedData_P>
   {
   public:
      typedef OsModel_P OsModel;
      typedef Radio_P Radio;
      typedef Clock_P Clock;
      typedef Debug_P Debug;
      typedef SharedData_P SharedData;

      typedef LocalizationGpsFreeNcsModule<OsModel, Radio, Clock, Debug, SharedData> self_type;
      typedef LocalizationModule<OsModel, Radio, SharedData> base_type;

      typedef typename Radio::size_t size_t;
      typedef typename Radio::node_id_t node_id_t;
      typedef typename Radio::block_data_t block_data_t;

      typedef typename Clock_P::time_t time_t;

      typedef typename SharedData::LocalCoordinateSystem LocalCoordinateSystem;
      typedef typename LocalCoordinateSystem::CorrectionData CorrectionData;

      typedef LocalizationGpsFreeNcsLcsMessage<OsModel, Radio, LocalCoordinateSystem> GpsFreeNcsLcsMessage;

      typedef typename SharedData::Neighborhood::NeighborhoodIterator NeighborhoodIterator;

      ///@name construction / destruction
      ///@{
      ///
      LocalizationGpsFreeNcsModule();
      ///
      ~LocalizationGpsFreeNcsModule();
      ///@}


      ///@name standard methods startup/simulation steps
      ///@{
      /** Handling of GPS-free-NCS-Messages.
       *
       *  \sa LocalizationModule::process_message()
       */
      void receive( node_id_t from, size_t len, block_data_t *data );
      /** Check, whether state can be set to finished or not. Moreover, send
       *  local coordinate system, compute correction angle and build network
       *  coordinate system.
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
      // --------------------------------------------------------------------
      void init( Radio& radio, Clock& clock, Debug& debug, SharedData& shared_data )
      {
         radio_ = &radio;
         clock_ = &clock;
         debug_ = &debug;
         this->set_shared_data( shared_data );
      }
      // --------------------------------------------------------------------
      void set_root_node( bool root )
      {
         if ( root )
         {
            root_node_ = true;
            this->shared_data().set_anchor( true );
         }
         else
         {
            root_node_ = false;
            this->shared_data().set_anchor( false );
         }
      }
      // --------------------------------------------------------------------
      bool set_root_node( void )
      { return root_node_; }

   protected:

      ///@name processing gpsfree ncs messages
      ///@{
      /**
       */
      bool process_gpsfree_ncs_lcs_message( node_id_t from, size_t len, block_data_t *data );
      ///@}

   private:

      enum MessageIds
      {
         GPSFREE_NCS_LCS_MESSAGE = 208
      };

      enum GPSfreeNCSState
      {
         gfncs_init,
         gfncs_wait_lcs,
         gfncs_build_ncs,
         gfncs_finished
      };

      GPSfreeNCSState state_;

      bool root_node_;
      bool computed_ncs_;
      time_t last_useful_msg_;

      Radio* radio_;
      Clock* clock_;
      Debug* debug_;
   };
   // ----------------------------------------------------------------------
   // ----------------------------------------------------------------------
   // ----------------------------------------------------------------------
   template<typename OsModel_P,
            typename Radio_P,
            typename Clock_P,
            typename Debug_P,
            typename SharedData_P>
   LocalizationGpsFreeNcsModule<OsModel_P, Radio_P, Clock_P, Debug_P, SharedData_P>::
   LocalizationGpsFreeNcsModule()
      : state_            ( gfncs_init ),
         root_node_       ( false ),
         computed_ncs_    ( false )
   {}
   // ----------------------------------------------------------------------
   template<typename OsModel_P,
            typename Radio_P,
            typename Clock_P,
            typename Debug_P,
            typename SharedData_P>
   LocalizationGpsFreeNcsModule<OsModel_P, Radio_P, Clock_P, Debug_P, SharedData_P>::
   ~LocalizationGpsFreeNcsModule()
   {}
   // ----------------------------------------------------------------------
   template<typename OsModel_P,
            typename Radio_P,
            typename Clock_P,
            typename Debug_P,
            typename SharedData_P>
   void
   LocalizationGpsFreeNcsModule<OsModel_P, Radio_P, Clock_P, Debug_P, SharedData_P>::
   receive( node_id_t from, size_t len, block_data_t *data )
   {
      switch ( data[0] )
      {
         case GPSFREE_NCS_LCS_MESSAGE:
            process_gpsfree_ncs_lcs_message( from, len, data );
            break;
      }
   }
   // ----------------------------------------------------------------------
   template<typename OsModel_P,
            typename Radio_P,
            typename Clock_P,
            typename Debug_P,
            typename SharedData_P>
   void
   LocalizationGpsFreeNcsModule<OsModel_P, Radio_P, Clock_P, Debug_P, SharedData_P>::
   work( void )
   {
      if ( state_ == gfncs_finished )
         return;

      // send initial message (only root_node)
      if ( root_node_ && state_ == gfncs_init )
      {
         state_ = gfncs_build_ncs;

         CorrectionData cd;

         this->local_coord_sys().correct_lcs_to_real_ncs( cd );
         this->local_coord_sys().perform_correction( cd );
         this->shared_data().set_position( cd.pos );

         GpsFreeNcsLcsMessage message;
         message.set_msg_id( GPSFREE_NCS_LCS_MESSAGE );
         message.set_local_coord_sys( &this->local_coord_sys() );
         radio_->send( Radio::BROADCAST_ADDRESS,
                       message.buffer_size(), (block_data_t*)&message );

         computed_ncs_ = true;
         state_ = gfncs_finished;
      }
      else if ( state_ == gfncs_init )
      {
         state_ = gfncs_wait_lcs;
      }

      // maybe shutdown processor
      if ( clock_->time() - last_useful_msg_ > this->shared_data().idle_time() )
      {
         state_ = gfncs_finished;
      }
   }
   // ----------------------------------------------------------------------
   template<typename OsModel_P,
            typename Radio_P,
            typename Clock_P,
            typename Debug_P,
            typename SharedData_P>
   bool
   LocalizationGpsFreeNcsModule<OsModel_P, Radio_P, Clock_P, Debug_P, SharedData_P>::
   finished( void )
   {
      return state_ == gfncs_finished;
   }
   // ----------------------------------------------------------------------
   template<typename OsModel_P,
            typename Radio_P,
            typename Clock_P,
            typename Debug_P,
            typename SharedData_P>
   void
   LocalizationGpsFreeNcsModule<OsModel_P, Radio_P, Clock_P, Debug_P, SharedData_P>::
   rollback( void )
   {
      state_ = gfncs_init;
      root_node_ = false;
      computed_ncs_ = false;
      last_useful_msg_ = clock_->time();
   }
   // ----------------------------------------------------------------------
   template<typename OsModel_P,
            typename Radio_P,
            typename Clock_P,
            typename Debug_P,
            typename SharedData_P>
   bool
   LocalizationGpsFreeNcsModule<OsModel_P, Radio_P, Clock_P, Debug_P, SharedData_P>::
   process_gpsfree_ncs_lcs_message( node_id_t from, size_t len, block_data_t* data )
   {
      if ( computed_ncs_ )
         return true;

      GpsFreeNcsLcsMessage* msg = (GpsFreeNcsLcsMessage*)data;
      last_useful_msg_ = clock_->time();

      CorrectionData cd;
      // compute NCS
      if ( this->local_coord_sys().correct_lcs( *msg->local_coord_sys(), cd ) )
      {
         this->local_coord_sys().perform_correction( cd );
         this->shared_data().set_position( cd.pos );

         GpsFreeNcsLcsMessage message;
         message.set_msg_id( GPSFREE_NCS_LCS_MESSAGE );
         message.set_local_coord_sys( &this->local_coord_sys() );
         radio_->send( Radio::BROADCAST_ADDRESS,
                       message.buffer_size(), (block_data_t*)&message );
      }
      else
      {
         Vec pos = msg->local_coord_sys()->node_position( radio_->id() );
         if ( pos != UNKNOWN_POSITION )
            this->shared_data().set_position( pos );
      }

      computed_ncs_ = true;
      state_ = gfncs_finished;

      return true;
   }

}// namespace wiselib
#endif
