/************************************************************************
 ** This file is part of the network simulator Shawn.                  **
 ** Copyright (C) 2004,2005 by  SwarmNet (www.swarmnet.de)             **
 **                         and SWARMS   (www.swarms.de)               **
 ** Shawn is free software; you can redistribute it and/or modify it   **
 ** under the terms of the GNU General Public License, version 2.      **
 ************************************************************************/
#include "../buildfiles/_legacyapps_enable_cmake.h"
#ifdef ENABLE_DUTY_CYCLING

#include "sys/processors/processor_keeper.h"
#include "legacyapps/duty_cycling/wiselib_duty_cycling_processor_factory.h"
#include "legacyapps/duty_cycling/wiselib_duty_cycling_processor.h"
#include "sys/simulation/simulation_controller.h"

namespace duty_cycling
{

   void
   WiselibDutyCyclingProcessorFactory::
   register_factory( shawn::SimulationController& sc )
      throw()
   {
      sc.processor_keeper_w().add( new WiselibDutyCyclingProcessorFactory );
   }
   // ----------------------------------------------------------------------
   WiselibDutyCyclingProcessorFactory::
   WiselibDutyCyclingProcessorFactory()
   {}
   // ----------------------------------------------------------------------
   WiselibDutyCyclingProcessorFactory::
   ~WiselibDutyCyclingProcessorFactory()
   {}
   // ----------------------------------------------------------------------
   std::string
   WiselibDutyCyclingProcessorFactory::
   name( void )
      const throw()
   { return "wiselib_duty_cycling"; }
   // ----------------------------------------------------------------------
   std::string
   WiselibDutyCyclingProcessorFactory::
   description( void )
      const throw()
   {
      return "...";
   }
   // ----------------------------------------------------------------------
   shawn::Processor*
   WiselibDutyCyclingProcessorFactory::
   create( void )
      throw()
   {
      return new WiselibDutyCyclingProcessor;
   }


}

#endif
/*-----------------------------------------------------------------------
 * Source  $Source: /cvs/shawn/shawn/tubsapps/duty_cycling/duty_cycling_processor_factory.cpp,v $
 * Version $Revision: 1.3 $
 * Date    $Date: 2005/08/05 10:00:52 $
 *-----------------------------------------------------------------------
 * $Log: duty_cycling_processor_factory.cpp,v $
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
