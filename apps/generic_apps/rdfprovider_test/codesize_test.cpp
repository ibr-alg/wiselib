
#if defined(CONTIKI) || defined(TINYOS)
extern "C" {
	#include <string.h>
}
	#define assert(X)
#endif

#include <external_interface/external_interface.h>
#include <external_interface/external_interface_testing.h>

using namespace wiselib;

#if defined(ISENSE)
	#include <string.h>
	void assert(int) { }
	char *strncpy(char *dest, const char *src, size_t n) {
		const char *end = src + n;
		for( ; src < end && *src; src++, dest++) {
			*dest = *src;
		}
		if(src < end) { *dest = '\0'; }
		return dest;
	}
#endif
typedef OSMODEL Os;

#if defined(CONTIKI) || defined(TINYOS)
	#warning "Using BITMAP allocator"
	#include <util/allocators/bitmap_allocator.h>
	//typedef wiselib::BitmapAllocator<Os, 2000> Allocator;
	typedef wiselib::BitmapAllocator<Os, 1400> Allocator;
#else
	#include <util/allocators/malloc_free_allocator.h>
	typedef wiselib::MallocFreeAllocator<Os> Allocator;
#endif
Allocator allocator_;
Allocator& get_allocator() { return allocator_; }

#include <util/pstl/unique_container.h>
#include <util/tuple_store/static_dictionary.h>
#include <util/pstl/unbalanced_tree_dictionary.h>
#include <util/tuple_store/prescilla_dictionary.h>
#include <util/tuple_store/avl_dictionary.h>
#include <util/pstl/vector_static.h>
#include <util/pstl/set_vector.h>
#include <util/tuple_store/tuplestore.h>
#include <algorithms/hash/sdbm.h>
#include <util/meta.h>
#include <util/broker/broker.h>

typedef Os::block_data_t block_data_t;
typedef Os::size_t size_type;
typedef ::uint16_t bitmask_t;

typedef BrokerTuple<Os, bitmask_t> BrokerTupleT;
typedef BrokerTupleT TupleT;

#if defined(CODESIZE_DICT_TREE) || defined(CODESIZE_DICT_PRESCILLA) || defined(CODESIZE_DICT_CHOPPER) || defined(CODESIZE_DICT_AVL)
	#define CODESIZE_DICT 1
#endif
#if defined(CODESIZE_CONTAINER) || defined(CODESIZE_DICT) || defined(CODESIZE_BROKER)
	#define CODESIZE_TS 1
#endif


#if defined(CODESIZE_CONTAINER)
	typedef vector_static<Os, TupleT, 10> TupleVector;
	typedef set_vector<Os, TupleVector> TupleContainer;
#endif


#if defined(CODESIZE_DICT_TREE)
	typedef UnbalancedTreeDictionary<Os> Dictionary;
#elif defined(CODESIZE_DICT_PRESCILLA)
	typedef PrescillaDictionary<Os> Dictionary;
#elif defined(CODESIZE_DICT_CHOPPER)
	typedef StaticDictionary<Os, 10, 8> Dictionary;
#elif defined(CODESIZE_DICT_AVL)
	typedef AvlDictionary<Os> Dictionary;
#endif


#if defined(CODESIZE_TS)
	typedef TupleStore<Os, TupleContainer, Dictionary, Os::Debug,
		BIN(111), &TupleT::compare> PlainTupleStoreT;
	typedef PlainTupleStoreT::column_mask_t column_mask_t;
#endif

#if defined(CODESIZE_TS)
	#if defined(CODESIZE_HUFFMAN)
		#include <algorithms/codecs/huffman_codec.h>
		#include <util/tuple_store/codec_tuplestore.h>
		typedef CodecTupleStore<
				Os, PlainTupleStoreT,
				HuffmanCodec<Os> /* use this codec */,
				BIN(111) /* Use codec on these columns */
			> TupleStoreT;
	#else
		typedef PlainTupleStoreT TupleStoreT;
	#endif
#endif

#if defined(CODESIZE_BROKER)
	typedef Broker<Os, TupleStoreT, bitmask_t> broker_t;
#endif

#if defined(CODESIZE_SHDT)
	#include <util/broker/shdt_serializer.h>
	typedef wiselib::ShdtSerializer<Os, 64, 4> Shdt;
#endif

// TODO: SHDT

class App {
	public:
		void init(Os::AppMainParameter& amp) {
			debug_ = &wiselib::FacetProvider<Os, Os::Debug>::get_facet(amp);

		#if defined(CODESIZE_DICT)
			dict.init(debug_);
		#endif
			
		#if defined(CODESIZE_TS)
			ts.init(&dict, &container, debug_);
		#endif
		#if defined(CODESIZE_BROKER)
			broker.init(&ts);
		#endif

			use_allocator();
			use_ts();
			use_broker();
			use_shdt();
		}

		void use_allocator() {
			::get_allocator().allocate<int>();
			::get_allocator().allocate_array<int>(2);
			int x;
			::get_allocator().free(&x);
			::get_allocator().free_array(&x);
		}

		void use_broker() {
#if defined(CODESIZE_BROKER)
			bitmask_t doc1mask = broker.create_document("x");
			for(broker_t::iterator iter = broker.begin_document(doc1mask); iter != broker.end_document(doc1mask); ++iter) {
				broker_t::Tuple t = *iter;
			}

			// TODO:
			// publish/subscribe?
#endif
		}

		void use_ts() {
#if defined(CODESIZE_TS)
			TupleT t;
			ts.insert(t);
			for(TupleStoreT::iterator iter = ts.begin(); iter != ts.end(); ++iter) {
				iter->get(0);
				iter = ts.erase(iter);
			}
#endif
		}

		void use_shdt() {
#if defined(CODESIZE_SHDT)
			block_data_t buffer[1];
			Shdt::Writer w(&shdt, buffer, 100, Shdt::write_callback_t::from_method<App, &App::shdt_recv_tuple>(this));
			w.write_header(64, 3);
			TupleT tuple;
			w.write_tuple(tuple);
#endif
		}

#if defined(CODESIZE_SHDT)
		void shdt_recv_tuple(Shdt::Writer& w) {
		}
#endif

	private:
		Os::Debug::self_pointer_t debug_;

#if defined(CODESIZE_CONTAINER)
		TupleContainer container;
#endif
#if defined(CODESIZE_DICT)
		Dictionary dict;
#endif
#if defined(CODESIZE_TS)
		TupleStoreT ts;
#endif
#if defined(CODESIZE_BROKER)
		broker_t broker;
#endif
#if defined(CODESIZE_SHDT)
		Shdt shdt;
#endif
};

//wiselib::WiselibApplication<Os, App> app;
App app;
void application_main(Os::AppMainParameter& amp) {
	app.init(amp);
}


