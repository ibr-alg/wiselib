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

#ifndef ARDUINO_BLUETOOTH_RADIO_H
#define ARDUINO_BLUETOOTH_RADIO_H

#if ARDUINO_USE_BLUETOOTH

#include "arduino_types.h"
#include <Arduino.h>
#include <SoftwareSerial.h>

#include "util/delegates/delegate.hpp"
#include "util/serialization/simple_types.h"
#include "util/pstl/vector_static.h"
#include "util/base_classes/radio_base.h"


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
   class ArduinoBluetoothRadio
   {
   public:
      typedef OsModel_P OsModel;

      typedef ArduinoBluetoothRadio<OsModel> self_type;
      typedef self_type* self_pointer_t;

      typedef uint32_t node_id_t;
      typedef uint8_t  block_data_t;
      typedef uint8_t  size_t;
      typedef uint8_t  message_id_t;

      typedef SoftwareSerial Bluetooth_t; 

      typedef delegate3<void, unsigned long, uint8_t, uint8_t*> arduino_radio_delegate_t;
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
         BROADCAST_ADDRESS      = 0       ///< Unknown/No node id
      };
      // --------------------------------------------------------------------
      enum Restrictions
      {
         MAX_MESSAGE_LENGTH = 32 ///< Maximal number of bytes in payload
      };

      ArduinoBluetoothRadio();
      ~ArduinoBluetoothRadio();

      int send( node_id_t id, size_t len, block_data_t* data );

      int enable_radio();
	
      int disable_radio();
	
      int connect_radio(size_t mode);
      node_id_t id();

      void get_slave_name(String name);

      template<class T, void ( T::*TMethod )( node_id_t, size_t, block_data_t* )>
      int reg_recv_callback( T* obj_pnt );

      int unreg_recv_callback( int idx );
      void received( unsigned char* data, size_t len, node_id_t from );

   private:
      node_id_t id_;
      unsigned long baud_rate_;       
      size_t rxd_;
      size_t txd_;
      String slave_name_;
      String slave_addr_;
      arduino_radio_delegate_t arduino_radio_callbacks_[MAX_INTERNAL_RECEIVERS];
   };

   // -----------------------------------------------------------------------
   // -----------------------------------------------------------------------
   // -----------------------------------------------------------------------
   template<typename OsModel_P>
   ArduinoBluetoothRadio<OsModel_P>::ArduinoBluetoothRadio()
   {
	//pinMode(RXD, INPUT);
  	//pinMode(TXD, OUTPUT);
   }
   // -----------------------------------------------------------------------
   template<typename OsModel_P>
   ArduinoBluetoothRadio<OsModel_P>::~ArduinoBluetoothRadio()
   {
   }
   // -----------------------------------------------------------------------
   template<typename OsModel_P>
   void ArduinoBluetoothRadio<OsModel_P>::get_slave_name(String name)
   {
	slave_name_=name;
   }
   // -----------------------------------------------------------------------
   template<typename OsModel_P>
   int ArduinoBluetoothRadio<OsModel_P>::enable_radio()
   {	
	rxd_ = RXD;
	txd_ = TXD;	
	baud_rate_ = DEFAULT_BAUD_RATE;
	Serial.begin(9600);
	return SUCCESS;
   }
   // -----------------------------------------------------------------------
   template<typename OsModel_P>
   int ArduinoBluetoothRadio<OsModel_P>::connect_radio(size_t mode = 0)
   {
	Bluetooth_t Bluetooth_(rxd_,txd_);
	Bluetooth_.begin(baud_rate_); //Set BluetoothBee BaudRate to default baud rate 38400
	if(mode = 1)
	{
		Bluetooth_.print("\r\n+STWMOD=1\r\n");//set the bluetooth work in master mode
	  	Bluetooth_.print("\r\n+STNA=BTMaster\r\n");//set the bluetooth name as "SeeedBTMaster"
	  	Bluetooth_.print("\r\n+STAUTO=0\r\n");// Auto-connection is forbidden here
	  	delay(2000); // This delay is required.
	  	Bluetooth_.flush();
	  	Bluetooth_.print("\r\n+INQ=1\r\n");//make the master inquire
	  	Serial.println("Master is inquiring!");
	  	delay(2000); // This delay is required.
	    
	  	//find the target slave
	  	char recv_char_;
		int name_index_ = 0;
		int addr_index_ = 0;
		String ret_symb_="+RTINQ=";	
		String recv_buf_;

	  	while(1)
		{
		    	if(Bluetooth_.available())
			{
			      	recv_char_ = Bluetooth_.read();
			      	recv_buf_ += recv_char_;
	      			name_index_ = recv_buf_.indexOf(slave_name_);//get the position of slave name
			      	//nameIndex -= 1;//decrease the ';' in front of the slave name, to get the position of the end of the slave address
	      			if ( name_index_ != -1 )
				{
					//Serial.print(recvBuf);
				 	addr_index_ = (recv_buf_.indexOf(ret_symb_,(name_index_ - ret_symb_.length()- 18) ) + ret_symb_.length());//get the start position of slave address	 
	 				slave_addr_ = recv_buf_.substring(addr_index_, name_index_);//get the string of slave address 			
	 				break;
	      			}
	    		}
	  	}
	
	  	String connect_cmd_ = "\r\n+CONN=";
		//form the full connection command
	  	connect_cmd_ += slave_addr_;
	  	connect_cmd_ += "\r\n";
	  	int connectOK = 0;
	  	Serial.print("Connecting to slave:");
	        char char_connect_cmd_[connect_cmd_.length()] ;
		connect_cmd_.toCharArray(char_connect_cmd_,connect_cmd_.length());
	  	do
		{
	    		Bluetooth_.print(char_connect_cmd_);//send connection command
	    		recv_buf_ = "";
	    		while(1)
			{
	      			if(Bluetooth_.available())
				{
					recv_char_ = Bluetooth_.read();
	 				recv_buf_ += recv_char_;
	 				if(recv_buf_.indexOf("CONNECT:OK") != -1)
					{
						connectOK = 1;
	 	  				Serial.println("Connected!");
	 	  				Bluetooth_.print("Connected!");
	 	  				break;
	 				}
					else if(recv_buf_.indexOf("CONNECT:FAIL") != -1)
					{
	 	  			Serial.println("Connect again!");
	 	  			break;
	 				}
	      			}
	    		}
	  	}while(0 == connectOK);
	  	
		return SUCCESS;
	}
	else if(mode = 0)
	{
 	 	Bluetooth_.print("\r\n+STWMOD=0\r\n"); //set the bluetooth work in slave mode
  		Bluetooth_.print("\r\n+STNA=BTSlave\r\n"); //set the bluetooth name as "SeeedBTSlave"
  		Bluetooth_.print("\r\n+STOAUT=1\r\n"); // Permit Paired device to connect me
  		Bluetooth_.print("\r\n+STAUTO=0\r\n"); // Auto-connection should be forbidden here
  		delay(2000); // This delay is required.
  		Bluetooth_.print("\r\n+INQ=1\r\n"); //make the slave bluetooth inquirable 
  		Serial.println("The slave bluetooth is inquirable!");
  		delay(2000); // This delay is required.
  		Bluetooth_.flush();
		return SUCCESS;
	}
	else
	{
		Serial.println("The mode does not exist");
		return ERR_UNSPEC;
	}
   }
   // -----------------------------------------------------------------------
   template<typename OsModel_P>
   int ArduinoBluetoothRadio<OsModel_P>::disable_radio()
   {
      return SUCCESS;
   }
   // -----------------------------------------------------------------------
   template<typename OsModel_P>
   typename ArduinoBluetoothRadio<OsModel_P>::node_id_t ArduinoBluetoothRadio<OsModel_P>::id()
   {
      return id_;
   }
   // -----------------------------------------------------------------------
   template<typename OsModel_P>
   int ArduinoBluetoothRadio<OsModel_P>::
   send( node_id_t dest, size_t len, block_data_t* data )
   {
      Bluetooth_t Bluetooth_(RXD,TXD);
      for (unsigned int i = 0; i < len; i++)
      	Bluetooth_.write(data[i]);

      return SUCCESS;
   }
   // -----------------------------------------------------------------------
   template<typename OsModel_P>
   template < class T,
            void ( T::*TMethod )( typename ArduinoBluetoothRadio<OsModel_P>::node_id_t,
                                  typename ArduinoBluetoothRadio<OsModel_P>::size_t,
                                  typename ArduinoBluetoothRadio<OsModel_P>::block_data_t* ) >
   int
   ArduinoBluetoothRadio<OsModel_P>::
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
   int ArduinoBluetoothRadio<OsModel_P>::
   unreg_recv_callback( int idx )
   {
      arduino_radio_callbacks_[idx] = arduino_radio_delegate_t();
      return SUCCESS;
   }
   // -----------------------------------------------------------------------
   template<typename OsModel_P>
   void ArduinoBluetoothRadio<OsModel_P>::
   received( unsigned char* data, size_t len, unsigned long from )
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


#endif // ARDUINO_USE_BLUETOOTH

#endif // ARDUINO_BLUETOOTH_RADIO_H
