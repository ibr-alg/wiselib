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

#ifndef IOS_FACET_PROVIDER_H
#define IOS_FACET_PROVIDER_H

#include "external_interface/facet_provider.h"
#include "external_interface/ios/ios_model.h"

namespace wiselib {
	template<typename Facet_P>
	class FacetProvider<iOsModel, Facet_P> {
		public:
			typedef iOsModel OsModel;
			typedef Facet_P Facet;
			
			static Facet& get_facet(typename OsModel::System& system) {
				if(!facet_) {
					facet_ = new Facet(system);
				}
				return *facet_;
			}
			
		private:
			static Facet *facet_;
	};
	
	template<typename Facet_P>
	typename FacetProvider<iOsModel, Facet_P>::Facet* FacetProvider<iOsModel, Facet_P>::facet_ = 0;
}

#endif // IOS_FACET_PROVIDER_H
