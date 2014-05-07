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
// #include "util/base_classes/radio_base.h"

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
      typedef unsigned char  block_data_t;
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

      enum
      {
	POLL_INTERVAL = 30,
	BAUDRATE = 9600
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

      void read_recv_packet(void*);

  private:
      //typename OsModel::Debug debug;
      typename OsModel::Timer* timer_;

      bool initialized_;

      bool first_flush;
      node_id_t id_;
      size_t rxd_;
      size_t txd_;
      String slave_name_;
      String slave_addr_;
      arduino_radio_delegate_t arduino_radio_callbacks_[MAX_INTERNAL_RECEIVERS];

//       void checkOK();
  };

  // -----------------------------------------------------------------------

  SoftwareSerial Bluetooth_(RXD,TXD);
  // -----------------------------------------------------------------------
  // -----------------------------------------------------------------------
  template<typename OsModel_P>
  ArduinoBluetoothRadio<OsModel_P>::ArduinoBluetoothRadio():initialized_(false),first_flush(true)
  {
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
	return SUCCESS;
  }/*
  // -----------------------------------------------------------------------
  template<typename OsModel_P>
  void ArduinoBluetoothRadio<OsModel_P>::checkOK()
    {
      while(Bluetooth_.available()) Bluetooth_.read();
	//ArduinoDebug<ArduinoOsModel>(true).debug("%c",Bluetooth_.read());
    }*/
  // -----------------------------------------------------------------------
  template<typename OsModel_P>
  int ArduinoBluetoothRadio<OsModel_P>::connect_radio(size_t mode = 0)
  {
    if(!initialized_)
    {
      id_ = mode;
      Bluetooth_.begin(BAUDRATE); //Set BluetoothBee BaudRate to default baud rate 38400
      if(mode == 1)
      {
	//------------initializing Master------------//
	/*Bluetooth_.print("\r\n+STWMOD=1\r\n");//set the bluetooth work in master mode
	 checkOK();
	 delay(1000);
	 Bluetooth_.print("\r\n+STNA=BTMaster\r\n");//set name as BTMaster
	 checkOK();
	 delay(1000);
	 Bluetooth_.print("\r\n+STPIN=0000\r\n");  //set pin
	 checkOK();
	 delay(1000);
	 Bluetooth_.print("\r\n+STAUTO=0\r\n");// Auto-connection is forbidden here
	 checkOK();
	 delay(1000);
	 Bluetooth_.print("\r\n+STOAUT=1\r\n");     // permit paired devices to connect
	 checkOK();
	 delay(2000);*/
	Bluetooth_.print("\r\n+INQ=1\r\n");//make the master inquire
	delay(2000);

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
	    if ( name_index_ != -1 )
	    {
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
	char char_connect_cmd_[connect_cmd_.length()] ;
	connect_cmd_.toCharArray(char_connect_cmd_,connect_cmd_.length()+1);
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
		break;
	      }
	      else if(recv_buf_.indexOf("CONNECT:FAIL") != -1)
	      {
		//ArduinoDebug<ArduinoOsModel>(true).debug("Connect again!");
		return ERR_UNSPEC;
	      }
	    }
	  }
	}while(0 == connectOK);
	initialized_ = true;
	delay(500);
      }
      else if(mode == 0)
      {
	//ArduinoDebug<ArduinoOsModel>(true).debug("Slave");
	//-----------initializing Slave--------------//
	/*Bluetooth_.print("\r\n+STWMOD=0\r\n"); //set the bluetooth work in slave mode
	checkOK();
	delay(1000);
	Bluetooth_.print("\r\n+STNA=BTSlave\r\n"); //set the bluetooth name as "SeeedBTSlave"
	checkOK();
	delay(1000);
	Bluetooth_.print("\r\n +STPIN=0000\r\n");  // existing default
	checkOK();
	delay(1000);
	Bluetooth_.print("\r\n+STOAUT=1\r\n"); // Permit Paired device to connect me
	checkOK();
	delay(1000);
	Bluetooth_.print("\r\n+STAUTO=0\r\n"); // Auto-connection should be forbidden here
	checkOK();*/
	delay(2000); // This delay is required.
	Bluetooth_.print("\r\n+INQ=1\r\n"); //make the slave bluetooth inquirable
	delay(2000); // This delay is required.
	String recv_buf_ = "";
	while(1)
	{
	  char recv_char_;
	  if(Bluetooth_.available())
	  {
	    recv_char_ = Bluetooth_.read();
	    recv_buf_+= recv_char_;
	    if(recv_buf_.indexOf("+BTSTATE:4") != -1)
	    {
	      //ArduinoDebug<ArduinoOsModel>(true).debug("Connected!");
	      break;
	    }
	    else if(recv_buf_.indexOf("CONNECT:FAIL") != -1)
	    {
	      //ArduinoDebug<ArduinoOsModel>(true).debug("Connect again!");
	      return ERR_UNSPEC;
	    }
	  }
	}
	initialized_ = true;
      }
      else
      {
	//ArduinoDebug<ArduinoOsModel>(true).debug("The mode does not exist");
	return ERR_UNSPEC;
      }
    }
    if(initialized_)
    {
      Bluetooth_.flush();
      //if (mode == 0)
      //{
	timer_->template set_timer<ArduinoBluetoothRadio<OsModel_P> , &ArduinoBluetoothRadio<OsModel_P>::read_recv_packet > ( POLL_INTERVAL, this , ( void* )timer_ );
      //}
      return SUCCESS;
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
    for (unsigned int i = 0; i < len; i++)
    {
	Bluetooth_.write((char)data[i]);
    }
    return SUCCESS;
  }
  // -----------------------------------------------------------------------
  template<typename OsModel_P>
  void ArduinoBluetoothRadio<OsModel_P>::read_recv_packet(void*)
  {
    size_t len = 0;
    int i = 0;
    block_data_t data[64];
    while(true)
    {
      if(Bluetooth_.available())
      {
	unsigned long from;
	if(Bluetooth_.peek()=='\0')
	{
	  Bluetooth_.read();
	  break;
	}
	data[i] = Bluetooth_.read();
// 	ArduinoDebug<ArduinoOsModel>(true).debug("ch: %c",data[i]);
	i++;
	len=i;
      }
      else
      {
	break;
      }
    }
    if(i>0)
    {
      data[i]='\0';
      received(data,len,!id_);
//       ArduinoDebug<ArduinoOsModel>(true).debug("%s",data);
    }
    timer_->template set_timer<ArduinoBluetoothRadio<OsModel_P> , &ArduinoBluetoothRadio<OsModel_P>::read_recv_packet > ( POLL_INTERVAL, this , ( void* )timer_ );
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
