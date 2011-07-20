/************************************************************************
 ** This file is part of the network simulator Shawn.                  **
 ** Copyright (C) 2004,2005 by  SwarmNet (www.swarmnet.de)             **
 **                         and SWARMS   (www.swarms.de)               **
 ** Shawn is free software; you can redistribute it and/or modify it   **
 ** under the terms of the GNU General Public License, version 2.      **
 ************************************************************************/
#include "../buildfiles/_legacyapps_enable_cmake.h"
#ifdef ENABLE_DUTY_CYCLING


#ifndef __SHAWN_LEGACYAPPS_DUTY_CYCLING_PROCESSOR_H
#define __SHAWN_LEGACYAPPS_DUTY_CYCLING_PROCESSOR_H

#include "sys/processor.h"
#include "sys/event_scheduler.h"
#include "sys/misc/random/random_variable.h"
#include "sys/misc/random/random_variable_keeper.h"

namespace duty_cycling
{

   class DutyCyclingProcessor
      : public shawn::Processor,
         public shawn::EventScheduler::EventHandler
   {
   public:
      DutyCyclingProcessor();
      virtual ~DutyCyclingProcessor();

      ///@name Inherited from Processor
      ///@{
      virtual void boot( void ) throw();
      virtual bool process_message( const shawn::ConstMessageHandle& ) throw();
      virtual void work( void ) throw();
      ///@}

      ///@name Inherited from EventScheduler::EventHandler
      ///@{
      /**
       */
      virtual void timeout( shawn::EventScheduler&, 
                            shawn::EventScheduler::EventHandle, 
                            double, 
                            shawn::EventScheduler::EventTagHandle& ) throw();
      ///@}

//    private:

      double sun( double time );
      void energy_harvest( double, double );
      void consume_energy();
      void update_active_state();
      void try_spontaneous_awakening();
      void update_radius();
      void update_probability_awakening();
      void update_actvity();

      double battery_;
      bool active_;
      double radius_;
      double last_time_;

      double S_, h_, f_;

      double energy_consumption_awake_, energy_consumption_sleep_;
      double activation_threshold_;
      double probability_awakening_;
      double probability_awakening_min_, probability_awakening_max_;
      double spontaneous_awakening_activity_level_;
      double radius_min_, radius_max_;
      double g_;

      double time_step_;

      shawn::ConstRandomVariableHandle time_step_rnd_;
      shawn::ConstRandomVariableHandle uni_rnd_;
   };


}

#endif
#endif
/*-----------------------------------------------------------------------
 * Source  $Source: /cvs/shawn/shawn/tubsapps/duty_cycling/duty_cycling_processor.h,v $
 * Version $Revision: 1.3 $
 * Date    $Date: 2005/08/05 10:00:52 $
 *-----------------------------------------------------------------------
 * $Log: duty_cycling_processor.h,v $
 * Revision 1.3  2005/08/05 10:00:52  ali
 * 2005 copyright notice
 *
 * Revision 1.2  2005/06/09 15:28:09  tbaum
 * added module functionality
 *
 * Revision 1.1  2004/11/25 11:16:53  tbaum
 * added duty_cycling
 *
 *-----------------------------------------------------------------------*/
