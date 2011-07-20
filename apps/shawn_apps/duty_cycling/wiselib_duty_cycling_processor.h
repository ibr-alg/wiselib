/************************************************************************
 ** This file is part of the network simulator Shawn.                  **
 ** Copyright (C) 2004,2005 by  SwarmNet (www.swarmnet.de)             **
 **                         and SWARMS   (www.swarms.de)               **
 ** Shawn is free software; you can redistribute it and/or modify it   **
 ** under the terms of the GNU General Public License, version 2.      **
 ************************************************************************/
#include "../buildfiles/_legacyapps_enable_cmake.h"
#ifdef ENABLE_DUTY_CYCLING


#ifndef __SHAWN_LEGACYAPPS_WISELIB_DUTY_CYCLING_PROCESSOR_H
#define __SHAWN_LEGACYAPPS_WISELIB_DUTY_CYCLING_PROCESSOR_H

#include "sys/processor.h"
#include "sys/event_scheduler.h"
#include "sys/misc/random/random_variable.h"
#include "sys/misc/random/random_variable_keeper.h"

#include "external_interface/shawn/shawn_os.h"
#include "external_interface/shawn/shawn_radio.h"
#include "external_interface/shawn/shawn_timer.h"
#include "external_interface/shawn/shawn_clock.h"
#include "external_interface/shawn/shawn_debug.h"
#include "util/metrics/energy_consumption_radio.h"
#include "algorithms/energy_preservation/ants_duty_cycling/ants_duty_cycling_algorithm.h"

typedef wiselib::ShawnOsModel Os;
typedef wiselib::ShawnClockModel<Os> ShawnClock;
typedef wiselib::EneryConsumptionRadioModel<Os, Os::Radio, ShawnClock> EnergyRadio;

typedef wiselib::AntsDutyCyclingAlgorithm<Os, EnergyRadio, Os::Timer, ShawnClock, Os::Debug>
   AntsDutyCycling;

namespace duty_cycling
{

   class WiselibDutyCyclingProcessor
      : public wiselib::ExtIfaceProcessor
   {
   public:
      WiselibDutyCyclingProcessor();
      virtual ~WiselibDutyCyclingProcessor();

      ///@name Inherited from Processor
      ///@{
      virtual void boot( void ) throw();
      virtual void work( void ) throw();
      ///@}

      void energy_preservation( int ) throw();

      wiselib::ShawnOs os_;
      Os::Radio wiselib_radio_;
      Os::Timer wiselib_timer_;
      ShawnClock wiselib_clock_;
      Os::Debug wiselib_debug_;
      EnergyRadio energy_consumption_radio_;
      AntsDutyCycling ants_duty_cycling_;
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
