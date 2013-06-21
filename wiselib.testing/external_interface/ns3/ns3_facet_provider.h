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
#ifndef __EXTERNAL_INTERFACE_NS3_FACET_PROVIDER_H__
#define __EXTERNAL_INTERFACE_NS3_FACET_PROVIDER_H__


#include "external_interface/facet_provider.h"
#include "external_interface/ns3/ns3_os.h"

namespace wiselib
{

   template<typename Facet_P>
   class FacetProvider<Ns3OsModel, Facet_P>
   {
   public:
      typedef Ns3OsModel OsModel;
      typedef Facet_P Facet;
      // --------------------------------------------------------------------
      static Facet& get_facet( Ns3Os& os )
      {
         return *(new Facet(os));
      }

   };
}

#endif
