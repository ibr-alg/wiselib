
// ---- platform stuff

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

// ---- /platform stuff 
 

#include <algorithms/hash/sdbm.h>
#include <algorithms/rdf/inqp/query_processor.h>
#include <algorithms/rdf/task_execution.h>
#include <util/broker/broker.h>
#include <util/pstl/list_dynamic.h>
#include <util/pstl/map_static_vector.h>
#include <util/pstl/priority_queue_dynamic.h>
#include <util/pstl/unique_container.h>
#include <util/tuple_store/prescilla_dictionary.h>
#include <util/tuple_store/tuplestore.h>
//#include "tuple.h"

typedef BrokerTuple<Os, ::uint8_t> TupleT;
typedef wiselib::list_dynamic<Os, TupleT> TupleList;
typedef wiselib::UniqueContainer<TupleList> TupleContainer;
typedef wiselib::PrescillaDictionary<Os> Dictionary;
typedef wiselib::TupleStore<Os, TupleContainer, Dictionary, Os::Debug, BIN(111), &TupleT::compare> TS;

typedef Sdbm<Os> Hash;

typedef INQPQueryProcessor<Os, TS, Hash> Processor;
typedef Processor::Query Query;
typedef Processor::Value Value;
typedef Processor::BOD BOD;
typedef Processor::SelectionDescriptionT SD;

typedef Broker<Os, TS, ::uint8_t> BrokerT;
typedef TaskExecution<Os, BrokerT, Processor> TaskExecutionT;

#define LEFT 0
#define RIGHT 0x80
#define AGAIN 0x80
#define LEFT_COL(X) ((X) << 4)
#define RIGHT_COL(X) ((X) & 0x0f)

class ExampleApplication {
	public:
		void init(Os::AppMainParameter& value) {
			timer_ = &wiselib::FacetProvider<Os, Os::Timer>::get_facet(value);
			debug_ = &wiselib::FacetProvider<Os, Os::Debug>::get_facet(value);
			
			dictionary_.init(debug_);
			tuple_store_.init(&dictionary_, &container_, debug_);
			broker_.init(&tuple_store_);
			
			query_processor_.init(&tuple_store_, timer_);
			
			task_execution_.init(&broker_, &query_processor_, timer_);
			
			fill_ts();
			print_ts();
			
			add_task3();
			print_ts();
			
			//add_task2();
			
			//print_ts();
			
			//timer_->set_timer<ExampleApplication, &ExampleApplication::after_some_time>(20000, this, 0);
		}
		
		void ins(char* s, char* p, char* o) {
			TupleT t;
			t.set(0, (block_data_t*)s);
			t.set(1, (block_data_t*)p);
			t.set(2, (block_data_t*)o);
			tuple_store_.insert(t);
		}
		
		void fill_ts() {
			ins("<six>", "<http://example.org/#greaterThan>", "<five>");
			ins("<five>", "<http://example.org/#greaterThan>", "<four>");
			ins("<four>", "<http://example.org/#greaterThan>", "<three>");
			ins("<one>", "a", "<number>");
			ins("<two>", "a", "<number>");
			ins("<three>", "a", "<number>");
			ins("<four>", "a", "<number>");
			ins("<five>", "a", "<number>");
			ins("<six>", "<http://example.org/#greaterThan>", "<three>");
			ins("<four>", "<http://example.org/#greaterThan>", "<two>");
			ins("<three>", "<http://example.org/#greaterThan>", "<one>");
			ins("<two>", "<http://example.org/#greaterThan>", "<one>");
			ins("<one>", "ex:hasNumericalValue", "1");
			ins("<two>", "ex:hasNumericalValue", "2");
			ins("<three>", "ex:hasNumericalValue", "3");
			ins("<four>", "ex:hasNumericalValue", "4");
			ins("<five>", "ex:hasNumericalValue", "5");
			ins("<six>", "ex:hasNumericalValue", "6");
			ins("<seven>", "ex:hasNumericalValue", "7");
		}
		
		void print_ts() {
			debug_->debug("TS contents:");
			for(TS::iterator iter = tuple_store_.begin(); iter != tuple_store_.end(); ++iter) {
				debug_->debug("  (%s %s %s)", (char*)iter->get(0), (char*)iter->get(1), (char*)iter->get(2));
			}
		}
	
		void after_some_time(void* = 0) {
			print_ts();
			timer_->set_timer<ExampleApplication, &ExampleApplication::after_some_time>(1000, this, 0);
		}
		
		
		void hash_string(const char* s, block_data_t* r) {
			Value v = Hash::hash(reinterpret_cast<const block_data_t*>(s), strlen(s));
			r[0] = v >> 24;
			r[1] = (v >> 16) & 0xff;
			r[2] = (v >> 8) & 0xff;
			r[3] = v & 0xff;
		}
		
		void add_task1() {
			
			/* 
			 * CONSTRUCT ?a <gt> ?c WHERE {
			 *   ?a <gt> ?b .
			 *   ?b <gt> ?c .
			 * }
			 * 
			 * collect().str(0).str(1).str(2) [
			 * 		join(2, 0).str(0).str(3).str(4) [
			 * 		  gps(STAR, "<http://example.org/#greaterThan>", STAR).str(0).str(1).str(2),
			 * 		  gps(STAR, "<http://example.org/#greaterThan>", STAR).str(0).str(2)
			 * 		]
			 * ]
			 * 
			 */
			
			Query *q = query_processor_.create_query(1);
			q->set_expected_operators(4);
			
			block_data_t gt_[4];
			hash_string("<http://example.org/#greaterThan>", gt_);
			
			// construct
			// oid, type, parent idx & port, 4x projection
			block_data_t op100[] = { 100, 'C', 0, BIN(111111), 0, 0, 0 };
			query_processor_.handle_operator(q, (BOD*)op100);
		
			// join (?a <gt> ?b) with (?b ?c) return ?a <gt> ?c
			block_data_t op90[] = { 90, 'j', LEFT | 100, BIN(00001111), BIN(00000011), 0, 0, LEFT_COL(2) | RIGHT_COL(0) };
			query_processor_.handle_operator(q, (BOD*)op90);
		
			// ?b <gt> ?c return ?b ?c
			block_data_t op80[] = { 80, 'g', RIGHT | 90, BIN(110011), 0, 0, 0, BIN(010), gt_[0], gt_[1], gt_[2], gt_[3] };
			query_processor_.handle_operator(q, (BOD*)op80);
		
			// ?a <gt> ?b return all
			// oid, type, parent idx & port, 4x projection,
			// affects, values....
			block_data_t op70[] = { 70, 'g', LEFT | 90, BIN(111111), 0, 0, 0, BIN(010), gt_[0], gt_[1], gt_[2], gt_[3] };
			query_processor_.handle_operator(q, (BOD*)op70);
			
			task_execution_.add_task(1);
		}
		
		void add_task2() {
			
			/*
			 * DELETE { ?a ?b ?c } WHERE {
			 *   ?a <gt> <three> .
			 *   ?a ?b ?c .
			 * }
			 */
			
			Query *q = query_processor_.create_query(2);
			q->set_expected_operators(4);
			
			block_data_t gt_[4], three_[4];
			hash_string("<http://example.org/#greaterThan>", gt_);
			hash_string("<three>", three_);
			
			// delete
			block_data_t op100[] = { 100, 'D', 0, 0, 0, 0, 0, BIN(000) };
			query_processor_.handle_operator(q, (BOD*)op100);
			
			// join (?a <gt> <three>) with (?a ?b ?c) return (?a ?b ?c)
			block_data_t op90[] = { 90, 'j', LEFT | 100, BIN(00111111), 0, 0, 0, LEFT_COL(0) | RIGHT_COL(0) };
			query_processor_.handle_operator(q, (BOD*)op90);
			
			// ?a <gt> <three>
			block_data_t op80[] = { 80, 'g', RIGHT | 90, BIN(11), 0, 0, 0, BIN(110), gt_[0], gt_[1], gt_[2], gt_[3], three_[0], three_[1], three_[2], three_[3] };
			query_processor_.handle_operator(q, (BOD*)op80);
			
			// ?a ?b ?c
			block_data_t op70[] = { 70, 'g', LEFT | 90, BIN(111111), 0, 0, 0, BIN(000), 0, 0, 0, 0 };
			query_processor_.handle_operator(q, (BOD*)op70);
			
			task_execution_.add_task(2);
			
		}
		
		void add_task3() {
			
			/*
			 * CONSTRUCT { ?a <lt> ?c } WHERE {
			 * 	?a <numericalValue> ?b .
			 * 	?c <numericalValue> ?d .
			 * }
			 * FILTER { ?b < ?d }
			 */
			block_data_t lt_[4], num_[4];
			hash_string("<http://example.org/#lowerThanOrEqual>", lt_);
			hash_string("ex:hasNumericalValue", num_);
			
			Query *q = query_processor_.create_query(3);
			q->set_expected_operators(5);
			
			// construct
			block_data_t op100[] = { 100, 'C', 0, 0, 0, 0, 0, BIN(010), lt_[0], lt_[1], lt_[2], lt_[3] };
			query_processor_.handle_operator(q, (BOD*)op100);
			
			// FILTER { ?b < ?d }
			block_data_t op90[] = { 90, 's', LEFT | 100, BIN(0101), 0, 0, 0, 1, 0, SD::LEQ | 1 };
			query_processor_.handle_operator(q, (BOD*)op90);
				
			// cross join (?b ?d)
			block_data_t op80[] = { 80, 'j', LEFT | 90, BIN(0101), 0, 0, 0, 0xff };
			query_processor_.handle_operator(q, (BOD*)op80);
			
			// ?c <numericalValue> ?d
			block_data_t op70[] = { 70, 'g', RIGHT | 80, BIN(010000), 0, 0, 0, BIN(010), num_[0], num_[1], num_[2], num_[3] };
			query_processor_.handle_operator(q, (BOD*)op70);
			
			// ?a <numericalValue> ?b
			block_data_t op60[] = { 60, 'g', LEFT | 80, BIN(010000), 0, 0, 0, BIN(010), num_[0], num_[1], num_[2], num_[3] };
			query_processor_.handle_operator(q, (BOD*)op60);
			
			task_execution_.add_task(3);
		}
		
	private:
		TupleContainer container_;
		Dictionary dictionary_;
		TS tuple_store_;
		Processor query_processor_;
		TaskExecutionT task_execution_;
		BrokerT broker_;
		Os::Timer::self_pointer_t timer_;
		Os::Debug::self_pointer_t debug_;
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
