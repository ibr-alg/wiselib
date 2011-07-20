/************************************************************************
 ** This file is part of the network simulator Shawn.                  **
 ** Copyright (C) 2004,2005 by  SwarmNet (www.swarmnet.de)             **
 **                         and SWARMS   (www.swarms.de)               **
 ** Shawn is free software; you can redistribute it and/or modify it   **
 ** under the terms of the GNU General Public License, version 2.      **
 ************************************************************************/
#include "../buildfiles/_legacyapps_enable_cmake.h"
#ifdef ENABLE_DUTY_CYCLING

#ifndef __SHAWN_LEGACYAPPS_DUTY_CYCLING_PROCESSOR_FACTORY_H
#define __SHAWN_LEGACYAPPS_DUTY_CYCLING_PROCESSOR_FACTORY_H

#include "sys/processors/processor_factory.h"
#include "sys/simulation/simulation_controller.h"


namespace duty_cycling
{

   class DutyCyclingProcessorFactory
      : public shawn::ProcessorFactory
   {
   public:
      DutyCyclingProcessorFactory();
      virtual ~DutyCyclingProcessorFactory();

      virtual std::string name( void ) const throw();
      virtual std::string description( void ) const throw();
      virtual shawn::Processor* create( void ) throw();

      static void register_factory( shawn::SimulationController& ) throw();
   };

}

#endif
#endif
/*-----------------------------------------------------------------------
 * Source  $Source: /cvs/shawn/shawn/tubsapps/duty_cycling/duty_cycling_processor_factory.h,v $
 * Version $Revision: 1.3 $
 * Date    $Date: 2005/08/05 10:00:52 $
 *-----------------------------------------------------------------------
 * $Log: duty_cycling_processor_factory.h,v $
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
