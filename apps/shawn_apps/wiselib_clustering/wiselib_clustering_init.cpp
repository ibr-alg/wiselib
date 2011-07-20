/************************************************************************
 ** This file is part of the network simulator Shawn.                  **
 ** Copyright (C) 2004-2007 by the SwarmNet (www.swarmnet.de) project  **
 ** Shawn is free software; you can redistribute it and/or modify it   **
 ** under the terms of the BSD License. Refer to the shawn-licence.txt **
 ** file in the root of the Shawn source tree for further details.     **
 ************************************************************************/
#include "legacyapps/wiselib_clustering/wiselib_clustering_init.h"
#ifdef ENABLE_WISELIBCLUSTERING

#include <iostream>
#include "sys/simulation/simulation_controller.h"
#include "legacyapps/wiselib_clustering/wiselib_clustering_processor_factory.h"

#include "sys/simulation/simulation_controller.h"

extern "C" void init_wiselibclustering( shawn::SimulationController& sc )
{
   std::cout << "Initialising Wiselib-Shawn module" << std::endl;
   wiselib::WiselibClusteringProcessorFactory::register_factory( sc );
}

#endif
