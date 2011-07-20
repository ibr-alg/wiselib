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

#ifndef __AUTOCAST_ALGORITHM_H__
#define __AUTOCAST_ALGORITHM_H__

#include "util/base_classes/routing_base.h"
#include "util/delegates/delegate.hpp"

#include "util/pstl/vector_static.h"

#include "data_unit.h"
#include "autocast_message.h"

// the max size of vector to save the data units and hash values
#define MAX_VECTOR_SIZE 13

#define DELTA 8

#define MSG_ID 112
// 
#define BEACON_TIMER_NR 0
#define FLOOD_TIMER_NR 1
#define ANSWER_TIMER_NR 2
#define REQUEST_TIMER_NR 3
#define NUM_TIMER 4

typedef wiselib::OSMODEL Os;

namespace wiselib
{
   /**
    * \brief AutoCast routing implementation of \ref routing_concept "Routing Concept".
    *
    *  \ingroup routing_concept
    *  \ingroup radio_concept
    *  \ingroup basic_algorithm_concept
    *  \ingroup routing_algorithm
    *
    * AutoCast routing implementation of \ref routing_concept "Routing Concept" ...
    */
   template<typename OsModel_P,
            typename Radio_P = typename OsModel_P::Radio,
            typename Timer_P = typename OsModel_P::Timer,
            typename Debug_P = typename OsModel_P::Debug>
   class AutoCast
      : public RoutingBase<OsModel_P, Radio_P>
   {

   public:
		typedef OsModel_P OsModel;
		typedef Radio_P Radio;
		typedef Timer_P Timer;
		typedef Debug_P Debug;

		typedef AutoCast<OsModel, Radio_P, Timer, Debug> self_type;
		typedef self_type* self_pointer_t;

		typedef typename Radio::node_id_t node_id_t;
		typedef typename Radio::size_t size_t;
		typedef typename Radio::block_data_t block_data_t;
		typedef typename Radio::message_id_t message_id_t;

		typedef typename Timer::millis_t millis_t;
		//------------------------------------------------------------
		//---------------------------------------------------------------
		typedef wiselib::AutoCast_Message<OsModel, Radio, Debug> Message;

 		typedef DataUnit<Os, Radio> dataUnit_t;
		
		// for the callback functions 
		typedef delegate1<uint8_t, dataUnit_t*> radio_delegate_t;

		typedef uint16_t hash_Value_t;

		typedef wiselib::vector_static<Os, dataUnit_t, MAX_VECTOR_SIZE> VectorStatic_t;

		//typedef wiselib::vector_static<Os, dataUnit_t*, MAX_VECTOR_SIZE> VectorStatic_ptr_t;

		typedef wiselib::vector_static<Os, hash_Value_t, MAX_VECTOR_SIZE> dataHash_list_t;

		typedef wiselib::vector_static<Os, node_id_t, MAX_VECTOR_SIZE> VS_Neighbor_t;
	
		typedef typename VectorStatic_t::iterator iterator_dataUnit;
//		typedef typename VectorStatic_ptr_t::iterator iterator_dataUnitForSend;
		typedef typename dataHash_list_t::iterator iterator_dataHash;

		typedef wiselib::TriSOSClockModel<Os> c_time_t;
		
		//----------------------------------------------------------
		//----------------------------------------------------------


      // --------------------------------------------------------------------
      enum ErrorCodes
      {
         SUCCESS = OsModel::SUCCESS,
         ERR_UNSPEC = OsModel::ERR_UNSPEC,
         ERR_NOTIMPL = OsModel::ERR_NOTIMPL,
         ERR_HOSTUNREACH = OsModel::ERR_HOSTUNREACH
      };
      // --------------------------------------------------------------------
      enum SpecialNodeIds {
         BROADCAST_ADDRESS = Radio_P::BROADCAST_ADDRESS, ///< All nodes in communication range
         NULL_NODE_ID      = Radio_P::NULL_NODE_ID      ///< Unknown/No node id
      };
     
      // --------------------------------------------------------------------
      ///@name Construction / Destruction
      ///@{
      AutoCast();
      ~AutoCast();
      ///@}

      int init( Radio& radio, Timer& timer, Debug& debug )
      {
         radio_ = &radio;
         timer_ = &timer;
         debug_ = &debug;
         return SUCCESS;
      }

	 //	Callback functions for the getstate function in the application
	//------------------------------------------------------
	//-----------------------------------------------------
      template<class T, uint8_t (T::*TMethod)(dataUnit_t*)>
      int reg_getstate_callback( T *obj_pnt )
      {

	  	 if ( callback == NULL )
		 {
		 	callback = radio_delegate_t();
		 }

         if ( callback == radio_delegate_t())
         {
             callback = radio_delegate_t::template from_method<T, TMethod>( obj_pnt);
			 
             return 0;
         }
         return -1;
      }
      // --------------------------------------------------------------------
      int unreg_state_callback()
      {
         return SUCCESS;
      }
      // --------------------------------------------------------------------
       uint8_t notify_getstate(dataUnit_t *du)
      {
         if ( callback != radio_delegate_t())
		 {

          	return (callback)(du);
		 }
      }
	//-----------------------------------------------------------------------
	//-----------------------------------------------------------------------


      inline int init();
      inline int destruct();

      ///@name Routing Control
      ///@{
      int enable_radio( void );
      int disable_radio( void );

      ///@name Radio Concept
      ///@{
      /**
       */
      int send( node_id_t receiver, size_t len, block_data_t *data );
      /**
       */
      void receive( node_id_t from, size_t len, block_data_t *data );
      /**
       */
      typename Radio::node_id_t id()
      { return radio_->id(); }
      ///@}


   private:
	
      Radio& radio()
      { return *radio_; }

      Timer& timer()
      { return *timer_; }

      Debug& debug()
      { return *debug_; }

      typename Radio::self_pointer_t radio_;
      typename Timer::self_pointer_t timer_;
      typename Debug::self_pointer_t debug_;

	  uint8_t numberOfData;

	  c_time_t c_time;

	  VectorStatic_t setDataUnits;
	  VectorStatic_t setDUsForSend_t;
	  VectorStatic_t setStaleDataUnits;
	  //VectorStatic_ptr_t setDUsForSend;
	  VS_Neighbor_t setNeighbors;

	  //VectorStatic_ptr_t setNewDataUnits; 
	  VectorStatic_t setNewDataUnits_t;

	  dataHash_list_t setReqHashes;

	  dataHash_list_t test_HashValue;

	  bool find_InSetDataUnits(hash_Value_t hashValue);
	  
	  bool find_InSetStaleDataUnits(hash_Value_t hashValue);

	  bool find_InSetDUsForSend(hash_Value_t hashValue);

	  bool find_InSetNeighbors(node_id_t nID);


	  void onTimerExpired();

	  void deltaTimerElapsed( void* userdata);

	  void test( void* userdata);

	  void updateDataUnits();
	  
	  // to save, that the timer allready set or not (rrue)
	  bool timerPending[NUM_TIMER];
	  // to save the expire time for all timer
	  millis_t timerExpireTime[NUM_TIMER];

	  // timer function to expire, cancel and test the validit of the 4 actions (Beacon, Requesr, Answer and flood)
	  // ---------------------------------------------------
	  void expireIn(uint8_t timerNr, millis_t timedelta);

	  bool isPending(uint8_t timerNr);

	  void cancel(uint8_t timerNr);
	  //----------------------------------------------------

	  radio_delegate_t callback;
	  // auxiliary variable to check, that the element already in msg.setHashes.
	  bool element_is_in_setDUsForSend;
	  // auxiliary variable to check, that the element already in msg.setDUsForSend.
	  bool element_is_in_msgSetH;
	   // auxiliary variable to check, that the element already in msg.setDataUnits.
	   // use for the setDataUnits
	  bool element_is_in_msgSetDU;
	  // auxiliary variable to check, that the element already in msg.setStaleHashes.
	  bool element_is_in_msgSetSH;
	   // auxiliary variable to check, that the element already in msg.setDataUnits.
	   // use for the setReqHashes
	  bool element_is_in_msgSetDU_1;
   };
   // -----------------------------------------------------------------------
   // -----------------------------------------------------------------------
   // -----------------------------------------------------------------------
   template<typename OsModel_P,
         
            typename Radio_P,
            typename Timer_P,
            typename Debug_P>
   AutoCast<OsModel_P, Radio_P,Timer_P, Debug_P>::
   AutoCast()
      : radio_ ( 0 ),
         timer_ ( 0 ),
         debug_ ( 0 )
   {}
   // -----------------------------------------------------------------------
   template<typename OsModel_P,
            
            typename Radio_P,
            typename Timer_P,
            typename Debug_P>
   AutoCast<OsModel_P, Radio_P,Timer_P, Debug_P>::
   ~AutoCast()
   {

#ifdef ROUTING_AUTOCAST_DEBUG
      debug().debug( "AutoCast: Destroyed\n" );
#endif
   }
   // -----------------------------------------------------------------------
   template<typename OsModel_P,
            
            typename Radio_P,
            typename Timer_P,
            typename Debug_P>
   int
   AutoCast<OsModel_P, Radio_P,Timer_P, Debug_P>::
   init( void )
   {
      enable_radio();

      return SUCCESS;
   }
   // -----------------------------------------------------------------------
   template<typename OsModel_P,
            
            typename Radio_P,
            typename Timer_P,
            typename Debug_P>
   int
   AutoCast<OsModel_P, Radio_P,Timer_P, Debug_P>::
   destruct( void )

   {
      return disable_radio();
   }
   // -----------------------------------------------------------------------
   template<typename OsModel_P,
           
            typename Radio_P,
            typename Timer_P,
            typename Debug_P>
   int
   AutoCast<OsModel_P, Radio_P,Timer_P, Debug_P>::
   enable_radio( void )
   {

#ifdef ROUTING_AUTOCAST_DEBUG
      debug().debug( "AutoCast: Boot for %i\n", radio().id() );
#endif
      radio().enable_radio();
      radio().template reg_recv_callback<self_type, &self_type::receive>( this );

	  //reg_getstate_callback<self_type, &self_type::updateDataUnits>( this ); 
	
	  // intialisierung of all auxiliary variables
	  element_is_in_msgSetH = false;
	  element_is_in_setDUsForSend = false;
	  element_is_in_msgSetDU = false;
	  element_is_in_msgSetSH = false;
	  element_is_in_msgSetDU = false;

	  timerPending[BEACON_TIMER_NR]= false;
	  timerPending[FLOOD_TIMER_NR]= false;
	  timerPending[FLOOD_TIMER_NR]= false;
	  timerPending[REQUEST_TIMER_NR]= false;


      //expireIn(BEACON_TIMER_NR,5000);

	  timer().template set_timer<self_type, &self_type::deltaTimerElapsed>(DELTA, this, 0 );

      return SUCCESS;
   }

   // -----------------------------------------------------------------------
   template<typename OsModel_P,
            
            typename Radio_P,
            typename Timer_P,
            typename Debug_P>
   int
   AutoCast<OsModel_P, Radio_P,Timer_P, Debug_P>::
   disable_radio( void )
   {

#ifdef ROUTING_AUTOCAST_DEBUG
      debug().debug( "AutoCast: Disable\n" );
#endif
      return ERR_NOTIMPL;
   }

/* --------------------------------------------------------------------
	this function contains the timer. This timer will be called periodic for the time DELTA
	and test, that the one time of the furth timers allready expired, if yes then call the function
	onTimerExpired() and canceld all other timer expect the beacon timer
	 -----------------------------------------------------------------------*/
   template<typename OsModel_P,
           
            typename Radio_P,
            typename Timer_P,
            typename Debug_P>
   void
   AutoCast<OsModel_P, Radio_P,Timer_P, Debug_P>::
   deltaTimerElapsed( void* userdata)
   {
 		millis_t t = c_time.time();
		millis_t time = (c_time.seconds(t) * 1000) + c_time.milliseconds(t);

		if( (time > timerExpireTime[BEACON_TIMER_NR] && isPending(BEACON_TIMER_NR)) ||
		   	(time > timerExpireTime[FLOOD_TIMER_NR] && isPending(FLOOD_TIMER_NR))   ||
		   	(time > timerExpireTime[ANSWER_TIMER_NR] && isPending(FLOOD_TIMER_NR))  ||
		   	(time > timerExpireTime[REQUEST_TIMER_NR] && isPending(REQUEST_TIMER_NR)) )
		{
			onTimerExpired();
			
		}

	  	timer().template set_timer<self_type, &self_type::deltaTimerElapsed>(DELTA, this, 0 );
	
   }

/* -----------------------------------------------------------------------
	this function test, that the max life time of all data units in setDataUnits
	expired, if yes then remove it from setDataUnits and put it in setStaleDataUnits 
   -----------------------------------------------------------------------*/
   template<typename OsModel_P,
           
            typename Radio_P,
            typename Timer_P,
            typename Debug_P>
   void
   AutoCast<OsModel_P, Radio_P,Timer_P, Debug_P>::
   updateDataUnits()
   {
 		// data units are unintersting 
		for(int i =0; i< setDataUnits.size(); i++){

			if(notify_getstate(&(setDataUnits[i])) == dataUnit_t::STALE /*&& !find_InSetStaleDataUnits(setDataUnits[i].getHashValue())*/){
				
				setStaleDataUnits.push_back(setDataUnits[i]);
				setDataUnits.erase(setDataUnits.begin()+i);
			}else{
				if(notify_getstate(&(setDataUnits[i])) == dataUnit_t::INVALID)
					
					setDataUnits.erase(setDataUnits.begin()+i);
			}
		}

		// uninteresting data units
		for(int j =0; j < setStaleDataUnits.size(); j++){

			if(notify_getstate(&(setStaleDataUnits[j])) == dataUnit_t::VALID  /*&& !find_InSetDataUnits(setStaleDataUnits[j].getHashValue())*/){
				
				// again interesting
				setDataUnits.push_back(setStaleDataUnits[j]);
				setStaleDataUnits.erase(setStaleDataUnits.begin()+j);
			}
			else{
				
				if(notify_getstate(&(setStaleDataUnits[j])) == dataUnit_t::INVALID){
					
					// data unit from set of stale data units will be final deleted
					setStaleDataUnits.erase(setStaleDataUnits.begin()+j);
				}
			}
		}

    }

/* -----------------------------------------------------------------------
	this function test, that the given data unit is already in setDataUnits,
	if yes then return true otherwise false
   -----------------------------------------------------------------------*/
   template<typename OsModel_P,
           
            typename Radio_P,
            typename Timer_P,
            typename Debug_P>
   bool
   AutoCast<OsModel_P, Radio_P,Timer_P, Debug_P>::
   find_InSetDataUnits(hash_Value_t hashValue)
   {
		for(iterator_dataUnit it= setDataUnits.begin(); it != setDataUnits.end(); ++it)
		{	
			if(it->getHashValue() ==  hashValue)
			{
				return true;
			}
		 }
		return false;	
   }

   /* -----------------------------------------------------------------------
	this function test, that the given data unit is already in setDataUnits,
	if yes then return true otherwise false
   -----------------------------------------------------------------------*/
   template<typename OsModel_P,
           
            typename Radio_P,
            typename Timer_P,
            typename Debug_P>
   bool
   AutoCast<OsModel_P, Radio_P,Timer_P, Debug_P>::
   find_InSetDUsForSend(hash_Value_t hashValue)
   {
		for(iterator_dataUnit it= setDUsForSend_t.begin(); it != setDUsForSend_t.end(); ++it)
		{	
			if(it->getHashValue() ==  hashValue)
			{
				return true;
			}
		 }
		return false;	
   }



/* -----------------------------------------------------------------------
	this function test, that the given data unit is already in setStaleDataUnits,
	if yes then return true otherwise false
   -----------------------------------------------------------------------*/
   template<typename OsModel_P,
           
            typename Radio_P,
            typename Timer_P,
            typename Debug_P>
   bool
   AutoCast<OsModel_P, Radio_P,Timer_P, Debug_P>::
   find_InSetStaleDataUnits(hash_Value_t hashValue)
   {
		for(iterator_dataUnit it= setStaleDataUnits.begin(); it != setStaleDataUnits.end(); ++it)
		{
			if(it->getHashValue() ==  hashValue)
			{
				return true;
			}
		 }
		return false;	
   }



/* -----------------------------------------------------------------------
	this function test, that the given neighbor is already in setNeighbors,
	if yes then return true otherwise false
   -----------------------------------------------------------------------*/
   template<typename OsModel_P,
           
            typename Radio_P,
            typename Timer_P,
            typename Debug_P>
   bool
   AutoCast<OsModel_P, Radio_P,Timer_P, Debug_P>::
   find_InSetNeighbors(node_id_t nID)
   {
		for(int i=0; i < setNeighbors.size(); i++)
		{
			if(setNeighbors[i] ==  nID)
			{
				return true;
			}
		 }
		return false;	
   }


/* --------------------------------------------------------------------
	this function move the time to the given timedelta for one given timer number
	and saver it in the array timerExpireTime
	 -----------------------------------------------------------------------*/
   template<typename OsModel_P,
            
            typename Radio_P,
            typename Timer_P,
            typename Debug_P>
   void
   AutoCast<OsModel_P, Radio_P,Timer_P, Debug_P>::
   expireIn(uint8_t timerNr, millis_t timedelta)
   {	
		timerPending[timerNr]=true;
		millis_t t = c_time.time();
		millis_t time = (c_time.seconds(t) * 1000) + c_time.milliseconds(t);
		time+=timedelta;
		timerExpireTime[timerNr]= time;

   } 
/* ----------------------------------------------------------------------- 
	set the timer of false for the given timer number
	-----------------------------------------------------------------------*/
   template<typename OsModel_P,
            
            typename Radio_P,
            typename Timer_P,
            typename Debug_P>
   void
   AutoCast<OsModel_P, Radio_P,Timer_P, Debug_P>::
   cancel(uint8_t timerNr)
   {	
	timerPending[timerNr]=false;
		
   }

/* ----------------------------------------------------------------------- 
	return true if the timer number allready set otherwise false
	-----------------------------------------------------------------------*/
   template<typename OsModel_P,
            
            typename Radio_P,
            typename Timer_P,
            typename Debug_P>
   bool
   AutoCast<OsModel_P, Radio_P,Timer_P, Debug_P>::
   isPending(uint8_t timerNr)
   {	
		return timerPending[timerNr];
   }
/* ----------------------------------------------------------------------- 
	this function will be called after every action (flood, answer, request and beacon timer).
	Greate and send the message
   -----------------------------------------------------------------------*/
   template<typename OsModel_P,
            
            typename Radio_P,
            typename Timer_P,
            typename Debug_P>
   void
   AutoCast<OsModel_P, Radio_P,Timer_P, Debug_P>::
   onTimerExpired()
   {	
		updateDataUnits();

		// cancel all timers, it dose not need more timer, message will be send
		cancel(FLOOD_TIMER_NR);
		cancel(ANSWER_TIMER_NR);
		cancel(REQUEST_TIMER_NR);
		
		// generate a message with the number of byte of the hash value
		Message message(sizeof(hash_Value_t), radio_->id());

		// add the stale data Unit from setStaleDUsForSend to the message
		for(int i = 0; i < setStaleDataUnits.size(); i++){
			
			hash_Value_t hv_stale = setStaleDataUnits[i].getHashValue();

			message.addStaleDataHash((block_data_t *) &(hv_stale));
		}		

		// add the hash value of all data unit from setDataUnit to the message
		for(iterator_dataUnit it = setDataUnits.begin(); it != setDataUnits.end(); ++it){

			hash_Value_t hv_DataUnit = it->getHashValue();

			for(int i=0; i < setDUsForSend_t.size(); i++){

				if(hv_DataUnit == setDUsForSend_t[i].getHashValue()){
					
					element_is_in_setDUsForSend = true; //TODO
				}
			}
			// if the data unit from setDataUnits is not in setDUsForSend
			if(!element_is_in_setDUsForSend){

				// then add it to the message
				message.addDataHash((block_data_t *) &(hv_DataUnit));
			}	
		}
		
		element_is_in_setDUsForSend = false;	

		// add the data from setDUsForSend to the message
		for(int i = 0; i < setDUsForSend_t.size(); i++){

			message.addDataUnit(setDUsForSend_t[i].buffer_size() , (block_data_t *) &(setDUsForSend_t[i]));

		}


		// send the message
      	radio().send( radio().BROADCAST_ADDRESS, message.buffer_size(), (block_data_t*) &message);
		//debug_->debug( "#");

		
		
		// clear setDUsForSend
		setDUsForSend_t.clear();
		// clear setReqHashes
		setReqHashes.clear();

		// set the beacon time of 5 seconds again
	    //expireIn(BEACON_TIMER_NR,5000);

   }
 
// ----------------------------------------------------------------------- 
// -----------------------------------------------------------------------
   template<typename OsModel_P,
            
            typename Radio_P,
            typename Timer_P,
            typename Debug_P>
   int
   AutoCast<OsModel_P, Radio_P,Timer_P, Debug_P>::
   send( node_id_t destination, size_t len, block_data_t *data)
   {
      
#ifdef ROUTING_AUTOCAST_DEBUG
      debug().debug( "AutoCast: Send at %d\n", radio_->id() );
#endif

		dataUnit_t *du = (dataUnit_t*) data;

		//dataUnit with the same node_id already in list?
		if(!find_InSetDataUnits(du->getHashValue()) && !find_InSetDUsForSend(du->getHashValue())){
					
			setDataUnits.push_back(*du);
			// save the data unit in setDUsForSend
			//------------------------------------------------
			//setDUsForSend.push_back(&(setDataUnits.back()));
			setDUsForSend_t.push_back(*du);
			
		}

		
		onTimerExpired();
		

      	return SUCCESS;
   }
// ----------------------------------------------------------------------- 
// -----------------------------------------------------------------------
  template<typename OsModel_P,
            
            typename Radio_P,
            typename Timer_P,
            typename Debug_P>
   void
   AutoCast<OsModel_P, Radio_P,Timer_P, Debug_P>::
   receive( node_id_t from, size_t len, block_data_t *data)
   {	
   			//debug_->debug( "+");
			Message *message = (Message*)data;

			/*for(int i=0; i<len; i++){

				debug_->debug("%d: %d(%c) \n", i, (uint8_t) data[i],(char) data[i]);
			}*/

		

		if(message->getMessageId() == MSG_ID){

			// intialization of the auxiliary variable for the number of hash value to answer or to request
			uint8_t nrOfHashesForAnswer = 0;
			uint8_t nrOfHashesForRequst = 0;
			

			// delay the planned actions to avoid the overhead of tha radio channel
			//////////////////////////////////////////////////
			if(isPending(FLOOD_TIMER_NR)){
				expireIn(FLOOD_TIMER_NR,2 * DELTA);
			}
			if(isPending(ANSWER_TIMER_NR)){
				expireIn(ANSWER_TIMER_NR, DELTA);
			}
			if(isPending(REQUEST_TIMER_NR)){
				expireIn(REQUEST_TIMER_NR, 3 * DELTA);
			}
			
			// sort the received data units 			
			/////////////////////////////////////////////////
			for(int i = 0; i < message->getNrOfDataUnits(); i++){
	
				dataUnit_t *du = (dataUnit_t*) message->getDataUnit(i);

				millis_t *rTimeStamp = (millis_t *) du->getPayload();

				// is the data unit from message already in setDataUnits or in setStaleDataUnits ?
				if(!find_InSetDataUnits(du->getHashValue()) && !find_InSetStaleDataUnits(du->getHashValue())){

					notify_receivers(message->getSendId(), du->buffer_size(), (block_data_t *) du );

					// callback function to check the validation of the data unit
 					if(notify_getstate(du) == dataUnit_t::VALID){						

						setDataUnits.push_back(*du);
						// add the new data unit to th set setNewDataUnit
						//-------------------------------------------------
						//setNewDataUnits.push_back(&(setDataUnits.back()));
						setNewDataUnits_t.push_back(*du);
	
					}
					else{
						if(notify_getstate(du) == dataUnit_t::STALE){
							
							setStaleDataUnits.push_back(*du);
						}
					}
				}
   			}
			
			// save a new neighbor node in the list "setNeighbors"
			if(!find_InSetNeighbors(message->getSendId())){

				setNeighbors.push_back(message->getSendId());
			}

			// Flood action
			//---------------------------------------------------------------------------
			//---------------------------------------------------------------------------
			if(setNewDataUnits_t.size() > 0){

				for(int j=0; j< setNewDataUnits_t.size(); j++){
					
					// add the new data units to the set "setDUsForSend"
					//--------------------------------------------------
					//setDUsForSend.push_back((setNewDataUnits.at(j)));
					
					if(!find_InSetDUsForSend(setNewDataUnits_t[j].getHashValue())){

						setDUsForSend_t.push_back(setNewDataUnits_t[j]);
					}
				}
				// 
				 expireIn( FLOOD_TIMER_NR, 2 * DELTA); 

				 //debug_->debug( "flood is finish \n");
			}

			setNewDataUnits_t.clear();

			
			// Answer action
			//---------------------------------------------------------------------------
			//--------------------------------------------------------------------------
			for(int k=0; k < setDataUnits.size(); k++){
				
				// to check, that the data unit is already in msg.setHashes.
				for(int i = 0; i< message->getNrOfDataHashes(); i++){
					
					hash_Value_t *temp = (hash_Value_t *) message->getDataHash(i);

					if(setDataUnits[k].getHashValue() == *temp){

						element_is_in_msgSetH = true;  // TODO wenn das Element gefunden wurde soll nicht meher suchen (raus springen von for schleife)
					}
				}
				// to check, that the data unit is already in msg.setDataUnits.
				for(int j = 0; j< message->getNrOfDataUnits(); j++){
					
				    dataUnit_t *du = (dataUnit_t *) message->getDataUnit(j);

					if(setDataUnits[k].getHashValue() == du->getHashValue()){

						element_is_in_msgSetDU = true;  // TODO wenn das Element gefunden wurde soll nicht meher suchen (raus springen von for schleife)
					}
				}
				// to check, that the data unit is already in msg.setStaleHashes.
				for(int v = 0; v < message->getNrOfStaleHashes(); v++){
					
				    dataUnit_t *du = (dataUnit_t *) message->getStaleDataHash(v);

					if(setDataUnits[k].getHashValue() == du->getHashValue()){

						element_is_in_msgSetSH = true;  // TODO wenn das Element gefunden wurde soll nicht meher suchen (raus springen von for schleife)
					}
				}

				// if the the data unit from setDataUnits is not already in msg.setHashVales, meg.setDataUnits or in msg.setStaleDataUnits available 
				if(!element_is_in_msgSetH && !element_is_in_msgSetDU && !element_is_in_msgSetSH){
					//-----------------------------------------------
					//setDUsForSend.push_back(&(setDataUnits.at(k)));
					if(!find_InSetDUsForSend(setDataUnits[k].getHashValue())){
						setDUsForSend_t.push_back(setDataUnits[k]);
					}
					//debug_->debug( "answer DU : %d \n", setDataUnits[k].getHashValue());
					nrOfHashesForAnswer++;
				}
			}

			if(nrOfHashesForAnswer > 0){

				expireIn( ANSWER_TIMER_NR, DELTA);
				//debug_->debug( "answer is finish \n");
			}

			// set the auxiliary variable of false again to be ready for the next message
			element_is_in_msgSetH = false;
			element_is_in_msgSetDU = false;
			element_is_in_msgSetSH = false;			

			// Request action
			//---------------------------------------------------------------------------
			//---------------------------------------------------------------------------
			for(int i = 0; i< message->getNrOfDataHashes(); i++){

				hash_Value_t *temp = (hash_Value_t *) message->getDataHash(i);

				if(!find_InSetDataUnits(*temp) && !find_InSetStaleDataUnits(*temp)){

					setReqHashes.push_back(*temp);
					nrOfHashesForRequst++;
				}
			}

			if(nrOfHashesForRequst > 0){

				expireIn( REQUEST_TIMER_NR, 3 * DELTA);
				//debug_->debug( "request is finish \n");
			}

			// Request cancel
			//---------------------------------------------------------------------------
			//---------------------------------------------------------------------------
			for(int i=0; i < setReqHashes.size(); i++){

				
				for(int j = 0; j< message->getNrOfDataUnits(); j++){
					
					dataUnit_t *du = (dataUnit_t *) message->getDataUnit(j);

					if(setReqHashes[i] == du->getHashValue()){

						element_is_in_msgSetDU_1 = true;
					}
				}
				if(element_is_in_msgSetDU_1){

					setReqHashes.erase(setReqHashes.begin()+i);
				}
			}
			// if the set of the request hashes is zero (no hash value more )
			if(setReqHashes.size() == 0){
				
				cancel(REQUEST_TIMER_NR);
				//debug_->debug( "cancel request \n");
			}

			// set the auxiliary variable of false again to be ready for the next message
			element_is_in_msgSetDU_1 = false;
		}	
   }

}
#endif
