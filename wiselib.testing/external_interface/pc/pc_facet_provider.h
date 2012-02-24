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
