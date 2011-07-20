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
#ifndef __WISELIBSCHOOL_PROCESSOR_FACTORY_H
#define __WISELIBSCHOOL_PROCESSOR_FACTORY_H

#include "sys/processors/processor_factory.h"

namespace shawn
{
   class SimulationController;
   class Processor;
}

namespace wiselib
{

   class WiselibShawnStandaloneProcessorFactory
      : public shawn::ProcessorFactory
   {
      public:
         WiselibShawnStandaloneProcessorFactory();
         virtual ~WiselibShawnStandaloneProcessorFactory();

         virtual std::string name( void ) const throw();
         virtual std::string description( void ) const throw();
         virtual shawn::Processor* create( void ) throw();

         static void register_factory( shawn::SimulationController& ) throw();
   };

}

#endif
