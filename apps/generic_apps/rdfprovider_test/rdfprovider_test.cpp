
#include "external_interface/external_interface.h"
#include "external_interface/external_interface_testing.h"


//#define BITMAP_CHUNK_ALLOCATOR_CHECK 1

using namespace wiselib;

typedef OSMODEL Os;

int freeRam () {
	#ifdef ARDUINO
		extern int __heap_start, *__brkval; 
		int v; 
		return (int) &v - (__brkval == 0 ? (int) &__heap_start : (int) __brkval); 
	#else
		return -1;
	#endif
}


#ifdef PC
	#define DEBUG_GRAPHVIZ 1
#endif

#include "util/allocators/malloc_free_allocator.h"
typedef MallocFreeAllocator<Os> Allocator;

//#include "util/allocators/first_fit_allocator.h"
//typedef FirstFitAllocator<Os, 1024, 128> Allocator;
Allocator& get_allocator();

typedef Os::block_data_t block_data_t;
typedef Os::size_t size_type;
typedef uint16_t bitmask_t;

#include <util/broker/broker.h>
#include <util/broker/direct_broker_protocol.h>
#include <util/broker/direct_broker_protocol_command_message.h>
#include <util/broker/protobuf_rdf_serializer.h>
#include <util/broker/shdt_serializer.h>
#include <algorithms/codecs/huffman_codec.h>
#include <util/pstl/vector_dynamic.h>
#include <util/pstl/vector_dynamic_set.h>
#include <util/tuple_store/codec_tuplestore.h>
#include <util/tuple_store/null_dictionary.h>
#include <util/tuple_store/tuplestore.h>
#include <util/meta.h>

typedef BrokerTuple<Os, bitmask_t> BrokerTupleT;

#define COL(X) (1 << (X))
#define RDF_COLS (COL(0) | COL(1) | COL(2))

// TupleStore(s)


// --- < USE RAM >
	/**/

	#include <util/tuple_store/prescilla_dictionary.h>
	#include <util/pstl/unbalanced_tree_dictionary.h>
	#include <util/pstl/list_dynamic.h>

	typedef list_dynamic<Os, BrokerTupleT> TupleContainer;
	//typedef PrescillaDictionary<Os> Dictionary;
	typedef UnbalancedTreeDictionary<Os> Dictionary;
	typedef TupleStore<Os, TupleContainer, Dictionary, Os::Debug, RDF_COLS, &BrokerTupleT::compare> TupleStoreT;
	typedef CodecTupleStore<Os, TupleStoreT, HuffmanCodec<Os>, RDF_COLS> CodecTupleStoreT;

	/**/

// --- < / USE RAM >

// --- < USE BLOCKMEMORY >
	/*

	#if (ARDUINO || ISENSE)
		typedef Os::BlockMemory PhysicalBlockMemory;
	#else
		#include <algorithms/block_memory/file_block_memory.h>
		typedef FileBlockMemory<Os> PhysicalBlockMemory;
	#endif

	#include <algorithms/block_memory/bitmap_chunk_allocator.h>
	#include <algorithms/block_memory/cached_block_memory.h>

	#include <algorithms/block_memory/b_plus_hash_set.h>
	#include <algorithms/block_memory/b_plus_dictionary.h>
	#include <algorithms/hash/fnv.h>


	#if ARDUINO
		typedef CachedBlockMemory<Os, PhysicalBlockMemory, 2, 1, true> BlockMemory;
		typedef BitmapChunkAllocator<Os, BlockMemory, 8, ::uint16_t> BlockAllocator;
	#else
		typedef CachedBlockMemory<Os, PhysicalBlockMemory, 20, 4, true> BlockMemory;
		//typedef BitmapChunkAllocator<Os, BlockMemory, 8, Os::size_t> BlockAllocator;
		typedef BitmapChunkAllocator<Os, BlockMemory, 8, Os::size_t> BlockAllocator;
	#endif

	typedef Fnv32<Os> Hash;
	typedef BPlusHashSet<Os, BlockAllocator, Hash, BrokerTupleT, true> TupleContainer;
	typedef BPlusDictionary<Os, BlockAllocator, Hash> Dictionary;
	typedef TupleStore<Os, TupleContainer, Dictionary, Os::Debug, RDF_COLS, &BrokerTupleT::compare> TupleStoreT;

	// Use Huffman Codec
	typedef CodecTupleStore<Os, TupleStoreT, HuffmanCodec<Os>, RDF_COLS> CodecTupleStoreT;

	// Use no Codec
	//typedef TupleStoreT CodecTupleStoreT;

	#define USE_BLOCK_TS 1

	*/
// --- < / USE_BLOCKMEMORY >

//PrintInt<sizeof(block_data_t*)> _0;
//PrintInt<sizeof(Dictionary::key_type)> _1;
//PrintInt<sizeof(Os::size_t)> _size;

// --- Broker
typedef uint16_t bitmask_t;
typedef Broker<Os, CodecTupleStoreT, bitmask_t> broker_t;

// --- Protocols & Serializations

/*
typedef DirectBrokerProtocol<Os, broker_t, Os::Radio, Os::Debug> B2B;
typedef ShdtSerializer<Os, 10> Shdt;
typedef ProtobufRdfSerializer<Os> Protobuf;
*/


typedef broker_t::Tuple Tuple;
typedef broker_t::TupleStore::column_mask_t column_mask_t;

class App {
	public:
		void init(Os::AppMainParameter& amp) {
			debug_ = &wiselib::FacetProvider<Os, Os::Debug>::get_facet(amp);
			
			//DBG("rdfprovider test initializing (DBG)!");
		#ifdef ARDUINO
			debug_->debug("init free: %d", freeRam());
		#endif
			//radio_ = &wiselib::FacetProvider<Os, Os::Radio>::get_facet(amp);
			//timer_ = &wiselib::FacetProvider<Os, Os::Timer>::get_facet(amp);
			
			// --- Dictionary init
			
			//digitalWrite(13, LOW);
			
		#if USE_BLOCK_TS
			#if ARDUINO
				block_memory_.physical().init();
			#else
				// init actual block memory
				block_memory_.physical().init("block_memory.img");
				//block_memory_.physical().init("sde_dump");
			#endif
			
			// init cache
			block_memory_.init();
			
			// init allocator
			block_allocator_.init(&block_memory_, debug_);
			block_allocator_.wipe();
			block_memory_.reset_stats();
			
			// init block-dictionary
			dict.init(&block_allocator_, debug_);
			container.init(&block_allocator_, debug_);
		#else 
			dict.init(debug_);
		#endif
			
			debug_->debug("--- init codec_ts");
			//ts.init(&dict, &container, debug_);
			codec_ts.init(&dict, &container, debug_);
			
			debug_->debug("--- init broker");
			broker.init(&codec_ts);
			
			//broker.init(debug_);
			
			debug_->debug("--- inserting documents");
			//insert_large_document();
			insert_small_documents();
			
			//retrieve_document("doc1");
			
			debug_->debug("--- testing broker");
			test_broker();
			
		#ifdef ARDUINO
			debug_->debug("init end free: %d", freeRam());
		#endif
		}
		
		void ins(const char* s, const char *p, const char* o, bitmask_t bm) {
			typename broker_t::Tuple t;
			t.set(0, (block_data_t*)s);
			t.set(1, (block_data_t*)p);
			t.set(2, (block_data_t*)o);
			t.set_bitmask(bm);
			//broker.insert_tuple(t, bm);
			broker.tuple_store().insert(t);
		}
		
		/*
		void insert_large_document() {
			bitmask_t largemask = broker.create_document("large");
			for(size_t i=0; i < sizeof(large_document) / sizeof(large_document[0]); i++) {
				ins(large_document[i][0], large_document[i][1], large_document[i][2], largemask);
			}
		}
		*/
		
		bitmask_t doc1mask;
		bitmask_t doc2mask;
		bitmask_t doc3mask;	
		
		void insert_small_documents() {
			debug_->debug("create doc1");
			debug_->debug("init free: %d", freeRam());
			doc1mask = broker.create_document("doc1");
			debug_->debug("create doc1");
			debug_->debug("init free: %d", freeRam());
			doc2mask = broker.create_document("doc2");
			debug_->debug("create doc1");
			debug_->debug("init free: %d", freeRam());
			doc3mask = broker.create_document("doc3");
			debug_->debug("ins");
			debug_->debug("init free: %d", freeRam());
			ins("ab", "7777777", "dd", doc1mask );
			debug_->debug("ins");
			debug_->debug("init free: %d", freeRam());
			ins("ab", "88888888", "xx", doc1mask );
			debug_->debug("ins");
			debug_->debug("init free: %d", freeRam());
			ins("yy1", "999999999", "abx", doc1mask | doc3mask);
			debug_->debug("ins");
			ins("yy2", "999999999", "ab", doc1mask | doc3mask);
			debug_->debug("ins");
			ins("ab", "a1", "xx", doc1mask | doc2mask);
			debug_->debug("ins");
			ins("yy3", "999999999", "abx", doc1mask | doc3mask);
			debug_->debug("ins");
			ins("ab", "a2", "xx", doc1mask | doc2mask);
			debug_->debug("ins");
			ins("ab", "a3", "xx", doc1mask | doc2mask);
			debug_->debug("ins");
			ins("yy4", "999999999", "ab", doc1mask | doc3mask);
			debug_->debug("ins");
			ins("yy5", "999999999", "ab", doc1mask | doc3mask);
			debug_->debug("ins");
			ins("yy6", "999999999", "ab", doc1mask | doc3mask);
			debug_->debug("ins done");
			
			//debug_->debug("--------- after even more:");
			//block_memory_.flush();
			//block_memory_.print_stats();
		}
		
		void test_broker() {
			//bitmask_t doc1mask = broker.create_document("doc1");
			//bitmask_t doc2mask = broker.create_document("doc2");
			//bitmask_t doc3mask = broker.create_document("doc3");
			//recvmask = broker.create_document("recv");
			
			debug_->debug("masks=%x %x %x\n", (int)doc1mask, (int)doc2mask, (int)doc3mask); // (int)recvmask);
			
			typename broker_t::Tuple t;
			broker_t::iterator iter;
			
			//ins("ab", "7777777", "dd", doc1mask );
			//ins("ab", "88888888", "xx", doc1mask | doc2mask);
			//ins("yy", "999999999", "ab", doc1mask | doc3mask);
			
			debug_->debug("--- iter doc1:");
			for(iter = broker.begin_document(doc1mask); iter != broker.end_document(doc1mask); ++iter) {
				broker_t::Tuple t = *iter;
				debug_->debug("(%s %s %s  mask=%x)", t.get(0), t.get(1), t.get(2), t.bitmask());
			}
			
			debug_->debug("--- iter doc2:");
			for(iter = broker.begin_document(doc2mask); iter != broker.end_document(doc2mask); ++iter) {
				broker_t::Tuple t = *iter;
				debug_->debug("(%s %s %s  mask=%x)", t.get(0), t.get(1), t.get(2), t.bitmask());
			}
			
			debug_->debug("--- iter doc3:");
			for(iter = broker.begin_document(doc3mask); iter != broker.end_document(doc3mask); ++iter) {
				broker_t::Tuple t = *iter;
				debug_->debug("(%s %s %s  mask=%x)", t.get(0), t.get(1), t.get(2), t.bitmask());
			}
			
			debug_->debug("--- now deleting all (ab ? ?) tuples");
			broker_t::Tuple query;
			query.set(0, (block_data_t*)"ab");
			broker_t::TupleStore::iterator it = broker.tuple_store().begin(&query, (1 << 0));
			
			while(it != broker.tuple_store().end()) {
				broker_t::Tuple t = *it;
				debug_->debug("would erase: (%s %s %s mask=%x)",
					t.get(0), t.get(1), t.get(2), t.bitmask());	
				it = broker.tuple_store().erase(it);
				//++it;
			}
			
			debug_->debug("--- iter doc1:");
			for(broker_t::iterator iter = broker.begin_document(doc1mask); iter != broker.end_document(doc1mask); ++iter) {
				debug_->debug("(%s %s %s  mask=%x)", iter->get(0), iter->get(1), iter->get(2), iter->bitmask());
			}
			
			//query.set(0, (block_data_t*)"yy");
			//it = broker.tuple_store().begin(&query, (1 << 0));
			//broker.tuple_store().erase(it);
			
			//debug_->debug("--- iter doc1:\n");
			//for(broker_t::iterator iter = broker.begin_document(doc1mask); iter != broker.end_document(doc1mask); ++iter) {
				//debug_->debug("(%s %s %s  mask=%x)\n", iter->get(0), iter->get(1), iter->get(2), iter->bitmask());
			//}
		}
		
#if 0
		void retrieve_document(char * docname) {
			bitmask_t mask = broker.get_document_mask(docname);
			debug_->debug("--- retrieve: %s\n", docname);
			for(broker_t::iterator iter = broker.begin_document(mask); iter != broker.end_document(mask); ++iter) {
				debug_->debug("(%s %s %s  mask=%x qrymask=%x)\n", iter->get(0), iter->get(1), iter->get(2), iter->bitmask(), iter.query().bitmask());
			}
		}
#endif	
	private:
	
	#if USE_BLOCK_TS
		BlockMemory block_memory_;
		BlockAllocator block_allocator_;
	#endif
		
		TupleStoreT::Dictionary dict;
		TupleStoreT::TupleContainer container;
		//DictStore ts;
		CodecTupleStoreT codec_ts;
		broker_t broker;
		bitmask_t recvmask;
		
		Os::Debug::self_pointer_t debug_;
};

wiselib::WiselibApplication<Os, App> app;

Allocator allocator_;
Allocator& get_allocator() { return allocator_; }

void application_main(Os::AppMainParameter& amp) {
	app.init(amp);
}

/* vim: set ts=4 sw=4 tw=78 noexpandtab foldmethod=marker foldenable :*/

