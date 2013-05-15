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

#include "util/delegates/delegate.hpp"
#include "util/serialization/simple_types.h"
#include "util/pstl/vector_static.h"


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
      // --------------------------------------------------------------------
      enum BaudRates
      {
	 DEFAULT_BAUD_RATE = 9600
      };

      ArduinoBluetoothRadio();
      ~ArduinoBluetoothRadio();

      int send( node_id_t id, size_t len, block_data_t* data );

      int enable_radio();

      int disable_radio();

      node_id_t id();

      template<class T, void ( T::*TMethod )( node_id_t, size_t, block_data_t* )>
      int reg_recv_callback( T* obj_pnt );

      int unreg_recv_callback( int idx );
      void received( unsigned char* data, size_t len, node_id_t from );

   private:
      // the node is identified by the ipv4 address
      node_id_t id_;

      unsigned long baud_rate_;

      arduino_radio_delegate_t arduino_radio_callbacks_[MAX_INTERNAL_RECEIVERS];
   };
   // -----------------------------------------------------------------------
   // -----------------------------------------------------------------------
   // -----------------------------------------------------------------------
   template<typename OsModel_P>
   ArduinoBluetoothRadio<OsModel_P>::ArduinoBluetoothRadio()
   {
	baud_rate_ = DEFAULT_BAUD_RATE;
   }
   // -----------------------------------------------------------------------
   template<typename OsModel_P>
   ArduinoBluetoothRadio<OsModel_P>::~ArduinoBluetoothRadio()
   {
   }
   // -----------------------------------------------------------------------
   template<typename OsModel_P>
   int ArduinoBluetoothRadio<OsModel_P>::enable_radio()
   {
      Serial.begin( baud_rate_);
      return SUCCESS;
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
      	Serial.write( data[i]);

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
