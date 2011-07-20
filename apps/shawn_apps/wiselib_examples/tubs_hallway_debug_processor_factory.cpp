/************************************************************************
 ** This file is part of the network simulator Shawn.                  **
 ** Copyright (C) 2004-2007 by the SwarmNet (www.swarmnet.de) project  **
 ** Shawn is free software; you can redistribute it and/or modify it   **
 ** under the terms of the BSD License. Refer to the shawn-licence.txt **
 ** file in the root of the Shawn source tree for further details.     **
 ************************************************************************/
#include "_legacyapps_enable_cmake.h"
#ifdef ENABLE_WISELIB_EXAMPLES

#include "legacyapps/wiselib_examples/tubs_hallway_debug_processor_factory.h"
#include "legacyapps/wiselib_examples/tubs_hallway_debug_processor.h"
#include "sys/processors/processor_keeper.h"
#include "sys/simulation/simulation_controller.h"
#include <iostream>

using namespace std;
using namespace shawn;

namespace wiselib_examples
{

   // ----------------------------------------------------------------------
   void
   TubsHallwayDebugProcessorFactory::
   register_factory( SimulationController& sc )
   throw()
   {
      sc.processor_keeper_w().add( new TubsHallwayDebugProcessorFactory );
   }
   // ----------------------------------------------------------------------
   TubsHallwayDebugProcessorFactory::
   TubsHallwayDebugProcessorFactory()
   {}
   // ----------------------------------------------------------------------
   TubsHallwayDebugProcessorFactory::
   ~TubsHallwayDebugProcessorFactory()
   {}
   // ----------------------------------------------------------------------
   std::string
   TubsHallwayDebugProcessorFactory::
   name( void )
   const throw()
   {
      return "hallway_debug";
   }
   // ----------------------------------------------------------------------
   std::string
   TubsHallwayDebugProcessorFactory::
   description( void )
   const throw()
   {
      return "Help debugging the TUBS sensor hallway.";
   }
   // ----------------------------------------------------------------------
   shawn::Processor*
   TubsHallwayDebugProcessorFactory::
   create( void )
   throw()
   {
      return new TubsHallwayDebugProcessor;
   }
}

#endif
