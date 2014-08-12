

#define INQP_AGGREGATE_CHECK_INTERVAL 1000
#define WISELIB_MAX_NEIGHBORS 10

#if defined(CONTIKI) || defined(TINYOS)
extern "C" {
	#include <string.h>
}
	#define assert(X)
#endif

#include <external_interface/external_interface.h>
#include <external_interface/external_interface_testing.h>

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

typedef wiselib::OSMODEL Os;
typedef Os::block_data_t block_data_t;
//typedef Os::Radio Radio;
using namespace wiselib;

#define WISELIB_TIME_FACTOR 1

#if defined(CODESIZE_LINQ)
	#define CODESIZE_TS 1
#endif

#if defined(CODESIZE_DELETE)
	#define INQP_ENABLE_Delete 1
#else
	#define INQP_ENABLE_Delete 0
#endif
#if defined(CODESIZE_GRAPHPATTERNSELECTION)
	#define INQP_ENABLE_GraphPatternSelection 1
#else
	#define INQP_ENABLE_GraphPatternSelection 0
#endif
#if defined(CODESIZE_SIMPLELOCALJOIN)
	#define INQP_ENABLE_SimpleLocalJoin 1
#else
	#define INQP_ENABLE_SimpleLocalJoin 0
#endif
#if defined(CODESIZE_COLLECT)
	#define INQP_ENABLE_Collect 1
#else
	#define INQP_ENABLE_Collect 0
#endif
#if defined(CODESIZE_CONSTRUCT)
	#define INQP_ENABLE_Construct 1
#else
	#define INQP_ENABLE_Construct 0
#endif
#if defined(CODESIZE_AGGREGATE)
	#define INQP_ENABLE_Aggregate 1
#else
	#define INQP_ENABLE_Aggregate 0
#endif
#if defined(CODESIZE_SELECT)
	#define INQP_ENABLE_Selection 1
#else
	#define INQP_ENABLE_Selection 0
#endif


// --- BEGIN Allocator Section
//     Allocators must be defined before most wiselib includes

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
#include <util/pstl/vector_static.h>
#include <util/tuple_store/tuplestore.h>
#include <algorithms/hash/sdbm.h>
#include <util/meta.h>

#include <algorithms/rdf/inqp/query_processor.h>
#include <algorithms/rdf/inqp/communicator.h>
#include <algorithms/neighbor_discovery/static_neighborhood.h>
#include "tuple.h"


#if defined(CODESIZE_TS)
	// --- TupleStore

	typedef Tuple<Os> TupleT;

	typedef vector_static<Os, TupleT, 25> TupleContainer;
	typedef StaticDictionary<Os, 120, 8> Dictionary;
	typedef TupleStore<Os, TupleContainer, Dictionary, Os::Debug, BIN(111), &TupleT::compare> TS;
#endif

#if defined(CODESIZE_LINQ)
	// --- QueryProcessor

	typedef Sdbm<Os> Hash;

	typedef INQPQueryProcessor<Os, TS, Hash> Processor;

	typedef Processor::Query Query;
	typedef Processor::Value Value;
	//typedef Processor::AggregateT AggregateT;

	// --- Communicator / Radios

	//typedef PackingRadio<Os, FloodingNd<Os, Os::Radio> > OneShotQueryRadio;
	//typedef FloodingNd<Os, Os::Radio> OneShotQueryRadio;
	typedef Os::Radio OneShotQueryRadio;

	typedef StaticNeighborhood<Os> Neighborhood;
	typedef ForwardOnDirectedNd<Os, Neighborhood> StaticResultRadio;
			typedef StaticResultRadio ResultRadio;
	typedef INQPCommunicator<Os, Processor, Os::Timer, Os::Debug, OneShotQueryRadio, StaticResultRadio, Neighborhood> Communicator;
#endif

//const char* rdf[][3] = {
	//{ 0, 0, 0 }
//};

enum { SINK = 1 };

//enum {
	//// multiples of 10s
	//LOAD_PREINSTALLED_AFTER = 3,
	//REPEAT_PREINSTALLED = 60,
//};

//#include "static_data.h"

//
// Preinstalled query
//

#define LEFT 0
#define RIGHT 0x80
#define AGAIN 0x80
#define LEFT_COL(X) ((X) << 4)
#define RIGHT_COL(X) ((X) & 0x0f)
#define COL(L, R) (LEFT_COL(L) | RIGHT_COL(R))
enum { QID = 1 };
struct OpInfo { int len; block_data_t *op; };

OpInfo g_query[] = { {0, 0} };

// Simple temperature aggregation query
//#include "query_node2_aggregate_temperature.h"
//#include "query_collect.h"
//#include "query_roomlight10.h"

////#include "query_test_both.h"
//// query should now be available as OpInfo g_query[];

//
//
//


class App {
	public:
		typedef ::uint32_t abs_millis_t;
		typedef Os::Clock::time_t time_t;

		void init(Os::AppMainParameter& value) {
			radio_ = &wiselib::FacetProvider<Os, Os::Radio>::get_facet( value );
			timer_ = &wiselib::FacetProvider<Os, Os::Timer>::get_facet( value );
			rand_ = &wiselib::FacetProvider<Os, Os::Rand>::get_facet( value );
			debug_ = &wiselib::FacetProvider<Os, Os::Debug>::get_facet( value );
			clock_ = &wiselib::FacetProvider<Os, Os::Clock>::get_facet( value );


			#if defined(CODESIZE_TS)
				init_tuplestore();
				insert_tuple("", "", "");
				tuplestore_.erase(tuplestore_.begin());
			#endif
			#if defined(CODESIZE_LINQ)
				result_radio().reg_recv_callback<App, &App::on_sink_receive>(this);
				init_query_processor();
				init_communicator();
				run_query(0);
			#endif
		}

		#if defined(CODESIZE_TS)
		void init_tuplestore() {
			dictionary_.init(debug_);
			tuplestore_.init(&dictionary_, &container_, debug_);
		}

		void insert_tuple(const char* s, const char* p, const char* o) {
			TupleT t;
			t.set(0, (block_data_t*)const_cast<char*>(s));
			t.set(1, (block_data_t*)const_cast<char*>(p));
			t.set(2, (block_data_t*)const_cast<char*>(o));
			tuplestore_.insert(t);
		}
		#endif

		#if defined(CODESIZE_LINQ)
		void init_query_processor() {
			query_processor_.init(&tuplestore_, timer_, clock_);
		}

		void init_communicator() {
			//query_radio_.init(radio_);
			//query_radio_.enable_radio();

			result_radio_.init(neighborhood_, *radio_, *timer_, *rand_, *debug_);
			result_radio_.enable_radio();

			communicator_.init(query_processor_, *radio_, result_radio_, neighborhood_, *timer_, *debug_);
			communicator_.set_sink(SINK);
		}

		void run_query(void*) {
			for(OpInfo *q = g_query; q->len; q++) {
				process(q->len, q->op);
			}
		}



		/**
		 * @param op { 'O', qid, oid, .... } or { 'Q', qid, ops }
		 */
		void process(int sz, block_data_t* op) {
			//ian_.handle_operator(op100, 0, sizeof(op100));
			//communicator_.on_receive_query(radio_->id(), sz, op);
			if(op[0] == 'O') {
				query_processor().handle_operator(op[1], sz - 2, op + 2);
			}
			else if(op[0] == 'Q') {
				query_processor().handle_query_info(op[1], op[2]);
			}
		}

		Communicator::RowT::Value row[10];

		void on_sink_receive(ResultRadio::node_id_t from, ResultRadio::size_t size, ResultRadio::block_data_t *data) {
			ResultRadio::message_id_t msgid = wiselib::read<Os, block_data_t, ResultRadio::message_id_t>(data);
			typedef Communicator::RowT::Value Value;
			if(from == SINK) {
				Communicator::ResultMessage &msg = *reinterpret_cast<Communicator::ResultMessage*>(data);
				for(size_t i = 0; i < msg.payload_size() / sizeof(Value); i++) {
					row[i] = wiselib::read<Os, block_data_t, Value>(msg.payload_data() + i * sizeof(Value));
				}
				//send_result_row_to_selda(msg.query_id(), msg.operator_id(), *(Communicator::RowT*)(void*)&row);
			}
		}
		ResultRadio& result_radio() { return result_radio_; }
		Processor& query_processor() { return query_processor_; }

		#endif

	private:
		Os::Radio::self_pointer_t radio_;
		Os::Timer::self_pointer_t timer_;
		Os::Debug::self_pointer_t debug_;
		Os::Clock::self_pointer_t clock_;
		Os::Rand::self_pointer_t rand_;

		#if defined(CODESIZE_TS)
		TupleContainer container_;
		Dictionary dictionary_;
		TS tuplestore_;
		#endif

		#if defined(CODESIZE_LINQ)
		Processor query_processor_;
		Communicator communicator_;
		StaticResultRadio result_radio_;
		Neighborhood neighborhood_;
		#endif
		//OneShotQueryRadio query_radio_;
		Os::Radio::node_id_t root_;

};

wiselib::WiselibApplication<Os, App> example_app;
void application_main(Os::AppMainParameter& value) {
  example_app.init(value);
}


