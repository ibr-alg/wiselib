
#ifndef NULL_DICTIONARY_H
#define NULL_DICTIONARY_H

namespace wiselib {
	
	/**
	 * This is a simple concrete dictionary on block_data_t*
	 * that doesn't actually store anything but
	 * instead returns the inserted values themselves as keys.
	 * 
	 * @ingroup ConcreteBDTDictionary_concept
	 */
	template<
		typename OsModel_P
	>
	class NullDictionary {
		public:
			typedef OsModel_P OsModel;
			typedef typename OsModel::block_data_t block_data_t;
			typedef typename OsModel::size_t size_type;
			
			typedef block_data_t* mapped_type;
			typedef block_data_t* key_type;
			
			enum { ABSTRACT_KEYS = false };
			static const key_type NULL_KEY;
			
			key_type insert(mapped_type value) {
				size_type l = strlen((char*)value) + 1;
				key_type r = get_allocator().allocate_array<block_data_t>(l) .raw();
				memcpy(r, value, l);
				return r;
			}
			size_type erase(key_type key) {
				get_allocator().free_array<char>((char*)key);
				return 1;
			} 
			key_type find(mapped_type value) { return value; }
			mapped_type get(key_type key) { return key; }
			mapped_type get_value(key_type key) { return key; }
			
			void free_value(mapped_type m) { }
			
			void init(typename OsModel::Debug::self_pointer_t debug_) { }
	};
	
	template<
		typename OsModel_P
	>
	const typename NullDictionary<OsModel_P>::key_type
	NullDictionary<OsModel_P>::NULL_KEY =
	typename NullDictionary<OsModel_P>::key_type();
	
} // namespace wiselib

#endif

