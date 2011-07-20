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
#ifndef __ALGORITHMS_LOCALIZATION_DISTANCE_BASED_LOCALIZATION_SUM_DIST_MODULE_H
#define __ALGORITHMS_LOCALIZATION_DISTANCE_BASED_LOCALIZATION_SUM_DIST_MODULE_H

#include "algorithms/localization/distance_based/modules/localization_module.h"
#include "algorithms/localization/distance_based/modules/distance/localization_sum_dist_messages.h"
#include "algorithms/localization/distance_based/util/localization_defutils.h"
#include "config_testing.h"


namespace wiselib
{

   /// Module implementing Sum-dist
   /** This module implements Sum-dist. Distance to anchors is get by simply
    *  flooding the network beginning at the anchors. Then the unknowns adding
    *  the distance estimated each hop and broadcast this information again.
    *
    *  Each unknown takes the minimal distance to at most 'floodlimit'
    *  anchors, where the floodlimit is taken from the LocalizationObserver.
    */
   template<typename OsModel_P,
            typename Radio_P,
            typename Clock_P,
            typename Distance_P,
            typename Debug_P,
            typename SharedData_P,
            typename Arithmatic_P >
   class LocalizationSumDistModule
      : public LocalizationModule<OsModel_P, Radio_P, SharedData_P>
   {

   public:
      typedef OsModel_P OsModel;
      typedef Radio_P Radio;
      typedef Clock_P Clock;
      typedef Distance_P Distance;
      typedef Debug_P Debug;
      typedef SharedData_P SharedData;
      typedef Arithmatic_P Arithmatic;


      typedef LocalizationSumDistModule<OsModel, Radio, Clock, Distance, Debug, SharedData, Arithmatic> self_type;
      typedef LocalizationModule<OsModel, Radio, SharedData> base_type;

      typedef typename Radio::size_t size_t;
      typedef typename Radio::node_id_t node_id_t;
      typedef typename Radio::block_data_t block_data_t;

      typedef typename Clock_P::time_t time_t;

      typedef LocalizationSumDistMessage<OsModel, Radio, Arithmatic> SumDistMessage;

      typedef typename SharedData::Neighborhood::NeighborhoodIterator NeighborhoodIterator;

      ///@name construction / destruction
      ///@{
      ///
      LocalizationSumDistModule();
      ~LocalizationSumDistModule();
      ///@}


      ///@name standard methods startup/simulation steps
      ///@{
      /** Handling of Sum-dist-Messages.
       *
       *  \sa LocalizationModule::process_message()
       */
      void receive( node_id_t from, size_t len, block_data_t *data );
      /** Check, whether state can be set to finished or not. Moreover, if
       *  owner is an anchor, initial message is send.
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

         last_useful_msg_ = clock_->time();

         messages_counter_ = 0;
      }


   protected:

      ///@name message handling methods
      ///@{
      /** Message handling by unknowns such as described above.
       *
       *  \sa LocalizationSumDistMessage
       */
      bool process_sum_dist_message( node_id_t from, size_t len, block_data_t *data );
      ///@}


   private:

      enum MessagesIds
      {
         SUM_DIST_MESSAGE = 203
      };

      enum SumDistState
      {
         sd_init,
         sd_work,
         sd_finished
      };

      SumDistState state_;
      time_t last_useful_msg_;
      uint8_t messages_counter_;

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
            typename Arithmatic_P>
   LocalizationSumDistModule<OsModel_P, Radio_P, Clock_P, Distance_P, Debug_P, SharedData_P, Arithmatic_P>::
   LocalizationSumDistModule()
      : state_( sd_init )
   {}
   // ----------------------------------------------------------------------
   template<typename OsModel_P,
            typename Radio_P,
            typename Clock_P,
            typename Distance_P,
            typename Debug_P,
            typename SharedData_P,
            typename Arithmatic_P>
   LocalizationSumDistModule<OsModel_P, Radio_P, Clock_P, Distance_P, Debug_P, SharedData_P, Arithmatic_P>::
   ~LocalizationSumDistModule()
   {}
   // ----------------------------------------------------------------------
   template<typename OsModel_P,
            typename Radio_P,
            typename Clock_P,
            typename Distance_P,
            typename Debug_P,
            typename SharedData_P,
            typename Arithmatic_P>
   void
   LocalizationSumDistModule<OsModel_P, Radio_P, Clock_P, Distance_P, Debug_P, SharedData_P, Arithmatic_P>::
   receive( node_id_t from, size_t len, block_data_t *data )
   {

#ifdef LOCALIZATION_DISTANCEBASED_SUMDIST_DEBUG
      debug_->debug(  "Message received data[0] = %d from %x \n",
                    data[0],from);
#endif

      if ( data[0] == SUM_DIST_MESSAGE )
         process_sum_dist_message( from, len, data );
   }
   // ----------------------------------------------------------------------
   template<typename OsModel_P,
            typename Radio_P,
            typename Clock_P,
            typename Distance_P,
            typename Debug_P,
            typename SharedData_P,
            typename Arithmatic_P>
   void
   LocalizationSumDistModule<OsModel_P, Radio_P, Clock_P, Distance_P, Debug_P, SharedData_P, Arithmatic_P>::
   work( void )
   {
      // do initial stuff;
      //    if anchor, send 'init-message' and set state to 'finished';
      //    if unknown, start working


      if ( state_ == sd_init )
      {
         if ( this->shared_data().is_anchor() )
         {
            SumDistMessage message;
            message.set_msg_id( SUM_DIST_MESSAGE );
            message.set_path_length( 0.0 );
            message.set_anchor( radio_->id() );
            message.set_anchor_position( this->shared_data().position() );

#ifdef LOCALIZATION_DISTANCEBASED_SUMDIST_DEBUG
      debug_->debug(  "Anchor sendet data[0]= %d \n",
    		  ((block_data_t*)&message)[0] );
#endif


            radio_->send(  Radio::BROADCAST_ADDRESS, message.buffer_size(), (block_data_t*)&message );
            messages_counter_++;
            if(messages_counter_>5)
            	state_ = sd_finished;
         }
         else
            state_ = sd_work;
      }

      // if given idle-time is passed ( no more useful messages came in ),
      // set state to 'finished'
      if ( clock_->time() - last_useful_msg_ > this->shared_data().idle_time() ){
         state_ = sd_finished;
#ifdef LOCALIZATION_DISTANCEBASED_SUMDIST_DEBUG
         int shareddatatime = this->shared_data().idle_time().sec();
         time_t def =  clock_->time() - last_useful_msg_;
   	  if(radio_->id()==0x9999)
   	  debug_->debug(  "SUMDIST::finished due to idle time %d  %d \n ",shareddatatime,def.sec());
#endif
      }
   }
   // ----------------------------------------------------------------------
   template<typename OsModel_P,
            typename Radio_P,
            typename Clock_P,
            typename Distance_P,
            typename Debug_P,
            typename SharedData_P,
            typename Arithmatic_P>
   bool
   LocalizationSumDistModule<OsModel_P, Radio_P, Clock_P, Distance_P, Debug_P, SharedData_P, Arithmatic_P>::
   process_sum_dist_message( node_id_t from, size_t len, block_data_t* data )
   {

#ifdef LOCALIZATION_DISTANCEBASED_SUMDIST_DEBUG
      debug_->debug(  "Process sum dist message state= %d",state_ );
#endif
      if ( state_ == sd_finished || this->shared_data().is_anchor() )
         return true;

#ifdef LOCALIZATION_DISTANCEBASED_SUMDIST_DEBUG
      debug_->debug(  "state_ != sd_finished || this->shared_data().is_anchor()" );
#endif

      SumDistMessage* msg = (SumDistMessage*)data;
      node_id_t anchor = msg->anchor();
      Vec<Arithmatic> anchor_pos = msg->anchor_position();
      Arithmatic rcvd_path = msg->path_length();



      Arithmatic distance = distance_->distance( from );

#ifdef LOCALIZATION_DISTANCEBASED_SUMDIST_DEBUG
      debug_->debug(  "rcvd_path %f distance %f  ",rcvd_path, distance);
#endif

      if ( distance == -1 ){
    	  #ifdef LOCALIZATION_DISTANCEBASED_SUMDIST_DEBUG
    	  if(radio_->id()==0x9999)
    	  debug_->debug(  "unknown dist  ");
    	#endif
         return false;
      }
         else{
        	 #ifdef LOCALIZATION_DISTANCEBASED_SUMDIST_DEBUG
        	 if(radio_->id()==0x9999)
        	 debug_->debug(  "known dist  ");
       	 #endif
         }

      rcvd_path += distance;

#ifdef LOCALIZATION_DISTANCEBASED_SUMDIST_DEBUG
      if(radio_->id()==0x9999)
      debug_->debug(  "rcvd_path %f distance after adding %f", rcvd_path, distance);
#endif
      // if anchor is new and floodlimit not reached, insert new anchor;
      // if anchor is known and new distance is smaller than old one,
      //    update info;
      // otherwise return
      NeighborhoodIterator it = this->neighborhood().find( anchor );
      if ( it == this->neighborhood().end_neighborhood() )
      {
#ifdef LOCALIZATION_DISTANCEBASED_SUMDIST_DEBUG
    	  if(radio_->id()==0x9999)
      debug_->debug(  "SUMDIST::adding anker");
#endif
         if ( this->neighborhood().anchor_cnt() >= (int)this->shared_data().floodlimit() )
            return true;

         this->neighborhood().update_anchor( anchor, anchor_pos );
#ifdef LOCALIZATION_DISTANCEBASED_SUMDIST_DEBUG
    	  if(radio_->id()==0x9999)
      debug_->debug(  "SUMDIST::updateanker %f %f",anchor_pos.x(),anchor_pos.y());
#endif

         // set iterator to the new inserted anchor
         it = this->neighborhood().find( anchor );

         it->second.set_distance( rcvd_path );
         this->neighborhood().set_ref_node( anchor, from );
      }
      else
      {
#ifdef LOCALIZATION_DISTANCEBASED_SUMDIST_DEBUG
         debug_->debug(  "SUMDIST:: not adding anker");
#endif
          if ( it->second.distance() <= rcvd_path )
             return true;
         if ( it->second.distance() > rcvd_path )
         {
            it->second.set_distance( rcvd_path );
            this->neighborhood().set_ref_node( anchor, from );
         }
      }

      // set time of last useful message to actual and broadcast new info
      last_useful_msg_ = clock_->time(  );
      SumDistMessage message;
      message.set_msg_id( SUM_DIST_MESSAGE );
      message.set_anchor( msg->anchor() );
      message.set_anchor_position( msg->anchor_position() );
      message.set_path_length( it->second.distance() );
      radio_->send( Radio::BROADCAST_ADDRESS, message.buffer_size(), (block_data_t*)&message );

#ifdef LOCALIZATION_DISTANCEBASED_SUMDIST_DEBUG
      debug_->debug(  "New or smaller path at %x to %x is %d\n",
                    radio_->id(), msg->anchor(), (uint8_t)rcvd_path );
#endif

      return true;
   }
   // ----------------------------------------------------------------------
   template<typename OsModel_P,
            typename Radio_P,
            typename Clock_P,
            typename Distance_P,
            typename Debug_P,
            typename SharedData_P,
            typename Arithmatic_P>
   bool
   LocalizationSumDistModule<OsModel_P, Radio_P, Clock_P, Distance_P, Debug_P, SharedData_P,Arithmatic_P>::
   finished( void )
   {
      return state_ == sd_finished;
   }
   // ----------------------------------------------------------------------
   template<typename OsModel_P,
            typename Radio_P,
            typename Clock_P,
            typename Distance_P,
            typename Debug_P,
            typename SharedData_P,
            typename Arithmatic_P>
   void
   LocalizationSumDistModule<OsModel_P, Radio_P, Clock_P, Distance_P, Debug_P, SharedData_P,Arithmatic_P>::
   rollback( void )
   {
      state_ = sd_init;

      last_useful_msg_ = clock_->time();


      this->shared_data().reset_neighborhood_();


      messages_counter_ = 0;
   }

}// namespace wiselib
#endif
