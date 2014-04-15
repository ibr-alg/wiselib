
#if defined(CONTIKI)
	extern "C" {
		#include <string.h>
	#include <contiki.h>
	#include <netstack.h>
	}
#endif


/// ----- Config


#if defined(SHAWN)
	#define WISELIB_MAX_NEIGHBORS 10000
	#define WISELIB_TIME_FACTOR 1
	#define SINK 0

#elif defined(PC)
	#define WISELIB_MAX_NEIGHBORS 10000
	#define WISELIB_TIME_FACTOR 1
	#define WISELIB_DISABLE_DEBUG 0
	#define WISELIB_DISABLE_DEBUG_MESSAGES 0
	#define SINK 0

#elif defined(CONTIKI_TARGET_SKY)
	#define WISELIB_MAX_NEIGHBORS 40
	#define WISELIB_TIME_FACTOR 1
	#define WISELIB_DISABLE_DEBUG 1

	//#define SINK 47430UL
	#define SINK 18045UL // Blue node without battery holder

	#define USE_UART 0
	#define USE_PREDEFINED_QUERY 1
	#define BITMAP_ALLOCATOR_RAM_SIZE (1024 + 512)
	//#define BITMAP_ALLOCATOR_RAM_SIZE (100)

	#define TS_MAX_TUPLES 10

//#else
	//#define WISELIB_MAX_NEIGHBORS 4
	//#define WISELIB_TIME_FACTOR 1

	////#define BITMAP_ALLOCATOR_RAM_SIZE 1000
	//#define BITMAP_ALLOCATOR_RAM_SIZE (1024 + 512)
	//#define SINK 47430UL
	////#define TS_MAX_TUPLES 76

	//#define USE_UART 0
	//#define USE_PREDEFINED_QUERY 1
	//#define INQP_TEST_USE_BLOCK 0

#endif

#define INQP_AGGREGATE_CHECK_INTERVAL 1000
//#define WISELIB_MAX_NEIGHBORS 100

#define ENABLE_DEBUG 1


/// ------ /Config



#if ISENSE
	extern "C" void assert(int) { }
#endif

#include <external_interface/external_interface.h>
#include <external_interface/external_interface_testing.h>
#ifdef ISENSE
	void* malloc(size_t n) { return isense::malloc(n); }
	void free(void* p) { isense::free(p); }
#endif
	
#if defined(ARDUINO)
	#include <external_interface/arduino/arduino_monitor.h>
#elif defined(ISENSE)
	#include <external_interface/isense/isense_monitor.h>
#endif

#ifndef DBG
	#define DBG(X)
	#define DBG(X, ...)
#endif
	
// Is there any Os for which we really want this at all?
#if !defined(ISENSE) && !defined(PC) && !defined(ARDUINO) && !defined(SHAWN)
	void assert(int) { }
#endif
	
typedef wiselib::OSMODEL Os;
typedef Os::block_data_t block_data_t;
using namespace wiselib;

#if defined(CONTIKI)
	#warning "Using BITMAP allocator"

	#include <util/allocators/bitmap_allocator.h>
	typedef wiselib::BitmapAllocator<Os, BITMAP_ALLOCATOR_RAM_SIZE> Allocator;
	Allocator allocator_;
	Allocator& get_allocator() { return allocator_; }
#else
	#include <util/allocators/malloc_free_allocator.h>
	typedef wiselib::MallocFreeAllocator<Os> Allocator;
	Allocator allocator_;
	Allocator& get_allocator() { return allocator_; }
#endif

#include <util//string_util.h>
//#include <algorithms/rdf/inqp/query.h>
#include <algorithms/rdf/inqp/query_processor.h>
//#include <algorithms/rdf/inqp/table.h>
//#include <algorithms/rdf/inqp/row.h>
#include <algorithms/rdf/inqp/communicator.h>
#include <util/meta.h>
#include <util/debugging.h>
//#include <util/pstl/map_static_vector.h>
//#include <util/pstl/priority_queue_dynamic.h>
#include <util/tuple_store/tuplestore.h>
#include "tuple.h"

#include <algorithms/routing/flooding_nd/flooding_nd.h>
#include <algorithms/protocols/packing_radio/packing_radio.h>
//#include <algorithms/routing/tree_routing_ndis/tree_routing_ndis.h>
#include <algorithms/routing/forward_on_directed_nd/forward_on_directed_nd.h>
#include <algorithms/hash/sdbm.h>
typedef wiselib::Sdbm<Os> Hash;

typedef wiselib::FloodingNd<Os, Os::Radio> FNDRadio;
typedef wiselib::PackingRadio<Os, FNDRadio> PRadio;

//typedef wiselib::TreeRoutingNdis<Os, Os::Radio, Os::Clock, Os::Timer, FNDRadio, Os::Debug> TRadio;
typedef wiselib::ForwardOnDirectedNd<Os, Os::Radio, FNDRadio> TRadio;
typedef wiselib::PackingRadio<Os, TRadio> PAnsRadio;

// -------- BEGIN TS SETUP

typedef Tuple<Os> TupleT;


// RAM
#if !INQP_TEST_USE_BLOCK
	//#include <util/pstl/list_dynamic.h>
	#include <util/pstl/unique_container.h>
	//#include <util/tuple_store/prescilla_dictionary.h>
	#include <util/tuple_store/static_dictionary.h>
	#include <util/pstl/vector_static.h>
	//#include <util/pstl/unbalanced_tree_dictionary.h>
	//typedef wiselib::list_dynamic<Os, TupleT> TupleList;
	//typedef wiselib::UniqueContainer<TupleList> TupleContainer;
	
	typedef wiselib::vector_static<Os, TupleT, TS_MAX_TUPLES> TupleContainer;
	//typedef wiselib::PrescillaDictionary<Os> Dictionary;
	typedef wiselib::StaticDictionary<Os, 50, 15> Dictionary;
	//typedef UnbalancedTreeDictionary<Os> Dictionary;
	
	//#include "precompiled_ts.cpp"
	//typedef PrecompiledDictionary Dictionary;
	//typedef PrecompiledTupleContainer TupleContainer;


#else

	// BLOCK
	#define BLOCK_CACHE_SIZE 2
	#define BLOCK_CACHE_SPECIAL 1
	#define BLOCK_CACHE_WRITE_THROUGH 1
	#define BLOCK_CHUNK_SIZE 8
	#define BLOCK_CHUNK_ADDRESS_TYPE ::uint32_t
	
	#include <algorithms/block_memory/bitmap_chunk_allocator.h>
	#include <algorithms/block_memory/cached_block_memory.h>
	typedef CachedBlockMemory<Os, Os::BlockMemory, BLOCK_CACHE_SIZE, BLOCK_CACHE_SPECIAL, BLOCK_CACHE_WRITE_THROUGH> BlockMemory;
	typedef BitmapChunkAllocator<Os, BlockMemory, BLOCK_CHUNK_SIZE, BLOCK_CHUNK_ADDRESS_TYPE> BlockAllocator;
	#include <algorithms/block_memory/b_plus_hash_set.h>
	typedef BPlusHashSet<Os, BlockAllocator, Hash, TupleT, true> TupleContainer;
	#include <algorithms/block_memory/b_plus_dictionary.h>
	typedef BPlusDictionary<Os, BlockAllocator, Hash> Dictionary;
#endif

typedef wiselib::TupleStore<Os, TupleContainer, Dictionary, Os::Debug, BIN(111), &TupleT::compare> TS;

// -------- END TS SETUP


typedef INQPQueryProcessor<Os, TS, Hash> Processor;
typedef INQPCommunicator<Os, Processor> Communicator;
typedef Processor::Query Query;
typedef Processor::Value Value;
typedef Processor::AggregateT AggregateT;

#define LEFT 0
#define RIGHT 0x80
#define AGAIN 0x80

#define LEFT_COL(X) ((X) << 4)
#define RIGHT_COL(X) ((X) & 0x0f)

#ifdef ISENSE
#include <isense/util/get_os.h>
#endif

const char* rdf[][3] = {
	#include "incontextsensing_short.cpp",
	{ 0, 0, 0 }
};

template<typename OsModel_P>
class NullMonitor {
	public:
		void init(typename OsModel_P::Debug* d) { debug_ = d; }
		
		void report() { }
		void report(const char *remark) { debug_->debug(remark); }
		int free() { return 666; }
		
		typename OsModel_P::Debug* debug_;
};

	/*
	 * MIN(?v) MEAN(?v) MAX(?v) {
	 *    ?sens <http://purl.oclc.org/NET/ssnx/ssn#observedProperty> <http://spitfire-project.eu/property/Temperature> .
	 *    ?sens <http://www.loa-cnr.it/ontologies/DUL.owl#hasValue> ?v .
	 * }
	 * 

	<http://www.loa-cnr.it/ontologies/DUL.owl#hasValue>          4d0f60b4
	<http://spitfire-project.eu/property/Temperature>            b23860b3
	<http://purl.oclc.org/NET/ssnx/ssn#observedProperty>         bf26b82e

	 */
	enum { Q = Communicator::MESSAGE_ID_QUERY, OP = Communicator::MESSAGE_ID_OPERATOR };
	enum { ROOT = 0 };
	enum AggregationType { GROUP = 0, SUM = 1, AVG = 2, COUNT = 3, MIN = 4, MAX = 5 };
	enum { QID = 1 };
	block_data_t op100[] = { OP, QID, 100, 'a', ROOT, BIN(010101), BIN(0), BIN(0), BIN(0), 3, MIN | AGAIN, AVG | AGAIN, MAX };
	block_data_t op90[]  = { OP, QID,  90, 'j', LEFT | 100, BIN(010000), BIN(0), BIN(0), BIN(0), LEFT_COL(0) | RIGHT_COL(0) };
	block_data_t op80[]  = { OP, QID,  80, 'g', RIGHT | 90, BIN(010011), BIN(0), BIN(0), BIN(0), BIN(010), 0x4d, 0x0f, 0x60, 0xb4 };
	block_data_t op70[]  = { OP, QID,  70, 'g', LEFT | 90, BIN(11), BIN(0), BIN(0), BIN(0), BIN(110), 0xbf, 0x26, 0xb8, 0x2e, 0xb2, 0x38, 0x60, 0xb3 };
	block_data_t cmd[]   = { Q, QID, 4 };


//char *preinstalled_rdf[] = {
	//#include "incontextsensing.cpp",
	//{ 0, 0, 0 }
//};



class App {
	public:
		
		template<typename TS>
		void ins(TS& ts, const char* s, const char* p, const char* o) {
			TupleT t;
			t.set(0, (block_data_t*)const_cast<char*>(s));
			t.set(1, (block_data_t*)const_cast<char*>(p));
			t.set(2, (block_data_t*)const_cast<char*>(o));
			ts.insert(t);
		}

		// Not exploitable ;)
		int heart;
		void heartbeat(void*_ =0) {
			++heart;
			if(heart == 0) {
				#if ENABLE_DEBUG
					print_memstat();
				#endif
				init2();
			}
			else {
				debug_->debug("<3 %d", heart);
				timer_->set_timer<App, &App::heartbeat>(10000, this ,0);
			}
		}

		void disable_radio(void* =0) {
			radio_->disable_radio();
			#if defined(CONTIKI)
				NETSTACK_RDC.off(false);
			#endif
		}

		void enable_radio(void* =0) {
			#if defined(CONTIKI)
				NETSTACK_RDC.on();
			#endif
			radio_->enable_radio();
		}


		///@name Initialization methods.
		///@{
		
		void init( Os::AppMainParameter& value ) {
			radio_ = &wiselib::FacetProvider<Os, Os::Radio>::get_facet( value );
			timer_ = &wiselib::FacetProvider<Os, Os::Timer>::get_facet( value );
			debug_ = &wiselib::FacetProvider<Os, Os::Debug>::get_facet( value );
			clock_ = &wiselib::FacetProvider<Os, Os::Clock>::get_facet( value );

			#if ENABLE_DEBUG
				debug_->debug("bwait %lu", (unsigned long)radio_->id());
			#endif

			//disable_radio();
			//heart = -30; // active in that many beats
			heart = -1; // active in that many beats
			heartbeat();
		}
		
		void init2() {
			#if ENABLE_DEBUG
				monitor_.init(debug_);
			#endif
			
			radio_->enable_radio();
			
			// query direction: packing radio over flooding
			
			fndradio_.init(radio_);
			query_radio_.init(fndradio_, *debug_);
			query_radio_.enable_radio();
			
			// answer direction: packing radio over treerouting
			
			tradio_.init(radio_, &fndradio_);
			result_radio_.init(tradio_, *debug_, *timer_);
			result_radio_.enable_radio();
			
			#if ENABLE_DEBUG
				debug_->debug("boot %lu", (unsigned long)radio_->id());
			#endif
			
			init_ts();
			fill_ts();
			debug_ts();
			if(radio_->id() == SINK) {
				debug_->debug("sink\n");
				timer_->set_timer<App, &App::be_sink>(100, this, 0);
			}
			else {
				be();
			}

			#if USE_PREDEFINED_QUERY
				timer_->set_timer<App, &App::load_predefined_query>(10000, this, 0);
			#endif
		}
		
		Dictionary dictionary;
		TupleContainer container;
		TS ts;
		
		void init_ts() {
		#if !INQP_TEST_USE_BLOCK
			debug_->debug("its0");

			#if VECTOR_STATIC_OUTSOURCE
				container.set_data(tuple_data_);
			debug_->debug("its1");
				container.set_size(VECTOR_STATIC_SIZE);
			#endif
			debug_->debug("its2");
			#if STATIC_DICTIONARY_OUTSOURCE
				dictionary.set_data(dict_data_);
			#endif

			debug_->debug("its3");
			dictionary.init(debug_);
			debug_->debug("its4");
			ts.init(&dictionary, &container, debug_);
		#else
			block_memory_.physical().init();
			delay(1000);
			block_memory_.init();
			block_allocator_.init(&block_memory_, debug_);
			container.init(&block_allocator_, debug_);
			dictionary.init(&block_allocator_, debug_);
		#endif
			debug_->debug("its5");
		}

		void fill_ts() {
			// thanks cdecl.org, without you I wouldnt have made it.
			//const char * (*p)[3] = rdf;
			const char **p = reinterpret_cast<const char **>(rdf);
			while(*p) {
				ins(ts, p[0], p[1], p[2]);
				p += 3;
			}
		}

		void debug_ts() {
			for(TS::iterator iter = ts.begin(); iter != ts.end(); ++iter) {
				debug_->debug("(%s %s %s)", (char*)iter->get(0), (char*)iter->get(1), (char*)iter->get(2));
				for(int i = 0; i < 1000; i++) ;
			}
		}

		/**
		 * Initialization for the sink node.
		 */
		void be_sink(void*) {
			debug_->debug("bs0");
			ian_.init(&ts, timer_);
			debug_->debug("bs1");
			communicator_.init(ian_, query_radio_, result_radio_, fndradio_, *timer_);
			debug_->debug("bs2");
			communicator_.set_sink(SINK);
			debug_->debug("bs3");
			
			// set self as parent.
			// when we receive packets from ourself, we play sink, otherwise
			// we play in-network node
			fndradio_.set_parent(SINK);
			
			debug_->debug("bs4");
			result_radio_.reg_recv_callback<App, &App::sink_receive_answer>( this );
			
			debug_->debug("bs5");
			timer_->set_timer<App, &App::sink_send_query>(30000, this, 0);
			debug_->debug("bs6");
		}

		#pragma GCC diagnostic push
		#pragma GCC diagnostic ignored "-Wwrite-strings"
		/**
		 * Initialization for a non-sink node.
		 */
		void be() {
			ian_.init(&ts, timer_);
			communicator_.init(ian_, query_radio_, result_radio_, fndradio_, *timer_);
			communicator_.set_sink(SINK);
			debug_->debug("rtr");
		}
		#pragma GCC diagnostic pop

		///@}
		//
		
	#if USE_UART

		///@name Methods for UART communication with SELDA (aka NQXE).

		block_data_t uart_query[512];
		size_t uart_query_pos;
		
		void uart_receive(Os::Uart::size_t len, Os::Uart::block_data_t *buf) {
			process_nqxe_message((block_data_t*)buf);
		}

		void send_result_row_to_nqxe(
				Processor::query_id_t query_id,
				Processor::operator_id_t operator_id,
				Processor::RowT& row
		) {
			block_data_t msg[100];

			/*
		void on_result_row(int type, QueryProcessor::size_type cols, QueryProcessor::RowT& row,
				QueryProcessor::query_id_t qid, QueryProcessor::operator_id_t oid) {
			*/
			
			typedef Processor::BasicOperator BasicOperator;
			
			Query *q = ian_.get_query(query_id);
			if(!q) {
				debug_->debug("!q %d", (int)query_id);
				return;
			}
			BasicOperator *op = q->get_operator(operator_id);
			if(!op) {
				debug_->debug("!op %d", (int)operator_id);
				return;
			}
			
			int cols = op->projection_info().columns();
			if(op->type() == Processor::BOD::AGGREGATE) {
				Processor::AggregateT *aggr = (Processor::AggregateT*)op;
				//int cols = op-> projection_info().columns();
				//cols = aggr->columns_physical();
				cols = aggr->columns_logical();
			}
			
			int msglen =  1 + 1 + 1 + 1 + 4*cols;
			block_data_t chk = 0;
			msg[0] = 'R';
			msg[1] = 2 + 4 * cols; // # bytes to follow after this one
			msg[2] = query_id;
			msg[3] = operator_id;
			
			if(op->type() == Processor::BOD::AGGREGATE) {
				Processor::AggregateT *aggr = (Processor::AggregateT*)op;
				int j = 0; // physical column
				for(int i = 0; i< cols; i++, j++) { // logical column
					//printf("tach %d", Processor::AggregateT::AD::AGAIN);
					if((aggr->aggregation_types()[i] & ~AGAIN) == AggregateT::AD::AVG) {
						j++;
					}
					
					Value v;
					memcpy(&v, &row[j], sizeof(Value));
					wiselib::write<Os, block_data_t, Processor::Value>(msg + 4 + 4*i, v);
				}
			}
			else {
				for(int i = 0; i< cols; i++) {
					Value v;
					memcpy(&v, &row[i], sizeof(Value));
					wiselib::write<Os, block_data_t, Processor::Value>(msg + 4 + 4*i, v);
				}
			}
			
			for(int i = 0; i<msglen; i++) { chk ^= msg[i]; }
			msg[msglen] = chk;
			uart_->write(msglen + 1, (Os::Uart::block_data_t*)msg);

		} // send_result_row_to_nqxe()

		///@}

	#endif // USE_UART
	
	#if USE_PREDEFINED_QUERY
		void load_predefined_query(void*) {
			debug_->debug("loading pre-installed query...");

			process(sizeof(op100), op100);
			process(sizeof(op90), op90);
			process(sizeof(op80), op80);
			process(sizeof(op70), op70);
			process(sizeof(cmd), cmd);
		}
	#endif
		
		/**
		 * @param q { len, 'O' or 'Q', ... }
		 */
		void process_nqxe_message(block_data_t* q) {
			assert(q[1] == 'O' || q[1] == 'Q');
			size_t l = q[0];
			process(l, q + 1);
		}
		
		/**
		 * @param op { 'O', qid, oid, .... } or { 'Q', qid, ops }
		 */
		void process(int sz, block_data_t* op) {
			//ian_.handle_operator(op100, 0, sizeof(op100));
			//communicator_.on_receive_query(radio_->id(), sz, op);
			if(op[0] == 'O') {
				ian_.handle_operator(op[1], sz - 2, op + 2);
			}
			else if(op[0] == 'Q') {
				ian_.handle_query_info(op[1], op[2]);
			}
		}
		
		void sink_ask_hash_resolve(void*) {
			result_radio_.reg_recv_callback<App, &App::sink_receive_answer>( this );
			
			block_data_t msg[] = {
				Communicator::MESSAGE_ID_RESOLVE_HASHVALUE,
				0xf4, 0x59, 0xe3, 0x2b,
			};
			send(sizeof(msg), msg);
			query_radio_.flush();
		}
		
		void sink_send_query(void*) {

			debug_->debug("sink_send_query");
			
			/*
			 * MIN(?v) MEAN(?v) MAX(?v) {
			 *    ?sens <http://purl.oclc.org/NET/ssnx/ssn#observedProperty> <http://spitfire-project.eu/property/Temperature> .
			 *    ?sens <http://www.loa-cnr.it/ontologies/DUL.owl#hasValue> ?v .
			 * }
			 * 

			<http://www.loa-cnr.it/ontologies/DUL.owl#hasValue>          4d0f60b4
			<http://spitfire-project.eu/property/Temperature>            b23860b3
			<http://purl.oclc.org/NET/ssnx/ssn#observedProperty>         bf26b82e

			 */

			send(sizeof(op100), op100);
			debug_->debug("sq0");
			send(sizeof(op90), op90);
			debug_->debug("sq1");
			send(sizeof(op80), op80);
			debug_->debug("sq2");
			send(sizeof(op70), op70);
			debug_->debug("sq3");
			send(sizeof(cmd), cmd);
			debug_->debug("sq4");
			
			debug_->debug("flushing");

			query_radio_.flush();
			//timer_->set_timer<App, &App::sink_send_query>(10000, this, 0);
			debug_->debug("query sent!");
			
			/*
			 * Some hash values:
			 * // {{{
	
			10.2                                                         ff9ee843
			12-04-02T12:48Z                                              1b07abf2
			<http://purl.oclc.org/NET/muo/muo#UnitOfMeasurement>         bebc18a8
			<http://purl.oclc.org/NET/muo/muo#measuredIn>                078d9668
			<http://purl.oclc.org/NET/muo/muo#prefSymbol>                3fcd6c38
			<http://purl.oclc.org/NET/ssnx/ssn#Property>                 c41d15d6
			<http://purl.oclc.org/NET/ssnx/ssn#Sensor>                   7e1d99b1
			<http://purl.oclc.org/NET/ssnx/ssn#Stimulus>                 641c1231
			<http://purl.oclc.org/NET/ssnx/ssn#detects>                  18231901
			<http://purl.oclc.org/NET/ssnx/ssn#hasMeasurementCapability> 7ed3bfb1
			<http://purl.oclc.org/NET/ssnx/ssn#hasMeasurementProperty>   5b80e314
			<http://purl.oclc.org/NET/ssnx/ssn#isProxyFor>               bc98b186
			<http://purl.oclc.org/NET/ssnx/ssn#observedProperty>         bf26b82e
			<http://purl.org/dc/terms/date>                              62d5797c
			<http://spitfire-project.eu/cc/spitfireCC_n3.owl#uomInUse>   3d490f0a
			<http://spitfire-project.eu/property/Battery_Life_Time>      0dd47ba9
			<http://spitfire-project.eu/property/Temperature>            b23860b3
			<http://spitfire-project.eu/sensor/sensor1234/capabilities>  3fd78899
			<http://spitfire-project.eu/sensor/sensor1234/capabilities_sensor1234> 5b8d5808
			<http://spitfire-project.eu/sensor/sensor1234>               170b9e7e
			<http://spitfire-project.eu/sensor_stimulus/silver_expansion> c12e5e70
			<http://spitfire-project.eu/uom/Centigrade>                  24e0767b
			<http://spitfire-project.eu/uom/month>                       1d93598b
			<http://www.loa-cnr.it/ontologies/DUL.owl#hasValue>          4d0f60b4
			<http://www.w3.org/1999/02/22-rdf-syntax-ns#type>            8b477de0
			<http://www.w3.org/2000/01/rdf-schema#subClassOf>            d3e5ee53
			<http://www.w3.org/2000/01/rdf-schema#type>                  b5f766c8
			<http://www.w3.org/2002/07/owl#Class>                        c632c40e
			<http://www.w3.org/2002/07/owl#Restriction>                  2e94a87a
			<http://www.w3.org/2002/07/owl#intersectionOf>               88bdb576
			<http://www.w3.org/2002/07/owl#onProperty>                   003aed82
			<http://www.w3.org/2002/07/owl#someValuesFrom>               51b52a36
			C                                                            00000043
			bnode0                                                       0aa5b6cc
			bnode1                                                       0aa5b6cd
			bnode2                                                       0aa5b6ce
			m                                                            0000006d
			
			* // }}}
			* 
			*/
			
		}
		
		void send(size_t len, block_data_t *data) {
			query_radio_.send(SINK, len, data);
		}
		
		void sink_receive_answer( PAnsRadio::node_id_t from, PAnsRadio::size_t len, PAnsRadio::block_data_t *buf ) {
			PAnsRadio::message_id_t msgid = wiselib::read<Os, block_data_t, PRadio::message_id_t>(buf);
			
			debug_->debug("@%lu sink recv %lu -> %lu s=%lu", (unsigned long)radio_->id(), (unsigned long)from, (unsigned long)result_radio_.id(), (unsigned long)SINK);
			
			if(from == SINK) {
				debug_->debug("sink recv from %d", from);
				wiselib::debug_buffer<Os, 16, Os::Debug>(debug_, buf, len);
			}
		}
		
		void print_memstat() {
			debug_->debug("rad %d tim %d dbg %d clk %d",
					(int)sizeof(Os::Radio), (int)sizeof(Os::Timer), (int)sizeof(Os::Debug),
					(int)sizeof(Os::Clock));
			debug_->debug("qrad %d fndrad %d pansrad %d trad %d",
					(int)sizeof(PRadio), (int)sizeof(FNDRadio), (int)sizeof(PAnsRadio),
					(int)sizeof(TRadio));
			debug_->debug("proc %d com %d alloc %d",
					(int)sizeof(Processor), (int)sizeof(Communicator), (int)sizeof(Allocator));
		}
		
	private:
		Os::Radio::self_pointer_t radio_;
		Os::Timer::self_pointer_t timer_;
		Os::Debug::self_pointer_t debug_;
		Os::Clock::self_pointer_t clock_;
		Os::Uart::self_pointer_t uart_;
		
		PRadio query_radio_;
		FNDRadio fndradio_;
		
		PAnsRadio result_radio_;
		TRadio tradio_;
		
		Processor ian_;
		Communicator communicator_;
		
		#if defined(ARDUINO)
			ArduinoMonitor<Os, Os::Debug> monitor_;
		#elif defined(ISENSE)
			IsenseMonitor<Os, Os::Debug> monitor_;
		#else
			NullMonitor<Os> monitor_;
		#endif
		
		
		#if INQP_TEST_USE_BLOCK
			BlockMemory block_memory_;
			BlockAllocator block_allocator_;
		#endif
};

//Allocator allocator_;
//Allocator& get_allocator() { return allocator_; }
// --------------------------------------------------------------------------
wiselib::WiselibApplication<Os, App> example_app;
// --------------------------------------------------------------------------
void application_main( Os::AppMainParameter& value )
{
  example_app.init( value );
}
