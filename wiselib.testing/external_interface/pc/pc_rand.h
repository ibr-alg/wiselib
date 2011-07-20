// vim: set noexpandtab ts=4 sw=4:

#ifndef PC_RAND_H
#define PC_RAND_H

#include "external_interface/pc/pc_os.h"
#include <boost/random.hpp>

namespace wiselib {
	
	template<typename OsModel_P>
	class PCRandModel {
		public:
			typedef OsModel_P OsModel;
			typedef uint32_t value_t;
			
			typedef PCRandModel<OsModel> self_type;
			typedef self_type* self_pointer_t;
			
			enum { RANDOM_MAX = 0xffffffff };
			
			enum States
			{
				READY = OsModel::READY,
				NO_VALUE = OsModel::NO_VALUE,
				INACTIVE = OsModel::INACTIVE
			};
			
			PCRandModel();
			PCRandModel(PCOs& os);
			PCRandModel(PCOs& os, value_t seed);
			void srand(value_t seed);
			value_t operator()();
			int state();
			
		private:
			boost::uniform_int<uint32_t> distribution_;
			boost::mt19937 generator_;
			boost::variate_generator<
				boost::mt19937&, boost::uniform_int<uint32_t>
			> die_;
	};
	
	template<typename OsModel_P>
	PCRandModel<OsModel_P>::
	PCRandModel() :
		distribution_(0, RAND_MAX),
		generator_(),
		die_(generator_, distribution_)
	{
	}
	
	template<typename OsModel_P>
	PCRandModel<OsModel_P>::
	PCRandModel(PCOs& os) :
		distribution_(0, RAND_MAX),
		generator_(),
		die_(generator_, distribution_)
	{
	}
	
	template<typename OsModel_P>
	PCRandModel<OsModel_P>::
	PCRandModel(PCOs& os, value_t seed) :
		distribution_(0, RAND_MAX),
		generator_(seed),
		die_(generator_, distribution_)
	{
	}
	
	template<typename OsModel_P>
	void PCRandModel<OsModel_P>::
	srand(value_t seed) {
		generator_.seed(seed);
	}
	
	template<typename OsModel_P>
	typename PCRandModel<OsModel_P>::value_t PCRandModel<OsModel_P>::
	operator()() {
		return die_();
	}
	
	template<typename OsModel_P>
	int PCRandModel<OsModel_P>::
	state() {
		return READY;
	}
	
}; // ns wiselib

#endif // PC_RAND_H

