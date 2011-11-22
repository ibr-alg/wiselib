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

#include "util/base_classes/radio_base.h"
#include "util/metrics/energy_consumption_traits_jn5139.h"

namespace wiselib {


   /** \brief Implementation of \ref radio_concept "Radio Concept" that
   *     approximates the consumed energy.
   *  \ingroup radio_concept
   *
   */
   template<typename OsModel_P,
            typename Radio_P,
            typename Clock_P,
            typename Debug_P,
            typename EneryConsumptionTraits_P = EnergyConsumptionTraitsJennic5139,
            int BATTERY_MAX = 2600>
   class EneryConsumptionRadioModel
      : public RadioBase<OsModel_P, typename Radio_P::node_id_t, typename Radio_P::size_t, typename Radio_P::block_data_t, 10>
   {
   public:
      typedef OsModel_P OsModel;
      typedef Radio_P Radio;
      typedef Clock_P Clock;
      typedef Debug_P Debug;
      typedef EneryConsumptionTraits_P ConsumptionTraits;
      typedef EneryConsumptionRadioModel <OsModel, Radio, Clock, Debug, ConsumptionTraits, BATTERY_MAX> self_type;
      typedef RadioBase<OsModel_P, Radio_P, typename OsModel_P::size_t, typename OsModel_P::block_data_t, 10> radio_base_t;
      
      typedef self_type* self_pointer_t;

      typedef typename Radio::node_id_t node_id_t;
      typedef typename Radio::size_t size_t;
      typedef typename Radio::block_data_t block_data_t;
      typedef typename Radio::message_id_t message_id_t;

      typedef typename Clock::time_t time_t;
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
      EneryConsumptionRadioModel()
         : energy_      ( 0.0 ),
            consumed_rx_ ( 0.0 ),
            consumed_tx_ ( 0.0 ),
            consumed_active_ ( 0.0 ),
            consumed_idle_   ( 0.0 ),
            active_ ( false )
      {}
      // --------------------------------------------------------------------
      void send(node_id_t id, size_t len, block_data_t *data)
      {
//          debug_->debug("ec-traits hs=%d, tx-m=%f\n",
//                          ConsumptionTraits::MESSAGE_HEADER_SIZE,
//                          ConsumptionTraits::TX_MULTIPLIER );
//          double tx = ((((13 + len) * 8.) / 250.) * 38.) / (3600. * 1000.);
         double tx = (ConsumptionTraits::MESSAGE_HEADER_SIZE + len) * ConsumptionTraits::TX_MULTIPLIER;
         energy_ += tx;
         consumed_tx_ += tx;

         radio().send( id, len, data );
      }
      // --------------------------------------------------------------------
      void init( Radio& radio, Clock& clock, Debug& debug )
      {
         radio_ = &radio;
         clock_ = &clock;
         debug_ = &debug;
         
         last_changed_ = clock_->time();
         
         radio_->template reg_recv_callback<self_type, &self_type::receive>( this );
      }
      // --------------------------------------------------------------------
      void destruct()
      {}
      // --------------------------------------------------------------------
      void enable_radio()
      {
         consume_period();
         active_ = true;

         radio().enable_radio();
      }
      // --------------------------------------------------------------------
      void disable_radio()
      {
         consume_period();
         active_ = false;

         radio().disable_radio();
      }
      // --------------------------------------------------------------------
      node_id_t id()
      {
         return radio().id();
      }
      // --------------------------------------------------------------------
      double battery_level()
      { 
         if ( energy_ > BATTERY_MAX )
            return 0.0;
         
         return (((double)BATTERY_MAX - energy_) / (double)BATTERY_MAX) * 100.;
      }
      // --------------------------------------------------------------------
      int operator()( void )
      {
         return (int)battery_level();
      }
      // --------------------------------------------------------------------
      double energy() { return energy_; }
      // --------------------------------------------------------------------
      double consumed_rx() { return consumed_rx_; }
      // --------------------------------------------------------------------
      double consumed_tx() { return consumed_tx_; }
      // --------------------------------------------------------------------
      double consumed_active() { return consumed_active_; }
      // --------------------------------------------------------------------
      double consumed_idle() { return consumed_idle_; }

   private:
      
      // --------------------------------------------------------------------
      void receive( node_id_t id, size_t len, block_data_t* data )
      {
         // double rx = ((((15. + len) * 8.) / 250.) * 44.) / (3600. * 1000.);
         double rx = (ConsumptionTraits::MESSAGE_HEADER_SIZE + len) * ConsumptionTraits::RX_MULTIPLIER;
         energy_ += rx;
         consumed_rx_ += rx;
         
         self_type::notify_receivers( id, len, data );
      }
      // --------------------------------------------------------------------
      void consume_period()
      {
         double consumed = 0.0;
         if ( active_ )
         {
            // consumed = (clock().time() - last_changed_) * 12.8 / (3600. * 1000.);
            typename Clock::time_t period = clock().time() - last_changed_;
            uint32_t millis = clock().milliseconds(period) + clock().seconds(period)*1000;
            consumed = millis * ConsumptionTraits::ACTIVE_MULTIPLIER;
            consumed_active_ += consumed;
         }
         else
         {
            // consumed = (clock().time() - last_changed_) * 0.025 / (3600. * 1000.);
            typename Clock::time_t period = clock().time() - last_changed_;
            uint32_t millis = clock().milliseconds(period) + clock().seconds(period)*1000;
            consumed = millis * ConsumptionTraits::IDLE_MULTIPLIER;
            consumed_idle_ += consumed;
         }
         energy_ += consumed;

         last_changed_ = clock().time();
      }

   private:
      Radio& radio()
      { return *radio_; }

      Clock& clock()
      { return *clock_; }
      
      Debug& debug()
      { return *debug_; }

      typename Radio::self_pointer_t radio_;
      typename Clock::self_pointer_t clock_;
      typename Debug::self_pointer_t debug_;

      double energy_;
      double consumed_rx_;
      double consumed_tx_;
      double consumed_active_;
      double consumed_idle_;
      
      bool active_;
         
      typename Clock::time_t last_changed_;
   };

}

#endif
