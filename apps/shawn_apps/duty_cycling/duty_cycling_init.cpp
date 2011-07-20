/************************************************************************
 ** This file is part of the network simulator Shawn.                  **
 ** Copyright (C) 2004,2005 by  SwarmNet (www.swarmnet.de)             **
 **                         and SWARMS   (www.swarms.de)               **
 ** Shawn is free software; you can redistribute it and/or modify it   **
 ** under the terms of the GNU General Public License, version 2.      **
 ************************************************************************/
#include "../buildfiles/_legacyapps_enable_cmake.h"
#ifdef ENABLE_DUTY_CYCLING

#include "legacyapps/duty_cycling/duty_cycling_init.h"
#include "legacyapps/duty_cycling/duty_cycling_task.h"
#include "legacyapps/duty_cycling/duty_cycling_processor_factory.h"
#include "legacyapps/duty_cycling/wiselib_duty_cycling_processor_factory.h"
#include "sys/simulation/simulation_controller.h"
#include "sys/simulation/simulation_task_keeper.h"

using namespace duty_cycling;

extern "C" void init_duty_cycling( shawn::SimulationController& sc )
{
   DutyCyclingProcessorFactory::register_factory( sc );
   WiselibDutyCyclingProcessorFactory::register_factory( sc );
   sc.simulation_task_keeper_w().add( new DutyCyclingTask );
}


#endif
/*-----------------------------------------------------------------------
* Source  $Source: /cvs/shawn/shawn/tubsapps/duty_cycling/duty_cycling_init.cpp,v $
* Version $Revision: 1.2 $
* Date    $Date: 2005/08/05 10:00:52 $
*-----------------------------------------------------------------------
* $Log: duty_cycling_init.cpp,v $
* Revision 1.2  2005/08/05 10:00:52  ali
* 2005 copyright notice
*
* Revision 1.1  2005/06/09 15:28:08  tbaum
* added module functionality
*
*-----------------------------------------------------------------------*/
