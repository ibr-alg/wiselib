
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
			typedef typename OsModel::size_t size_type;
			typedef ::uint32_t hash_t;
			
			enum { MAX_VALUE = (hash_t)(-1) };
			
			static hash_t hash(const block_data_t *s, size_type l) {
				hash_t hashval = 0x811c9dc5UL;
				hash_t magicprime = 0x1000193UL;
				const block_data_t *end = s + l;
				for( ; s != end; s++) {
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
			typedef typename OsModel::size_t size_type;
			typedef ::uint64_t hash_t;
			
			enum { MAX_VALUE = (hash_t)(-1) };
			
			static hash_t hash(const block_data_t *s, size_type l) {
				hash_t hashval = 0xcbf29ce484222325ULL;
				hash_t magicprime = 0x00000100000001b3ULL;
				const block_data_t *end = s + l;
				for( ; s != end; s++) {
					hashval ^= *s;
					hashval *= magicprime;
				}
				return hashval;
			}
		
	};
	
}

#endif // FNV_H

