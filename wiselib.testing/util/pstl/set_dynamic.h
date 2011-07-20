
#ifndef SET_DYNAMIC_H
#define SET_DYNAMIC_H

#pragma warning("Dynamic set is not usable yet!")

#include "avl_tree.h"

namespace wiselib {
	
	template<
		typename OsModel_P,
		typename Value_P,
		typename Allocator_P
	>
	class set_dynamic {
		public:
			typedef Value_P value_type;
			typedef value_type* pointer;
			typedef value_type& reference;
			typedef set_dynamic<OsModel_P, Value_P, Allocator_P> set_type;
			typedef set_dynamic<OsModel_P, Value_P, Allocator_P> self_type;
			typedef self_type* self_pointer_t;
			typedef OsModel_P OsModel;
			typedef Allocator_P Allocator;
			typedef AVLTree<OsModel, Allocator> avl_t;
			
			typedef ... iterator;
			
			typedef typename OsModel_P::size_t size_type;
			typedef typename OsModel_P::size_t size_t;
			
			set_dynamic(Allocator::self_pointer_t alloc) : allocator(alloc) { }
			
			~set_dynamic() { }
			
			iterator begin() { }
			iterator end() { return iterator(); }
			
			size_type size() const { }
			
			size_type capacity() const { return (size_type)-1; }
			bool empty() const { return size() == 0; }
			
			
			
			iterator insert(iterator position, const value_type& v);
			iterator insert(const value_type& v) { return insert(begin(), v); }
			void insert(iterator position, size_type n, const value_type& v) {
				for(size_type i=0; i<n; ++i)
					insert(position, v);
			}
			
			/*
			
			iterator erase(iterator position);
			size_type erase(const value_type& v);
			iterator erase(iterator first, iterator last);
			
			*/
			
			iterator find(const value_type& v);
			size_type count(const value_type& v);
			
			void clear();
			
		private:
			AVLTree tree;
	};
	
} // namespace

#endif // SET_DYNAMIC_H

