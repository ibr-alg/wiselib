
/*
 * This file defines a Tuple type
 * and instantiates an according TupleStore using lots of comments
 * to explain what is going on.
 */

// Some utility methods for template magic
#include <util/meta.h>

// First, define a Tuple type.
// Usually, that will look like this:

/**
 * See doc/concepts/tuple_store/tuple.dox for details on what the methods do
 * @ingroup Tuple_concept
 */
class Tuple {
	// {{{
	public:
		// Each tuple element must be able to hold a dictionary key (32 bit)
		// or a block_data_t*. Depending on the OS, one or the other
		// may be larger.
		// Define value_t to be an unsigned integer that is large
		// enough for either of them using black template magic.
		
		typedef Uint< Max< sizeof(block_data_t*), 4 >::value >::t value_t;
		
		
		typedef Tuple self_type;
		
		// Number of elements per tuple
		enum { SIZE = 3 };
		
		Tuple() {
			for(size_type i = 0; i < SIZE; i++) { data_[i] = 0; }
		}
		
		void free_deep(size_type i) {
			if(get(i)) {
				::get_allocator().free(get(i));
				set(i, 0);
			}
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
		
		void set_deep(size_type i, block_data_t* data) {
			size_type l = strlen((char*)data) + 1;
			block_data_t *p = ::get_allocator().allocate_array<block_data_t>(l * sizeof(block_data_t)) .raw();
			memcpy(p, data, l);
			set(i, p);
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

#if !TS_USE_BLOCK_MEMORY

	// ---- TupleStore instantiation in RAM ----

	/*
	 * Now instantiate the TupleStore.
	 * A TupleStore has a TUPLE CONTAINER, storing tuples of dictionary keys
	 * and a DICTIONARY storing strings.
	 * 
	 * As container we can use almost any STL / pSTL container of tuples, eg.
	 * list_dynamic, vector_dynamic, vector_static, list_static.
	 * 
	 * Not that the implementation of the TupleContainer defines whether
	 * multiple instances of the same tuple can occur in your store or not!
	 */

	//#include <util/pstl/list_dynamic.h>
	//typedef list_dynamic<Os, Tuple> TupleContainer;
	
	#if TS_USE_VECTOR_STATIC_CONTAINER
		#include <util/pstl/vector_static.h>
		//typedef wiselib::vector_static<Os, Tuple, 76, false> TupleContainer;
		typedef wiselib::vector_static<Os, Tuple, TS_CONTAINER_SIZE, false> TupleContainer;

	#elif TS_USE_SET_STATIC_CONTAINER
		#include <util/pstl/vector_static.h>
		#include <util/pstl/set_vector.h>
		typedef wiselib::vector_static<Os, Tuple, TS_CONTAINER_SIZE, false> TupleVector;
		typedef wiselib::set_vector<Os, TupleVector> TupleContainer;

	#endif

	/* There are a number of dictionary implementations available currently
	 * available:
	 * 
	 * NullDictionary:
	 * 		Don't really store strings into the dictionary. Therefore it is
	 * 		necessary that any strings inserted into the TS are never freed or
	 * 		changed!
	 * 		
	 * UnbalancedTreeDictionary:
	 * 		A simple non-rebalancing binary search tree of strings that stores
	 * 		each string only once (+ a reference count).
	 * 		
	 * AvlDictionary:
	 * 		Like UnbalancedTreeDictionary, but use an balanced AVL tree to
	 * 		guarantee logarithmic string insertion (note that lookup by dict key
	 * 		is in O(1) in both cases.
	 * 		
	 * PrescillaDictionary:
	 * 		Dictionary implementation based on a PATRICIA Trie (aka Radix Trie),
	 * 		This stores common prefixes only once, useful if tuple elements
	 * 		exhibit common prefixes, e.g. because they are URIs.
	 */
	//#include <util/tuple_store/null_dictionary.h>
	//typedef NullDictionary<Os> Dictionary;
	
	//#include <util/pstl/unbalanced_tree_dictionary.h>
	//typedef UnbalancedTreeDictionary<Os> Dictionary;

	//#include <util/tuple_store/prescilla_dictionary.h>
	//typedef PrescillaDictionary<Os> Dictionary;

	#if TS_USE_CHOPPER_DICT
		#warning "USING CHOPPER DICT"
		#include <util/tuple_store/static_dictionary.h>
		//typedef StaticDictionary<Os, 200, 15> Dictionary;
		//typedef StaticDictionary<Os, 100, 15> Dictionary;
		typedef StaticDictionary<Os, TS_DICT_SIZE, TS_DICT_SLOTSIZE> Dictionary;

	#elif TS_USE_TREE_DICT
		#warning "USING TREE DICT"
		#include <util/pstl/unbalanced_tree_dictionary.h>
		typedef UnbalancedTreeDictionary<Os> Dictionary;

	#elif TS_USE_PRESCILLA_DICT
		#warning "USING PRESCILLA DICT"
		#include <util/tuple_store/prescilla_dictionary.h>
		typedef PrescillaDictionary<Os> Dictionary;

	#elif TS_USE_AVL_DICT
		#warning "USING AVL DICT"
		#include <util/tuple_store/avl_dictionary.h>
		typedef AvlDictionary<Os> Dictionary;
	#endif
	
#else
	// ---- TupleStore instantiation on BLOCK MEMORY

	/* TupleContainer and/or dictionary can also be stored on any block memory,
	 * e.g. an SD card.
	 * Both the block memory based tuple container as well as the block memory
	 * based dictionary rely on a block allocator which manages free space on the
	 * block device. That in turn works on top of a block memory cache which needs
	 * to be able to cache at least two two blocks in RAM.
	 */

	#ifdef PC
		#include <algorithms/block_memory/file_block_memory.h>
		typedef FileBlockMemory<Os> PhysicalBlockMemory;
	#else
		typedef Os::BlockMemory PhysicalBlockMemory;
	#endif
	
	#include <algorithms/block_memory/bitmap_chunk_allocator.h>
	#include <algorithms/block_memory/cached_block_memory.h>
	typedef CachedBlockMemory<Os, PhysicalBlockMemory,
			10 /* Total cache size (>= special area + 1) */,
			 3 /* Size of special cache are (for allocator) */,
			true /* enable write-through */
		> BlockCache;
		
	typedef BitmapChunkAllocator<Os, BlockCache,
			8 /* Chunk size in bytes */,
			Os::size_t
		> BlockAllocator;

	/* Both block tuple container and block dictionary build on a b-plus-tree
	 * that uses hash values of tuples / strings as keys.
	 * We can select from a large number of Wiselib hash functions or provide our
	 * own.
	 * For strings from the billion triple challenge dataset it turns out that
	 * SDBM is good choice if you want a 32 bit hash.
	 */
	#include <algorithms/hash/sdbm.h>
	typedef Sdbm<Os> Hash;

	#include <algorithms/block_memory/b_plus_hash_set.h>
	typedef BPlusHashSet<
			Os, BlockAllocator,
			Hash, Tuple,
			true /* Only allow one instance of each value, ie. not a multiset */
		> TupleContainer;

	#include <algorithms/block_memory/b_plus_dictionary.h>
	typedef BPlusDictionary<Os, BlockAllocator, Hash> Dictionary;

#endif // TS_USE_BLOCK_MEMORY



// Actual TupleStore instantiation
#include <util/tuple_store/tuplestore.h>
typedef TupleStore<
		Os,
		TupleContainer, Dictionary,
		Os::Debug,
		//BIN(111), /* Which columns should the dictionary be used for? */
		BIN(111),
		&Tuple::compare /* How to compare tuples? */
	> TupleStoreT;

// Independently of what container / dictionary implementation we use,
// we may want to choose to apply element-wise compression, eg. using Huffman
// codec.

// Don't use a codec

#if TS_CODEC_NONE
	typedef TupleStoreT CodecTupleStoreT;

#elif TS_CODEC_HUFFMAN

	// Use Huffman codec
	#include <algorithms/codecs/huffman_codec.h>
	#include <util/tuple_store/codec_tuplestore.h>
	typedef CodecTupleStore<
			Os, TupleStoreT,
			HuffmanCodec<Os> /* use this codec */,
			BIN(111) /* Use codec on these columns */
		> CodecTupleStoreT;
#endif


