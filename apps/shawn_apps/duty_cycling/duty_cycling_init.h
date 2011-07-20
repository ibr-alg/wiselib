/************************************************************************
 ** This file is part of the network simulator Shawn.                  **
 ** Copyright (C) 2004,2005 by  SwarmNet (www.swarmnet.de)             **
 **                         and SWARMS   (www.swarms.de)               **
 ** Shawn is free software; you can redistribute it and/or modify it   **
 ** under the terms of the GNU General Public License, version 2.      **
 ************************************************************************/

#ifndef __SHAWN_LEGACYAPPS_DUTY_CYCLING_INIT_H
#define __SHAWN_LEGACYAPPS_DUTY_CYCLING_INIT_H
#include "../buildfiles/_legacyapps_enable_cmake.h"
#ifdef ENABLE_DUTY_CYCLING

namespace shawn {class SimulationController;}

extern "C" void init_duty_cycling( shawn::SimulationController& );


#endif
#endif
/*-----------------------------------------------------------------------
 * Source  $Source: /cvs/shawn/shawn/tubsapps/duty_cycling/duty_cycling_init.h,v $
 * Version $Revision: 1.2 $
 * Date    $Date: 2005/08/05 10:00:52 $
 *-----------------------------------------------------------------------
 * $Log: duty_cycling_init.h,v $
 * Revision 1.2  2005/08/05 10:00:52  ali
 * 2005 copyright notice
 *
 * Revision 1.1  2005/06/09 15:28:09  tbaum
 * added module functionality
 *
 *-----------------------------------------------------------------------*/
