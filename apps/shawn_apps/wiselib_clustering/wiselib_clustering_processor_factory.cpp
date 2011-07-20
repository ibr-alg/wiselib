/************************************************************************
 ** This file is part of the network simulator Shawn.                  **
 ** Copyright (C) 2004-2007 by the SwarmNet (www.swarmnet.de) project  **
 ** Shawn is free software; you can redistribute it and/or modify it   **
 ** under the terms of the BSD License. Refer to the shawn-licence.txt **
 ** file in the root of the Shawn source tree for further details.     **
 ************************************************************************/
#include "_legacyapps_enable_cmake.h"
#ifdef ENABLE_WISELIBCLUSTERING

#include "legacyapps/wiselib_clustering/wiselib_clustering_processor_factory.h"
#include "legacyapps/wiselib_clustering/wiselib_clustering_processor.h"
#include "sys/processors/processor_keeper.h"
#include "sys/simulation/simulation_controller.h"
#include <iostream>

using namespace std;
using namespace shawn;

namespace wiselib
{

   // ----------------------------------------------------------------------
   void
   WiselibClusteringProcessorFactory::
   register_factory( SimulationController& sc )
   throw()
   {
      sc.processor_keeper_w().add( new WiselibClusteringProcessorFactory );
   }
   // ----------------------------------------------------------------------
   WiselibClusteringProcessorFactory::
   WiselibClusteringProcessorFactory()
   {}
   // ----------------------------------------------------------------------
   WiselibClusteringProcessorFactory::
   ~WiselibClusteringProcessorFactory()
   {}
   // ----------------------------------------------------------------------
   std::string
   WiselibClusteringProcessorFactory::
   name( void )
   const throw()
   {
      return "wiselibclustering";
   }
   // ----------------------------------------------------------------------
   std::string
   WiselibClusteringProcessorFactory::
   description( void )
   const throw()
   {
      return "Wiselib Clustering Processor";
   }
   // ----------------------------------------------------------------------
   shawn::Processor*
   WiselibClusteringProcessorFactory::
   create( void )
   throw()
   {
      return new WiselibClusteringProcessor;
   }
}

#endif
