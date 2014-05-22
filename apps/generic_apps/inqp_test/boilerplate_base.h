
#include <external_interface/external_interface.h>
#include <external_interface/external_interface_testing.h>

typedef wiselib::OSMODEL Os;
typedef Os::block_data_t block_data_t;
//typedef Os::Radio Radio;
using namespace wiselib;

#define WISELIB_TIME_FACTOR 1

// --- BEGIN Allocator Section
//     Allocators must be defined before most wiselib includes

#if defined(CONTIKI)
	#warning "Using BITMAP allocator"
	#include <util/allocators/bitmap_allocator.h>
	//typedef wiselib::BitmapAllocator<Os, 2000> Allocator;
	typedef wiselib::BitmapAllocator<Os, 1500> Allocator;
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

typedef vector_static<Os, TupleT, 25> TupleContainer;
typedef StaticDictionary<Os, 120, 8> Dictionary;
typedef TupleStore<Os, TupleContainer, Dictionary, Os::Debug, BIN(111), &TupleT::compare> TS;

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

class AppBase {
	public:
		
		void init( Os::AppMainParameter& value ) {
			radio_ = &wiselib::FacetProvider<Os, Os::Radio>::get_facet( value );
			timer_ = &wiselib::FacetProvider<Os, Os::Timer>::get_facet( value );
			rand_ = &wiselib::FacetProvider<Os, Os::Rand>::get_facet( value );
			debug_ = &wiselib::FacetProvider<Os, Os::Debug>::get_facet( value );
			clock_ = &wiselib::FacetProvider<Os, Os::Clock>::get_facet( value );
		#if ENABLE_UART
			uart_ = &wiselib::FacetProvider<Os, Os::Uart>::get_facet(value);
		#endif

			init_tuplestore();
			init_query_processor();
		}

		void init_tuplestore() {
			dictionary_.init(debug_);
			tuplestore_.init(&dictionary_, &container_, debug_);
		}

		void init_query_processor() {
			query_processor_.init(&tuplestore_, timer_, clock_);
		}

		void insert_tuple(const char* s, const char* p, const char* o) {
			#if ENABLE_DEBUG
				debug_->debug("ins (%s %s %s)", s, p, o);
			#endif
			TupleT t;
			t.set(0, (block_data_t*)const_cast<char*>(s));
			t.set(1, (block_data_t*)const_cast<char*>(p));
			t.set(2, (block_data_t*)const_cast<char*>(o));
			tuplestore_.insert(t);
			//debug_->debug("ts %d c %d", (int)tuplestore_.size(), (int)tuplestore_.container().size9());
		}

		void insert_tuples(const char* (*rdf)[3]) {
			for(const char* (*p)[3] = rdf; **p; ++p) {
				insert_tuple((*p)[0], (*p)[1], (*p)[2]);
			}
		}

			block_data_t msg[100];
		void send_result_row_to_selda(
				Processor::query_id_t query_id,
				Processor::operator_id_t operator_id,
				Processor::RowT& row
		) {

#if ENABLE_UART_RESULTS
			Query *q = query_processor().get_query(query_id);
			if(!q) {
				//debug_->debug("!q %d", (int)query_id);
				return;
			}
			Processor::BasicOperator *op = q->get_operator(operator_id);
			if(!op) {
				//debug_->debug("!op %d", (int)operator_id);
				return;
			}
			
			int cols = op->projection_info().columns();
			if(op->type() == Processor::BOD::AGGREGATE) {
				Processor::AggregateT *aggr = (Processor::AggregateT*)op;
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
					enum { AGAIN = 0x80 };
					if((aggr->aggregation_types()[i] & ~AGAIN) == Processor::AggregateT::AD::AVG) {
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
			//uart_->write(msglen + 1, (Os::Uart::block_data_t*)msg);
			write_uart_isensestyle(msg, msglen + 1);
#endif // ENABLE_UART
		} // send_result_row_to_nqxe()

#if ENABLE_UART_RESULTS
		void write_uart_isensestyle(block_data_t *msg, size_t msglen) {
			block_data_t dle = 0x10, stx = 0x02, etx = 0x03;
			uart_->write(1, &dle);
			clock_wait(10);
			uart_->write(1, &stx);
			clock_wait(10);

			block_data_t b;
			b = 105; uart_->write(1, &b);
			clock_wait(10);
			for(size_t i = 0; i<msglen; i++) {
				if(msg[i] == dle) {
					uart_->write(1, &dle);
			clock_wait(10);
					uart_->write(1, &dle);
			clock_wait(10);
				}
				else {
					uart_->write(1, msg + i);
			clock_wait(10);
				}
			}

			uart_->write(1, &dle);
			clock_wait(10);
			uart_->write(1, &etx);
			clock_wait(10);
		}
#endif // ENABLE_UART

		Processor& query_processor() { return query_processor_; }

	protected:
		Os::Radio::self_pointer_t radio_;
		Os::Timer::self_pointer_t timer_;
		Os::Debug::self_pointer_t debug_;
		Os::Clock::self_pointer_t clock_;
		Os::Rand::self_pointer_t rand_;
#if ENABLE_UART
		Os::Uart::self_pointer_t uart_;
#endif // ENABLE_UART

		Processor query_processor_;

		TupleContainer container_;
		Dictionary dictionary_;
		TS tuplestore_;
};

