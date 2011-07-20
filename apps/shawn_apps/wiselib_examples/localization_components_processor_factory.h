/************************************************************************
 ** This file is part of the network simulator Shawn.                  **
 ** Copyright (C) 2004-2007 by the SwarmNet (www.swarmnet.de) project  **
 ** Shawn is free software; you can redistribute it and/or modify it   **
 ** under the terms of the BSD License. Refer to the shawn-licence.txt **
 ** file in the root of the Shawn source tree for further details.     **
 ************************************************************************/
#ifndef __SHAWN_LEGACYAPPS_WISELIB_EXAMPLES_LOCALIZATION_COMPONENTS_PROCESSOR_FACTORY_H
#define __SHAWN_LEGACYAPPS_WISELIB_EXAMPLES_LOCALIZATION_COMPONENTS_PROCESSOR_FACTORY_H
#include "_legacyapps_enable_cmake.h"
#ifdef ENABLE_WISELIB_EXAMPLES

#include "sys/processors/processor_factory.h"

namespace shawn
{
   class SimulationController;
   class Processor;
}

namespace wiselib_examples
{

   class LocalizationComponentsProcessorFactory
      : public shawn::ProcessorFactory
   {
      public:
         LocalizationComponentsProcessorFactory();
         virtual ~LocalizationComponentsProcessorFactory();

         virtual std::string name( void ) const throw();
         virtual std::string description( void ) const throw();
         virtual shawn::Processor* create( void ) throw();

         static void register_factory( shawn::SimulationController& ) throw();
   };

}

#endif
#endif
