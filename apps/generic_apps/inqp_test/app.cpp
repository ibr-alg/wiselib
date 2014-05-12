
//#define QUERY_SIMPLE_TEMPERATURE 0
//#define QUERY_COLLECT 0

#define ENABLE_DEBUG 0
#define ENABLE_PREINSTALLED_QUERY 1

#ifdef SHAWN
	#include "boilerplate_shawn.h"
#elif CONTIKI_TARGET_SKY
	#include "boilerplate_sky.h"
#endif

const char* rdf[][3] = {
	#include "node2.h"
	{ 0, 0, 0 }
};

enum {
	// multiples of 10s
	LOAD_PREINSTALLED_AFTER = 3,
	REPEAT_PREINSTALLED = 3,
};

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

// Simple temperature aggregation query
//#include "query_node2_aggregate_temperature.h"
//#include "query_collect.h"
#include "query_test_both.h"

// query should now be available as OpInfo g_query[];

//
//
//


#include <util/meta.h>

class App : public AppBoilerplate {
	public:
		typedef ::uint32_t abs_millis_t;
		typedef Os::Clock::time_t time_t;

		abs_millis_t absolute_millis(const time_t& t) { return clock_->seconds(t) * 1000 + clock_->milliseconds(t); }
		abs_millis_t now() { return absolute_millis(clock_->time()); }
			
		void init(Os::AppMainParameter& v) {
			AppBoilerplate::init(v);

			insert_tuples(rdf);
			insert_special_tuples();

		#if ENABLE_PREINSTALLED_QUERY
			timer_->set_timer<App, &App::load_predefined_query>(10000, this, (void*)LOAD_PREINSTALLED_AFTER);
		#endif
			result_radio().reg_recv_callback<App, &App::on_sink_receive>(this);
		}

		void load_predefined_query(void* x) {
			Uvoid x2 = (Uvoid)x;
			#if ENABLE_DEBUG
				//debug_->debug("<3 %lu", (unsigned long)now());
			#endif
			x2--;
			if(x2 == 0) {
				x2 = REPEAT_PREINSTALLED;
				query_processor().erase_query(QID);
				// wait some time between queries so aggregate can properly
				// destruct
				timer_->set_timer<App, &App::run_query>(2000, this, 0);
			}

			timer_->set_timer<App, &App::load_predefined_query>(10000, this, (void*)x2);
		}

		void run_query(void*) {
			#if ENABLE_DEBUG
				debug_->debug("QRY");
			#endif
			
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

			//debug_->debug("@%lu rcv from %lu sink %lu", (unsigned long)radio_->id(), (unsigned long)from, (unsigned long)SINK);
			
			if(from == SINK) {
				Communicator::ResultMessage &msg = *reinterpret_cast<Communicator::ResultMessage*>(data);

				for(size_t i = 0; i < msg.payload_size() / sizeof(Value); i++) {
					//row[i / sizeof(Communicator::RowT::Value)] = wiselib::read<Os, block_data_t, Value>(msg.payload_data() + i);
					//wiselib::write<Os, block_data_t, Value>((block_data_t*)&row + i * sizeof(Value), 
					//row[i] = *(Value*)(msg.payload_data() + i * sizeof(Value));
					row[i] = wiselib::read<Os, block_data_t, Value>(msg.payload_data() + i * sizeof(Value));
				}

				//Communicator::RowT &row = *reinterpret_cast<Communicator::RowT*>(msg.payload_data());

				#if ENABLE_DEBUG
				debug_->debug("sink recv from %lu", (unsigned long)from);
				//debug_->debug("ANS %lu", (unsigned long)
				//wiselib::debug_buffer<Os, 16, Os::Debug>(debug_, data, size);
				//debug_->debug("RESULT");
				wiselib::debug_buffer<Os, 16, Os::Debug>(debug_, msg.payload_data(), msg.payload_size());
				wiselib::debug_buffer<Os, 16, Os::Debug>(debug_, (block_data_t*)&row, msg.payload_size());

				//debug_->debug("RESULT %lu ", (int)row.as_int(0), (float)row.as_float(1));
				#endif

				send_result_row_to_selda(msg.query_id(), msg.operator_id(), *(Communicator::RowT*)(void*)&row);
				//Processor::query_id_t query_id,
				//Processor::operator_id_t operator_id,
				//Processor::RowT& row
		//) {
			}
		}

	private:

};

wiselib::WiselibApplication<Os, App> example_app;
void application_main(Os::AppMainParameter& value) {
  example_app.init(value);
}


