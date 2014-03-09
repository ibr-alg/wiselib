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
#ifndef __EXTERNAL_INTERFACE_CONTIKI_FACET_PROVIDER_H__
#define __EXTERNAL_INTERFACE_CONTIKI_FACET_PROVIDER_H__

#include "project-conf.h"

#include "external_interface/facet_provider.h"
#include "external_interface/contiki/contiki_os.h"

namespace wiselib
{

   class ContikiOsModel;

   template<typename Facet_P>
   class FacetProvider<ContikiOsModel, Facet_P>
   {
   public:
      typedef ContikiOsModel OsModel;
      typedef Facet_P Facet;
      // --------------------------------------------------------------------
      static Facet& get_facet( ContikiOsModel& os )
      {
         facet_.init();
         return facet_;
      }

   private:
      static Facet facet_;
   };
   // -----------------------------------------------------------------------
   // -----------------------------------------------------------------------
   // -----------------------------------------------------------------------
   template<typename Facet_P>
   typename FacetProvider<ContikiOsModel, Facet_P>::Facet
      FacetProvider<ContikiOsModel, Facet_P>::facet_ =
      typename FacetProvider<ContikiOsModel, Facet_P>::Facet();

}

#endif
