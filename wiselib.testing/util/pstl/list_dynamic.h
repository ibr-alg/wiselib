
#ifndef __WISELIB_UTIL_PSTL_LIST_DYNAMIC_H
#define __WISELIB_UTIL_PSTL_LIST_DYNAMIC_H

namespace wiselib {
	
	namespace {
		template<
			typename Value_P,
			typename Allocator_P
		>
		struct DoublyConnectedListNode {
			typedef DoublyConnectedListNode<Value_P, Allocator_P> self_type;
			typename Allocator_P::template pointer_t<self_type> next;
			typename Allocator_P::template pointer_t<self_type> prev;
			Value_P value;
			
			DoublyConnectedListNode() { }
		};
	
	
		template<
			typename List_P
		>
		class list_dynamic_iterator {
			public:
				typedef List_P List;
				typedef typename List::Allocator Allocator;
				typedef typename List::value_type value_type;
				typedef typename List::node_type node_type;
				typedef value_type& reference;
				typedef value_type* pointer;
				typedef list_dynamic_iterator<List> iterator_type;
				typedef list_dynamic_iterator<List> self_type;
				typedef typename Allocator::template pointer_t<node_type> node_pointer_t;
				
				list_dynamic_iterator() : list_(0) { }
				list_dynamic_iterator(List& l) : list_(&l) { }
				list_dynamic_iterator(List& l, const node_pointer_t node) : list_(&l), node_(node) { }
				list_dynamic_iterator(const self_type& other) : list_(other.list_), node_(other.node_) { }
				
				reference operator*() { return node_->value; }
				pointer operator->() { return &node_->value; }
				
				node_pointer_t node() { return node_; }
				List& list() { return *list_; }
				
				iterator_type& operator++() { node_ = node_->next; return *this; }
				
				iterator_type& operator--() {
					if(!node_) { node_ = list_->last(); }
					else { node_ = node_->prev; }
					return *this;
				}
				bool operator==(const iterator_type& other) const {
					return node_ == other.node_;
				}
				bool operator!=(const iterator_type& other) const {
					return node_ != other.node_;
				}
				
			private:
				List* list_;
				node_pointer_t node_;
		};
	} // ns
	
	
	
	
	/**
	 */
	template<
		typename OsModel_P,
		typename Value_P,
		typename Allocator_P
	>
	class list_dynamic {
		public:
			typedef DoublyConnectedListNode<Value_P, Allocator_P> Node_P;
			
			typedef OsModel_P OsModel;
			typedef typename OsModel::size_t size_t;
			typedef Value_P value_type;
			typedef value_type& reference;
			typedef const value_type& const_reference;
			typedef Allocator_P Allocator;
			typedef Node_P node_type;
			typedef typename Allocator::template pointer_t<node_type> node_pointer_t;
			typedef list_dynamic<OsModel_P, Value_P, Allocator_P> self_type;
			typedef list_dynamic_iterator<self_type> iterator;
			typedef list_dynamic_iterator<const self_type> const_iterator;
			
			list_dynamic() : allocator_(0) { };
			list_dynamic(Allocator& alloc) : allocator_(&alloc) { };
			list_dynamic(const list_dynamic& other) { *this = other; }
			
			~list_dynamic() {
				clear();
			}
			
			list_dynamic& operator=(const self_type& other) {
				// TODO: Implement copy-on-write
				allocator_ = other.allocator_;
				clear();
				for(self_type::const_iterator iter = other.begin(); iter != other.end(); ++iter) {
					push_back(*iter);
				}
				return *this;
			}
			
			void set_allocator(Allocator& alloc) { allocator_ = &alloc; }
			
			iterator begin() { return iterator(*this, first_node_); }
			iterator end() { return iterator(*this); }
			
			const_iterator begin() const { return const_iterator(*this, first_node_); }
			const_iterator end() const { return const_iterator(*this); }
			
			bool empty() const { return begin() == end(); }
			
			size_t size() const {
				size_t s = 0;
				for(const_iterator i(begin()); i != end(); ++i) {
					s++;
				}
				return s;
			}
			
			const value_type& front() const { return first_node_->value; }
			const value_type& back() const { return last_node_->value; }
			iterator back_iterator() { return iterator(*this, last_node_); }
			
			iterator insert(iterator iter, const_reference v) {
				node_pointer_t n = allocator_-> template allocate<node_type>();
				n->value = v;
				
				iterator before(iter), after(iter);
				--before;
				if(before.node()) { before.node()->next = n; }
				else { first_node_ = n; }
				
				if(after.node()) { after.node()->prev = n; }
				else { last_node_ = n; }
				
				n->prev = before.node();
				n->next = after.node();
				
				iterator new_iter(*this, n);
				return new_iter;
			}
			
			iterator push_back(value_type v) {
				return insert(end(), v);
			}
			iterator push_front(value_type v) {
				return insert(begin(), v);
			}
			
			void pop_back() { erase(iterator(*this, last_node_)); }
			
			iterator erase(iterator iter) {
				if(!iter.node()) { return iter; }
				if(iter.node() == first_node_) { first_node_ = iter.node()->next; }
				if(iter.node() == last_node_) { last_node_ = iter.node()->prev; }
				if(iter.node()->next) { iter.node()->next->prev = iter.node()->prev; }
				if(iter.node()->prev) { iter.node()->prev->next = iter.node()->next; }
				
				iterator n(*this, iter.node()->next);
				allocator_-> template free<node_type>(iter.node());
				return n;
			}
			
			void swap(self_type& other) {
				node_pointer_t tmp_first = first_node_, tmp_last = last_node_;
				typename Allocator::self_pointer_t tmp_alloc = allocator_;
				
				first_node_ = other.first_node_;
				last_node_ = other.last_node_;
				allocator_ = other.allocator_;
				
				other.first_node_ = tmp_first;
				other.last_node_ = tmp_last;
				other.allocator_ = tmp_alloc;
			}
			
			
			void clear() {
				iterator rm = begin();
				while(rm.node()) { rm = erase(rm); }
			}
			
			node_pointer_t first() const { return first_node_; }
			node_pointer_t last() const { return last_node_; }
			
		private:
			typename Allocator::self_pointer_t allocator_;
			node_pointer_t first_node_, last_node_;
	};
	
	
	template<
		typename OsModel_P,
		typename Allocator_P
	>
	struct maplist_adaptors {
		template<
			typename Value_P
		>
		class list_dynamic : public wiselib::list_dynamic<OsModel_P, Value_P, Allocator_P> {
		};
	};
	
} // ns

#endif // __WISELIB_UTIL_PSTL_LIST_DYNAMIC_H

