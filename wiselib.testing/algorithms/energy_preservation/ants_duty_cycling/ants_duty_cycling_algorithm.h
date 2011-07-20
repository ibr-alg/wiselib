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
#ifndef __ANTS_DUTY_CYCLING_ALGORITHM_H__
#define __ANTS_DUTY_CYCLING_ALGORITHM_H__

#include "algorithms/energy_preservation/ants_duty_cycling/ants_duty_cycling_message.h"
#include "util/delegates/delegate.hpp"

// TODO: change shawn os()-> calls to wiselib calls

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
#include "sys/comm_models/disk_graph_model.h"

// #define ANTS_DUTY_CYCLING_DEBUG

namespace wiselib
{

   /** Duty cycling algorithm using ant behavior
    */
   template<typename OsModel_P,
            typename Radio_P,
            typename Timer_P,
            typename Clock_P,
            typename Debug_P = typename OsModel_P::Debug>
   class AntsDutyCyclingAlgorithm
   {
   public:
      typedef OsModel_P OsModel;
      typedef Radio_P Radio;
      typedef Timer_P Timer;
      typedef Clock_P Clock;
      typedef Debug_P Debug;

      typedef AntsDutyCyclingAlgorithm<OsModel, Radio, Timer, Clock, Debug> self_type;

      typedef typename Radio::node_id_t node_id_t;
      typedef typename Radio::size_t size_t;
      typedef typename Radio::block_data_t block_data_t;
      typedef typename Radio::message_id_t message_id_t;

      typedef typename Clock::time_t time_t;

      typedef AntDutyCyclingMessage<OsModel, Radio> Message;

      typedef delegate1<void, int> energy_preservation_delegate_t;
      // --------------------------------------------------------------------
      enum ErrorCodes
      {
         SUCCESS = OsModel::SUCCESS,
         ERR_UNSPEC = OsModel::ERR_UNSPEC,
         ERR_NOTIMPL = OsModel::ERR_NOTIMPL
      };
      // --------------------------------------------------------------------
      enum EnergyPreservationActivity
      {
         EPA_ACTIVE = 1,
         EPA_INACTIVE
      };
      // --------------------------------------------------------------------
      ///@name Construction / Destruction
      ///@{
      AntsDutyCyclingAlgorithm();
      ~AntsDutyCyclingAlgorithm();
      ///@}
      // --------------------------------------------------------------------
      ///@name Duty Cycling Control
      ///@{
      /// @deprecated use init() instead!
      void enable( void );
      /// @deprecated use destruct() instead!
      void disable( void );
      void set_configuration( void );
      ///@}
      // --------------------------------------------------------------------
      ///@name Duty Cycling Callback Mechanism
      ///@{
      // --------------------------------------------------------------------
      template<class T, void (T::*TMethod)(int)>
      inline int reg_changed_callback( T *obj_pnt )
      {
         callback_ = energy_preservation_delegate_t::from_method<T, TMethod>( obj_pnt );
         return 0;
      }
      // --------------------------------------------------------------------
      inline int unreg_changed_callback( int )
      {
         callback_ = energy_preservation_delegate_t();
         return SUCCESS;
      }
      // --------------------------------------------------------------------
      inline void notify_receivers( EnergyPreservationActivity value )
      {
         if (callback_)
            callback_( value );
      }
      ///@}
      // --------------------------------------------------------------------
      ///@name Methods called by RadioModel
      ///@{
      void receive( node_id_t from, size_t len, block_data_t *data );
      ///@}
      // --------------------------------------------------------------------
      ///@name Methods called by Timer
      ///@{
      void enable_transmission_phase( void *userdata );
      void algorithm_step( void *userdata );
      void disable_transmission_phase( void *userdata );
      ///@}
      // --------------------------------------------------------------------
      inline double activity()
      { return S_; }
      // --------------------------------------------------------------------
      inline double battery()
      {
         return battery_;
      }
      // --------------------------------------------------------------------
      inline void set_battery( double battery )
      {
         if ( battery < 0.0 )
             battery = 0.0;
         if ( battery > 1.0 )
             battery = 1.0;
         battery_ = battery;
      }
      // --------------------------------------------------------------------
      inline bool active()
      { return active_; }
      // --------------------------------------------------------------------
      double sun( time_t time );
      // --------------------------------------------------------------------
      int init( Radio& radio, Timer& timer, Clock& clock, Debug& debug )
      {
         radio_ = &radio;
         timer_ = &timer;
         clock_ = &clock;
         debug_ = &debug;
         return SUCCESS;
      }
      // --------------------------------------------------------------------
      int init()
      {
         enable();
         return SUCCESS;
      }
      // --------------------------------------------------------------------
      int destruct()
      {
         disable();
         return SUCCESS;
      }

   private:
      Radio& radio()
      { return *radio_; }
      // --------------------------------------------------------------------
      Timer& timer()
      { return *timer_; }
      // --------------------------------------------------------------------
      Clock& clock()
      { return *clock_; }
      // --------------------------------------------------------------------
      Debug& debug()
      { return *debug_; }
      // --------------------------------------------------------------------
      typename Radio::self_pointer_t radio_;
      typename Timer::self_pointer_t timer_;
      typename Clock::self_pointer_t clock_;
      typename Debug::self_pointer_t debug_;
      // --------------------------------------------------------------------
      void energy_harvest( time_t t1, time_t t2 );
      void consume_energy();
      void update_active_state();
      void try_spontaneous_awakening();
      void update_radius();
      void update_probability_awakening();
      void update_actvity();
      // --------------------------------------------------------------------
      enum MessageIds
      {
         ANTS_DUTY_CYCLING_MESSAGE_ID = 147
      };
      // --------------------------------------------------------------------
      energy_preservation_delegate_t callback_;

      double battery_;
      bool active_;
      double radius_;
      double transmission_range_;
      double application_consumption_;
      time_t last_time_;

      double S_, h_, f_;

      double energy_consumption_awake_, energy_consumption_sleep_;
      double activation_threshold_;
      double probability_awakening_;
      double probability_awakening_min_, probability_awakening_max_;
      double spontaneous_awakening_activity_level_;
      double radius_min_, radius_max_;
      double g_;
      double cloud_;

      double time_step_;

      // TODO
      shawn::ConstRandomVariableHandle time_step_rnd_;
      shawn::ConstRandomVariableHandle uni_rnd_;
   };
   // -----------------------------------------------------------------------
   // -----------------------------------------------------------------------
   // -----------------------------------------------------------------------
   template<typename OsModel_P,
            typename Radio_P,
            typename Timer_P,
            typename Clock_P,
            typename Debug_P>
   AntsDutyCyclingAlgorithm<OsModel_P, Radio_P, Timer_P, Clock_P, Debug_P>::
   AntsDutyCyclingAlgorithm()
      : callback_  ( energy_preservation_delegate_t() ),
         battery_   ( 1.0 ),
         active_    ( true ),
         radius_    ( 0.8 ),
         transmission_range_ ( 1.0 ),
         application_consumption_ ( 0.001 ),
         last_time_ ( time_t() ),
         S_         ( 0 ),
         h_         ( 0 ),
         f_         ( 0.0027 )
   {};
   // -----------------------------------------------------------------------
   template<typename OsModel_P,
            typename Radio_P,
            typename Timer_P,
            typename Clock_P,
            typename Debug_P>
   AntsDutyCyclingAlgorithm<OsModel_P, Radio_P, Timer_P, Clock_P, Debug_P>::
   ~AntsDutyCyclingAlgorithm()
   {
#ifdef ANTS_DUTY_CYCLING_DEBUG
      debug().debug( "AntsDutyCyclingAlgorithm:dtor\n" );
#endif
   };
   // -----------------------------------------------------------------------
   template<typename OsModel_P,
            typename Radio_P,
            typename Timer_P,
            typename Clock_P,
            typename Debug_P>
   void
   AntsDutyCyclingAlgorithm<OsModel_P, Radio_P, Timer_P, Clock_P, Debug_P>::
   enable( void )
   {
#ifdef ANTS_DUTY_CYCLING_DEBUG
      debug().debug( "AntsDutyCyclingAlgorithm: Boot for %i\n", radio().id(  ) );
#endif

// TODO:
//       os()->proc->owner_w().template write_simple_tag<double>( "battery", 1.0 );
//       os()->proc->owner_w().template write_simple_tag<double>( "consumed-app", 0.0 );
//       os()->proc->owner_w().template write_simple_tag<double>( "consumed-rx", 0.0 );
//       os()->proc->owner_w().template write_simple_tag<double>( "consumed-tx", 0.0 );
//       os()->proc->owner_w().template write_simple_tag<double>( "consumed-idle", 0.0 );
//       os()->proc->owner_w().template write_simple_tag<double>( "consumed-active", 0.0 );

      radio().enable_radio(  );
      radio().template reg_recv_callback<self_type, &self_type::receive>( this );

      active_ = true;
      notify_receivers( EPA_ACTIVE );
      radio().set_app_active( active_ );

      last_time_ = clock().time( );

      time_t next_step = 1 - (clock().time( ) - (int)clock().time(  ));
      timer().template set_timer<self_type, &self_type::enable_transmission_phase>(
                                  int(next_step * 1000), this, 0 );
   }
   // -----------------------------------------------------------------------
   template<typename OsModel_P,
            typename Radio_P,
            typename Timer_P,
            typename Clock_P,
            typename Debug_P>
   void
   AntsDutyCyclingAlgorithm<OsModel_P, Radio_P, Timer_P, Clock_P, Debug_P>::
   disable( void )
   {
#ifdef ANTS_DUTY_CYCLING_DEBUG
      debug().debug(  "AntsDutyCyclingAlgorithm: Disable\n" );
#endif
// TODO
//       radio().unreg_recv_callback(  callback_id_ );
      radio().disable(  );
   }
   // -----------------------------------------------------------------------
   template<typename OsModel_P,
            typename Radio_P,
            typename Timer_P,
            typename Clock_P,
            typename Debug_P>
   void
   AntsDutyCyclingAlgorithm<OsModel_P, Radio_P, Timer_P, Clock_P, Debug_P>::
   set_configuration( void )
   {
// TODO:
//       const shawn::SimulationEnvironment& se = os_->proc->owner().world().
//                                     simulation_controller().environment();
// 
//       // TODO
//       uni_rnd_ = os_->proc->owner().world().simulation_controller().
//                       random_variable_keeper().find( "uni[0;1]" );
// 
//       // TODO
//       std::string time_step_prob = se.required_string_param( "time_step_prob" );
//       time_step_rnd_ = os_->proc->owner().world().simulation_controller().
//                      random_variable_keeper().find( time_step_prob );
//       time_step_ = *time_step_rnd_;
// 
//       // TODO
//       energy_consumption_awake_ = se.optional_double_param(
//          "energy_consumption_awake", energy_consumption_awake_ );
//       energy_consumption_sleep_ = se.optional_double_param(
//          "energy_consumption_sleep", energy_consumption_sleep_ );
//       activation_threshold_ = se.optional_double_param(
//          "activation_threshold", activation_threshold_ );
//       probability_awakening_min_ = se.optional_double_param(
//          "probability_awakening_min", probability_awakening_min_ );
//       probability_awakening_max_ = se.optional_double_param(
//          "probability_awakening_max", probability_awakening_max_ );
//       spontaneous_awakening_activity_level_ = se.optional_double_param(
//          "spontaneous_awakening_activity_level", spontaneous_awakening_activity_level_ );
//       radius_min_ = se.optional_double_param(
//          "radius_min", radius_min_ );
//       radius_max_ = se.optional_double_param(
//          "radius_max", radius_max_ );
//       // parameter already set for disk graph model
//       transmission_range_ = se.required_double_param( "range" );
//       application_consumption_ = se.optional_double_param(
//          "application_consumption", application_consumption_ );
//       g_ = se.optional_double_param( "g", g_ );
//       cloud_ = se.optional_double_param( "cloud", cloud_ );
// 
//       S_ = spontaneous_awakening_activity_level_;
//       h_ = S_;
// 
//       probability_awakening_ = 0.0;
   }
   // -----------------------------------------------------------------------
   template<typename OsModel_P,
            typename Radio_P,
            typename Timer_P,
            typename Clock_P,
            typename Debug_P>
   void
   AntsDutyCyclingAlgorithm<OsModel_P, Radio_P, Timer_P, Clock_P, Debug_P>::
   receive( node_id_t from, size_t len, block_data_t *data )
   {
      if ( from == radio().id() )
         return;

      message_id_t msg_id = *data;
      if ( msg_id == ANTS_DUTY_CYCLING_MESSAGE_ID )
      {
         Message *message = (Message *)data;
         h_ += message->activity();
      }
   }
   // -----------------------------------------------------------------------
   template<typename OsModel_P,
            typename Radio_P,
            typename Timer_P,
            typename Clock_P,
            typename Debug_P>
   void
   AntsDutyCyclingAlgorithm<OsModel_P, Radio_P, Timer_P, Clock_P, Debug_P>::
   enable_transmission_phase( void *userdata )
   {
      radio().enable(  );
      timer().template set_timer<self_type, &self_type::algorithm_step>(
                                  int(time_step_ * 1000), this, 0 );
   }
   // -----------------------------------------------------------------------
   template<typename OsModel_P,
            typename Radio_P,
            typename Timer_P,
            typename Clock_P,
            typename Debug_P>
   void
   AntsDutyCyclingAlgorithm<OsModel_P, Radio_P, Timer_P, Clock_P, Debug_P>::
   algorithm_step( void *userdata )
   {
#ifdef ANTS_DUTY_CYCLING_DEBUG
//       debug().debug(  "AntsDutyCyclingAlgorithm: step at %i at %f \n",  radio().id(  ), os_->proc->owner().world().current_time() );
#endif
// TODO
//       energy_harvest( last_time_, os_->proc->owner().world().current_time() );
      consume_energy();

      if ( battery_ < 0.01 )
      {
         active_ = false;
         notify_receivers( EPA_INACTIVE );
         radio().set_app_active(  active_ );
         S_ = 0.0;
      }
      else
      {
         update_actvity();
         update_active_state();
         update_probability_awakening();
         try_spontaneous_awakening();
         update_radius();
         if (active_)
         {
            Message message;
            message.set_msg_id( ANTS_DUTY_CYCLING_MESSAGE_ID );
            message.set_activity( S_ );
            radio().send(  radio().BROADCAST_ADDRESS,
                     message.buffer_size(), (block_data_t*)&message );
         }
      }

      last_time_ = clock().time(  );
      timer().template set_timer<self_type, &self_type::disable_transmission_phase>(
                                  int(time_step_ * 1000), this, 0 );
   }
   // -----------------------------------------------------------------------
   template<typename OsModel_P,
            typename Radio_P,
            typename Timer_P,
            typename Clock_P,
            typename Debug_P>
   void
   AntsDutyCyclingAlgorithm<OsModel_P, Radio_P, Timer_P, Clock_P, Debug_P>::
   disable_transmission_phase( void *userdata )
   {
      radio().disable(  );
      time_t next_step = 1 - (clock().time(  ) - (int)clock().time(  ));
      timer().template set_timer<self_type, &self_type::enable_transmission_phase>(
                                  int(next_step * 1000), this, 0 );
   }
   // ----------------------------------------------------------------------
   template<typename OsModel_P,
            typename Radio_P,
            typename Timer_P,
            typename Clock_P,
            typename Debug_P>
   double
   AntsDutyCyclingAlgorithm<OsModel_P, Radio_P, Timer_P, Clock_P, Debug_P>::
   sun( time_t t )
   {
      while (t >= 1440.)
         t -= 1440.;

      if (t >= 420. and t < 1140.)
         return (1. - cos(2. * M_PI * (t - 420.) / (1140. - 420.))) / 2.;
      return 0.;
   }
   // ----------------------------------------------------------------------
   template<typename OsModel_P,
            typename Radio_P,
            typename Timer_P,
            typename Clock_P,
            typename Debug_P>
   void
   AntsDutyCyclingAlgorithm<OsModel_P, Radio_P, Timer_P, Clock_P, Debug_P>::
   energy_harvest( time_t t1, time_t t2 )
   {
      assert( t2 > t1 );
      double bat = battery() +
         f_ * ((t2 - t1) * ((sun(t1) + sun(t2))*(1-cloud_) / 2));
      set_battery( bat );
      // TODO
//       os()->proc->owner_w().template write_simple_tag<double>( "battery", battery_ );
   }
   // ----------------------------------------------------------------------
   template<typename OsModel_P,
            typename Radio_P,
            typename Timer_P,
            typename Clock_P,
            typename Debug_P>
   void
   AntsDutyCyclingAlgorithm<OsModel_P, Radio_P, Timer_P, Clock_P, Debug_P>::
   consume_energy()
   {
      // set by the energy consumption radio
      //TODO
//       double e = os()->proc->owner().template read_simple_tag<double>( "energy" );
//       set_battery( battery() - e / 2600. );
      // TODO
//       os()->proc->owner_w().template write_simple_tag<double>( "battery", battery_ );
//       os()->proc->owner_w().template write_simple_tag<double>( "energy", 0.0 );

      // fixed/constant application consumption if active
      if (active_)
      {
         set_battery( battery() - application_consumption_ );

         // TODO
//          double consumed = os()->proc->owner().template read_simple_tag<double>( "consumed-app" );
//          consumed += application_consumption_;
         // TODO:
//          os()->proc->owner_w().template write_simple_tag<double>( "consumed-app", consumed );
      }
   }
   // ----------------------------------------------------------------------
   template<typename OsModel_P,
            typename Radio_P,
            typename Timer_P,
            typename Clock_P,
            typename Debug_P>
   void
   AntsDutyCyclingAlgorithm<OsModel_P, Radio_P, Timer_P, Clock_P, Debug_P>::
   update_active_state()
   {
      if ( S_ >= activation_threshold_ )
      {
         active_ = true;
         notify_receivers( EPA_ACTIVE );
      }
      else
      {
         active_ = false;
         notify_receivers( EPA_INACTIVE );
      }
      radio().set_app_active(  active_ );
   }
   // ----------------------------------------------------------------------
   template<typename OsModel_P,
            typename Radio_P,
            typename Timer_P,
            typename Clock_P,
            typename Debug_P>
   void
   AntsDutyCyclingAlgorithm<OsModel_P, Radio_P, Timer_P, Clock_P, Debug_P>::
   try_spontaneous_awakening()
   {
      if ( !active_ )
      {
         // TODO
         double p = *uni_rnd_;
         if ( p <= probability_awakening_ )
         {
            S_ = spontaneous_awakening_activity_level_;
            active_ = true;
            notify_receivers( EPA_ACTIVE );
            radio().set_app_active(  active_ );
         }
      }
   }
   // ----------------------------------------------------------------------
   template<typename OsModel_P,
            typename Radio_P,
            typename Timer_P,
            typename Clock_P,
            typename Debug_P>
   void
   AntsDutyCyclingAlgorithm<OsModel_P, Radio_P, Timer_P, Clock_P, Debug_P>::
   update_radius()
   {
      radius_ = radius_min_ * ( 1 - battery_ ) + radius_max_ * battery_;

      double ratio = radius_ / transmission_range_;
      double value = ratio;
      if ( ratio <= .15/2.)
         value = 0.;
      else if( ratio <= (.15+.32)/2. )
         value = 0.15;
       else if( ratio <= (.32+.49)/2. )
         value = 0.32;
       else if( ratio <= (.49+.66)/2. )
         value = 0.49;
       else if( ratio <= (.66+.83)/2. )
         value = 0.66;
       else if( ratio <= (.83+.1)/2. )
         value = 0.83;
       else
         value = 1.0;

//        if ( ratio <= .15 )
//           value = 0.15;
//        else if( ratio <= .32 )
//           value = 0.32;
//        else if( ratio <= .49 )
//           value = 0.49;
//        else if( ratio <= .66 )
//           value = 0.66;
//        else if( ratio <= .83 )
//           value = 0.83;
//        else
//           value = 1.0;

//       std::cout << "UPADTE to " << ratio << "; value is " << value << std::endl;

// TODO
//        os_->proc->owner_w().set_transmission_range( value );
   }
   // ----------------------------------------------------------------------
   template<typename OsModel_P,
            typename Radio_P,
            typename Timer_P,
            typename Clock_P,
            typename Debug_P>
   void
   AntsDutyCyclingAlgorithm<OsModel_P, Radio_P, Timer_P, Clock_P, Debug_P>::
   update_probability_awakening()
   {
      probability_awakening_ =
         probability_awakening_min_ * ( 1 - battery_ ) +
         probability_awakening_max_ * battery_;
   }
   // ----------------------------------------------------------------------
   template<typename OsModel_P,
            typename Radio_P,
            typename Timer_P,
            typename Clock_P,
            typename Debug_P>
   void
   AntsDutyCyclingAlgorithm<OsModel_P, Radio_P, Timer_P, Clock_P, Debug_P>::
   update_actvity()
   {
      S_ = tanh(g_*h_);
      h_ = S_;
   }

}
#endif
