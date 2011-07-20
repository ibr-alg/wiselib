/***************************************************************************
 ** This file is part of the generic algorithm library Wiselib.           **
 ** Copyright (C) 2008,2009 by the Wisebed (www.wisebed.eu) project.      **
 **                                                                       **
 ** The Wiselib is free software: you can redistribute it and/or modify   **
 ** it under the terms of the GNU Lesser General Public License as        **
 ** published by the Free Software Foundation, either version 3 of the    **
 ** License, or (at your option) any later version.                       **
 **                                                                       **
 ** The Wiselib is distributed in the hope that it will be useful,        **
 ** but WITHOUT ANY WARRANTY; without even the implied warranty of        **
 ** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         **
 ** GNU Lesser General Public License for more details.                   **
 **                                                                       **
 ** You should have received a copy of the GNU Lesser General Public      **
 ** License along with the Wiselib.                                       **
 ** If not, see <http://www.gnu.org/licenses/>.                           **
 ***************************************************************************/

#include "external_interface/shawn/standalone/shawn_standalone_processor_factory.h"
#include "external_interface/shawn/standalone/shawn_standalone_processor.h"
#include "sys/processors/processor_keeper.h"
#include "sys/simulation/simulation_controller.h"
#include <iostream>

using namespace std;
using namespace shawn;

namespace wiselib
{

   // ----------------------------------------------------------------------
   void
   WiselibShawnStandaloneProcessorFactory::
   register_factory( SimulationController& sc )
   throw()
   {
      sc.processor_keeper_w().add( new WiselibShawnStandaloneProcessorFactory );
   }
   // ----------------------------------------------------------------------
   WiselibShawnStandaloneProcessorFactory::
   WiselibShawnStandaloneProcessorFactory()
   {}
   // ----------------------------------------------------------------------
   WiselibShawnStandaloneProcessorFactory::
   ~WiselibShawnStandaloneProcessorFactory()
   {}
   // ----------------------------------------------------------------------
   std::string
   WiselibShawnStandaloneProcessorFactory::
   name( void )
   const throw()
   {
      return "wiselib_shawn_standalone";
   }
   // ----------------------------------------------------------------------
   std::string
   WiselibShawnStandaloneProcessorFactory::
   description( void )
   const throw()
   {
      return "Wiselib Shawn Standalone Processor";
   }
   // ----------------------------------------------------------------------
   shawn::Processor*
   WiselibShawnStandaloneProcessorFactory::
   create( void )
   throw()
   {
      return new WiselibShawnStandaloneProcessor;
   }
}
