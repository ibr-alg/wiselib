

template<typename OsModel_P>
class Tuple {
   // {{{
   public:
      typedef OsModel_P OsModel;
      typedef typename OsModel::block_data_t block_data_t;
      typedef typename OsModel::size_t size_type;

      enum {
         SIZE = 3
      };

      typedef Tuple self_type;

      Tuple() {
         for(size_type i = 0; i < SIZE; i++) {
            spo_[i] = 0;
         }
      }

      void free_deep(size_type i) {
         //delete[] spo_[i];
         //free(spo_[i]);
         ::get_allocator().free(spo_[i]);
         spo_[i] = 0;
      }
      void destruct_deep() {
         for(size_type i = 0; i < SIZE; i++) { free_deep(i); }
      }

      // operator= as default

      block_data_t* get(size_type i) {
         return spo_[i];
      }
      size_type length(size_type i) {
         //DBG("length(%s)=%d", (char*)spo_[i], strlen((char*)spo_[i]));
         return spo_[i] ? strlen((char*)spo_[i]) : 0;
      }
      void set(size_type i, block_data_t* data) {
         spo_[i] = data;
      }
      void set_deep(size_type i, block_data_t* data) {
         size_type l = strlen((char*)data) + 1;
         //spo_[i] = new block_data_t[l];
         spo_[i] = ::get_allocator().template allocate_array<block_data_t>(l * sizeof(block_data_t)) .raw();
            //(block_data_t*)malloc(l * sizeof(block_data_t));
			memcpy(spo_[i], data, l);
		}
		
		static int compare(int col, ::uint8_t *a, int alen, ::uint8_t *b, int blen) {
			if(alen != blen) { return (int)blen - (int)alen; }
			for(int i = 0; i < alen; i++) {
				if(a[i] != b[i]) { return (int)b[i] - (int)a[i]; }
			}
			return 0;
		}
		
		// comparison ops are only necessary for some tuple container types.
	
		// note that this comparasion function
		// -- in contrast to compare() -- has to assume that spo_[i] might
		// contain a dictionary key instead of a pointer to an actual value
		// and thus compares differently.

		int cmp(const self_type& other) const {
			#if USE_NULL_DICTIONARY
				for(size_type i = 0; i < SIZE; i++) {
					//DBG("strcmp(%s, %s)", (const char*)spo_[i], (const char*)other.spo_[i]);
					
					int c = strcmp((const char*)spo_[i], (const char*)other.spo_[i]);
					if(c != 0) { return c; }
				}
			#else
				for(size_type i = 0; i < SIZE; i++) {
					if(spo_[i] != other.spo_[i]) {
						return spo_[i] < other.spo_[i] ? -1 : (spo_[i] > other.spo_[i]);
					}
				}
			#endif
			return 0;
		}
		
		bool operator==(const self_type& other) const { return cmp(other) == 0; }
		bool operator>(const self_type& other) const { return cmp(other) > 0; }
		bool operator<(const self_type& other) const { return cmp(other) < 0; }
		bool operator>=(const self_type& other) const { return cmp(other) >= 0; }
		bool operator<=(const self_type& other) const { return cmp(other) <= 0; }
		
        #if (DEBUG_GRAPHVIZ || DEBUG_OSTREAM)
		friend std::ostream& operator<<(std::ostream& os, const Tuple& t) {
			os << "(" << (void*)t.spo_[0] << " " << (void*)t.spo_[1] << " " << (void*)t.spo_[2] << ")";
			return os;
		}	
        #endif
		
	private:
		block_data_t *spo_[SIZE];
		//char spo_[100][3];
	// }}}
};

