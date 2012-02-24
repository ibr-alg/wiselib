// vim: set noexpandtab ts=4 sw=4:

#ifndef PC_RAND_H
#define PC_RAND_H

#include <cstdlib>
#include "external_interface/pc/pc_os_model.h"

namespace wiselib {
	
	template<typename OsModel_P>
	class PCRandModel {
		public:
			typedef OsModel_P OsModel;
			typedef uint32_t value_t;
			
			typedef PCRandModel<OsModel> self_type;
			typedef self_type* self_pointer_t;
			
			enum { RANDOM_MAX = RAND_MAX };
			
			enum States
			{
				READY = OsModel::READY,
				NO_VALUE = OsModel::NO_VALUE,
				INACTIVE = OsModel::INACTIVE
			};
			
			PCRandModel();
			PCRandModel(value_t seed);
			void srand(value_t seed);
			value_t operator()();
			int state();
			
		private:
	};
	
	template<typename OsModel_P>
	PCRandModel<OsModel_P>::
	PCRandModel() {
	}
	
	template<typename OsModel_P>
	PCRandModel<OsModel_P>::
	PCRandModel(value_t seed) {
		::srand(seed);
	}
	
	template<typename OsModel_P>
	void PCRandModel<OsModel_P>::
	srand(value_t seed) {
		::srand(seed);
	}
	
	template<typename OsModel_P>
	typename PCRandModel<OsModel_P>::value_t PCRandModel<OsModel_P>::
	operator()() {
		return rand();
	}
	
	template<typename OsModel_P>
	int PCRandModel<OsModel_P>::
	state() {
		return READY;
	}
	
}; // ns wiselib

#endif // PC_RAND_H

