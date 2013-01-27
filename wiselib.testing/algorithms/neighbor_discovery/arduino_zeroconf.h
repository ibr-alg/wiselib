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

#ifndef ARDUINO_ZEROCONF_H
#define ARDUINO_ZEROCONF_H

#include "EthernetBonjour.h"
//wiselib includes
#include "external_interface/arduino/arduino_types.h"
#include "util/delegates/delegate.hpp"
#include "util/pstl/vector_static.h"
#include "util/pstl/pair.h"
#include "pgb_payloads_ids.h"

#include "echomsg.h"

#include <string.h>

#define MAX_PG_PAYLOAD 255
#define ECHO_MAX_NODES 5

namespace wiselib
{
   template < typename OsModel_P, typename Radio_P, typename Debug_P >
   class WrapperEchoArduino;
   /**
    * \brief ArduinoZeroconf
    *
    *  \ingroup neighbourhood_discovery_concept
    *  \ingroup basic_algorithm_concept
    *  \ingroup neighbourhood_discovery_algorithm
    *
    */
   // -----------------------------------------------------------------------
   // -----------------------------------------------------------------------
   // -----------------------------------------------------------------------
   template < typename OsModel_P, typename Radio_P, typename Debug_P >
   class ArduinoZeroconf
   {
   public:
      typedef ArduinoZeroconf<OsModel_P, Radio_P, Debug_P> self_type;
      typedef self_type* self_pointer_t;

      // Type definitions
      typedef OsModel_P OsModel;

      typedef Radio_P Radio;
      //       typedef Timer_P Timer;
      typedef Debug_P Debug;
      typedef typename OsModel_P::Clock Clock;

      typedef typename Radio::node_id_t node_id_t;
      typedef typename Radio::size_t size_t;
      typedef typename Radio::block_data_t block_data_t;
      typedef typename Radio::message_id_t message_id_t;
      typedef typename Clock::time_t time_t;

      typedef EchoMsg<OsModel, Radio> ArduinoZeroconfMsg_t;
      typedef ArduinoZeroconf<OsModel_P, Radio_P, Debug_P> self_t;

      typedef delegate4<void, uint8_t, node_id_t, uint8_t, uint8_t*>
      event_notifier_delegate_t;
      // --------------------------------------------------------------------
      struct neighbor_entry
      {
         node_id_t id;
      };
      // --------------------------------------------------------------------
      struct reg_alg_entry
      {
         uint8_t alg_id;
         uint8_t data[Radio::MAX_MESSAGE_LENGTH];
         uint8_t size;
         event_notifier_delegate_t event_notifier_callback;
         uint8_t events_flag;
      };
      // --------------------------------------------------------------------
      typedef struct reg_alg_entry reg_alg_entry_t;
      typedef wiselib::vector_static<OsModel, reg_alg_entry_t, TOTAL_REG_ALG>
      reg_alg_vector_t;
      typedef typename reg_alg_vector_t::iterator reg_alg_iterator_t;

      /**
       * Actual Vector containing callbacks for all the register applications.
       */
      reg_alg_vector_t registered_apps;

      /**
       * Type of the structure that hold the relevant neighbor data.
       */
      typedef struct neighbor_entry neighbor_entry_t;

      /**
       * Type of the Vector Containing information for all nodes in the Neighborhood.
       */
      typedef wiselib::vector_static<OsModel, neighbor_entry_t, ECHO_MAX_NODES>
      node_info_vector_t;
      typedef typename node_info_vector_t::iterator iterator_t;

      /**
       * Actual Vector containing the nodes in the neighborhood
       */
      node_info_vector_t neighborhood;

      typedef node_info_vector_t Neighbors;
      // --------------------------------------------------------------------

      enum error_codes
      {
         SUCCESS = OsModel::SUCCESS, /*!< The method return with no errors */
         RGD_NUM_INUSE = 1, /*!< This app number is already registered */
         RGD_LIST_FULL = 2, /*!< The list with the registered apps is full*/
         INV_ALG_ID = 3,    /*!< The alg id is invalid*/
         AVAHI_THREAD_FAIL = 4 /*!< The avahi thread failed publishing the
	                           service or creating browser*/
      };

      enum event_codes
      {
         NEW_NB = 1, /*!< Event code for a newly added stable neighbor */
         DROPPED_NB = 2 /*!< Event code for a neighbor removed from nb list */
      };

      /**
       * Constructor.
       *
       */
      ArduinoZeroconf();

      /**
       * Destructor.
       *
       */
      ~ArduinoZeroconf();

      /**
       * Enable the Echo system enable radio
       * and register receive callback
       * initialize vectors
       * */
      void enable();

      // --------------------------------------------------------------------

      /**
       * \brief Disable the Echo protocol.
       *
       * Disables the Echo protocol. The
       * module will unregister the receive
       * callback, and will stop send beacons.
       * The timer will keep triggering every
       * beacon_period Millis.
       * All the existing neighbors will timeout
       * at most after timeout_period and the
       * NB_DROPPED events will be generated.
       * */
      void disable();

      /**
       * \brief Initialize vectors and variables
       * for Neighborhood Discovery.
       *
       * It will initialize all the the relevant
       * structures and variables of the module:
       * clear neighborhood vector
       * set the node stability to zero.
       * */
      void init_echo()
      {
         neighborhood.clear();
      }
      ;

      /**
       * \brief Initialize the module.
       */
      void init( Radio& radio, Clock& clock, Debug& debug )
      {
         radio_ = &radio;
         clock_ = &clock;
         //          timer_ = &timer;
         debug_ = &debug;
      };
      // --------------------------------------------------------------------
      //Neighbors&
      void topology();
      // --------------------------------------------------------------------
      template<class T, void( T::*TMethod )( uint8_t, node_id_t, uint8_t, uint8_t* )>
      uint8_t reg_event_callback( uint8_t alg_id, uint8_t events_flag, T* obj_pnt )
      {

         for ( reg_alg_iterator_t it = registered_apps.begin(); it
               != registered_apps.end(); it++ )
         {
            if ( it->alg_id == alg_id )
            {
               it->event_notifier_callback
               = event_notifier_delegate_t::template from_method < T,
               TMethod > ( obj_pnt );
               it->events_flag = events_flag;
               return 0;
            }
         }

         reg_alg_entry_t entry;
         entry.alg_id = alg_id;
         entry.size = 0;
         entry.event_notifier_callback
         = event_notifier_delegate_t::template from_method<T, TMethod>(
            obj_pnt );
         entry.events_flag = events_flag;
         registered_apps.push_back( entry );

         return 0;
      }
      // --------------------------------------------------------------------
      void unreg_event_callback( uint8_t alg_id )
      {
         for ( reg_alg_iterator_t it = registered_apps.begin(); it
               != registered_apps.end(); it++ )
         {
            if ( it->alg_id == alg_id )
            {
               it->event_notifier_callback = event_notifier_delegate_t();
               return;
            }
         }
      }
      // --------------------------------------------------------------------
      Radio& radio()
      {
         return *radio_;
      }
      // --------------------------------------------------------------------
      Clock& clock()
      {
         return *clock_;
      }
      // --------------------------------------------------------------------
      //       Timer& timer()
      //       {
      //          return *timer_;
      //       }
      // --------------------------------------------------------------------
      Debug& debug()
      {
         return *debug_;
      }

   private:

      // --------------------------------------------------------------------
      void notify_listeners( uint8_t event, node_id_t from, uint8_t len,
                             uint8_t* data )
      {

         for ( reg_alg_iterator_t ait = registered_apps.begin(); ait
               != registered_apps.end(); ++ait )
         {

            if ( ( ait->event_notifier_callback != 0 ) && ( ( ait->events_flag
                  & ( uint8_t ) event ) == ( uint8_t ) event ) )
            {

               ait->event_notifier_callback( event, from, len, data );

            }
         }
      }
      // --------------------------------------------------------------------
      void send_resolver ( node_id_t node_id )
      {
         bool found = false;
         iterator_t it;

         neighbor_entry_t entry;

         entry.id = node_id;

         for ( it = neighborhood.begin();
               it != neighborhood.end(); ++it )
         {
            if ( it->id == node_id )
            {
               found = true;
               break;
            }

         }

         if ( found )
         {
            neighborhood.erase( it );
            notify_listeners( DROPPED_NB, node_id, 0, 0 );
         }
         else
         {
            neighborhood.push_back( entry );
            notify_listeners( NEW_NB, node_id, 0, 0 );
         }
      }
      friend class WrapperEchoArduino<OsModel_P, Radio_P, Debug_P>;
      WrapperEchoArduino< OsModel_P, Radio_P, Debug_P > *zeroconf_;

      /**
       * Callback for receive function
       */
      int recv_callback_id_;

      char* service_;

      Radio* radio_;
      Clock* clock_;
      //       Timer* timer_;
      Debug* debug_;
   };
   // -----------------------------------------------------------------------
   // -----------------------------------------------------------------------
   // -----------------------------------------------------------------------
   class BaseWrapperEchoArduino
   {
   public:
      virtual void send_resolver( unsigned long ) = 0;

      virtual ~BaseWrapperEchoArduino() {}
   };
   // -----------------------------------------------------------------------
   // -----------------------------------------------------------------------
   // -----------------------------------------------------------------------
   template <typename OsModel_P, typename Radio_P, typename Debug_P>
   class WrapperEchoArduino : public BaseWrapperEchoArduino
   {
   protected:
      ArduinoZeroconf<OsModel_P, Radio_P, Debug_P> *zeroconf_;
   public:
      WrapperEchoArduino () {}
      WrapperEchoArduino ( ArduinoZeroconf<OsModel_P, Radio_P, Debug_P> *zeroconf ) : zeroconf_( zeroconf ) {}

      void send_resolver( unsigned long node_id )
      {
         zeroconf_->send_resolver( node_id );
      }
   };
   // -----------------------------------------------------------------------
   BaseWrapperEchoArduino* resolved_ = NULL;

   void service_callback ( const char* type, MDNSServiceProtocol proto,
                           const char* name, const byte ip[4], unsigned short port,
                           const char* txt )
   {
      // does not work
      if ( name != NULL )
      {
         unsigned long node_id = 0;
         node_id |= ip[0] << 24;
         node_id |= ip[1] << 16;
         node_id |= ip[2] << 8;
         node_id |= ip[3];

         resolved_->send_resolver( node_id );
      }
   }
   // -----------------------------------------------------------------------
   // -----------------------------------------------------------------------
   // -----------------------------------------------------------------------
   template <typename OsModel_P, typename Radio_P, typename Debug_P >
   ArduinoZeroconf<OsModel_P, Radio_P, Debug_P>::ArduinoZeroconf()
   {
      zeroconf_ = new WrapperEchoArduino< OsModel_P, Radio_P, Debug_P > ( this );
      resolved_ = zeroconf_;
   }
   // -----------------------------------------------------------------------
   template <typename OsModel_P, typename Radio_P, typename Debug_P >
   ArduinoZeroconf<OsModel_P, Radio_P, Debug_P>::~ArduinoZeroconf()
   {
      delete zeroconf_;
   }
   // -----------------------------------------------------------------------
   template <typename OsModel_P, typename Radio_P, typename Debug_P >
   //   typename ArduinoZeroconf<OsModel_P, Radio_P, Debug_P>::Neighbors&
   void ArduinoZeroconf<OsModel_P, Radio_P, Debug_P>::topology()
   {
      if ( !EthernetBonjour.isDiscoveringService() )
      {
         EthernetBonjour.startDiscoveringService( ZEROCONF_DISCOVER_SERVICE,
               MDNSServiceUDP,
               5000 );
      }

      EthernetBonjour.run();
      //  return NULL;
   }
   // -----------------------------------------------------------------------
   template <typename OsModel_P, typename Radio_P, typename Debug_P >
   void ArduinoZeroconf<OsModel_P, Radio_P, Debug_P>::enable()
   {

      /**
       * Enable normal radio and register the receive callback.
       */
      //radio().enable_radio();

      EthernetBonjour.begin( HOST_NAME );

      EthernetBonjour.setServiceFoundCallback( service_callback );

      EthernetBonjour.addServiceRecord( ZEROCONF_PUBLISH_SERVICE, PORT, MDNSServiceUDP );

      /**
       * Initialize vectors and variables.
       */
      init_echo();


   }
   // -----------------------------------------------------------------------
   template <typename OsModel_P, typename Radio_P, typename Debug_P >
   void ArduinoZeroconf<OsModel_P, Radio_P, Debug_P>::disable()
   {
      //     radio().disable_radio();

   }

}

#endif	/* ARDUINO_ZEROCONF_H */
