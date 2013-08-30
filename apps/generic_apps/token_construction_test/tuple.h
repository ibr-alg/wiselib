
#include <util/meta.h>


/**
 * @ingroup Tuple_concept
 */
template<typename OsModel_P>
class Tuple {
	// {{{
	public:
		typedef OsModel_P OsModel;
		typedef typename OsModel::block_data_t block_data_t;
		typedef typename OsModel::size_t size_type;
		
		typedef typename Uint< Max< sizeof(block_data_t*), 4 >::value >::t value_t;

		enum {
			SIZE = 3
		};

		typedef Tuple self_type;

		Tuple() {
			for(size_type i = 0; i < SIZE; i++) {
				data_[i] = 0;
			}
		}

		void free_deep(size_type i) {
			//delete[] spo_[i];
			//free(spo_[i]);
			::get_allocator().free(get(i));
			set(i, 0);
		}
		void destruct_deep() {
			for(size_type i = 0; i < SIZE; i++) { free_deep(i); }
		}

		// operator= as default

		const block_data_t* get(size_type i) const {
			return *reinterpret_cast<block_data_t* const *>(data_ + i);
		}
		block_data_t* get(size_type i) {
			return *reinterpret_cast<block_data_t**>(data_ + i);
		}
		size_type length(size_type i) {
			//DBG("length(%s)=%d", (char*)spo_[i], strlen((char*)spo_[i]));
			return get(i) ? strlen((char*)get(i)) : 0;
		}
		
		void set_key(size_type i, ::uint32_t k) {
			data_[i] = 0;
			// TODO
			*reinterpret_cast< ::uint32_t*>(data_ + i) = k;
			//data_[i] = k;
		}
		
		::uint32_t get_key(size_type i) const {
			// TODO
			return *reinterpret_cast<const ::uint32_t*>(data_ + i);
		}
		
		void set(size_type i, block_data_t* data) {
			data_[i] = 0;
			*reinterpret_cast<block_data_t**>(data_ + i) = data;
		}
		void set_deep(size_type i, block_data_t* data) {
			size_type l = strlen((char*)data) + 1;
			//spo_[i] = new block_data_t[l];
			set(i, ::get_allocator().template allocate_array<block_data_t>(l * sizeof(block_data_t)) .raw());
				//(block_data_t*)malloc(l * sizeof(block_data_t));
			memcpy(get(i), data, l);
		}
		
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
	
		bool operator==(const self_type& other) const { return cmp(other) == 0; }
		bool operator>(const self_type& other) const { return cmp(other) > 0; }
		bool operator<(const self_type& other) const { return cmp(other) < 0; }
		bool operator>=(const self_type& other) const { return cmp(other) >= 0; }
		bool operator<=(const self_type& other) const { return cmp(other) <= 0; }
		
	#if (DEBUG_GRAPHVIZ || DEBUG_OSTREAM)
		friend std::ostream& operator<<(std::ostream& os, const Tuple& t) {
			//os << "(" << (void*)t.spo_[0] << " " << (void*)t.spo_[1] << " " << (void*)t.spo_[2] << ")";
			os << "(" << std::hex() << data_[0] << " " << data_[1] << " " << data_[2] << ")";
			return os;
		}
	#endif
		
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
		
		//block_data_t *spo_[SIZE];
		//char spo_[100][3];
	// }}}
};

