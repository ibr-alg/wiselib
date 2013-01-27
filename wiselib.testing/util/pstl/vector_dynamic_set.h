
#ifndef VECTOR_DYNAMIC_SET_H
#define VECTOR_DYNAMIC_SET_H

#include "vector_dynamic.h"

namespace wiselib {
	
	/**
	 * insert: O(1)
	 * find: O(n)
	 * erase: O(1)
	 */
	template<
		typename OsModel_P,
		typename Value_P
	>
	class vector_dynamic_set {
		public:
			typedef OsModel_P OsModel;
			typedef Value_P value_type;
			
			typedef vector_dynamic<OsModel, value_type> vector_t;
			typedef typename vector_t::iterator iterator;
			
			typedef typename OsModel::size_t size_type;
			
			iterator begin() { return vector_.begin(); }
			
			iterator end() { return vector_.end(); }
			
			size_type size() { return vector_.size(); }
			
			bool empty() { return size() == 0; }
			
			iterator insert(value_type& v) {
				vector_.push_back(v);
				return end() - (typename OsModel::size_t)1;
			}
			
			iterator find(value_type& v) { return vector_.find(v); }
			
			size_type count(value_type& v) { return find(v) != end(); }
			
			iterator erase(iterator& iter) {
				//if(iter != (end() - (typename OsModel::size_t)1)) {
					*iter = *(end() - (typename OsModel::size_t)1);
				//}
				vector_.pop_back();
				return iter;
			}
			
			
		private:
			vector_t vector_;
	};
}

#endif // VECTOR_DYNAMIC_SET_H

/* vim: set ts=3 sw=3 noexpandtab :*/
