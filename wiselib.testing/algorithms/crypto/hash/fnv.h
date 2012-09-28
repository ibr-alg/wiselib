
#ifndef FNV_H
#define FNV_H

namespace wiselib {

	template<
		typename OsModel_P
	>
	class Fnv32 {
		public:
			typedef OsModel_P OsModel;
			typedef typename OsModel::block_data_t block_data_t;
			typedef ::uint32_t hash_t;
			
			enum { MAX_VALUE = (hash_t)(-1) };
			
			static hash_t hash(block_data_t *s) {
				hash_t hashval = 0x811c9dc5UL;
				hash_t magicprime = 0x1000193UL;
				for( ; *s; s++) {
					hashval ^= *s;
					hashval *= magicprime;
				}
				return hashval;
			}
		
	};
	
	template<
		typename OsModel_P
	>
	class Fnv64 {
		public:
			typedef OsModel_P OsModel;
			typedef typename OsModel::block_data_t block_data_t;
			typedef ::uint64_t hash_t;
			
			enum { MAX_VALUE = (hash_t)(-1) };
			
			static hash_t hash(block_data_t *s) {
				hash_t hashval = 0xcbf29ce484222325ULL;
				hash_t magicprime = 0x00000100000001b3ULL;
				for( ; *s; s++) {
					hashval ^= *s;
					hashval *= magicprime;
				}
				return hashval;
			}
		
	};
	
}

#endif // FNV_H

