
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

//#include "static_data.h"

// Simple temperature aggregation query
#include "query_node2_aggregate_temperature.h"
//#include "query_collect.h"

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

		#if ENABLE_PREINSTALLED_QUERY
			timer_->set_timer<App, &App::load_predefined_query>(10000, this, (void*)3);
		#endif
			result_radio().reg_recv_callback<App, &App::on_sink_receive>(this);
		}

		void load_predefined_query(void* x) {
			Uvoid x2 = (Uvoid)x;
			#if ENABLE_DEBUG
				debug_->debug("<3 %lu", (unsigned long)now());
			#endif
			x2--;
			if(x2 == 0) {
				x2 = 30;
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

		void on_sink_receive(ResultRadio::node_id_t from, ResultRadio::size_t size, ResultRadio::block_data_t *data) {
			ResultRadio::message_id_t msgid = wiselib::read<Os, block_data_t, ResultRadio::message_id_t>(data);
			
			if(from == SINK) {
				#if ENABLE_DEBUG
				debug_->debug("sink recv from %lu", (unsigned long)from);
				//debug_->debug("ANS %lu", (unsigned long)
				//wiselib::debug_buffer<Os, 16, Os::Debug>(debug_, data, size);
				//debug_->debug("RESULT");
				Communicator::ResultMessage &msg = *reinterpret_cast<Communicator::ResultMessage*>(data);
				wiselib::debug_buffer<Os, 16, Os::Debug>(debug_, msg.payload_data(), msg.payload_size());

				//Communicator::RowT &row = *reinterpret_cast<Communicator::RowT*>(msg.payload_data());
				//debug_->debug("RESULT %lu ", (int)row.as_int(0), (float)row.as_float(1));
				#endif
			}
		}

};

wiselib::WiselibApplication<Os, App> example_app;
void application_main(Os::AppMainParameter& value) {
  example_app.init(value);
}


