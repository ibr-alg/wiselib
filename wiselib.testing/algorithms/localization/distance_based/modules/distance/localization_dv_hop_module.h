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
#ifndef __ALGORITHMS_LOCALIZATION_DISTANCE_BASED_LOCALIZATION_DVHOP_MODULE_H
#define __ALGORITHMS_LOCALIZATION_DISTANCE_BASED_LOCALIZATION_DVHOP_MODULE_H

#include "algorithms/localization/distance_based/modules/localization_module.h"
#include "algorithms/localization/distance_based/modules/distance/localization_dv_hop_messages.h"
#include "algorithms/localization/distance_based/util/localization_defutils.h"
#include "config_testing.h"
#include "algorithms/localization/distance_based/math/vec.h"

namespace wiselib
{

   /// Module implementing DV-hop
   /** This module implements DV-hop. On the one hand, unknown nodes store the
    *  minimal hop count to at most 'floodlimit' anchors, where the floodlimit
    *  is taken from the LocalizationObserver. On the other hand, anchors
    *  compute an average hop distance between each other and send this
    *  information out. The unknowns again convert their minimal hop counts
    *  via the average hop distance into distances to anchors.
    */
   template<typename OsModel_P,
            typename Radio_P,
            typename Clock_P,
            typename Debug_P,
            typename SharedData_P,
            typename NodeSet_P,
            typename Arithmatic_P >
   class LocalizationDvHopModule
      : public LocalizationModule<OsModel_P, Radio_P, SharedData_P>
   {

   public:
      typedef OsModel_P OsModel;
      typedef Radio_P Radio;
      typedef Clock_P Clock;
      typedef Debug_P Debug;
      typedef SharedData_P SharedData;
      typedef NodeSet_P NodeSet;
      typedef Arithmatic_P Arithmatic;

      typedef LocalizationDvHopModule<OsModel, Radio, Clock, Debug, SharedData, NodeSet,Arithmatic> self_type;
      typedef LocalizationModule<OsModel, Radio, SharedData> base_type;

      typedef typename Radio::size_t size_t;
      typedef typename Radio::node_id_t node_id_t;
      typedef typename Radio::block_data_t block_data_t;

      typedef typename Clock_P::time_t time_t;

      typedef LocalizationDvHopMessage<OsModel, Radio, Arithmatic> DvHopMessage;
      typedef LocalizationDvCalMessage<OsModel, Radio, Arithmatic> DvCalMessage;

      typedef typename SharedData::Neighborhood::NeighborhoodIterator NeighborhoodIterator;

      ///@name construction / destruction
      ///@{
      ///
      LocalizationDvHopModule();
      ///
      ~LocalizationDvHopModule();
      ///@}


      ///@name standard methods startup/simulation steps
      ///@{
      /** Handling of DV-Hop-Messages.
       *
       *  \sa LocalizationModule::process_message()
       */
      void receive( node_id_t from, size_t len, block_data_t *data );
      /** Check, whether state can be set to finished or not. Moreover, if
       *  owner is an anchor, initial and calibrating message are send.
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


      void init( Radio& radio, Clock& clock, Debug& debug, SharedData& shared_data ) {
         radio_ = &radio;
         clock_ = &clock;
         debug_ = &debug;
         this->set_shared_data( shared_data );

         message_counter1 = 0;
         //message_counter2 = 0;

//#define MAX_MESSAGE_COUNTER 5
         last_useful_msg_ = clock_->time();


           //this->state_  = dvh_init ;
           // avg_hop_dist_ = UNKNOWN_AVG_HOP_DIST ;
          //  hop_sum_    =  0 ;
          //  hop_cnt_   = 0 ;
      }

   protected:

      ///@name message handling methods
      ///@{
      /** Message handling by anchors. If anchor receives a dv-hop message, it
       *  computes the real distance between them and devides it by the
       *  hop count to get an average hop distance.
       *
       *  This is done for each received message (as long as floodlimit is not
       *  reached), so that the mean of average hop distances can be builded
       *  to send out a calibration message.
       *
       *  \sa LocalizationDVhopMessage
       */
      bool process_dv_hop_message_anchor( node_id_t from, size_t len, block_data_t *data );
      /** Message handling by unknowns. If unknown receives a dv-hop message,
       *  it decides, whether anchor and hop count should be stored or not.
       *
       *  If a message about a new anchor arrived and floodlimit is not
       *  reached, or the received hop count is lower than known one,
       *  information is stored and a new message containing this is sent out.
       *
       *  \sa LocalizationDVhopMessage
       */
      bool process_dv_hop_message_unknown( node_id_t from, size_t len, block_data_t *data );
      /** If an unknown receives dv-hop calibration message containing an
       *  average hop distance, the hops to known anchors are converted into
       *  distances by multiplying them with each other.
       *
       *  \sa LocalizationDVcalMessage
       */
      bool process_dv_cal_message( node_id_t from, size_t len, block_data_t *data );
      ///@}


   private:

      enum MessagesIds
      {
         DV_HOP_MESSAGE = 200,
         DV_CAL_MESSAGE = 201
      };

      enum DvHopState
      {
         dvh_init,
         dvh_work,
         dvh_calibrated,
         dvh_finished
      };

      DvHopState state_;
      Arithmatic avg_hop_dist_;
      Arithmatic hop_sum_;
      int hop_cnt_;

      time_t last_useful_msg_;
      NodeSet known_anchors_;

      Radio* radio_;
      Clock* clock_;
      Debug* debug_;
      uint8_t message_counter1;
     // uint8_t message_counter2;
      //uint8_t message_counter3;

   };
   // ----------------------------------------------------------------------
   // ----------------------------------------------------------------------
   // ----------------------------------------------------------------------
   template<typename OsModel_P,
            typename Radio_P,
            typename Clock_P,
            typename Debug_P,
            typename SharedData_P,
            typename NodeSet_P,
            typename Arithmatic_P>
   LocalizationDvHopModule<OsModel_P, Radio_P, Clock_P, Debug_P, SharedData_P, NodeSet_P, Arithmatic_P>::
   LocalizationDvHopModule()
      : state_            ( dvh_init ),
         avg_hop_dist_    ( UNKNOWN_AVG_HOP_DIST ),
         hop_sum_         ( 0 ),
         hop_cnt_         ( 0 )
   {



   }
   // ----------------------------------------------------------------------
   template<typename OsModel_P,
            typename Radio_P,
            typename Clock_P,
            typename Debug_P,
            typename SharedData_P,
            typename NodeSet_P,
            typename Arithmatic_P>
   LocalizationDvHopModule<OsModel_P, Radio_P, Clock_P, Debug_P, SharedData_P, NodeSet_P, Arithmatic_P>::
   ~LocalizationDvHopModule()
   {}
   // ----------------------------------------------------------------------
   template<typename OsModel_P,
            typename Radio_P,
            typename Clock_P,
            typename Debug_P,
            typename SharedData_P,
            typename NodeSet_P,
            typename Arithmatic_P>
   void
   LocalizationDvHopModule<OsModel_P, Radio_P, Clock_P, Debug_P, SharedData_P, NodeSet_P, Arithmatic_P>::
   receive( node_id_t from, size_t len, block_data_t *data )
   {


      switch ( data[0] )
      {
         case DV_HOP_MESSAGE:
            if ( this->shared_data().is_anchor() )
               process_dv_hop_message_anchor( from, len, data );
            else
               process_dv_hop_message_unknown( from, len, data );
            break;
         case DV_CAL_MESSAGE:
               process_dv_cal_message( from, len, data );
            break;
      }
   }
   // ----------------------------------------------------------------------
   template<typename OsModel_P,
            typename Radio_P,
            typename Clock_P,
            typename Debug_P,
            typename SharedData_P,
            typename NodeSet_P,
            typename Arithmatic_P>
   void
   LocalizationDvHopModule<OsModel_P, Radio_P, Clock_P, Debug_P, SharedData_P, NodeSet_P, Arithmatic_P>::
   work( void )
   {
#ifdef LOCALIZATION_DISTANCEBASED_DVHOP_DEBUG
	  if( this->shared_data().is_anchor()){

		   //debug_->debug(  "DVHOP:MODUL::Anchor %x is working in state %d ",radio_->id(), this->state_ );
	   }
#endif

     // do initial stuff and set state to 'working'
      if ( state_ == dvh_init )
      {
         if ( this->shared_data().is_anchor() )
         {
            DvHopMessage message;
            message.set_msg_id( DV_HOP_MESSAGE );
            message.set_anchor( radio_->id( ) );
            message.set_anchor_position( this->shared_data().position() );
            //debug_->debug(  "DVHOP:MODUL::Anchor %x sending DV_HOP_MESSAGE ",radio_->id() );

           radio_->send(  Radio::BROADCAST_ADDRESS, message.buffer_size(), (block_data_t*)&message );

             if(message_counter1>1){
                    	 state_ = dvh_work;
                    	 //debug_->debug(  "DVHOP:MODUL::Anchor %x date is work ",radio_->id() );
             }
              message_counter1++;
         //}else{

        // if(message_counter1>MAX_MESSAGE_COUNTER)
        	 state_ = dvh_work;
        // message_counter1++;
         }
         //state_ = dvh_work;
      }

      // check, whether state can set to be finished or not
      // - unknown node is set finished, if given idle-time is passed ( no
      //   more useful messages came in ) and hops were converted into distances
      // - anchor is finished, if given idle-time is passed and there are
      //   at least one more anchors known, so that the average hop distance
      //   could estimated
      if ( (state_ != dvh_finished) && (clock_->time() - last_useful_msg_ > this->shared_data().idle_time()) )
      {
         if ( !this->shared_data().is_anchor() && state_ == dvh_calibrated )
         {

             //if(message_counter2>MAX_MESSAGE_COUNTER){
             	state_ = dvh_finished;
#ifdef LOCALIZATION_DISTANCEBASED_DVHOP_DEBUG
             	if(radio_->id()==0x1c72)
             	               debug_->debug(  "DVHOP:MODUL:: timeout finish" );
#endif
             //}
             //message_counter2++;
         }
         else if ( this->shared_data().is_anchor() && state_ == dvh_calibrated )
         {
            DvCalMessage message;
            message.set_msg_id( DV_CAL_MESSAGE );
            message.set_avg_hop_dist( hop_sum_ / hop_cnt_ );

           // debug_->debug(  "DVHOP:MODUL::Anchor %x sending DV_CAL_MESSAGE ",radio_->id() );


            	radio_->send(  Radio::BROADCAST_ADDRESS, message.buffer_size(), (block_data_t*)&message );

            //if(message_counter2>MAX_MESSAGE_COUNTER){
            	state_ = dvh_finished;

            //}
            //message_counter2++;
         }
      }

   }
   // ----------------------------------------------------------------------
   template<typename OsModel_P,
            typename Radio_P,
            typename Clock_P,
            typename Debug_P,
            typename SharedData_P,
            typename NodeSet_P,
            typename Arithmatic_P>
   bool
   LocalizationDvHopModule<OsModel_P, Radio_P, Clock_P, Debug_P, SharedData_P, NodeSet_P, Arithmatic_P>::
   process_dv_hop_message_anchor( node_id_t from, size_t len, block_data_t *data )
   {


	   if( (this->shared_data().is_anchor()) && (state_ == 0))
		   return false;

      if ( state_ == dvh_finished ) return true;

      DvHopMessage* msg = (DvHopMessage*)data;
      node_id_t anchor = msg->anchor();

      // if the anchor is already known, 'floodlimit' is reached or
      // the message is from the anchor itself
      if ( ( known_anchors_.find( anchor ) != known_anchors_.end() ) ||
           ( known_anchors_.size() >= this->shared_data().floodlimit() ) ||
           ( radio_->id() == anchor ) )
         return true;

      // add anchor to known ones and set time of last useful message to
      // actual
      known_anchors_.insert( anchor );
      last_useful_msg_ = clock_->time();

      Arithmatic distance= Vec<Arithmatic>::euclidean_distance( this->shared_data().position(), msg->anchor_position() );
      Arithmatic avg_hop_dist = distance / (msg->hop_count() + 1);

      // build average hop distance of all known anchors
      ++hop_cnt_;
      hop_sum_ += avg_hop_dist;

#ifdef LOCALIZATION_DISTANCEBASED_DVHOP_DEBUG
      if(radio_->id()==0x9999)
      debug_->debug(  "Hop calc at %d: %f / %d = %f\n",
                    radio_->id(),
                    hop_sum_, hop_cnt_, hop_sum_ / hop_cnt_ );
#endif

    	  state_ = dvh_calibrated;

      return true;
   }
   // ----------------------------------------------------------------------
   template<typename OsModel_P,
            typename Radio_P,
            typename Clock_P,
            typename Debug_P,
            typename SharedData_P,
            typename NodeSet_P,
            typename Arithmatic_P>
   bool
   LocalizationDvHopModule<OsModel_P, Radio_P, Clock_P, Debug_P, SharedData_P, NodeSet_P, Arithmatic_P>::
   process_dv_hop_message_unknown( node_id_t from, size_t len, block_data_t *data )
   {
      if ( state_ == dvh_finished ) return true;

      DvHopMessage* msg = (DvHopMessage*)data;
      node_id_t source = from;
      node_id_t anchor = msg->anchor();

      Vec<Arithmatic> anchor_pos = msg->anchor_position();
      int rcvd_hops = msg->hop_count() + 1;

      // if anchor is new and floodlimit not reached, insert new anchor;
      // if anchor is known and new hop-distance is smaller than old one,
      //    update info;
      // otherwise return
      NeighborhoodIterator it = this->shared_data().neighborhood().find( anchor );

      if ( it == this->neighborhood().end_neighborhood() )
      {
         if ( this->neighborhood().anchor_cnt() >= (int)this->shared_data().floodlimit() ){
        	 #ifdef LOCALIZATION_DISTANCEBASED_DVHOP_DEBUG
        	 if(radio_->id()==0x1c72)
        	                debug_->debug(  "exceeding floodlimit %d " ,
        	             		   this->neighborhood().anchor_cnt()
        	                               );
        	 #endif
            return true;
         }

         this->neighborhood().update_anchor( anchor, anchor_pos );
         #ifdef LOCALIZATION_DISTANCEBASED_DVHOP_DEBUG
               if(radio_->id()==0x1c72)
               debug_->debug(  "adding anchor %d %x" ,
            		   this->neighborhood().anchor_cnt(), anchor
                              );
         #endif

         // set iterator to the new inserted anchor
         it = this->neighborhood().find( anchor );
      }
      else
      {
    	  #ifdef LOCALIZATION_DISTANCEBASED_DVHOP_DEBUG
    	          	 if(radio_->id()==0x1c72)
    	          	                debug_->debug(  "Anchor %x is already known  " ,
    	          	             		   anchor
    	          	                               );
    	          	 #endif
         if ( it->second.hops() < rcvd_hops )
            return true;
         else if ( it->second.hops() == rcvd_hops )
         {
            this->neighborhood().update_ref_node( anchor, source );
            return true;
         }
      }
      it->second.set_hops( rcvd_hops );
      if ( avg_hop_dist_ != UNKNOWN_AVG_HOP_DIST )
         it->second.convert_hops( avg_hop_dist_ );

      this->neighborhood().set_ref_node( anchor, source );

      // set time of last useful message to actual and broadcast new info
      last_useful_msg_ = clock_->time();
      DvHopMessage message;
      message.set_msg_id( DV_HOP_MESSAGE );
      message.set_anchor( msg->anchor() );
      message.set_anchor_position( msg->anchor_position() );
      message.set_hop_count( rcvd_hops );
      //TODO   than one message
      //for(uint8_t  i =0 ; i<MAX_MESSAGE_COUNTER ; i++)
    	  radio_->send( Radio::BROADCAST_ADDRESS, message.buffer_size(), (block_data_t*)&message );



      return true;
   }
   // ----------------------------------------------------------------------
   template<typename OsModel_P,
            typename Radio_P,
            typename Clock_P,
            typename Debug_P,
            typename SharedData_P,
            typename NodeSet_P,
            typename Arithmatic_P>
   bool
   LocalizationDvHopModule<OsModel_P, Radio_P, Clock_P, Debug_P, SharedData_P, NodeSet_P, Arithmatic_P>::
   process_dv_cal_message( node_id_t from, size_t len, block_data_t *data )
   {

	   if( (this->shared_data().is_anchor()) && (state_ == dvh_init))
		   return false;

//#ifdef LOCALIZATION_DISTANCEBASED_DVHOP_DEBUG
	   if(radio_->id()==0x9999)
      debug_->debug(  "dv_cal_message\n"
                    );
//#endif

      if ( state_ == dvh_finished ) return true;

      // if there is already an average hop distance known, return
      if ( avg_hop_dist_ != UNKNOWN_AVG_HOP_DIST )
         return true;

      DvCalMessage* msg = (DvCalMessage*)data;

      // set received info and convert hop-count into real distances
      avg_hop_dist_ = msg->avg_hop_dist();
      for ( NeighborhoodIterator
               it = this->neighborhood().begin_neighborhood();
               it != this->neighborhood().end_neighborhood();
               ++it )
         it->second.convert_hops( avg_hop_dist_ );

      // set state to calibrated and broadcast info
      //if(known_anchors_.size()>2)
    	  state_ = dvh_calibrated;
      //else return false;
      //TODO more than one message
      //for(uint8_t  i =0 ; i<MAX_MESSAGE_COUNTER ; i++)
    	  radio_->send(  Radio::BROADCAST_ADDRESS, len, data );

      return true;
   }
   // ----------------------------------------------------------------------
   template<typename OsModel_P,
            typename Radio_P,
            typename Clock_P,
            typename Debug_P,
            typename SharedData_P,
            typename NodeSet_P,
            typename Arithmatic_P>
   bool
   LocalizationDvHopModule<OsModel_P, Radio_P, Clock_P, Debug_P, SharedData_P, NodeSet_P, Arithmatic_P>::
   finished( void )
   {
      return state_ == dvh_finished;
   }
   // ----------------------------------------------------------------------
   template<typename OsModel_P,
            typename Radio_P,
            typename Clock_P,
            typename Debug_P,
            typename SharedData_P,
            typename NodeSet_P,
            typename Arithmatic_P>
   void
   LocalizationDvHopModule<OsModel_P, Radio_P, Clock_P, Debug_P, SharedData_P, NodeSet_P, Arithmatic_P>::
   rollback( void )
   {
      state_  = dvh_init;
      last_useful_msg_ = clock_->time();
      avg_hop_dist_ = UNKNOWN_AVG_HOP_DIST;
      hop_sum_ = 0;
      hop_cnt_ = 0;
      this->shared_data().reset_neighborhood_();
   }

}// namespace wiselib
#endif
