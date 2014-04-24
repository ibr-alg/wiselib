
#include <external_interface/external_interface.h>
#include <external_interface/external_interface_testing.h>

typedef wiselib::OSMODEL Os;
typedef Os::block_data_t block_data_t;
typedef Os::Radio Radio;
using namespace wiselib;

#define WISELIB_TIME_FACTOR 1
#define INQP_AGGREGATE_CHECK_INTERVAL 1000
#define WISELIB_MAX_NEIGHBORS 40

// --- BEGIN Allocator Section
//     Allocators must be defined before most wiselib includes

#if defined(CONTIKI)
	#warning "Using BITMAP allocator"
	#include <util/allocators/bitmap_allocator.h>
	typedef wiselib::BitmapAllocator<Os, 2000> Allocator;
#else
	#include <util/allocators/malloc_free_allocator.h>
	typedef wiselib::MallocFreeAllocator<Os> Allocator;
#endif
Allocator allocator_;
Allocator& get_allocator() { return allocator_; }

// --- END Allocator Section --


#include <util/pstl/unique_container.h>
#include <util/tuple_store/static_dictionary.h>
#include <util/pstl/vector_static.h>
#include <util/tuple_store/tuplestore.h>
#include <algorithms/hash/sdbm.h>
#include <algorithms/rdf/inqp/query_processor.h>
#include <algorithms/rdf/inqp/communicator.h>
#include "tuple.h"

// --- TupleStore

typedef Tuple<Os> TupleT;

typedef vector_static<Os, TupleT, 100> TupleContainer;
typedef StaticDictionary<Os, 50, 15> Dictionary;
typedef TupleStore<Os, TupleContainer, Dictionary, Os::Debug, BIN(111), &TupleT::compare> TS;

// --- QueryProcessor

typedef Sdbm<Os> Hash;

typedef INQPQueryProcessor<Os, TS, Hash> Processor;

typedef Processor::Query Query;
typedef Processor::Value Value;
//typedef Processor::AggregateT AggregateT;

// --- Communicator / Radios

//typedef PackingRadio<Os, FloodingNd<Os, Os::Radio> > OneShotQueryRadio;
typedef FloodingNd<Os, Os::Radio> OneShotQueryRadio;

class App;

class AppBase {
	public:
		
		void init( Os::AppMainParameter& value ) {
			radio_ = &wiselib::FacetProvider<Os, Os::Radio>::get_facet( value );
			timer_ = &wiselib::FacetProvider<Os, Os::Timer>::get_facet( value );
			debug_ = &wiselib::FacetProvider<Os, Os::Debug>::get_facet( value );
			clock_ = &wiselib::FacetProvider<Os, Os::Clock>::get_facet( value );

			init_tuplestore();
			init_query_processor();
		}

		void init_tuplestore() {
			dictionary_.init(debug_);
			tuplestore_.init(&dictionary_, &container_, debug_);
		}

		void init_query_processor() {
			query_processor_.init(&tuplestore_, timer_);
		}

		void insert_tuple(const char* s, const char* p, const char* o) {
			TupleT t;
			t.set(0, (block_data_t*)const_cast<char*>(s));
			t.set(1, (block_data_t*)const_cast<char*>(p));
			t.set(2, (block_data_t*)const_cast<char*>(o));
			tuplestore_.insert(t);
		}

		void insert_tuples(const char* (*rdf)[3]) {
			for(const char* (*p)[3] = rdf; **p; ++p) {
				debug_->debug("ins (%s %s %s)", (*p)[0], (*p)[1], (*p)[2]);
				insert_tuple((*p)[0], (*p)[1], (*p)[2]);
			}
		}


		Processor& query_processor() { return query_processor_; }

	protected:
		Os::Radio::self_pointer_t radio_;
		Os::Timer::self_pointer_t timer_;
		Os::Debug::self_pointer_t debug_;
		Os::Clock::self_pointer_t clock_;

		Processor query_processor_;

		TupleContainer container_;
		Dictionary dictionary_;
		TS tuplestore_;
};

wiselib::WiselibApplication<Os, App> example_app;
void application_main(Os::AppMainParameter& value) {
  example_app.init(value);
}

