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

// vim: set noexpandtab ts=4 sw=4:

#ifndef PC_FACET_PROVIDER_H
#define PC_FACET_PROVIDER_H

#include "external_interface/facet_provider.h"
#include "external_interface/pc/pc_os_model.h"

namespace wiselib {
	template<typename Facet_P>
	class FacetProvider<PCOsModel, Facet_P> {
		public:
			typedef PCOsModel OsModel;
			typedef Facet_P Facet;
			
			static Facet& get_facet(OsModel& os) {
				if(!facet_) {
					facet_ = new Facet();
				}
				return *facet_;
			}
			
		private:
			static Facet *facet_;
	};
	
	template<typename Facet_P>
	typename FacetProvider<PCOsModel, Facet_P>::Facet* FacetProvider<PCOsModel, Facet_P>::facet_ = 0;
}

#endif // PC_FACET_PROVIDER_H
