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
#include "legacyapps/duty_cycling/wiselib_duty_cycling_processor.h"

namespace duty_cycling
{

   WiselibDutyCyclingProcessor::
   WiselibDutyCyclingProcessor()
      : wiselib_radio_( os_ ),
         wiselib_timer_( os_ ),
         wiselib_clock_( os_ ),
         wiselib_debug_( os_ )
   {}
   // ----------------------------------------------------------------------
   WiselibDutyCyclingProcessor::
   ~WiselibDutyCyclingProcessor()
   {}
   // ----------------------------------------------------------------------
   void
   WiselibDutyCyclingProcessor::
   boot( void )
      throw()
   {
      os_.proc = this;
      energy_consumption_radio_.init( wiselib_radio_, wiselib_clock_ );
      ants_duty_cycling_.init( energy_consumption_radio_, wiselib_timer_, wiselib_clock_, wiselib_debug_ );
      ants_duty_cycling_.set_configuration();
      ants_duty_cycling_.reg_changed_callback<WiselibDutyCyclingProcessor,
         &WiselibDutyCyclingProcessor::energy_preservation>(this);
   }
   // ----------------------------------------------------------------------
   void
   WiselibDutyCyclingProcessor::
   work( void )
      throw()
   {}
   // ----------------------------------------------------------------------
   void
   WiselibDutyCyclingProcessor::
   energy_preservation( int value )
      throw()
   {
      if ( value == AntsDutyCycling::EPA_ACTIVE )
      {
         owner_w().write_simple_tag<double>( "VisBattery", 1.0 );
      }
      else if ( value == AntsDutyCycling::EPA_INACTIVE )
      {
         owner_w().write_simple_tag<double>( "VisBattery", 0.0 );
      }
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
