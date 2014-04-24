
#ifdef SHAWN
	#include "boilerplate_shawn.h"
#endif

#include "static_data.h"

class App : public AppBoilerplate {
	public:
		void init(Os::AppMainParameter& v) {
			AppBoilerplate::init(v);

			insert_tuples(rdf);

			timer_->set_timer<App, &App::load_predefined_query>(1000, this, 0);
		}

		void load_predefined_query(void*) {
			debug_->debug("loading pre-installed query...");

			process(sizeof(op100), op100);
			process(sizeof(op90), op90);
			process(sizeof(op80), op80);
			process(sizeof(op70), op70);
			process(sizeof(cmd), cmd);
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
};


