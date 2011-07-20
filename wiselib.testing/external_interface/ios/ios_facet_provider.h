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
