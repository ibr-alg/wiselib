/************************************************************************
 ** This file is part of the network simulator Shawn.                  **
 ** Copyright (C) 2004,2005 by  SwarmNet (www.swarmnet.de)             **
 **                         and SWARMS   (www.swarms.de)               **
 ** Shawn is free software; you can redistribute it and/or modify it   **
 ** under the terms of the GNU General Public License, version 2.      **
 ************************************************************************/
#include "../buildfiles/_legacyapps_enable_cmake.h"
#ifdef ENABLE_DUTY_CYCLING

#include "sys/node.h"
#include "sys/simulation/simulation_controller.h"
#include "sys/simulation/simulation_environment.h"
#include "sys/taggings/basic_tags.h"
#include "sys/tag.h"
#include "legacyapps/duty_cycling/duty_cycling_processor.h"
#include "legacyapps/duty_cycling/duty_cycling_message.h"

namespace duty_cycling
{

   DutyCyclingProcessor::
   DutyCyclingProcessor()
      : battery_    ( 1.0 ),
         active_    ( true ),
         radius_    ( 0.8 ),
         last_time_ ( -1 ),
         S_         ( 0 ),
         h_         ( 0 ),
         f_         ( 0.0027 )
   {}
   // ----------------------------------------------------------------------
   DutyCyclingProcessor::
   ~DutyCyclingProcessor()
   {}
   // ----------------------------------------------------------------------
   void
   DutyCyclingProcessor::
   boot( void )
      throw()
   {
      const shawn::SimulationEnvironment& se = owner().world().
                                    simulation_controller().environment();

      uni_rnd_ = owner().world().simulation_controller().
                      random_variable_keeper().find( "uni[0;1]" );

      std::string time_step_prob = se.required_string_param( "time_step_prob" );
      time_step_rnd_ = owner().world().simulation_controller().
                     random_variable_keeper().find( time_step_prob );
      time_step_ = *time_step_rnd_;

      energy_consumption_awake_ = se.optional_double_param(
         "energy_consumption_awake", energy_consumption_awake_ );
      energy_consumption_sleep_ = se.optional_double_param(
         "energy_consumption_sleep", energy_consumption_sleep_ );
      activation_threshold_ = se.optional_double_param(
         "activation_threshold", activation_threshold_ );
      probability_awakening_min_ = se.optional_double_param(
         "probability_awakening_min", probability_awakening_min_ );
      probability_awakening_max_ = se.optional_double_param(
         "probability_awakening_max", probability_awakening_max_ );
      spontaneous_awakening_activity_level_ = se.optional_double_param(
         "spontaneous_awakening_activity_level", spontaneous_awakening_activity_level_ );
      radius_min_ = se.optional_double_param(
         "radius_min", radius_min_ );
      radius_max_ = se.optional_double_param(
         "radius_max", radius_max_ );
      g_ = se.optional_double_param( "g", g_ );

      S_ = spontaneous_awakening_activity_level_;
      h_ = S_;

      probability_awakening_ = 0.0;

      owner_w().add_tag(new shawn::DoubleTag("active", 1.0));

      last_time_ = owner().world().current_time();
      owner_w().world_w().scheduler_w().new_event( *this, last_time_ + time_step_, NULL );
   }
   // ----------------------------------------------------------------------
   bool
   DutyCyclingProcessor::
   process_message( const shawn::ConstMessageHandle& mh )
      throw()
   {
      const DutyCyclingMessage* msg =
         dynamic_cast<const DutyCyclingMessage*>( mh.get() );
      if( msg != NULL )
      {
         h_ += msg->activity();
         return true;
      }

      return Processor::process_message( mh );
   }
   // ----------------------------------------------------------------------
   void
   DutyCyclingProcessor::
   work( void )
      throw()
   {}
   // ----------------------------------------------------------------------
   void
   DutyCyclingProcessor::
   timeout( shawn::EventScheduler& scheduler,
            shawn::EventScheduler::EventHandle eh,
            double time,
            shawn::EventScheduler::EventTagHandle& eth )
      throw()
   {
      energy_harvest( last_time_, owner().world().current_time() );
      consume_energy();

      if ( battery_ < 0.01 )
      {
         active_ = false;
         owner_w().remove_tag_by_name("VisBattery");
         owner_w().add_tag(new shawn::DoubleTag("VisBattery", 0.0));
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
            send( new DutyCyclingMessage(S_) );
      }

      last_time_ = owner().world().current_time();
      owner_w().world_w().scheduler_w().new_event( *this, last_time_ + time_step_, NULL );
   }
   // ----------------------------------------------------------------------
   double
   DutyCyclingProcessor::
   sun( double t )
   {
      while (t >= 1440.)
         t -= 1440.;

      if (t >= 420. and t < 1140.)
         return (1. - cos(2. * M_PI * (t - 420.) / (1140. - 420.))) / 2.;
      return 0.;
   }
   // ----------------------------------------------------------------------
   void
   DutyCyclingProcessor::
   energy_harvest( double t1, double t2 )
   {
      assert( t2 > t1 );
      battery_ += f_ * ((t2 - t1) * ((sun(t1) + sun(t2)) / 2));
      if (battery_ > 1.0)
         battery_ = 1.0;
   }
   // ----------------------------------------------------------------------
   void
   DutyCyclingProcessor::
   consume_energy()
   {
      if (active_)
         battery_ -= energy_consumption_awake_ * (1. + radius_);
      else
         battery_ -= energy_consumption_sleep_;

      if (battery_ < 0.0)
         battery_ = 0.0;
   }
   // ----------------------------------------------------------------------
   void
   DutyCyclingProcessor::
   update_active_state()
   {
      if ( S_ >= activation_threshold_ )
      {
         active_ = true;
         owner_w().remove_tag_by_name("VisBattery");
         owner_w().add_tag(new shawn::DoubleTag("VisBattery", 1.0));
      }
      else
      {
         active_ = false;
         owner_w().remove_tag_by_name("VisBattery");
         owner_w().add_tag(new shawn::DoubleTag("VisBattery", 0.0));
      }
   }
   // ----------------------------------------------------------------------
   void
   DutyCyclingProcessor::
   try_spontaneous_awakening()
   {
      if (!active_)
      {
         double p = *uni_rnd_;
         if ( p <= probability_awakening_ )
         {
           S_ = spontaneous_awakening_activity_level_;
           active_ = true;
         }
      }
   }
   // ----------------------------------------------------------------------
   void
   DutyCyclingProcessor::
   update_radius()
   {
      radius_ = radius_min_ * ( 1 - battery_ ) + radius_max_ * battery_;
      // FIXME
      owner_w().set_transmission_range( radius_ / 0.15 );
   }
   // ----------------------------------------------------------------------
   void
   DutyCyclingProcessor::
   update_probability_awakening()
   {
      probability_awakening_ =
         probability_awakening_min_ * ( 1 - battery_ ) +
         probability_awakening_max_ * battery_;
   }
   // ----------------------------------------------------------------------
   void
   DutyCyclingProcessor::
   update_actvity()
   {
      S_ = tanh(g_*h_);
      h_ = S_;
   }

}

#endif
/*-----------------------------------------------------------------------
 * Source  $Source: /cvs/shawn/shawn/tubsapps/duty_cycling/duty_cycling_processor.cpp,v $
 * Version $Revision: 1.3 $
 * Date    $Date: 2005/08/05 10:00:52 $
 *-----------------------------------------------------------------------
 * $Log: duty_cycling_processor.cpp,v $
 * Revision 1.3  2005/08/05 10:00:52  ali
 * 2005 copyright notice
 *
 * Revision 1.2  2005/06/09 15:28:09  tbaum
 * added module functionality
 *
 * Revision 1.1  2004/11/25 11:16:52  tbaum
 * added duty_cycling
 *
 *-----------------------------------------------------------------------*/
