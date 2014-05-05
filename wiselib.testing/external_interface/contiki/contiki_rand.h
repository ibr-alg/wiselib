
#ifndef CONTIKI_RAND_H
#define CONTIKI_RAND_H

extern "C" {
	#include <random.h>
}

namespace wiselib {
	template<
		typename OsModel_P
	>
	class ContikiRandModel {
		public:
			typedef OsModel_P OsModel;
			typedef typename OsModel::block_data_t block_data_t;
			typedef typename OsModel::size_t size_type;
			typedef ::uint16_t rand_t;
			typedef ::uint16_t value_t;
			typedef ContikiRandModel self_type;
			typedef self_type* self_pointer_t;

			enum { RANDOM_MAX = RANDOM_RAND_MAX };

			ContikiRandModel() {
			}

			void init() {
			}

			void srand(value_t seed) {
				random_init(seed);
			}

			value_t operator()() {
				return random_rand();
			}
	};
} // namespace

#endif // CONTIKI_RAND_H

