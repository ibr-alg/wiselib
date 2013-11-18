/***************************************************************************
 ** This file is part of the generic algorithm library Wiselib.           **
 ** Copyright (C) 2012 by the Wisebed (www.wisebed.eu) project.           **
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

#ifndef ARDUINO_XBEES2_RADIO_H
#define ARDUINO_XBEES2_RADIO_H

#if ARDUINO_USE_XBEES2

#include "arduino_types.h"
#include <Arduino.h>
#include <XBee.h>
#include <SoftwareSerial.h>

#include "util/delegates/delegate.hpp"
#include "util/serialization/simple_types.h"
#include "util/pstl/vector_static.h"
#include "arduino_timer.h"
#include "arduino_os.h"

namespace wiselib
{
   /** \brief Arduino Implementation of \ref radio_concept "Radio concept".
    *  \ingroup radio_concept
    *  \ingroup arduino_facets
    *
    * Arduino implementation of the \ref radio_concept "Radio concept" ...
    *
    * @tparam OsModel_P Has to implement @ref os_concept "Os concept".
    */

   template<typename OsModel_P>
   class ArduinoXBeeS2Radio
   {
   public:
      typedef OsModel_P OsModel;

      typedef ArduinoXBeeS2Radio<OsModel> self_type;
      typedef self_type* self_pointer_t;

      typedef uint32_t* node_id_t;
      typedef uint8_t  block_data_t;
      typedef uint8_t  size_t;
      typedef uint8_t  message_id_t;

      typedef delegate3<void, node_id_t, uint8_t, uint8_t*> arduino_radio_delegate_t;
      typedef arduino_radio_delegate_t radio_delegate_t;
      // --------------------------------------------------------------------
      enum ErrorCodes
      {
         SUCCESS = OsModel::SUCCESS,
         ERR_UNSPEC = OsModel::ERR_UNSPEC
      };
      // --------------------------------------------------------------------
      enum { MAX_INTERNAL_RECEIVERS = 5 };
      // --------------------------------------------------------------------
      //enum SpecialNodeIds
      //{
      //   BROADCAST_ADDRESS = 0xFFFF       ///< Unknown/No node id
      //};
      // --------------------------------------------------------------------
      enum Restrictions
      {
         MAX_MESSAGE_LENGTH = 100 ///< Maximal number of bytes in payload
      };

      ArduinoXBeeS2Radio();
      ~ArduinoXBeeS2Radio();

      int send( node_id_t id, size_t len, block_data_t* data );

      int enable_radio();

      int disable_radio();

      node_id_t id();

      template<class T, void ( T::*TMethod )( node_id_t, size_t, block_data_t* )>
      int reg_recv_callback( T* obj_pnt );

      int unreg_recv_callback( int idx );
      void received( unsigned char* data, size_t len, node_id_t from );

      void read_recv_packet(void*);

   private:
      typename OsModel::Debug debug;
      node_id_t id_;
      typename OsModel::Timer* timer_;
      unsigned long baud_rate_;
      arduino_radio_delegate_t arduino_radio_callbacks_[MAX_INTERNAL_RECEIVERS];
      ::XBee xbee_;      
   };

   // -----------------------------------------------------------------------
   template<typename OsModel_P>
   ArduinoXBeeS2Radio<OsModel_P>::ArduinoXBeeS2Radio()
   {
   }
   // -----------------------------------------------------------------------
   template<typename OsModel_P>
   ArduinoXBeeS2Radio<OsModel_P>::~ArduinoXBeeS2Radio()
   {
   }
   // -----------------------------------------------------------------------
   template<typename OsModel_P>
   int ArduinoXBeeS2Radio<OsModel_P>::enable_radio()
   {
	baud_rate_ = 9600;
	xbee_.begin(baud_rate_);
	id_ = id();
	timer_->template set_timer<ArduinoXBeeS2Radio<OsModel_P> , &ArduinoXBeeS2Radio<OsModel_P>::read_recv_packet > ( 1000, this , ( void* )timer_ );
	if(id_[0] == -1 || id_[1] == -1)
	  return ERR_UNSPEC;
	return SUCCESS;
   }
   // -----------------------------------------------------------------------
   template<typename OsModel_P>
   int ArduinoXBeeS2Radio<OsModel_P>::disable_radio()
   {
      return SUCCESS;
   }
   // -----------------------------------------------------------------------
   template<typename OsModel_P>
   typename ArduinoXBeeS2Radio<OsModel_P>::node_id_t ArduinoXBeeS2Radio<OsModel_P>::id()
   {
      node_id_t temp_id;
      size_t shCmd[] = {'S','H'};
      size_t slCmd[] = {'S','L'};
      AtCommandRequest atRequest = AtCommandRequest(shCmd);
      AtCommandResponse atResponse = AtCommandResponse();

      xbee_.send(atRequest);
      if (xbee_.readPacket(2500))
      {
	if (xbee_.getResponse().getApiId() == AT_COMMAND_RESPONSE)
	{
	  xbee_.getResponse().getAtCommandResponse(atResponse);
	  if (atResponse.isOk())
	  {
	    size_t MY_hb = atResponse.getValue()[0];
	    size_t MY_lb = atResponse.getValue()[1];

	    temp_id[0] = (MY_hb<<8)|(MY_lb);
	  }
	  else
	    temp_id[0] = -1;
	}
	else
	  temp_id[0] = -1;
      }
      else
	temp_id[0] = -1;
      
      atRequest.setCommand(slCmd);

      xbee_.send(atRequest);
      if (xbee_.readPacket(2500))
      {
	if (xbee_.getResponse().getApiId() == AT_COMMAND_RESPONSE)
	{
	  xbee_.getResponse().getAtCommandResponse(atResponse);
	  if (atResponse.isOk())
	  {
	    size_t MY_hb = atResponse.getValue()[0];
	    size_t MY_lb = atResponse.getValue()[1];

	    temp_id[1] = (MY_hb<<8)|(MY_lb);
	  }
	  else
	    temp_id[1] = -1;
	}
	else
	  temp_id[1] = -1;
      }
      else
	temp_id[1] = -1;
      return temp_id;
   }
   // -----------------------------------------------------------------------
   template<typename OsModel_P>
   int ArduinoXBeeS2Radio<OsModel_P>::
   send( node_id_t dest, size_t len, block_data_t* data )
   {
     XBeeAddress64 addr64;	//= XBeeAddress64(dest[0],dest[1]);
     addr64.setMsb(dest[0]);
     addr64.setLsb(dest[1]);
     //XBeeAddress64 addr64 = XBeeAddress64(0x0013a200,0x40a11e78);
     ZBTxRequest zbTx = ZBTxRequest(addr64, data, len);
     ZBTxStatusResponse txStatus = ZBTxStatusResponse();

     debug.debug("Sending Xbee packet...");
     xbee_.send(zbTx);
     typename OsModel::Debug debug;
     if (xbee_.readPacket(500))
     {
       debug.debug("Response received");
       if (xbee_.getResponse().getApiId() == TX_STATUS_RESPONSE)
       {
	 debug.debug("Tx status response");
	 xbee_.getResponse().getZBTxStatusResponse(txStatus);
	 if (txStatus.getDeliveryStatus() == SUCCESS)
	   return SUCCESS;
	 else
	   return ERR_UNSPEC;
       }
       else if (xbee_.getResponse().isError())
	 return ERR_UNSPEC;
     }
     else
	return ERR_UNSPEC;
     
     return SUCCESS;
   }
   // -----------------------------------------------------------------------
  template<typename OsModel_P>
  void ArduinoXBeeS2Radio<OsModel_P>::read_recv_packet(void*)
  {
    xbee_.readPacket();
    if(xbee_.getResponse().isAvailable())
    {
      ZBRxResponse rx = ZBRxResponse();
      ModemStatusResponse msr = ModemStatusResponse();
      node_id_t from_id;
      size_t length;
      block_data_t* data;

      if (xbee_.getResponse().getApiId() == ZB_RX_RESPONSE)
      {
	debug.debug("Rx response");
	xbee_.getResponse().getZBRxResponse(rx);
	if (rx.getOption() == ZB_PACKET_ACKNOWLEDGED)
	{
	  from_id[0] = rx.getRemoteAddress64().getMsb();
	  from_id[1] = rx.getRemoteAddress64().getLsb();
	  data = rx.getData();
	  length = rx.getDataLength();
	}
      }
      received(data, length, from_id);
    }
    timer_->template set_timer<ArduinoXBeeS2Radio<OsModel_P> , &ArduinoXBeeS2Radio<OsModel_P>::read_recv_packet > ( 1000, this , ( void* )timer_ );
  }
   // -----------------------------------------------------------------------
   template<typename OsModel_P>
   template < class T,
            void ( T::*TMethod )( typename ArduinoXBeeS2Radio<OsModel_P>::node_id_t,
                                  typename ArduinoXBeeS2Radio<OsModel_P>::size_t,
                                  typename ArduinoXBeeS2Radio<OsModel_P>::block_data_t* ) >
   int
   ArduinoXBeeS2Radio<OsModel_P>::
   reg_recv_callback( T* obj_pnt )
   {

      for ( int i = 0; i < MAX_INTERNAL_RECEIVERS; i++ )
      {
         if ( !arduino_radio_callbacks_[i] )
         {
            arduino_radio_callbacks_[i] = arduino_radio_delegate_t::template from_method<T, TMethod>( obj_pnt );
            return SUCCESS;
         }
      }
      return ERR_UNSPEC;
    }
   // -----------------------------------------------------------------------
   template<typename OsModel_P>
   int ArduinoXBeeS2Radio<OsModel_P>::
   unreg_recv_callback( int idx )
   {
      arduino_radio_callbacks_[idx] = arduino_radio_delegate_t();
      return SUCCESS;
   }
   // -----------------------------------------------------------------------
   template<typename OsModel_P>
   void ArduinoXBeeS2Radio<OsModel_P>::
   received( unsigned char* data, size_t len, node_id_t from )
   {
      for ( unsigned int i = 0; i < MAX_INTERNAL_RECEIVERS; ++i  )
      {
         if ( arduino_radio_callbacks_[i] )
         {
            arduino_radio_callbacks_[i]( from, len, data );
         }
      }
   }
}


#endif // ARDUINO_USE_XBEE

#endif // ARDUINO_XBEE_RADIO_H
