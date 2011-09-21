
// vim: set foldenable foldmethod=marker:

#ifndef AVL_TREE_H
#define AVL_TREE_H

//#pragma warning("AVL tree is not fully usable yet!")

//#undef NDEBUG
//#include <cassert>
#ifndef assert
	#define assert(X) 
#endif

#include "util/allocators/utils.h"
#include "util/pstl/list_dynamic.h" // TODO: Implement & use vector_dynamic instead!
#include "util/pstl/string_dynamic.h"

namespace wiselib {
	
	/*
	template<typename A>
	struct CompareLess {
		template<typename B>
		static int cmp(const A& a, const B& b) {
			return a < b ? -1 : a > b;
		}
	};
	*/
	
	/**
	 * An AVL tree implementation.
	 */
	template<
		typename OsModel_P,
		typename Allocator_P,
		//typename Compare_P = CompareLess<Key_P>,
		typename BlockData_P , //= char,
		typename Debug_P = typename OsModel_P::Debug
	>
	class DataAVLTree {
	public:
		typedef OsModel_P OsModel;
		typedef Allocator_P Allocator;
		typedef Debug_P Debug;
		
		typedef DataAVLTree<OsModel_P, Allocator_P, Debug_P> self_type;
		typedef self_type* self_pointer_t;
		
		typedef typename OsModel::size_t size_type;
		//typedef typename OsModel::block_data_t block_data_t;
		typedef BlockData_P block_data_t;
		
		typedef string_dynamic<OsModel, Allocator, block_data_t> data_t;
		typedef data_t key_type;
		typedef data_t value_type;
		typedef data_t mapped_type;
		
		typedef int (*comparator_t)(const data_t&, const data_t&);
		
		struct Node;
		typedef typename Allocator::template pointer_t<Node> node_ptr_t;
		
		enum ErrorCodes {
			SUCCESS = OsModel::SUCCESS
		};
		
		/**
		 * AVL tree node.
		 */
		class Node {
		public:
			enum Side { LEFT, RIGHT };
			
			void init() { balance_ = 0; set_left(0); set_right(0); }
			void init(const data_t& d) { init(); data_ = d; }
			
			const node_ptr_t left() const { return childs_[LEFT]; }
			const node_ptr_t right() const { return childs_[RIGHT]; }
			const node_ptr_t child(size_t i) const { return childs_[i]; }
			Side side(node_ptr_t child) const { return childs_[LEFT] == child ? LEFT : RIGHT; }
			static Side other(int s) { return (Side)(1 - s); }
			bool is_leaf() const { return !(childs_[0] || childs_[1]); }
			
			void set_child(size_t i, node_ptr_t c) {
				if(!childs_[i] && c) { balance_ += (i == 0) ? -1 : 1; }
				else if(childs_[i] && !c) { balance_ += (i == 0) ? 1 : -1; }
				childs_[i] = c;
			}
			void set_left(node_ptr_t l) { set_child(LEFT, l); }
			void set_right(node_ptr_t r) { set_child(RIGHT, r); }
			void remove_child(node_ptr_t c) {
				if(c == childs_[0]) { set_child(0, 0); }
				else if(c == childs_[1]) { set_child(1, 0); }
				else assert(false);
			}
			int balance() const { return balance_; }
			Side heavy_side() const { return (balance_ < 0) ? LEFT : RIGHT; }
			void add_balance(int i) { balance_ += i; }
			bool imbalanced() const { return (balance_ == 2) || (balance_ == -2); }
			
			//template<typename T>
			//int cmp(const T& d) { return Compare::cmp(data_, d); }
			
			const data_t& data() const { return data_; }
//			data_t& data() { return data_; }
			
			template<typename T> T& as() { return *reinterpret_cast<T*>(data_.c_str()); }
			template<typename T> const T& as() const { return *reinterpret_cast<const T*>(data_.c_str()); }
			
			friend class DataAVLTree;
			
		private:
			data_t data_;
			node_ptr_t childs_[2];
			signed char balance_;

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
			inorder_iterator(typename Allocator::self_pointer_t allocator) : path_(*allocator) { }
			inorder_iterator(node_ptr_t root, typename Allocator::self_pointer_t allocator) : path_(*allocator) {
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
			bool operator==(const inorder_iterator& other) const {
				// if one has empty path (end iterator) and the other hasn't -> unequal
				if(path_.empty() != other.path_.empty()) { return false; };
				// if both have empty path -> equal
				if(path_.empty()) { return true; }
				return path_.back() == other.path_.back();
			}
			
			///
			bool operator!=(const inorder_iterator& other) const { return !(*this == other); }
			
			///
			const data_t& operator*() const { return path_.back()->data(); }
			
			///
			const data_t* operator->() const { return &(path_.back()->data()); }
			
			//template<typename T> T& as() { return *reinterpret_cast<T*>(path_.back()->data().c_str()); }
			template<typename T> const T& as() const { return *reinterpret_cast<const T*>(path_.back()->data().c_str()); }
			
			/**
			 * Return the tree node holding the currently pointed-to value.
			 */
			const node_ptr_t node() const { return path_.empty() ? node_ptr_t(0) : path_.back(); }
			
			///
			inorder_iterator& operator++() {
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
			
			friend class DataAVLTree;
		private:
			list_dynamic<OsModel, node_ptr_t, Allocator> path_;
			// }}}
		};
		typedef inorder_iterator iterator;
		
		/**
		 * Initialize the AVL tree.
		 * \param alloc An allocator for dynamic memory management.
		 * \param debug A debug object for debugging output (if AVLTREE_DEBUG is defined).
		 */
		int init(typename Allocator::self_pointer_t alloc, comparator_t compare, typename Debug::self_pointer_t debug = 0) {
			allocator_ = alloc;
			#ifdef AVLTREE_DEBUG
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
		
		~DataAVLTree() {
			clear();
		}
		
		/// Beginning iterator for in-order iteration.
		inorder_iterator begin_inorder() const { return inorder_iterator(root(), allocator_); }
		
		/// End iterator for in-order iteration.
		inorder_iterator end_inorder() const { return inorder_iterator(); }
		
		/// Beginning iterator for default iteration
		iterator begin() { return iterator(root(), allocator_); }
		
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
		size_type size() const { return size_; }
		bool empty() const { return size_ == 0; }
		size_type max_size() const { return 0; }
		
		/**
		 * \tparam T Type of prototype element.
		 * \param data Element to search for
		 * \return An iterator to the found element or \ref end()
		 * 
		 * Using \ref Compare, find an element in the tree that equals data.
		 */
		/*template<typename T>
		inorder_iterator find(const T& data1) const {
			data_t data(reinterpret_cast<const block_data_t*>(&data1), sizeof(T), allocator_);
			return find(data);
		}*/
		
		inorder_iterator find(const data_t& data) {
			inorder_iterator iter(allocator_);
			find(data, root(), iter);
			if(iter.path_.size() && compare_(iter.path_.back()->data(), data) == 0) {
				return iter;
			}
			return end();
		}
		
		size_type count(const data_t& data) { return find(data) != end(); }
		
		/*template<typename T>
		const node_ptr_t insert(const T& t) {
			return insert(data_t(reinterpret_cast<const block_data_t*>(&t), sizeof(T), allocator_));
		}*/
		
		/**
		 * Insert data block into the tree. If the data is already in the tree,
		 * do nothing.
		 * 
		 * \return A pointer to the inserted or found node that contains \ref data.
		 */
		const node_ptr_t insert(const data_t& data) {
			int height_change = 0;
			node_ptr_t r;
			if(!root_) {
				root_ = allocator_->template allocate<Node>();
				root_->data_ = data;
				size_++;
				r = root_;
			}
			else {
				r = insert(data, root_, height_change);
			}
			
			check();
			//assert(r->cmp(data) == 0);
			return r;
		}
		
		/**
		 * \tparam Iterator A valid iterator type defined in this AVL tree class.
		 * Erase the node pointed to by iter from the tree and free its used
		 * memory.
		 */
		//template<typename Iterator>
		void erase(const iterator& iter) {
			int h = 0;
			node_ptr_t d = unlink(iter, root_, h);
			assert(d);
			size_--;
			allocator_->template free<Node>(d);
			
			check();
		}
	
#if AVLTREE_CHECK
		/**
		 * Check if AVL condition (balance in {-1,0,1}) is met for all nodes.
		 */
		void check_avl(node_ptr_t r) const {
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
		size_t check_balance(node_ptr_t n) const {
			if(!n) { return 0; }
			assert(n->left() != n);
			assert(n->right() != n);
			int l = check_balance(n->left()), r = check_balance(n->right());
			assert(n->balance() == (r - l)); 
			return (r >= l ? r : l) + 1;
		}
		
		/**
		 */
		size_t count_nodes(node_ptr_t n) const {
			if(!n) { return 0; }
			assert(n->left() != n);
			assert(n->right() != n);
			return 1 + count_nodes(n->left()) + count_nodes(n->right());
		}
		
		/**
		 */
		void check() const {
			check_balance(root_);
			check_avl(root_);
	
	#if DEBUG
				size_t counted = count_nodes(root_);
				assert(counted == size());
				assert((!root_) == (size_ == 0));
	#endif
		}
#else
		void check_avl(node_ptr_t) const { }
		void check_balance(node_ptr_t) const { }
		void check() const { }
#endif // AVLTREE_CHECK
		
		
#ifdef AVLTREE_DEBUG
	// {{{
		void print() {
			if(debug_) {
				debug_->debug("AVLTree< ");
				printSubtree(root_);
				debug_->debug(" size=%d>", size());
			}
		}
		
		void printNode(node_ptr_t node) {
			if(node) {
				debug_->debug("['");
				for(size_t i=0; i < (node->data().size()); i++) {
					char c = (reinterpret_cast<const char*>((node->data().c_str())))[i];
					debug_->debug("%c", c >= 32 ? c : '.');
				}
				debug_->debug("' %d]", node->balance());
			}
			else {
				debug_->debug("[null]");
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
		
			
	private:
		size_type size_;
		node_ptr_t root_;
		typename Allocator::self_pointer_t allocator_;
#ifdef AVLTREE_DEBUG
		typename Debug::self_pointer_t debug_;
#endif
		comparator_t compare_;
		
		// Return the root node
		const node_ptr_t root() const { return root_; }
		
		/* 
		 * \param data data to insert
		 * \param base base/root node under which to insert
		 * \param height_change reference to integer for tracking height changes of subtrees.
		 */
		node_ptr_t insert(const data_t& data, node_ptr_t& base, int& height_change) {
			node_ptr_t r;
			
			int c = compare_(base->data(), data); //base->cmp(data);
			if(c == 0) { // equivalent node already there, return existing node
				height_change = 0;
				return base;
			}
			
			typename Node::Side side = (c == 1) ? Node::LEFT : Node::RIGHT;
			if(base->child(side)) {
				node_ptr_t child = base->child(side);
				r = insert(data, child, height_change);
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
				r = allocator_->template allocate<Node>();
				r->init(data);
				size_++;
				base->set_child(side, r);
				if(base->balance() != 0) { height_change = +1; }
				else { height_change = 0; }
			} // else
			return r;
		} // insert()
		
		
		/*
		 * \param data Element to search for
		 * \param r root node to investigate
		 * \param iter reference to iterator that will be constructed
		 * \return an iterator pointing to an element that equals data if found,
		 *   (via \ref iter), an iterator pointing to the correct parent for
		 *   insertion of data else.
		 */
		template<typename T>
		void find(const T& data, node_ptr_t r, inorder_iterator& iter) const {
			if(r) {
				iter.path_.push_back(r);
				int c = compare_(r->data(), data); // r->cmp(data);
				if(c > 0 && r->left()) { find(data, r->left(), iter); }
				else if(c < 0 && r->right()) { find(data, r->right(), iter); }
			}
		}
		
		/*
		 * Unlink node pointed to by iter. (i.e. remove it from the tree but
		 * do not free its memory). Return the unlinked node upon success.
		 * 
		 * TODO: This is probably the ugliest piece of code I ever wrote,
		 * and could look a lot more readable and less redundant.
		 */
		template<typename Iterator>
		node_ptr_t unlink(Iterator iter, node_ptr_t& base, int& height_change) {
			assert(iter.node());
			assert(iter.path_.size());
			assert((iter.path_.size() == 1) ? (iter.path_.back() == root_) : 1);
			
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
		
		/*
		 * \param a node at the root of the rotation
		 * \param dir direction in which to rotate (left==CCW)
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
			node_ptr_t b = a->child(Node::other(dir));
			assert(b);
			height += (b->heavy_side() == Node::other(dir)) ? -1 : 0;
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
			
			check_balance(b);
			check_avl(b);
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
			typename Node::Side s = node->heavy_side(); //(node->balance_ == 2) ? Node::RIGHT : Node::LEFT;
			
			if(!(node->child(s)->is_leaf()) && (node->child(s)->heavy_side() == Node::other(s))) {
				node_ptr_t x = rotate(node->child(s), s, height);
				node->set_child(s, x);
			}
			
			node_ptr_t r = rotate(node, Node::other(s), height);
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
			
			allocator_->template free<Node>(node);
		} // clear()
		
	}; // class AVLTree
	
} // namespace

#endif // AVL_TREE_H

