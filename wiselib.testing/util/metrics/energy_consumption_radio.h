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
#ifndef __UTIL_METRICS_ENERGY_CONSUMPTION_RADIO_H
#define __UTIL_METRICS_ENERGY_CONSUMPTION_RADIO_H

#include "util/delegates/delegate.hpp"
#include <util/pstl/vector_static.h>
#include <util/pstl/pair.h>

//TODO: REMOVE!!!!!!!
#include "sys/processor.h"
#include "sys/event_scheduler.h"
#include "sys/misc/random/random_variable.h"
#include "sys/misc/random/random_variable_keeper.h"
#include "sys/node.h"
#include "sys/simulation/simulation_controller.h"
#include "sys/simulation/simulation_environment.h"
#include "sys/taggings/basic_tags.h"
#include "sys/tag.h"

namespace wiselib {

   template<typename OsModel_P,
            typename Radio_P = typename OsModel_P::Radio>
   class EnergyConsumptionRadioCallback
   {
   public:
      typedef OsModel_P OsModel;
      typedef typename OsModel::Os Os;
      typedef Radio_P Radio;

      typedef EnergyConsumptionRadioCallback <OsModel, Radio> self_type;

      typedef typename Radio::node_id_t node_id_t;
      typedef typename Radio::size_t size_t;
      typedef typename Radio::block_data_t block_data_t;
      typedef typename Radio::radio_delegate_t energy_consumption_delegate_t;
      // -----------------------------------------------------------------
      EnergyConsumptionRadioCallback( Os *os, energy_consumption_delegate_t del )
         : os_  ( os ),
            del_( del )
      {}
      // -----------------------------------------------------------------
      ~EnergyConsumptionRadioCallback()
      {}
      // -----------------------------------------------------------------
      void receive( node_id_t id, size_t len, block_data_t* data )
      {
         double e = os_->proc->owner().template read_simple_tag<double>( "energy" );
         double rx = ((((15. + len) * 8.) / 250.) * 44.) / (3600. * 1000.);
         e += rx;
         os_->proc->owner_w().template write_simple_tag<double>( "energy", e );

         double consumed = os_->proc->owner().template read_simple_tag<double>( "consumed-rx" );
         consumed += e;

         bool app_active = os_->proc->owner().template read_simple_tag<bool>( "app_active" );
         if ( app_active )
            os_->proc->owner_w().template write_simple_tag<double>( "consumed-rx", consumed );
//          std::cout << "consumed rx .... " << rx << "; all " << e << std::endl;

         if ( del_ )
            del_( id, len, data );
      }
      // -----------------------------------------------------------------
      bool is_empty()
      { return !del_; };
      // -----------------------------------------------------------------
      void clear()
      { del_ = energy_consumption_delegate_t(); };
      // -----------------------------------------------------------------
      energy_consumption_delegate_t delegate()
      { return del_; };

   private:
      Os *os_;
      energy_consumption_delegate_t del_;
   };


   /** \brief Implementation of \ref radio_concept "Radio Concept" that
   *     approximates the consumed energy.
   *  \ingroup radio_concept
   *
   */
   template<typename OsModel_P,
            typename Radio_P,
            typename Clock_P>
   class EneryConsumptionRadioModel {
   public:
      typedef OsModel_P OsModel;
      typedef Radio_P Radio;
      typedef Clock_P Clock;
      typedef EneryConsumptionRadioModel <OsModel, Radio, Clock> self_type;

      typedef typename Radio::node_id_t node_id_t;
      typedef typename Radio::size_t size_t;
      typedef typename Radio::block_data_t block_data_t;
      typedef typename Radio::message_id_t message_id_t;

      typedef typename Clock::time_t time_t;

      typedef typename Radio::radio_delegate_t energy_consumption_delegate_t;
      typedef EnergyConsumptionRadioCallback<OsModel> RadioCallback;
      // --------------------------------------------------------------------
      enum SpecialNodeIds {
         BROADCAST_ADDRESS = Radio::BROADCAST_ADDRESS, ///< All nodes in communication range
         NULL_NODE_ID      = Radio::NULL_NODE_ID       ///< Unknown/No node id
      };
      // --------------------------------------------------------------------
      enum Restrictions {
         MAX_MESSAGE_LENGTH = Radio::MAX_MESSAGE_LENGTH
      };
      // --------------------------------------------------------------------
      void
      send(node_id_t id, size_t len, block_data_t *data)
      {
         // TODO
//          double tx = ((((15. + len) * 8.) / 250.) * 44.) / (3600. * 1000.);
//          double e = energy( os ) + tx;
//          set_energy( os, e );
//          double consumed = os->proc->owner().template read_simple_tag<double>( "consumed-tx" );
//          consumed += e;
//          if ( app_active(os) )
//             os->proc->owner_w().template write_simple_tag<double>( "consumed-tx", consumed );
//          std::cout << "consumed tx .... " << tx << "; all " << e << std::endl;

         radio().send( id, len, data );
      }
      // --------------------------------------------------------------------
      void init( Radio& radio, Clock& clock )
      {
         radio_ = &radio;
         clock_ = &clock;
      }
      // --------------------------------------------------------------------
      void destruct()
      {}
      // --------------------------------------------------------------------
      void enable_radio()
      {
//          consume_period( os );
//          set_activity( os, true );

         radio().enable_radio();
      }
      // --------------------------------------------------------------------
      void disable_radio()
      {
//          consume_period( os );
//          set_activity( os, false );

         radio().disable_radio();
      }
      // --------------------------------------------------------------------
      node_id_t id()
      {
         return radio().id();
      }
      // --------------------------------------------------------------------
      template<class T, void (T::*TMethod)(node_id_t, size_t, block_data_t*)>
      int reg_recv_callback( T *obj_pnt )
      {
         energy_consumption_delegate_t del =
            energy_consumption_delegate_t::template from_method<T, TMethod>( obj_pnt );

         RadioCallback *cb = new RadioCallback( del );
         return radio().template reg_recv_callback<RadioCallback, &RadioCallback::receive>( cb );
      }
      // --------------------------------------------------------------------
      void unreg_recv_callback( int idx )
      {
         radio().unreg_recv_callback( idx );
      }

   private:

//       static void consume_period( Os *os )
//       {
//          // TODO: Replace with local storage!
//          if ( data_available(os) )
//          {
//             set_energy( os, 0.0 );
//             set_last_changed( os, Clock::time(os) );
//             set_activity( os, true );
//          }
//          else
//          {
//             double consumed = 0.0;
//             if ( activity(os) )
//             {
//                consumed = (Clock::time(os) - last_changed(os)) * 12.8 / (3600. * 1000.);
//                double tmp = os->proc->owner().template read_simple_tag<double>( "consumed-active" );
//                tmp += consumed;
//                if ( app_active(os) )
//                   os->proc->owner_w().template write_simple_tag<double>( "consumed-active", tmp );
//             }
//             else
//             {
//                consumed = (Clock::time(os) - last_changed(os)) * 0.025 / (3600. * 1000.);
//                double tmp = os->proc->owner().template read_simple_tag<double>( "consumed-idle" );
//                tmp += consumed;
//                if ( app_active(os) )
//                   os->proc->owner_w().template write_simple_tag<double>( "consumed-idle", tmp );
//             }
//             double e = energy( os ) + consumed;
// //             std::cout << "consumed t ....  " << activity(os) << "; " << consumed << "; all " << e << std::endl;
// 
//             set_energy( os, e );
//             set_last_changed( os, Clock::time(os) );
//          }
//       }
//       // --------------------------------------------------------------------
//       static bool data_available( Os *os )
//       {
//          return os->proc->owner().find_tag( "energy" ) == 0 ||
//                os->proc->owner().find_tag( "changed" ) == 0 ||
//                os->proc->owner().find_tag( "activity" ) == 0;
//       }
//       // --------------------------------------------------------------------
//       static void set_activity( Os *os, bool active )
//       {
//          os->proc->owner_w().template write_simple_tag<bool>( "activity", active );
//       }
//       // --------------------------------------------------------------------
//       static void set_last_changed( Os *os, time_t time )
//       {
//          os->proc->owner_w().template write_simple_tag<double>( "changed", time );
//       }
//       // --------------------------------------------------------------------
//       static void set_energy( Os *os, double e )
//       {
//          os->proc->owner_w().template write_simple_tag<double>( "energy", e );
//       }
//       // --------------------------------------------------------------------
//       static bool activity( Os *os )
//       {
//          return os->proc->owner().template read_simple_tag<bool>( "activity" );
//       }
//       // --------------------------------------------------------------------
//       static time_t last_changed( Os *os )
//       {
//          return os->proc->owner().template read_simple_tag<double>( "changed" );
//       }
//       // --------------------------------------------------------------------
//       static double energy( Os *os )
//       {
//          return os->proc->owner().template read_simple_tag<double>( "energy" );
//       }
//       // --------------------------------------------------------------------

//    public:
//       static void set_app_active( Os *os, bool active )
//       {
//          os->proc->owner_w().template write_simple_tag<bool>( "app_active", active );
//       }
//       // --------------------------------------------------------------------
//       static bool app_active( Os *os )
//       {
//          return os->proc->owner().template read_simple_tag<bool>( "app_active" );
//       }

   private:
      Radio& radio()
      { return radio_; }

      Clock& clock()
      { return clock_; }

      Radio *radio_;
      Clock *clock_;

   };

}

#endif
