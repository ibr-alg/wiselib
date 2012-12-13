
#ifndef SIMPLEHASH32_H
#define SIMPLEHASH32_H

namespace wiselib {

	template<
		typename OsModel_P
	>
	class SimpleHash32 {
		public:
			typedef OsModel_P OsModel;
			typedef typename OsModel::block_data_t block_data_t;
			typedef ::uint32_t hash_t;
			
			enum { MAX_VALUE = (hash_t)(-1) };
			
			hash_t hash(block_data_t *s) {
				hash_t r = 0;
				for(size_type i=0; s[i]; i++) {
					
				}
			}
	};
}

#endif // SIMPLEHASH32_H

