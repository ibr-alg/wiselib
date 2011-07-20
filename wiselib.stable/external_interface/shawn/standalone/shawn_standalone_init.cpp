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
#include "external_interface/shawn/standalone/shawn_standalone_init.h"

#include <iostream>
#include "sys/simulation/simulation_controller.h"
#include "external_interface/shawn/standalone/shawn_standalone_processor_factory.h"

void init_external_app( shawn::SimulationController& sc )
{
   std::cout << "Initialising Wiselib-Shawn-Standalone module" << std::endl;
   wiselib::WiselibShawnStandaloneProcessorFactory::register_factory( sc );
}
