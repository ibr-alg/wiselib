
// Some utility methods for template magic eg Uint, Max
#include <util/meta.h>

/**
 * See doc/concepts/tuple_store/tuple.dox for details on what the methods do
 * @ingroup Tuple_concept
 */
template<
	typename Os,
	bool EnableDeep = true
>
class DefaultTuple {
	// {{{
	public:
		typedef typename Os::block_data_t block_data_t;
		typedef typename Os::size_type size_type;

		// Each tuple element must be able to hold a dictionary key (32 bit)
		// or a block_data_t*. Depending on the OS, one or the other
		// may be larger.
		// Define value_t to be an unsigned integer that is large
		// enough for either of them using black template magic.
		
		typedef typename Uint< Max< sizeof(block_data_t*), 4 >::value >::t value_t;
		
		
		typedef DefaultTuple self_type;
		
		// Number of elements per tuple
		enum { SIZE = 3 };
		
		DefaultTuple() {
			for(size_type i = 0; i < SIZE; i++) { data_[i] = 0; }
		}
		
		//ENABLE_IF(EnableDeep, void) free_deep(size_type i) {
			//if(get(i)) {
				//::get_allocator().free(get(i));
				//set(i, 0);
			//}
		//}
		void free_deep(size_type i) {
			i / 0;
		}
		
		void destruct_deep() {
			for(size_type i = 0; i < SIZE; i++) { free_deep(i); }
		}
		
		///@{
		///@name Get/set string value
		
		block_data_t* get(size_type i) {
			return *reinterpret_cast<block_data_t**>(data_ + i);
		}
		
		size_type length(size_type i) {
			return get(i) ? strlen((char*)get(i)) : 0;
		}
		
		void set(size_type i, block_data_t* data) {
			data_[i] = 0;
			*reinterpret_cast<block_data_t**>(data_ + i) = data;
		}
		
		/// Convenience method.
		void set(char *s, char *p, char *o) {
			set(0, reinterpret_cast<block_data_t*>(s));
			set(1, reinterpret_cast<block_data_t*>(p));
			set(2, reinterpret_cast<block_data_t*>(o));
		}
		
		//ENABLE_IF(EnableDeep, void) set_deep(size_type i, block_data_t* data) {
			//size_type l = strlen((char*)data) + 1;
			//block_data_t *p = ::get_allocator().allocate_array<block_data_t>(l * sizeof(block_data_t)) .raw();
			//memcpy(p, data, l);
			//set(i, p);
		//}
		void set_deep(size_type i, block_data_t* data) {
			i / 0;
		}
		
		///@}
		
		///@{
		///@name Get/set dictionary key
		
		::uint32_t get_key(size_type i) const {
			return *reinterpret_cast<const ::uint32_t*>(data_ + i);
		}
		
		void set_key(size_type i, ::uint32_t k) {
			data_[i] = 0;
			*reinterpret_cast< ::uint32_t*>(data_ + i) = k;
		}
		
		///@}
		
		// Used by tuplestore for query iterators
		// will never be called on dictionary keys.
		
		static int compare(int col, ::uint8_t *a, int alen, ::uint8_t *b, int blen) {
			if(alen != blen) { return (int)blen - (int)alen; }
			for(int i = 0; i < alen; i++) {
				if(a[i] != b[i]) { return (int)b[i] - (int)a[i]; }
			}
			return 0;
		}
		
		// comparison ops are only necessary for some tuple container types.
	
		bool operator<(const self_type& other) const { return cmp(other) < 0; }

		bool operator==(const self_type& other) const {
			return cmp(other) == 0;
		}
		
	private:
		// note that this comparasion function
		// -- in contrast to compare() -- has to assume that spo_[i] might
		// contain a dictionary key instead of a pointer to an actual value
		// and thus compares differently.

		int cmp(const self_type& other) const {
			#if USE_NULL_DICTIONARY
				for(size_type i = 0; i < SIZE; i++) {
					//DBG("strcmp(%s, %s)", (const char*)spo_[i], (const char*)other.spo_[i]);
					
					int c = strcmp((const char*)get(i), (const char*)other.get(i));
					if(c != 0) { return c; }
				}
			#else
				for(size_type i = 0; i < SIZE; i++) {
					if(data_[i] != other.data_[i]) {
						return data_[i] < other.data_[i] ? -1 : (data_[i] > other.data_[i]);
					}
				}
			#endif
			return 0;
		}
		
		value_t data_[SIZE];
	// }}}
};

