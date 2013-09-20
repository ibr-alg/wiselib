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

#define ZB_SUCCESS 0x0
#undef SUCCESS

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

      typedef uint64_t node_id_t;
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
      enum SpecialNodeIds
      {
	 NULL_NODE_ID = 0,
         BROADCAST_ADDRESS = ZB_BROADCAST_ADDRESS       ///< Unknown/No node id
      };
      //---------------------------------------------------------------------
      enum Restrictions
      {
         MAX_MESSAGE_LENGTH = 100 ///< Maximal number of bytes in payload
      };

      enum
      {
         POLL_INTERVAL = 20,
	 BAUDRATE = 9600
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

      uint32_t getSH();
      uint32_t getSL();

      bool initialized_;
   };

   // -----------------------------------------------------------------------
   template<typename OsModel_P>
   ArduinoXBeeS2Radio<OsModel_P>::ArduinoXBeeS2Radio():initialized_(false),id_(0)
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
	if(!initialized_)
	{
	  xbee_.begin(BAUDRATE);
	  id_ = getSH();
	  debug.debug("MSB: %#lx",id_);
	  id_ = (id_<<32)|getSL();
	  debug.debug("LSB: %#lx",id_);
	  initialized_ = true;
	}
	timer_->template set_timer<ArduinoXBeeS2Radio<OsModel_P> , &ArduinoXBeeS2Radio<OsModel_P>::read_recv_packet > ( POLL_INTERVAL, this , ( void* )timer_ );
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
      return id_;
   }
   // -----------------------------------------------------------------------
   template<typename OsModel_P>
   uint32_t ArduinoXBeeS2Radio<OsModel_P>::getSH()
   {
	node_id_t temp_id = 0;
	size_t shCmd[] = {'S','H'};
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
	      int ValueLen = atResponse.getValueLength();
	      uint8_t* value = atResponse.getValue();
	      for(int i = 0;i < 4; i++)
	      {
		
		if(value[i] != 0)
		{
		  temp_id = (temp_id << 8)|(uint32_t)value[i];
		}
		else
		  temp_id = (temp_id << 8);
	      }
	      return temp_id;	      
	    }
	    else return 0;
	  }
	  else return 0;
	}
	else return 0;
   }
   //----------------------------------------------------------------------------------------------
   template<typename OsModel_P>
   uint32_t ArduinoXBeeS2Radio<OsModel_P>::getSL()
   {
	uint32_t temp_id = 0;
	size_t slCmd[] = {'S','L'};
	AtCommandRequest atRequest = AtCommandRequest(slCmd);
	AtCommandResponse atResponse = AtCommandResponse();

	xbee_.send(atRequest);
	if (xbee_.readPacket(2500))
	{
	  if (xbee_.getResponse().getApiId() == AT_COMMAND_RESPONSE)
	  {
	    xbee_.getResponse().getAtCommandResponse(atResponse);
	    if (atResponse.isOk())
	    {
	      int ValueLen = atResponse.getValueLength();
	      uint8_t* value = atResponse.getValue();
	      for(int i = 0;i < ValueLen; i++)
	      {
		if(value[i] != 0)
		  temp_id = (temp_id << 8)|(uint32_t)value[i];
		else
		  temp_id = (temp_id << 8);
	      }
	      return temp_id;
	    }
	    else return 0;
	  }
	  else return 0;
	}
	else return 0;
   }
   // -----------------------------------------------------------------------
   template<typename OsModel_P>
   int ArduinoXBeeS2Radio<OsModel_P>::
   send( node_id_t dest, size_t len, block_data_t* data )
   {
     TIMSK2 &= ~(1<<TOIE2); //disable timer when 
     TIMSK2 &= ~(1<<OCIE2A);//transmitting data
     uint32_t lsb = (uint32_t)dest;
     dest &= 0xFFFFFFFF00000000;
     uint32_t msb = (uint32_t)(dest>>32);
     XBeeAddress64 addr64 = XBeeAddress64(msb,lsb);
     
     ZBTxRequest zbTx = ZBTxRequest(addr64, data, len);
     ZBTxStatusResponse txStatus = ZBTxStatusResponse();

     debug.debug("Sending Xbee packet...");
     xbee_.send(zbTx);
     typename OsModel::Debug debug;
     if (xbee_.readPacket(500))
     {
       debug.debug("Response received");
       if (xbee_.getResponse().getApiId() == ZB_TX_STATUS_RESPONSE)
       {
	 debug.debug("Tx status response");
	 debug.debug("API id: %#x",(int)xbee_.getResponse().getApiId());
	 xbee_.getResponse().getZBTxStatusResponse(txStatus);
	 if (txStatus.isSuccess())
	 {
	   timer_->template set_timer<ArduinoXBeeS2Radio<OsModel_P> , &ArduinoXBeeS2Radio<OsModel_P>::read_recv_packet > ( POLL_INTERVAL, this , ( void* )timer_ );
	   return SUCCESS;
	 }
	 else
	   debug.debug("snd tx %d", (int)txStatus.getDeliveryStatus());
       }
       else if (xbee_.getResponse().isError())
	 debug.debug("snd err %d", (int)xbee_.getResponse().getErrorCode());
       else
	 debug.debug("API id: %#d",(int)xbee_.getResponse().getApiId());
     }
     else
     {
       timer_->template set_timer<ArduinoXBeeS2Radio<OsModel_P> , &ArduinoXBeeS2Radio<OsModel_P>::read_recv_packet > ( POLL_INTERVAL, this , ( void* )timer_ );
       return ERR_UNSPEC;
     }
     timer_->template set_timer<ArduinoXBeeS2Radio<OsModel_P> , &ArduinoXBeeS2Radio<OsModel_P>::read_recv_packet > ( POLL_INTERVAL, this , ( void* )timer_ );
     return SUCCESS;
   }
   // -----------------------------------------------------------------------
  template<typename OsModel_P>
  void ArduinoXBeeS2Radio<OsModel_P>::read_recv_packet(void*)
  {
    xbee_.readPacket();
    if(xbee_.getResponse().isAvailable())
    {
      debug.debug("Response available");
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
	  uint32_t msb = rx.getRemoteAddress64().getMsb();
	  uint32_t lsb = rx.getRemoteAddress64().getLsb();
	  from_id = (uint64_t)msb;
	  from_id = (from_id<<32);
	  from_id |=(uint64_t)lsb;
	  debug.debug("From SH: %#lx",msb);
	  debug.debug("From SL: %#lx",lsb);
	    
	  data = rx.getData();
	  length = rx.getDataLength();
	}
      }
      else
	debug.debug("snd APIid %d",(int)xbee_.getResponse().getApiId());
      received(data, length, from_id);
    }
    timer_->template set_timer<ArduinoXBeeS2Radio<OsModel_P> , &ArduinoXBeeS2Radio<OsModel_P>::read_recv_packet > ( POLL_INTERVAL, this , ( void* )timer_ );
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
