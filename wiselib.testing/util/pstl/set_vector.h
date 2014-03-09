
#ifndef SET_VECTOR_H
#define SET_VECTOR_H

namespace wiselib {
	
	/**
	 * insert: O(1)
	 * find: O(n)
	 * erase: O(1)
	 */
	template<
		typename OsModel_P,
		typename Vector_P
	>
	class set_vector {
		public:
			typedef OsModel_P OsModel;
			typedef Vector_P vector_t;
			typedef typename vector_t::value_type value_type;
			typedef typename vector_t::iterator iterator;
			
			typedef typename OsModel::size_t size_type;
			
			iterator begin() { return vector_.begin(); }
			
			iterator end() { return vector_.end(); }
			
			size_type size() { return vector_.size(); }
			
			bool empty() { return size() == 0; }
			
			iterator insert(value_type& v) {
				iterator it = find(v);
				if(it != end()) { return it; }
				
				vector_.push_back(v);
				return end() - (typename OsModel::size_t)1;
			}
			
			iterator find(value_type& v) { return vector_.find(v); }
			
			size_type count(value_type& v) { return find(v) != end(); }
			
			iterator erase(iterator& iter) {
				//if(iter != (end() - (typename OsModel::size_t)1)) {
					*iter = *(end() - (typename OsModel::size_t)1);
				//}
				size_type p = iter - vector_.begin();
				vector_.pop_back();
				iter = vector_.begin() + p;
				return iter;
			}
			
			void clear() {
				vector_.clear();
			}
			
		private:
			vector_t vector_;
	};
}

#endif // SET_VECTOR_SET_H

/* vim: set ts=3 sw=3 noexpandtab :*/
