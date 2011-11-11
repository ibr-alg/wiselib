
#ifndef __UTIL_ALLOCATORS_UTIL_H__
#define __UTIL_ALLOCATORS_UTIL_H__

namespace wiselib {
	
	template<typename OsModel_P>
	void mem_copy(
			typename OsModel_P::block_data_t* target,
			const typename OsModel_P::block_data_t* source,
			typename OsModel_P::size_t n) {
		for(typename OsModel_P::size_t i=0; i<n; i++) {
			target[i] = source[i];
		}
	}
	
	
}

#endif // __UTIL_ALLOCATORS_UTIL_H__

