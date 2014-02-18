
// vim: set foldenable foldmethod=marker:

#ifndef AVL_TREE_H
#define AVL_TREE_H

#include <util/standalone_math.h>
//#include <util/pstl/list_dynamic.h> // TODO: Implement & use vector_dynamic instead!
//#include <util/pstl/string_dynamic.h>
#include <util/delegates/delegate.hpp>


//#ifndef ISENSE
	#ifndef assert
		#define assert(X) 
	#endif
//#endif

namespace wiselib {
	
	/**
	 * An AVL tree implementation.
	 */
	template<
		typename OsModel_P,
        typename Value_P = typename OsModel_P::block_data_t*,
		typename Debug_P = typename OsModel_P::Debug
	>
	class AVLTree {
	public:
		typedef OsModel_P OsModel;
		typedef Debug_P Debug;
		typedef Value_P Value;
		typedef StandaloneMath<OsModel_P> Math;
		
		typedef AVLTree<OsModel, Value, Debug> self_type;
		typedef self_type* self_pointer_t;
		
		typedef typename OsModel::size_t size_type;
		typedef Value key_type;
		typedef Value value_type;
		typedef Value mapped_type;
		
		typedef delegate3<int, value_type, value_type, void*> comparator_t;
		
		struct Node;
		typedef Node* node_ptr_t;

		enum { MAX_DEPTH = 20 };
		
			//list_dynamic<OsModel, node_ptr_t> path_;
		typedef vector_static<OsModel, node_ptr_t, MAX_DEPTH> Path;
		
		enum ErrorCodes {
			SUCCESS = OsModel::SUCCESS
		};
		
		/**
		 * AVL tree node.
		 */
		class Node {
			// {{{
		public:
			enum Side { LEFT = 0, RIGHT = 1 };
			
			Node() : data_(0), balance_(0) {
				childs_[0] = 0;
				childs_[1] = 0;
			}
			
			Node(value_type data) : data_(data), balance_(0) {
				childs_[0] = 0;
				childs_[1] = 0;
			}
			
			void init() {
				data_ = 0;
				balance_ = 0;
				childs_[0] = 0;
				childs_[1] = 0;
			}
			void init(value_type d) {
				init();
				data_ = d;
			}
			
			node_ptr_t left()  { return childs_[LEFT]; }
			node_ptr_t right()  { return childs_[RIGHT]; }
			node_ptr_t child(size_t i)  { return childs_[i]; }
			Side side(node_ptr_t child)  { return childs_[LEFT] == child ? LEFT : RIGHT; }
			static Side other(int s) { return (Side)(1 - s); }
			bool is_leaf()  { return !(childs_[0] || childs_[1]); }
			
			void set_child(size_t i, node_ptr_t c) {
				if(!childs_[i] && c) { balance_ += (i == 0) ? -1 : 1; }
				else if(childs_[i] && !c) { balance_ += (i == 0) ? 1 : -1; }
				childs_[i] = c;
/*				assert(
					(childs_[0] || balance_ >= 0) &&
					(childs_[1] || balance_ <= 0)
				);*/
			}
			void set_left(node_ptr_t l) { set_child(LEFT, l); }
			void set_right(node_ptr_t r) { set_child(RIGHT, r); }
			void remove_child(node_ptr_t c) {
				if(c == childs_[0]) { set_child(0, 0); }
				else if(c == childs_[1]) { set_child(1, 0); }
				else assert(false);
			}
			int balance()  { return balance_; }
			Side heavy_side()  { return (balance_ < 0) ? LEFT : RIGHT; }
			void add_balance(int i) {
				balance_ += i;
				/*assert(
					(childs_[0] || balance_ >= 0) &&
					(childs_[1] || balance_ <= 0)
				);*/
			}
			bool imbalanced()  { return (balance_ >= 2) || (balance_ <= -2); }
			
			int childs() { return (childs_[0] ? 1 : 0) + (childs_[1] ? 1 : 0); }
			
			value_type& data()  { return data_; }
			
			friend class AVLTree;
			
		private:
			value_type data_;
			node_ptr_t childs_[2];
			signed char balance_;
			// }}}
		}; // class Node
		
		/**
		 * Inorder iterator. This iterator can be used to traverse the
		 * tree in in- i.e. sorted order.
		 * 
		 * In addition to the usual iterator methods, this iterator also allows
		 * accessing the tree node that holds the pointed-to value.
		 * 
		 * Internally, this iterator will keep the path from the root of the tree
		 * to the pointed-to value in memory.
		 */
		class inorder_iterator {
			// {{{
		public:
			inorder_iterator() { }
			inorder_iterator(node_ptr_t root) : path_() {
				node_ptr_t pos = root;
				while(pos && pos->left()) {
					path_.push_back(pos);
					pos = pos->left();
				}
				if(pos) {
					path_.push_back(pos);
				}
			}
			inorder_iterator(const inorder_iterator& other) : path_(other.path_) {
			}
			inorder_iterator& operator=(const inorder_iterator& other) {
				path_ = other.path_;
				return *this;
			}
			bool operator==(const inorder_iterator& other)  {
				// if one has empty path (end iterator) and the other hasn't -> unequal
				if(path_.empty() != other.path_.empty()) { return false; };
				// if both have empty path -> equal
				if(path_.empty()) { return true; }
				return path_.back() == other.path_.back();
			}
			
			///
			bool operator!=(const inorder_iterator& other)  { return !(*this == other); }
			
			///
			value_type& operator*()  { assert(!path_.empty()); return path_.back()->data(); }
			
			///
			value_type* operator->()  { assert(!path_.empty()); return &(path_.back()->data()); }
			
			/**
			 * Return the tree node holding the currently pointed-to value.
			 */
			 node_ptr_t node()  { return path_.empty() ? node_ptr_t(0) : path_.back(); }
			
			///
			inorder_iterator& operator++() {
				if(path_.empty()) { return *this; }
				
				node_ptr_t pos = path_.back();
				if(pos->right()) {
					path_.push_back(pos->right());
					pos = pos->right();
					while(pos->left()) {
						path_.push_back(pos->left());
						pos = pos->left();
					}
				}
				else {
					// go up until we enter parent from the left
					if(!path_.empty()) {
						path_.pop_back();
						// pos is the node below path_back())
						while(!path_.empty() && (pos != path_.back()->left())) {
							pos = path_.back();
							path_.pop_back();
						}
					} // if not empty
				} // else if pos->right
				return *this;
			} // operator++()
			
			friend class AVLTree;
		private:
			Path path_;
			// }}}
		};
		typedef inorder_iterator iterator;
		
		//template<typename T>
		//static typename T::self_pointer_t to_t_ptr(value_type a) {
			//typename T::self_pointer_t r;
			//memcpy((void*)&r, (void*)&a, sizeof(typename T::self_pointer_t));
			////return *reinterpret_cast<typename T::self_pointer_t*>(&a);
			//return r;
		//}
		
		//template<typename T>
		//static int ptr_cmp_comparator(value_type a, value_type b, void*) {
			//return to_t_ptr<T>(a)->cmp(*to_t_ptr<T>(b));
			////return reinterpret_cast<T*>(a)->cmp(*reinterpret_cast<T*>(b));
		//}
		
		static int string_comparator(value_type a, value_type b, void*) {
			return strcmp((char*)a, (char*)b);
		}
		
		/**
		 * Initialize the AVL tree.
		 * \param debug A debug object for debugging output (if AVLTREE_DEBUG is defined).
		 */
		int init(comparator_t compare, typename Debug::self_pointer_t debug = 0) {
			#if AVLTREE_DEBUG
				debug_ = debug;
			#endif
			compare_ = compare;
			root_ = 0;
			size_ = 0;
			return SUCCESS;
		}
		
		/**
		 * Destruct the AVL tree.
		 */
		int destruct() {
			clear();
			return SUCCESS;
		}
		
		~AVLTree() {
			clear();
		}
		
		/// Beginning iterator for in-order iteration.
		inorder_iterator begin_inorder()  { return inorder_iterator(root()); }
		
		/// End iterator for in-order iteration.
		inorder_iterator end_inorder()  { return inorder_iterator(); }
		
		/// Beginning iterator for default iteration
		iterator begin() { return iterator(root()); }
		
		/// End iterator for default iteration
		iterator end() { return iterator(); }
		
		/**
		 * Delete all elements from the tree.
		 */
		void clear() {
			if(root()) { clear(root()); }
			root_ = 0;
			size_ = 0;
			check();
		}
		
		/**
		 * \return the number of elements stored in the tree.
		 */
		size_type size()  { return size_; }
		bool empty()  { return size_ == 0; }
		size_type max_size()  { return 0; }
		
		inorder_iterator find(value_type data, void* arg=0) {
			inorder_iterator iter;
			find(data, root(), iter, arg);
			if(iter.path_.size() && compare_(iter.path_.back()->data(), data, arg) == 0) {
				return iter;
			}
			return end();
		}
		
		node_ptr_t find_n(value_type data, void* arg=0) {
			inorder_iterator iter = find(data, arg);
			if(iter == end()) { return node_ptr_t(0); }
			return iter.node();
		}
		
		size_type count(value_type data, void* arg=0) { return find(data, arg) != end(); }
		
		/**
		 * Insert data block into the tree. If the data is already in the tree,
		 * do nothing.
		 * In contrast to the "usual" insert() method this does not return an
		 * iterator to the inserted element but a pointer to the node holding
		 * it.
		 * 
		 * \return A pointer to the inserted or found node that contains \ref data.
		 */
		node_ptr_t insert_n(value_type data, void* arg=0) {
			int height_change = 0;
			node_ptr_t r;
			if(!root_) {
				root_ = get_allocator().template allocate<Node>() .raw();
				root_->init(data);
				size_++;
				r = root_;
			}
			else {
				r = insert(data, root_, height_change, arg);
			}
			
			#if AVLTREE_DEBUG && AVLTREE_GRAPHVIZ
			debug_graphviz("after insert_n", root_, r);
			#endif
			check();
			return r;
		}
		
		/**
		 * \tparam Iterator A valid iterator type defined in this AVL tree class.
		 * Erase the node pointed to by iter from the tree and free its used
		 * memory.
		 */
		//template<typename Iterator>
		void erase(iterator& iter) {
			int h = 0;
			node_ptr_t d = unlink(iter, root_, h);
			assert(d);
			assert(size_ > 0);
			size_--;
			get_allocator().template free<Node>(d);
			
			check();
		}
		
		void erase_n(node_ptr_t n) {
			iterator iter = find(n->data());
			if(iter != end()) {
				erase(iter);
			}
		}
		
#if AVLTREE_CHECK
		/**
		 * Check if AVL condition (balance in {-1,0,1}) is met for all nodes.
		 */
		void check_avl(node_ptr_t r)  {
			if(!r) return;
			assert(r->left() != r);
			assert(r->right() != r);
			assert(r->balance() == -1 || r->balance() == 0 || r->balance() == 1);
			check_avl(r->left());
			check_avl(r->right());
		}
		
		/**
		 * Check balance values for correctness.
		 * \return height of the tree rooted at node n.
		 */
		size_t check_balance(node_ptr_t n)  {
			if(!n) { return 0; }
			assert(n->left() != n);
			assert(n->right() != n);
			int l = check_balance(n->left()), r = check_balance(n->right());
			assert(n->balance() == (r - l)); 
			return (r >= l ? r : l) + 1;
		}
		
		/**
		 * Check ordering for correctness (i.e. for each node n:
		 * n.left <= n <= n.right)
		 */
		void check_ordering(node_ptr_t n) {
			if(!n) { return; }
			if(n->left()) {
				check_ordering(n->left());
				assert(compare_(n->left()->data(), n->data(), 0) <= 0);
				assert(compare_(n->data(), n->left()->data(), 0) >= 0);
			}
			if(n->right()) {
				check_ordering(n->right());
				assert(compare_(n->data(), n->right()->data(), 0) <= 0);
				assert(compare_(n->right()->data(), n->data(), 0) >= 0);
			}
		}
		
		void check_inorder() {
			iterator iter = begin();
			if(iter != end()) {
				value_type v = *iter;
				++iter;
				while(iter != end()) {
					assert(compare_(v, *iter, 0) <= 0);
					assert(compare_(*iter, v, 0) >= 0);
					v = *iter;
					++iter;
				}
			}
		}
		/**
		 */
		size_t count_nodes(node_ptr_t n)  {
			if(!n) { return 0; }
			assert(n->left() != n);
			assert(n->right() != n);
			return 1 + count_nodes(n->left()) + count_nodes(n->right());
		}
		
		/**
		 */
		void check()  {
			check_balance(root_);
			check_avl(root_);
			check_ordering(root_);
			check_inorder();
			
			size_t counted = count_nodes(root_);
			assert(counted == size());
			assert((!root_) == (size_ == 0));
		}
		
		int check_height(node_ptr_t n) {
			if(!n) { return 0; }
			return Math::max(check_height(n->left()), check_height(n->right())) + 1;
		}
#else
		void check_avl(node_ptr_t)  { }
		void check_balance(node_ptr_t)  { }
		void check_ordering(node_ptr_t) { }
		void check_inorder() { }
		void check()  { }
	//	int check_height() { return 0; }
#endif // AVLTREE_CHECK
		
		
#if AVLTREE_DEBUG
	// {{{
		void print() {
			if(debug_) {
				debug_->debug("AVLTree< ");
				printSubtree(root_);
				debug_->debug(" size=%d>", size());
			}
		}
		
		void printNode(node_ptr_t node, node_ptr_t mark1 = 0, node_ptr_t mark2 = 0, node_ptr_t mark3 = 0) {
			if(node) {
				char *mark = 0;
				if(node == mark1) { mark = (char*)" (1)"; }
				else if(node == mark2) { mark = (char*)" (2)"; }
				else if(node == mark3) { mark = (char*)" (3)"; }
				
				debug_->debug("\"");
				/*for(size_t i=0; i < (node->data().size()); i++) {
					char c = (reinterpret_cast< char*>((node->data().c_str())))[i];
					debug_->debug("%c", c >= 32 ? c : '.');
				}*/
				//debug_->debug("@%p %p:\\\"%s\\\"", node, node->data(), node->data());
				debug_->debug("@%p %p", node, node->data());
				debug_->debug(" b=%d", node->balance());
				if(mark) {
					debug_->debug("%s", mark);
				}
				debug_->debug("\"");
			}
			else {
				debug_->debug("\"null\"");
			}
		}
		
		void printSubtree(node_ptr_t node) {
			if(node && debug_) {
				printNode(node);
				debug_->debug("< ");
				printSubtree(node->left());
				debug_->debug(" | ");
				printSubtree(node->right());
				debug_->debug(" >");
			}
		}
	// }}}
#endif // AVLTREE_DEBUG

#if AVLTREE_GRAPHVIZ && AVLTREE_DEBUG
	// {{{
		void debug_graphviz(const char* message, node_ptr_t root, node_ptr_t mark1 = 0, node_ptr_t mark2 = 0, node_ptr_t mark3 = 0) {
			debug_->debug("  ----- %s -----\n", message);
			debug_->debug("  digraph G {\n");
			print_graphviz(message, root, mark1, mark2, mark3);
			debug_->debug("  }\n");
		}
					
		void print_graphviz(const char* message, node_ptr_t root, node_ptr_t mark1, node_ptr_t mark2, node_ptr_t mark3) {
			
			if(root && root->left()) {
				debug_->debug("  ");
				printNode(root, mark1, mark2, mark3);
				debug_->debug(" -> ");
				printNode(root->left(), mark1, mark2, mark3);
				debug_->debug(" [label=\"L\"];\n");
				print_graphviz(message, root->left(), mark1, mark2, mark3);
			}
			if(root && root->right()) {
				debug_->debug("  ");
				printNode(root, mark1, mark2, mark3);
				debug_->debug(" -> ");
				printNode(root->right(), mark1, mark2, mark3);
				debug_->debug(" [label=\"R\"];\n");
				print_graphviz(message, root->right(), mark1, mark2, mark3);
			}
		}
	// }}}
#endif // AVLTREE_GRAPHVIZ
		
		comparator_t compare() { return compare_; }
		
			
#if !AVLTREE_DEBUG
	private:
#endif
		
		size_type size_;
		node_ptr_t root_;
#if AVLTREE_DEBUG
		typename Debug::self_pointer_t debug_;
#endif
		comparator_t compare_;
		
		// Return the root node
		 node_ptr_t root()  { return root_; }


		
		/* 
		 * \param data data to insert
		 * \param base base/root node under which to insert
		 * \param height_change reference to integer for tracking height changes of subtrees.
		 */
		node_ptr_t insert(value_type data, node_ptr_t& base, int& height_change, void* arg=0) {

			inorder_iterator iter;
			
			// first, find place to insert
			find(data, base, iter, arg);

			// create new child node and connect it to parent
			node_ptr_t p = iter.path_.back();
			node_ptr_t n = get_allocator().template allocate<Node>() .raw();
			node_ptr_t new_node = n;
			n->init(data);
			size_++;
			int c = compare_(p->data(), n->data(), arg);
			//printf("--- ins %s below %s c=%d\n", (char*)data, (char*)iter.path_.back()->data(), (int)c);
			typename Node::Side side = (c > 0) ? Node::LEFT : Node::RIGHT;
			p->set_child(side, n);
			if(p->balance() != 0) { height_change = +1; }
			else { height_change = 0; }
			iter.path_.pop_back();
			n = p;

			// now walk up, checking balance
			while(iter.path_.size() > 0) {
				p = iter.path_.back();
				int c = compare_(p->data(), n->data(), arg);
	
				//printf("--- post ins n=%s p=%s c=%d\n", (char*)n->data(), (char*)p->data(), (int)c);

				typename Node::Side side = (c > 0) ? Node::LEFT : Node::RIGHT;
				p->set_child(side, n);
				p->balance_ += (side == Node::RIGHT) ? height_change : -height_change;
				if(p->imbalanced()) {
					p = rebalance(p);
					//printf("---- rebal: %s at top\n", (char*)p->data());
					height_change = 0;
				}
				if(height_change == +1 && p->balance() != 0) { height_change = +1; }
				else { height_change = 0; }

				n = p;
				iter.path_.pop_back();
			}
			base = p;
			return new_node;
		}
			

		/*
		node_ptr_t insert(value_type data, node_ptr_t& base, int& height_change, void* arg=0) {
			node_ptr_t r;
			
			int c = compare_(base->data(), data, arg);

			printf("-ins %p %s<>%s c=%d\n", (void*)base, (char*)data, (char*)base->data(), (int)c);

			
			if(c == 0) { // equivalent node already there, return existing node
				height_change = 0;
				return base;
			}
			
			typename Node::Side side = (c > 0) ? Node::LEFT : Node::RIGHT;
			if(base->child(side)) {
				node_ptr_t child = base->child(side);
				r = insert(data, child, height_change, arg);
				//r = child;
				base->set_child(side, child);
				
				base->balance_ += (side == Node::RIGHT) ? height_change : -height_change;
				if(base->imbalanced()) {
					base = rebalance(base);
					height_change = 0;
				}
				
				if(height_change == +1 && base->balance() != 0) { height_change = +1; }
				else { height_change = 0; }
			}
			else { // place for insertion found, create & insert node
				r = get_allocator().template allocate<Node>() .raw();
				r->init(data);
				size_++;
				base->set_child(side, r);
				if(base->balance() != 0) { height_change = +1; }
				else { height_change = 0; }
			} // else
			
			return r;
		} // insert()
		*/	
		
		/*
		 * \param data Element to search for
		 * \param r root node to investigate
		 * \param iter reference to iterator that will be constructed
		 * \return an iterator pointing to an element that equals data if found,
		 *   (via \ref iter), an iterator pointing to the correct parent for
		 *   insertion of data else.
		 */
		template<typename T>
		void find(const T& data, node_ptr_t r, inorder_iterator& iter, void* arg=0)  {
			assert(data != 0);
			
			while(r) {
				iter.path_.push_back(r);
				int c = compare_(r->data(), data, arg);
				if(c > 0 && r->left()) { r = r->left(); }
				else if(c < 0 && r->right()) { r = r->right(); }
				else { break; }
			}
		}
		
		/* Recursive implementation
		 * 
		void find(const T& data, node_ptr_t r, inorder_iterator& iter, void* arg=0)  {
			assert(data != 0);
			
			if(r) {
				iter.path_.push_back(r);
				int c = compare_(r->data(), data, arg);
				if(c > 0 && r->left()) { find(data, r->left(), iter, arg); }
				else if(c < 0 && r->right()) { find(data, r->right(), iter, arg); }
			}
		}
		*/
		
		/*
		 * Unlink node pointed to by iter. (i.e. remove it from the tree but
		 * do not free its memory). Return the unlinked node upon success.
		 * 
		 * everything up to (and including) base will be rebalanced.
		 * iter.path_.back should be base or a node above base.
		 * 
		 */
		node_ptr_t unlink(iterator iter, node_ptr_t& base, int& height_change) {
			
			assert(iter.node());
			assert(iter.path_.size());
			assert((iter.path_.size() == 1) ? (iter.path_.back() == base) : 1);
			
			
			node_ptr_t node = iter.node();
			iter.path_.pop_back();
			node_ptr_t parent = iter.node();
			
			#if AVLTREE_DEBUG
			printf("unlink(n=%p %s base=%p hc=%d)\n", node, node->data(), base, height_change);
			#endif
			#if AVLTREE_GRAPHVIZ && AVLTREE_DEBUG
			debug_graphviz("unlink", root_, base, parent, node);
			#endif
			
			if(node->childs() == 0) { // node is a leaf node
				if(parent) {
					#if AVLTREE_DEBUG
					printf("unlink(n=%p base=%p hc=%d): node is a leaf node and has parent\n", node, base, height_change);
					#endif
					
					typename Node::Side s = parent->side(node);
					parent->childs_[s] = 0;
					height_change = -1;
					
					base = rebalance_up(iter, s, height_change);
					
					#if AVLTREE_DEBUG
					printf("unlink(n=%p base=%p hc=%d): base updated after rebal_up\n", node, base, height_change);
					#endif
				}
				else {
					#if AVLTREE_DEBUG
					printf("unlink(n=%p base=%p hc=%d): node had no parent\n", node, base, height_change);
					#endif
					//typename Node::Side s = parent->side(node);
					base = 0;
					#if AVLTREE_DEBUG
					printf("unlink(n=%p base=%p hc=%d): zeroed base (leaf)\n", node, base, height_change);
					#endif
				}
			}
				
			else if(node->childs() == 1) { // node is a half-leaf
				if(parent) {
					#if AVLTREE_DEBUG
					printf("unlink(n=%p base=%p hc=%d): half-leaf with parent\n", node, base, height_change);
					#endif
					
					typename Node::Side s = parent->side(node);
					parent->childs_[s] = node->childs_[node->heavy_side()];
					height_change = -1;
					base = rebalance_up(iter, s, height_change);
					#if AVLTREE_DEBUG
					printf("unlink(n=%p base=%p hc=%d): updated base after rebal_up (half-leaf with parent)\n", node, base, height_change);
					#endif
				}
				else {
					base = node->childs_[node->heavy_side()];
					#if AVLTREE_DEBUG
					printf("unlink(n=%p base=%p hc=%d): updated base (half-leaf)\n", node, base, height_change);
					#endif
					height_change = -1;
				}
			}
			
			else {
				/**
				 *            x
				 *         /    \
				 *       /        \
				 *      a          .
				 *     / \        /
				 *    /   \     /
				 *   b     c   z
				 */
				iterator substitute_iter = leftmost_leaf(node->right());
				node_ptr_t right = node->right();
				int height_change_right = -1;
				
				#if AVLTREE_DEBUG
				printf("<<< // unlink(n=%p base=%p hc=%d): complex case, unlinking substitute: %p\n", node, base, height_change, substitute_iter.node());
				#endif
				
				node_ptr_t substitute = unlink(substitute_iter, right, height_change_right);
				
				#if AVLTREE_DEBUG
				printf(">>> // unlink(n=%p base=%p hc=%d hcr=%d): complex case, unlinking substitute: %p\n", node, base, height_change, height_change_right, substitute_iter.node());
				#endif
				
				substitute->childs_[Node::RIGHT] = right;
				substitute->childs_[Node::LEFT] = node->left();
				substitute->balance_ = node->balance(); // + height_change_right; // rebalance_up will take care of adding height_change_reight in 3a, in 3b we do it manually
				
				/*int new_balance = node->balance() + height_change_right;*/
				
				if(parent) {
					//printf("-- case 3a subst=%p %s\n", substitute, substitute->data());
					//fflush(stdout);
					
					typename Node::Side s = parent->side(node);
					parent->childs_[s] = substitute;
					
					iter.path_.push_back(substitute);
					//height_change_right = -1;
					//height_change_right
					//int hcsubst = 0;
					//int oldbal = node->balance();
					//if(height_change_right >= 0) {
						//if(height_change_right <= -oldbal) { hcsubst = -oldbal; }
						//else { hcsubst = height_change_right; }
					//}
					//else { // hcr < 0
						//if(height_change_right >= -oldbal) { hcsubst = height_change_right; }
						//else { hcsubst = 0; }
					//}
					//printf("-------- hcr=%d oldbal=%d hcsubst=%d\n", height_change_right, oldbal, hcsubst);
					
					
					base = rebalance_up(iter, Node::RIGHT, height_change_right);
					//printf("-- reset base 3a: %p\n", base);
					//height_change_right = hcsubst;
				}
				else {
				//printf("-- case 3b\n");
					substitute->balance_ += height_change_right;
					base = substitute;
					//printf("-- reset base 3b: %p\n", base);
				}
			}
			
			#if AVLTREE_DEBUG
			printf("root=%p base=%p\n", root_, base);
			fflush(stdout);
			#endif
			
			#if AVLTREE_DEBUG && AVLTREE_GRAPHVIZ
			debug_graphviz("after unlink", root_, base, parent, node);
			#endif
			
			check_balance(base);
			check_avl(base);
			check_ordering(base);
			
			return node;
		}
		
		/*
		 * Change the balance at the node pointed to by iter by
		 * balance_change. If necessary, rebalance. If rebalancing in turn
		 * makes it necessary to rebalance the parant node, do so, propagating
		 * rebalancing operations upwards as necessary, such that
		 * iter.path_.back() and its subtree will be balanced after a call to
		 * this (assuming unbalanced nodes where only on iter.path_).
		 */
		node_ptr_t rebalance_up(iterator iter, typename Node::Side s, int& height_change) {
			assert(iter.path_.size());
			
			node_ptr_t new_subtree_root = iter.path_.front();
			//printf("==== rbu init subtree root=%p\n", new_subtree_root);
			//fflush(stdout);
			
			
			#if AVLTREE_DEBUG
			node_ptr_t iter_root = iter.path_.front();
			size_t counted = count_nodes(iter_root);
			#endif 
			
			bool rooted = (iter.path_.front() == root_);
			
			int sign = (s == Node::RIGHT) ? +1 : -1;
			node_ptr_t front = iter.path_.front();
			#if AVLTREE_DEBUG
			node_ptr_t back = iter.path_.back();
			printf("rebalance_up(front=%p back=%p side=%d hc=%d)\n", front, back, s, height_change);
			#endif
			
			#if AVLTREE_DEBUG && AVLTREE_GRAPHVIZ
			debug_graphviz("before rebalance_up", root_, front, back);
			#endif
			
			node_ptr_t n, prev_n;
			//int new_balance;
			int hc = height_change;
			//typename Node::Side sp;
			
			
			//iter.path_.back()->balance_ += sign * height_change;
			
			while(iter.path_.size()) {
				prev_n = n;
				n = iter.path_.back();
				iter.path_.pop_back();
				
				#if AVLTREE_DEBUG
				printf("n=%p hc=%d sign=%d old_balance=%d\n", n, hc, sign, n->balance_);
				#endif
				#if AVLTREE_DEBUG && AVLTREE_GRAPHVIZ
				debug_graphviz("before rebalance_up iteration", root_, n);
				#endif
				
				// Calculate updated balance of node n
				
				int old_hc = hc;
				int balsign = Math::sgn(n->balance_);
				int absbal = Math::abs(n->balance_);
				//printf("bal = %d * %d oldhc=%d\n", balsign, absbal, hc);
				if(sign == balsign) {
					if(hc < -absbal) {
						hc = -absbal;
					}
				}
				else {
					if(hc >= absbal) {
						hc = hc - absbal;
					}
					else {
						hc = 0;
					}
				}
				n->balance_ += sign * old_hc;
				
				#if AVLTREE_DEBUG
				printf("new hc=%d sign=%d new balance=%d\n", hc, sign, n->balance_);
				#endif
				
				
				int h = 0;
				node_ptr_t parent;
				
				if(iter.node()) { //n != root_) {
					
					// if we have a parent and are imbalanced:
					// rebalance, and re-set child of parent
					
					parent = iter.node();
					s = parent->side(n);
					if(n->imbalanced()) {
						n = rebalance(n, h);
						hc += h;
					}
					parent->set_child(s, n);
					sign = (s == Node::RIGHT) ? +1 : -1;
				}
				else {
					//printf("--- rebalance_up: last node in path is %p\n", n);
					//fflush(stdout);
					if(n->imbalanced()) {
						node_ptr_t r = rebalance(n, h);
						new_subtree_root = r;
			//printf("=== rbu lastnode subtree root=%p rooted=%d\n", new_subtree_root, rooted);
			//fflush(stdout);
						if(rooted) {
							root_ = r;
						}
						hc += h;
					}
					break;
				}
				/*else {*/
				/*printf("n->balance=%d\n", n->balance_);*/
				/*}*/
				
				
				#if AVLTREE_DEBUG && AVLTREE_GRAPHVIZ
				debug_graphviz("after step rebalance_up", root_, front, back);
				#endif // AVLTREE_DEBUG && AVLTREE_GRAPHVIZ
				
			}
			
			#if AVLTREE_DEBUG && AVLTREE_GRAPHVIZ
			debug_graphviz("after rebalance_up", root_, front, back);
			#endif // AVLTREE_DEBUG && AVLTREE_GRAPHVIZ
			
			#if AVLTREE_DEBUG
			printf("new_subtree_root=%p\n", new_subtree_root);
			size_t counted2 = count_nodes(new_subtree_root);
			assert(counted2 == counted);
			#endif
			
			check_balance(front);
			//check_balance(back);
			
			height_change = hc;
			
			check_balance(new_subtree_root);
			
			//printf("=== rbu returning %p\n", new_subtree_root);
			return new_subtree_root;
		}
		
		void xxx_rebalance_up(iterator iter, typename Node::Side s, int& height_change) {
			int sign = (s == Node::RIGHT) ? +1 : -1;
			int new_height_change = 0;
			node_ptr_t n, prev_n;
			while(iter.path_.size()) {
				prev_n = n;
				n = iter.path_.back();
				iter.path_.pop_back();
				
				//if(new_height_change != 0 && prev_n) {
				//	Node::Side s = n->side(prev_n);
				//	if(s == Node::RIGHT) n->balance_ += new_height_change;
				if(prev_n) {
					/*Node::Side*/ s = n->side(prev_n);
					height_change = new_height_change;
					sign = (s == Node::RIGHT) ? +1 : -1;
				}
				
				int new_balance = n->balance() + height_change * sign;
				//int new_height_change = 0;
				if(s == n->heavy_side()) {
					new_height_change = Math::max(-Math::abs(n->balance()), height_change);
				}
				else {
					new_height_change = Math::max(0, height_change - Math::abs(n->balance()));
				}
				n->balance_ = new_balance;
				if(n->imbalanced()) {
					if(iter.path_.size()) {
						typename Node::Side s = iter.node()->side(n);
						n = rebalance(n, height_change);
						iter.node()->set_child(s, n);
					}
					else {
						//Node::Side s = root_->side(n);
						root_ = rebalance(n, height_change);
					}
				} // if n imbalanced
			} // while
			
			//root_->balance_ += sign * new_height_change;
		} // rebalance_up()
		
		iterator leftmost_leaf(node_ptr_t start) {
			iterator i;
			node_ptr_t n = start;
			while(n) {
				i.path_.push_back(n);
				n = n->left();
			}
			return i;
		}
		
		/*
			/// ------------------------------------------
			
			
			node_ptr_t node = iter.path_.back();
			node_ptr_t rem = node;
			inorder_iterator inorder(iter);
			iter.path_.pop_back();
			
			node_ptr_t parent;
			typename Node::Side side = Node::LEFT;
			if(!iter.path_.empty() && (node != base)) {
				parent = iter.path_.back();
				iter.path_.pop_back();
				side = parent->side(node);
			}
			else {
				parent = node_ptr_t();
			}
			
			bool is_base = (node == base);
			if(!node->left() && !node->right()) { // leaf node
				node = 0;
				height_change = -1;
			}
			else if(!node->left() || !node->right()) {
				node = node->child(node->heavy_side());
				height_change = -1;
			}
			else {
				int hc = 0;
				node_ptr_t next = unlink(++inorder, node->childs_[Node::RIGHT], hc);
				if(next) {
					next->childs_[Node::LEFT] = node->childs_[Node::LEFT];
					next->childs_[Node::RIGHT] = node->childs_[Node::RIGHT];
					next->balance_ = node->balance_;
					node = next;
					node->balance_ += hc;
					if((hc != 0) && (node->balance_ == 0)) { height_change = -1; }
					else if(node->imbalanced()) { node = rebalance(node, height_change); }
				}
				else { node = node->childs_[Node::LEFT]; }
			}
			
			if(is_base) { base = node; }
			if(parent) {
				parent->childs_[side] = node;
				
				if(parent && (node != base) && !iter.path_.empty()) {
					node = parent;
					parent = iter.path_.back();
					iter.path_.pop_back();
					typename Node::Side side = Node::LEFT;
					while(node != base && height_change != 0) {
						node->balance_ += (side == Node::LEFT) ? -height_change : height_change;
						
						side = parent->side(node);
						if(node->imbalanced()) {
							height_change = 0;
							node_ptr_t new_node = rebalance(node, height_change);
							parent->childs_[side] = new_node;
						}
						else if(node->balance_ == 0) { height_change = -1; }
						else { height_change = 0; }
						
						node = parent;
						if(!iter.path_.size()) { break; }
						parent = iter.path_.back();
						iter.path_.pop_back();
						
						assert((iter.path_.size() == 1) ? (iter.path_.back() == base) : 1);
					}
					
				} // if path not empty

				if(base) {
					base->balance_ += (side == Node::LEFT) ? -height_change : height_change;
					if(base->imbalanced()) {
						height_change = 0;
						base = rebalance(base, height_change);
					}
					else if(base->balance_ == 0 && (height_change != 0)) { height_change = -1; }
					else { height_change = 0; }
				}
			} // if(parent)
			
			check_balance(base);
			check_avl(base);

			return rem;
		} // unlink()
		*/
			
		/*
		 * \param a node at the root of the rotation
		 * \param dir direction in which to rotate (left==CCW)
		 * \param height will be modified according to the height change of
		 * 	the tree resulting from this operation
		 * \return new node at the top (b in the following example)
		 * 
		 * Example for dir == Node::LEFT:
		 * 
		 *      a               b
		 *    /  \            /  \
		 *   t1  b    ==>    a   t3
		 *      / \         / \
		 *     t2 t3       t1 t2
		 */
		node_ptr_t rotate(node_ptr_t a, typename Node::Side dir, int& height) {
			#if AVLTREE_DEBUG
				size_t count = count_nodes(a);
			#endif
			
			check_balance(a);
		//	check_ordering(a);
			#if AVLTREE_DEBUG
			int height_before = check_height(a);
				#if AVLTREE_GRAPHVIZ
				debug_graphviz("before rotate", a);
				#endif
			#endif
			
			node_ptr_t b = a->child(Node::other(dir));
			assert(b);
			
			
			int bal_a = a->balance_,
				bal_b = b->balance_;
			
			#if AVLTREE_DEBUG
			//int height_change = (b->heavy_side() == Node::other(dir)) ? -1 : 0;
			#endif
			
			//height += height_change; //(b->heavy_side() == Node::other(dir)) ? -1 : 0;
			
			a->childs_[Node::other(dir)] = b->child(dir);
			b->childs_[dir] = a;
			
			if(dir == Node::LEFT) {
				a->balance_ += -1 - ((b->balance_ <= 0) ? 0 : b->balance_);
				b->balance_ += -1 + ((a->balance_ >= 0) ? 0 : a->balance_);
			}
			else {
				a->balance_ +=  1 - ((b->balance_ >= 0) ? 0 : b->balance_);
				b->balance_ +=  1 + ((a->balance_ <= 0) ? 0 : a->balance_);
			}
			
			int bal_a2 = a->balance_,
				bal_b2 = b->balance_;
			
			// ---- calculate height change (the hard way) ----
			int s = 1;
			if(dir == Node::RIGHT) { s = -1; }
			
			bool z = s * bal_b2 < 0,
				 x = s * bal_a2 < 0,
				 w = s * bal_a < 0,
				 y = s * bal_b < 0;
			
			int hc = 0;
			if(!z && !w && !y) { hc = -1; }
			else if(!z && !w && y) { hc = s*bal_b - 1; }
			else if(!z && w) { hc = s*(bal_a2 + bal_b); }
			else if(z && !x && !w && !y) { hc = -s*bal_b; }
			else if(z && !x && !w && y) { hc = 0; }
			else if(z && !x && w) { hc = 1 + s*bal_a2; }
			else if(z && x && !w && !y) { hc = -s*(bal_a2 + bal_b); }
			else if(z && x && !w && y) { hc = -s*bal_a2; }
			else if(z && x && w) { hc = 1; }
			height += hc;
			
			#if AVLTREE_DEBUG
			printf("s=%d x=%d w=%d z=%d bal_b2=%d bal_a2=%d bal_a=%d bal_b=%d\n", s, x, w, z, bal_b2, bal_a2, bal_a, bal_b);
				#if AVLTREE_GRAPHVIZ
				debug_graphviz("after rotate", b);
				#endif
			int height_after = check_height(b);
			assert(height_before + hc == height_after);
			#endif
			
			#if AVLTREE_DEBUG
				assert(count == count_nodes(b));
			#endif
			
		//	check_balance(b);
		//	check_ordering(b);
			return b;
		} // rotate()
		
		/*
		 * \param node a node with balance in {-2, 2}
		 */
		node_ptr_t rebalance(node_ptr_t node) {
			int h = 0;
			return rebalance(node, h);
		}
		
		/*
		 * 
		 */
		node_ptr_t rebalance(node_ptr_t node, int& height) {
			#if AVLTREE_GRAPHVIZ && AVLTREE_DEBUG
			debug_graphviz("before rebalance", node);
			#endif
				
			typename Node::Side s = node->heavy_side(); //(node->balance_ == 2) ? Node::RIGHT : Node::LEFT;
			
			if(node->child(s) &&
					!(node->child(s)->is_leaf()) &&
					node->child(s)->child(Node::other(s)) &&
					(node->child(s)->heavy_side() == Node::other(s))
			) {
				int h = 0;
				node_ptr_t x = rotate(node->child(s), s, h);
				//node->set_child(s, x);
				node->childs_[s] = x;
				if(h != 0) {
					node->balance_ += ((s == Node::RIGHT) ? 1 : -1) * h;
				}
				height += h;
				
				check_balance(node->child(s));
			}
			
			#if AVLTREE_GRAPHVIZ && AVLTREE_DEBUG
			debug_graphviz("before final rotate", node);
			#endif
			
			node_ptr_t r = rotate(node, Node::other(s), height);
			
			
			check_balance(node);
			//check_avl(node);
			return r;
		} // rebalance()
		
		/*
		 * 
		 */
		void clear(node_ptr_t node) {
			if(!node) { return; }
			if(node->left()) {
				clear(node->left());
				node->set_left(0);
			}
			
			if(node->right()) {
				clear(node->right());
				node->set_right(0);
			}
			
			get_allocator().template free<Node>(node);
		} // clear()
		
	};
	
} // namespace

#endif // AVL_TREE_H

