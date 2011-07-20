/************************************************************************
 ** This file is part of the network simulator Shawn.                  **
 ** Copyright (C) 2004-2007 by the SwarmNet (www.swarmnet.de) project  **
 ** Shawn is free software; you can redistribute it and/or modify it   **
 ** under the terms of the BSD License. Refer to the shawn-licence.txt **
 ** file in the root of the Shawn source tree for further details.     **
 ************************************************************************/
#ifndef __SHAWN_LEGACYAPPS_WISELIB_EXAMPLES_INIT_H__
#define __SHAWN_LEGACYAPPS_WISELIB_EXAMPLES_INIT_H__

#include "_legacyapps_enable_cmake.h"
#ifdef ENABLE_WISELIB_EXAMPLES

namespace shawn
{ 
	class SimulationController; 
}

/** This method is invoked by Shawn to initialize the current module
  */
extern "C" void init_wiselib_examples( shawn::SimulationController& );


#endif
#endif
