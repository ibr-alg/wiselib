/************************************************************************
 ** This file is part of the network simulator Shawn.                  **
 ** Copyright (C) 2004-2007 by the SwarmNet (www.swarmnet.de) project  **
 ** Shawn is free software; you can redistribute it and/or modify it   **
 ** under the terms of the BSD License. Refer to the shawn-licence.txt **
 ** file in the root of the Shawn source tree for further details.     **
 ************************************************************************/
#include "legacyapps/wiselib_examples/wiselib_init.h"
#ifdef ENABLE_WISELIB_EXAMPLES

#include <iostream>
#include "sys/simulation/simulation_controller.h"
#include "legacyapps/wiselib_examples/tubs_hallway_debug_processor_factory.h"
#include "legacyapps/wiselib_examples/tubs_hallway_dutycycling_processor_factory.h"
#include "legacyapps/wiselib_examples/localization_components_processor_factory.h"

#include "sys/simulation/simulation_controller.h"

extern "C" void init_wiselib_examples( shawn::SimulationController& sc )
{
   std::cout << "Initialising Wiselib-Examples module" << std::endl;
   wiselib_examples::TubsHallwayDebugProcessorFactory::register_factory( sc );
   wiselib_examples::TubsHallwayDutycyclingProcessorFactory::register_factory( sc );
   wiselib_examples::LocalizationComponentsProcessorFactory::register_factory( sc );
}

#endif
