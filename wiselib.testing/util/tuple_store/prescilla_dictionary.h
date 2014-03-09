/*
 * File:   PrescillaDict.h
 * Author: maxpagel
 *
 * Created on 23. Februar 2012, 14:40
 */

#ifndef _PRESCILLADICT_H
#define _PRESCILLADICT_H

#include <util/pstl/bit_array.h>
#include <util/meta.h>

namespace wiselib
{

    template<
    typename OsModel_P,
             typename Debug_P = typename OsModel_P::Debug
    >
    class PrescillaDictionary
    {
        class Node;
    public:

        typedef OsModel_P OsModel;
        typedef Debug_P Debug;
        typedef PrescillaDictionary<OsModel> self_type;
        typedef self_type* self_pointer_t;

        typedef typename OsModel::block_data_t block_data_t;
        typedef typename OsModel::size_t size_type;

        //typedef block_data_t* value_type;
        typedef BitArray<OsModel> bitarray_t;
        //typedef bitarray_t* value_type;
        typedef block_data_t* value_type;

        typedef typename Node::self_pointer_t node_pointer;
        //typedef node_pointer key_type;
        typedef typename Uint<sizeof(node_pointer)>::t key_type;
        
        typedef block_data_t* mapped_type;

        enum
        {
            ABSTRACT_KEYS = true
        };

    private:
        typedef self_type PrescillaDict;

        class Node
        {
            // {{{
        public:
            typedef Node self_type;
            typedef self_type* self_pointer_t;

            Node() : parent_(0), count_(0), data_size_(0), data_(0)
            {
                children_[0] = 0;
                children_[1] = 0;
            }
            
            ~Node() {
               ::get_allocator().free_array(data_);
            }

            void init()
            {
                parent_ = 0;
                count_ = 0;
                data_ = 0;
                data_size_ = 0;
                children_[0] = 0;
                children_[1] = 0;
            }

            void init(typename bitarray_t::self_pointer_t data,
                    self_pointer_t left_child, self_pointer_t right_child,
                    self_pointer_t parent, uint16_t data_size)
            {
                data_ = data;
                data_size_ = data_size;
                //printf("node init data=%s len=%d\n", data->c_str(), data->size());

                parent_ = parent;
                children_[0] = left_child;
                children_[1] = right_child;
            }

            bool is_root()
            {
                return !parent_;
            }

            size_type data_size()
            {
                if (data_)
                {
                    return data_size_;
                }
                return 0;
            }

            bitarray_t& data()
            {
                return *data_;
            }
            
            bool is_leaf() {
               return children_[0] == 0 && children_[1] == 0;
            }
            Node* parent() { return parent_; }
            
            /*bool unused() { return refcount_ == 0; }
               
            void increment_refcount() { refcount_++; }
            void decrement_refcount() { refcount_--; }
            size_type refcount() { return refcount_; }*/
        private:
            self_pointer_t children_[2];
            self_pointer_t parent_;
            uint16_t count_;
            uint16_t data_size_;

            // TODO: make this a VLA
            typename bitarray_t::self_pointer_t data_;

            friend class PrescillaDictionary;
            // }}}
        };

    public:
        
        class iterator {
           public:
              
              iterator(Node* n) : node_(n) {
                 find_first();
                 if(node_ && node_->is_root()) {
                    go_down();
                 }
                 if(node_ && node_->is_root()) {
                    node_ = 0;
                 }
              }
              
              key_type operator*() { return reinterpret_cast<key_type>(node_); }
              iterator& operator++() {
                 if(node_->is_leaf()) {
                    go_up();
                    if(node_ && node_->is_root()) {
                       go_down();
                    }
                    else if(node_ && node_->count_ == 0) {
                       go_down();
                    }
                 }
                 else {
                    go_down();
                 }
                 return *this;
              }
              
              bool operator==(const iterator& other) {
                 return node_ == other.node_;
              }
              bool operator!=(const iterator& other) {
                 return !(*this == other);
              }
                 
              void go_up() {
                 Node *child = node_;
                 node_ = node_->parent();
                 
                 // as long coming from right, go up.
                 while(node_ && node_->children_[1] == child) {
                    child = node_;
                    node_ = node_->parent();
                 }
              }
              
              void go_down() {
                 node_ = node_->children_[1];
                 find_first();
              }
              
              void find_first() {
                 while(node_ && node_->children_[0]) {
                    node_ = node_->children_[0];
                 }
              }

              node_pointer node() { return node_; }
              
           private:
              Node *node_;
        };

        //enum { NULL_KEY = 0 };
        static const key_type NULL_KEY; // = 0; // = 0;

        PrescillaDictionary() : root_(0)
        {
        }
        
        ~PrescillaDictionary() {
           if(root_) {
              ::get_allocator().free(root_);
           }
        }

        int init(typename Debug::self_pointer_t debug)
        {
            if (!root_)
            {
                root_ = ::get_allocator().template allocate<Node > ().raw();
            }
           debug_ = debug;
            return OsModel::SUCCESS;
        }

        /**
         * value will be copied into the dictionary (non-shallowly, i.e.
         * you may remove the original value afterwards!)
         */
        key_type insert(value_type value)
        {
            size_type v_size = strlen(reinterpret_cast<char*>(value)) * 8;
            bitarray_t *v_; // = *reinterpret_cast<typename bitarray_t::self_pointer_t> (value);
            //printf("root data size: %d  v=%s\n", root_->data_size(), v.c_str());
            memcpy((void*)&v_, (void*)&value, sizeof(bitarray_t*));
            bitarray_t &v = *v_;
            
            node_pointer current_node = root_;
            if (!current_node->children_[v[0]])
            {
                node_pointer new_node = ::get_allocator().template allocate<Node > ().raw();
                //printf("--- A\n");
                typename bitarray_t::self_pointer_t bits_copy = bitarray_t::make(::get_allocator(), v_size);
                v.copy(bits_copy, 0, 0, v_size);
                        new_node->init(bits_copy, 0, 0, root_, v_size);
                        root_->children_[v[0]] = new_node;
                        new_node->count_++;
                        size_++;
            //printf("insert() a -> %p\n", new_node);
                return reinterpret_cast<key_type>(new_node);
            } else
            {
                current_node = current_node->children_[v[0]];
            }
            
            size_type string_index = 0;
                    size_type current_node_string_index = 0;
                    //printf("v=%s\n", v.c_str());
            while (string_index < v_size && current_node->data_->operator[](current_node_string_index) == v[string_index])
            {
            //debug_->debug("loop");
                //printf("%d(%d) ", v[string_index] ? 1 : 0, string_index);
                string_index++;
                        current_node_string_index++;

                if (current_node_string_index >= current_node->data_size() && string_index < v_size)
                {
                    if (current_node->children_[v[string_index]])
                    {
                        // decend to next child and continue matching
                        //printf("descend to %s (%d)\n", current_node->data_->c_str(), current_node->data_size());
                        current_node = current_node->children_[v[string_index]];
                                current_node_string_index = 0;
                    } else
                    {
                        // String has prefix in tree, tail will be added as new child node
                        typename bitarray_t::self_pointer_t bits_left = bitarray_t::make(::get_allocator(), v_size - string_index);

                                v.copy(bits_left, 0, string_index, v_size - string_index);
                                node_pointer new_node = ::get_allocator().template allocate<Node > ().raw();
                                //printf("--- B\n");
                                new_node->init(bits_left, 0, 0, current_node, v_size - string_index);
                                string_index = v_size;
                                current_node->children_[new_node->data_->operator[](0)] = new_node;
                                new_node->count_++;
                        if (new_node->count_ == 1)
                        {
                            size_++;
                        }
            //printf("insert() b -> %p\n", new_node);
                        return reinterpret_cast<key_type>(new_node);
                    }
                }
                //debug_->debug("le cn=%x cnsi=%d si=%d", (int)current_node, (int)current_node_string_index, (int)string_index);
                //debug_->debug("cn.data=%x", (int)current_node->data_);
                //debug_->debug("cn.datalen=%d", (int)current_node->data_size());
            } // while
            //debug_->debug("c");
            //printf("\n");
            //printf("stridx=%d vsize=%d curidx=%d v=%s cur=%s\n",
            //string_index, v.size(), current_node_string_index, v.c_str(),
            //current_node->data_->c_str());

            // String is path in tree, check if it already exists, or tail is prefix for current Node
            if (current_node_string_index < current_node->data_size())
            {
                //assert(current_node->data_size() >= current_node_string_index);
                //printf("current node data size: %d curidx: %d v=%s curnode=%s\n", current_node->data_size(),
                //current_node_string_index, current_node->data_->c_str(), v.c_str());


                typename bitarray_t::self_pointer_t bit_string_front = bitarray_t::make(::get_allocator(), current_node_string_index),
                        bit_string_back = bitarray_t::make(::get_allocator(), current_node->data_size() - current_node_string_index);
            size_type backsize = current_node->data_size() - current_node_string_index;
            size_type frontsize = current_node_string_index;
            
                        current_node->data_->copy(bit_string_front, 0, 0, current_node_string_index);
                        current_node->data_->copy(bit_string_back, 0, current_node_string_index, current_node->data_size() - current_node_string_index);

                        //printf("split! front=%s(%d) back=%s(%d)\n\n",
                        //bit_string_front->c_str(), bit_string_front->size(),
                        //bit_string_back->c_str(), bit_string_back->size()
                        //);

                        node_pointer split_node = ::get_allocator().template allocate<Node > ().raw();
                        //printf("--- C\n");
                        split_node->init(bit_string_front, 0, 0, current_node->parent_, frontsize);
                        current_node_string_index = current_node->data_size();
                        current_node->parent_->children_[split_node->data_->operator[](0)] = split_node;
                        
                        if(current_node->data_) {
                           ::get_allocator().free_array(current_node->data_);
                        }
                        
                        current_node->data_ = bit_string_back;
                        current_node->data_size_ = backsize; //current_node->data_size() - current_node_string_index;
                        current_node->parent_ = split_node;
                        split_node->children_[current_node->data_->operator[](0)] = current_node;
                        current_node = split_node;
            }

            if (string_index >= v_size)
            {
                // string is substring of existing Path, increment count
                current_node->count_++;
                if (current_node->count_ == 1)
                {
                    size_++;
                }
            //debug_->debug("f");
            //printf("insert() c -> %p\n", current_node);
                return reinterpret_cast<key_type>(current_node);
            } else
            {
            //debug_->debug("g");
                ///string shares prefix in tree up to string_index but has differnet suffix, append string as new branch after split
                // valid here: current_node->data_[current_node_string_index] != value[string_index]
                node_pointer value_rest = ::get_allocator().template allocate<Node > ().raw();
                        typename bitarray_t::self_pointer_t bitString = bitarray_t::make(::get_allocator(), v_size - string_index);
                        v.copy(bitString, 0, string_index, v_size - string_index);
                        //printf("--- D\n");
                        value_rest->init(bitString, 0, 0, current_node, v_size - string_index);
                        current_node->children_[value_rest->data_->operator[](0)] = value_rest;
                        value_rest->count_++;
                        size_++;

            //printf("insert() d -> %p\n", value_rest);
                return reinterpret_cast<key_type>(value_rest);
            }
        } // insert()

        key_type find(value_type value)
        {
            bitarray_t& v = *reinterpret_cast<typename bitarray_t::self_pointer_t> (value);
                    size_type v_size = strlen(reinterpret_cast<char*>(&v)) * 8;
            if (!root_)
            {
                return NULL_KEY;
            }
            size_type string_index = 0,
                    current_node_string_index = 0;
                    node_pointer current_node = root_;

            while (string_index < v_size)
            {
                if (current_node_string_index >= current_node->data_size())
                {
                    if (current_node->children_[v[string_index]])
                    {
                        // decend to next child and continue matching
                        current_node = current_node->children_[v[string_index]];
                                current_node_string_index = 0;
                                //printf("descend\n");
                    } else
                    {
                        return NULL_KEY;
                    }
                }

                if (current_node->data_->operator[](current_node_string_index) == v[string_index])
                {
                    string_index++;
                            current_node_string_index++;
                            //continue;
                } else
                {
                    return NULL_KEY; }
            }

            // String is path in tree, check if it already exists, or tail is prefix for current node
            if (current_node_string_index >= current_node->data_size() && (current_node->count_ > 0))
            {

                return reinterpret_cast<key_type>(current_node);
            }

            return NULL_KEY;
        }

        //void erase(DictionaryEntry* entry) {
        void erase(key_type entry_)
        {
           //printf("erase(%p)\n", entry);
           //fflush(stdout);
          
          node_pointer entry = reinterpret_cast<node_pointer>(entry_); 
            node_pointer current_node = entry;
                    entry->count_--;
            if (entry->count_ > 0)
            {
                return;
            }

            if (!current_node->is_root() && !current_node->children_[0]
                    && !current_node->children_[1] && !current_node->count_)
            {
                if (current_node->parent_)
                {
                    current_node->parent_->children_[current_node->data_->operator[](0)] = 0;
                }
                node_pointer del_this = current_node;
                        current_node = current_node->parent_;
           //printf("erase.free(%p)\n", del_this);
           //fflush(stdout);
                        ::get_allocator().free(del_this);
            }

            // check for merge
            if (current_node && !current_node->is_root() && current_node->count_ == 0)
            {
                node_pointer node_to_merge = 0;
                if (current_node->children_[0] && !current_node->children_[1])
                {
                    node_to_merge = current_node->children_[0];
                            current_node->children_[0] = 0;
                } else if (!current_node->children_[0])
                {
                    node_to_merge = current_node->children_[1];
                            current_node->children_[1] = 0;
                }

                if (node_to_merge)
                {

                    typename bitarray_t::self_pointer_t bit_string = bitarray_t::make(::get_allocator(), current_node->data_size() + node_to_merge->data_size());
                            current_node->data_->copy(bit_string, 0, 0, current_node->data_size());
                            node_to_merge->data_->copy(bit_string, current_node->data_size(), 0, node_to_merge->data_size());
                            if(node_to_merge->data_ != 0) {
                               ::get_allocator().free_array(node_to_merge->data_);
                            }
                            node_to_merge->data_ = bit_string;
                            node_to_merge->data_size_ = current_node->data_size() + node_to_merge->data_size();
                            node_to_merge->parent_ = current_node->parent_;
                            node_to_merge->parent_->children_[node_to_merge->data_->operator[](0)] = node_to_merge;
           //printf("erase.free(%p)\n", current_node);
           //fflush(stdout);
                            ::get_allocator().free(current_node);
                }
            }

            //return count;
        }

        iterator begin_keys() {
           return iterator(root_);
        }
         
        iterator end_keys() {
           return iterator(0);
        }
        
        size_type count(key_type k) {
           node_pointer current_node = reinterpret_cast<node_pointer>(k);
           return current_node->count_;
        }

        size_type count(iterator iter) {
           node_pointer n = iter.node();
           return n->count_;
        }

        value_type get_copy(key_type k)
        {
           //printf("get_copy(%p)\n", k);
           //fflush(stdout);
           
            size_type len = 0, h = 0;

            // first, find out height of this node and number of
            // bits on path to root.
            node_pointer current_node = reinterpret_cast<node_pointer>(k);
                //printf("current=%p\n", current_node);
                //fflush(stdout);
            for (; !current_node->is_root(); current_node = current_node->parent_)
            {
                len += current_node->data_size();
                        h++;
                //printf("current=%p\n", current_node);
                //fflush(stdout);
            }
            //printf("get_value k=%p l=%d h=%d\n", k, l, h);

            // Now store path to root in a bitarray
            typename bitarray_t::self_pointer_t path = bitarray_t::make(::get_allocator(), h);
                    size_type i = 0;
            for (current_node = reinterpret_cast<node_pointer>(k); !current_node->is_root(); current_node = current_node->parent_, i++)
            {
                path->set(i, current_node->data_->operator[](0));
            }

            // Concatenate all bits into $out
            typename bitarray_t::self_pointer_t out = bitarray_t::make(::get_allocator(), len + 8);
                    size_type out_pos = 0;
                    size_type p;
            for (p = h - 1; p > 0; p--)
            {
                current_node = current_node->children_[path->operator[](p)];

                        // ---- debug ----
                        //printf("copying: %s l=%d\n", current_node->data_->c_str(), current_node->data_size());
                        //for(int j=0; j<current_node->data_->size(); j++) {
                        //printf("%d", current_node->data_->operator[](j) ? 1 : 0);
                        //}
                        //printf("\n");
                        // ---- /debug ----

                        current_node->data_->copy(out, out_pos, 0, current_node->data_size());
                        out_pos += current_node->data_size();
            }
            current_node = current_node->children_[path->operator[](p)];

                    // ---- debug ----
                    //printf("copying: %s l=%d\n", current_node->data_->c_str(), current_node->data_size());
                    //for(int j=0; j<current_node->data_->size(); j++) {
                    //printf("%d", current_node->data_->operator[](j) ? 1 : 0);
                    //}
                    //printf("\n");
                    current_node->data_->copy(out, out_pos, 0, current_node->data_size());

                    ::get_allocator().free_array(path);
                    out->terminate(len); //set(len, '\0');

            return reinterpret_cast<block_data_t*> (out);
        }
        
        value_type get_value(key_type k) { return get_copy(k); }

        void free_value(value_type v) { ::get_allocator().free_array(v); }

    private:
        size_type size_;
        node_pointer root_;
        typename Debug::self_pointer_t debug_;

    friend class Node;
    };

    template<
    typename OsModel_P, typename Debug_P
            >
            const typename PrescillaDictionary<OsModel_P, Debug_P>::key_type
            PrescillaDictionary<OsModel_P, Debug_P>::NULL_KEY =
            typename PrescillaDictionary<OsModel_P, Debug_P>::key_type();
}

#endif   /* _PRESCILLADICT_H */

/* vim: set ts=3 sw=3 tw=78 expandtab :*/
