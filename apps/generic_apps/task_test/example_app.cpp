#if ISENSE
	extern "C" void assert(int) { }
#endif

#include <external_interface/external_interface.h>
#include <external_interface/external_interface_testing.h>
#ifdef ISENSE
	void* malloc(size_t n) { return isense::malloc(n); }
	void free(void* p) { isense::free(p); }
#endif

#ifndef DBG
	#define DBG(X)
	#define DBG(X, ...)
#endif
	
#if !defined(ISENSE) && !defined(PC)
	void assert(int) { }
#endif
	
typedef wiselib::OSMODEL Os;
typedef Os::block_data_t block_data_t;
using namespace wiselib;

#include <util/allocators/malloc_free_allocator.h>
typedef MallocFreeAllocator<Os> Allocator;
Allocator& get_allocator();

#include <util/pstl/string_utils.h>
//#include <algorithms/rdf/inqp/query.h>
#include <algorithms/rdf/inqp/query_processor.h>
//#include <algorithms/rdf/inqp/table.h>
//#include <algorithms/rdf/inqp/row.h>
#include <algorithms/rdf/inqp/communicator.h>
#include <util/meta.h>
#include <util/debugging.h>
#include <util/pstl/map_static_vector.h>
#include <util/pstl/priority_queue_dynamic.h>
#include <util/pstl/list_dynamic.h>
#include <util/pstl/unique_container.h>
#include <util/tuple_store/tuplestore.h>
#include <util/tuple_store/prescilla_dictionary.h>
#include "tuple.h"

#include <algorithms/routing/flooding_nd/flooding_nd.h>
#include <algorithms/protocols/packing_radio/packing_radio.h>
//#include <algorithms/routing/tree_routing_ndis/tree_routing_ndis.h>
#include <algorithms/routing/forward_on_directed_nd/forward_on_directed_nd.h>
#include <algorithms/hash/fnv.h>

typedef wiselib::FloodingNd<Os, Os::Radio> FNDRadio;
typedef wiselib::PackingRadio<Os, FNDRadio> PRadio;

//typedef wiselib::TreeRoutingNdis<Os, Os::Radio, Os::Clock, Os::Timer, FNDRadio, Os::Debug> TRadio;
typedef wiselib::ForwardOnDirectedNd<Os, Os::Radio, FNDRadio> TRadio;
typedef wiselib::PackingRadio<Os, TRadio> PAnsRadio;

typedef Tuple<Os> TupleT;
typedef wiselib::list_dynamic<Os, TupleT> TupleList;
typedef wiselib::UniqueContainer<TupleList> TupleContainer;
typedef wiselib::PrescillaDictionary<Os> Dictionary;
typedef wiselib::TupleStore<Os, TupleContainer, Dictionary, Os::Debug, BIN(111), &TupleT::compare> TS;

typedef INQPQueryProcessor<Os, TS> Processor;
typedef INQPCommunicator<Os, Processor> Communicator;

#define LEFT 0
#define RIGHT 0x80
#define AGAIN 0x80

#define LEFT_COL(X) ((X) << 4)
#define RIGHT_COL(X) ((X) & 0x0f)

class ExampleApplication
{
	public:
		
		template<typename TS>
		void ins(TS& ts, char* s, char* p, char* o) {
			TupleT t;
			t.set(0, (block_data_t*)s);
			t.set(1, (block_data_t*)p);
			t.set(2, (block_data_t*)o);
			ts.insert(t);
		}
		
		void init( Os::AppMainParameter& value )
		{
			radio_ = &wiselib::FacetProvider<Os, Os::Radio>::get_facet( value );
			timer_ = &wiselib::FacetProvider<Os, Os::Timer>::get_facet( value );
			debug_ = &wiselib::FacetProvider<Os, Os::Debug>::get_facet( value );
			clock_ = &wiselib::FacetProvider<Os, Os::Clock>::get_facet( value );
			
			//test_heap();
			//test_atol();
			//hashes();
			//return;
			
			
			// query direction: packing radio over flooding
			
			fndradio_.init(radio_);
			query_radio_.init(fndradio_, *debug_);
			query_radio_.enable_radio();
			
			// answer direction: packing radio over treerouting
			
			//tradio_.init(*radio_, *clock_, *timer_, fndradio_, *debug_);
			tradio_.init(radio_, &fndradio_);
			result_radio_.init(tradio_, *debug_, *timer_);
			result_radio_.enable_radio();
			
			debug_->debug( "Hello World from Example Application! my id=%d app=%p\n", radio_->id(), this );
			if(radio_->id() == 0) {
				debug_->debug("---- I AM THE SINK! ----\n");
				//be_sink();
				timer_->set_timer<ExampleApplication, &ExampleApplication::be_sink>(1000, this, 0);
				
				//timer_->set_timer<ExampleApplication, &ExampleApplication::sink_ask_hash_resolve>(1000, this, 0);
			}
			else {
				be();
			}
		}
		
		Dictionary dictionary;
		TupleContainer container;
		TS ts;
		
		void test_atol() {
			DBG("%ld", wiselib::atol((char*)"12345"));
			DBG("%f", wiselib::atof((char*)"12345.67"));
		}
		
		void test_heap() {
			typedef wiselib::PriorityQueueDynamic<Os, int> Heap;
			Heap h;
			
			/*
			h.push(77);
			h.push(2);
			h.push(-4);
			h.push(312);
			h.push(234);
			h.push(456);
			h.push(1);
			h.push(77);
			h.push(3);
			*/
			
			h.push(6);
			h.push(5);
			h.push(4);
			h.push(3);
			h.push(2);
			h.push(1);
			h.push(0);
			
			while(h.size()) {
				debug_->debug("%d", h.pop());
			}
		}
		
		void hashes() {
			typedef Fnv32<Os> Hash;
			const char *strings[] = {
				"A", "measures", "m1", "m2", "has_value", "12", "14"
			};
			for(const char **s = strings; *s; s++) {
				DBG("%-20s %08x", *s, Hash::hash((block_data_t*)*s, strlen(*s)));
			}
		}
			
			
		
		
		#pragma GCC diagnostic push
		#pragma GCC diagnostic ignored "-Wwrite-strings"
		void be() {
			dictionary.init(debug_);
			ts.init(&dictionary, &container, debug_);
			
			ins(ts, "A", "measures", "m1");
			ins(ts, "A", "measures", "m2");
			ins(ts, "m1", "has_value", "12");
			ins(ts, "m2", "has_value", "14");
			ins(ts, "B", "measures", "mb1");
			ins(ts, "B", "measures", "mb2");
			ins(ts, "mb1", "has_value", "20");
			ins(ts, "mb2", "has_value", "24");
			
			ian_.init(&ts, timer_);
			communicator_.init(ian_, query_radio_, result_radio_, fndradio_, *timer_);
			communicator_.set_sink(0);
			
			//ian_.reverse_translator().fill();
			
			//pradio_.reg_recv_callback<ExampleApplication, &ExampleApplication::node_receive_query>( this );
		}
		#pragma GCC diagnostic pop
		
		void sink_ask_hash_resolve(void*) {
			result_radio_.reg_recv_callback<ExampleApplication, &ExampleApplication::sink_receive_answer>( this );
			
			block_data_t msg[] = {
				Communicator::MESSAGE_ID_RESOLVE_HASHVALUE,
				0xf4, 0x59, 0xe3, 0x2b,
			};
			send(sizeof(msg), msg);
			query_radio_.flush();
		}
		
		void be_sink(void*) {
			dictionary.init(debug_);
			ts.init(&dictionary, &container, debug_);
			ian_.init(&ts, timer_);
			communicator_.init(ian_, query_radio_, result_radio_, fndradio_, *timer_);
			communicator_.set_sink(0);
			
			// set self as parent.
			// when we receive packets from ourself, we play sink, otherwise
			// we play in-network node
			fndradio_.set_parent(0);
			
			result_radio_.reg_recv_callback<ExampleApplication, &ExampleApplication::sink_receive_answer>( this );
			
			/*
			 * Some FNV32 hash values:
			 * 
			 * Das                  3f5412df
			 * Semantic Web         a360876b
			 * wird                 f459e32b
			 * in                   41387a9e
			 * der                  d1599170
			 * naechsten            13425124
			 * Dekade               d3435c31
			 * schrittweise         6fdaeebf
			 * das                  da4f123f
			 * bestehende           9ae973b6
			 * WWW                  54e8e80c
			 * erweitern            0c4f2b8e
			 * 
			 * A                    c40bf6cc
			 * measures             08ffeac4
			 * m1                   942e6feb
			 * m2                   952e717e
			 * has_value            d68814ad
			 * 12                   1deb2d6a
			 * 14                   17eb23f8
			 * 
			 * 
			 */ 
			
			block_data_t op0[] = {
				Communicator::MESSAGE_ID_QUERY,
				1, // query id
				4, // number of operators
			};
			send(sizeof(op0), op0);
			
			/*
			block_data_t op1[] = {
				Communicator::MESSAGE_ID_OPERATOR,
				1, // query id
				100,  // op id
				'c',  // collect
				0,    // no parent
				BIN(01), 0, 0, 0, // projection info
			};
			send(sizeof(op1), op1);
			*/
			
			block_data_t op1[] = {
				Communicator::MESSAGE_ID_OPERATOR,
				1, // qid
				100, // op id
				'a', // aggregate
				0, // no parent
				BIN(0111), 0, 0, 0, // proj info
				5, // number of bytes following
				//enum AggregationType { GROUP = 0, SUM = 1, AVG = 2, COUNT = 3, MIN = 4, MAX = 5 };
				0, AGAIN | 4, AGAIN | 5, AGAIN | 1, 2
			};
			send(sizeof(op1), op1);
			
			block_data_t op2[] = {
				Communicator::MESSAGE_ID_OPERATOR,
				1, // query id
				90,  // id
				'j', // simple local join
				LEFT | 100, // parent id & port
				BIN(01000011), 0, 0, 0, // projection info
				LEFT_COL(1) | RIGHT_COL(0), // left col & right col
			};
			send(sizeof(op2), op2);
			
			block_data_t op3[] = {
				Communicator::MESSAGE_ID_OPERATOR,
				1, // query id
				80, // id
				'g', // graph pattern selection
				RIGHT | 90,   // parent offset & port
				BIN(00010011), 0, 0, 0, // projection info
				BIN(010), // affects predicate
				0xd6, 0x88, 0x14, 0xad, // "has_value"
			};
			send(sizeof(op3), op3);
			
			block_data_t op4[] = {
				Communicator::MESSAGE_ID_OPERATOR,
				1, // query id
				70, // id
				'g', // graph pattern selection
				LEFT | 90,   // parent offset & port
				BIN(00110011), 0x00, 0x00, 0x00, // projection info
				BIN(010), // affects predicate
				0x08, 0xff, 0xea, 0xc4, // "measures"
			};
			send(sizeof(op4), op4);
			
			
			
			//block_data_t exec[] = {
				//Communicator::MESSAGE_ID_EXECUTE,
				//1, // query id
			//};
			//send(sizeof(exec), exec);
			query_radio_.flush();
			
			//timer_->set_timer<ExampleApplication, &ExampleApplication::sink_ask_hash_resolve>(10000, this, 0);
			
		}
		
		void send(size_t len, block_data_t *data) {
			query_radio_.send(0, len, data);
		}
		
		void sink_receive_answer( PAnsRadio::node_id_t from, PAnsRadio::size_t len, PAnsRadio::block_data_t *buf ) {
			PAnsRadio::message_id_t msgid = wiselib::read<Os, block_data_t, PRadio::message_id_t>(buf);
			
			DBG("sink recv %d -> %d", from, result_radio_.id());
			
			if(from == 0) {
				debug_->debug("sink recv from %d", from);
				wiselib::debug_buffer<Os, 16, Os::Debug>(debug_, buf, len);
			}
		}
		
	private:
		Os::Radio::self_pointer_t radio_;
		Os::Timer::self_pointer_t timer_;
		Os::Debug::self_pointer_t debug_;
		Os::Clock::self_pointer_t clock_;
		
		PRadio query_radio_;
		FNDRadio fndradio_;
		
		PAnsRadio result_radio_;
		TRadio tradio_;
		
		Processor ian_;
		Communicator communicator_;
};

Allocator allocator_;
Allocator& get_allocator() { return allocator_; }
// --------------------------------------------------------------------------
wiselib::WiselibApplication<Os, ExampleApplication> example_app;
// --------------------------------------------------------------------------
void application_main( Os::AppMainParameter& value )
{
  example_app.init( value );
}
