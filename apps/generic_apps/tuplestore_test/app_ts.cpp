
#include "defs.h"
#if APP_DATABASE_DEBUG
	#define APP_HEARTBEAT 1
#endif


#include "platform.h"

extern "C" {
	#include <string.h>
}

#include "external_interface/external_interface.h"
#include "external_interface/external_interface_testing.h"
using namespace wiselib;
typedef OSMODEL Os;
typedef OSMODEL OsModel;
typedef Os::block_data_t block_data_t;
typedef Os::size_t size_type;


#undef TS_USE_BLOCK_MEMORY
#define TS_CODEC_NONE 1
#define TS_CODEC_HUFFMAN 0

#include "setup_tuplestore.h"

#include <util/serialization/serialization.h>
#include <util/meta.h>

#if defined(CONTIKI)
extern "C" {
	#define delete why_on_earth_do_you_guys_name_a_variable_delete
	#define DOMAIN DOMAIN_WHAT_IS_THIS_I_DONT_EVEN

	#include <contiki.h>
	#include <netstack.h>
	#include <stdlib.h>
}
#endif

class App {
	public:

		#include "app_db_mixin.cpp"

		CodecTupleStoreT::Dictionary dictionary_;
		CodecTupleStoreT::TupleContainer container_;
		CodecTupleStoreT tuplestore_;

		/**
		 * Do all necessary runtime initialization for the Wiselib TupleStore.
		 */
		void initialize_db() {
			dictionary_.init(debug_);
			tuplestore_.init(&dictionary_, &container_, debug_);
			#if APP_DATABASE_DEBUG
				debug_->debug("ts initialized");
			#endif
		}

		void insert_tuple(char* s, char* p, char* o) {
			Tuple t;
			t.set(0, (block_data_t*)s);
			t.set(1, (block_data_t*)p);
			t.set(2, (block_data_t*)o);

			#if APP_DATABASE_DEBUG
				debug_->debug("ins (%s,%s,%s)", (char*)t.get(0), (char*)t.get(1), (char*)t.get(2));
			#endif
			tuplestore_.insert(t);
		}

		size_type size() { return tuplestore_.size(); }

		void find(block_data_t* s, block_data_t* p, block_data_t* o, char *out) {
			Tuple t;
			t.set(0, s);
			t.set(1, p);
			t.set(2, o);

			CodecTupleStoreT::column_mask_t mask =
				((s != 0) << 0) | ((p != 0) << 1) | ((o != 0) << 2);

			Tuple v;
			v = *tuplestore_.begin(&t, mask);
		}

		void find_erase(block_data_t* s, block_data_t* p, block_data_t* o) {
			Tuple t;
			t.set(0, s);
			t.set(1, p);
			t.set(2, o);

			CodecTupleStoreT::column_mask_t mask =
				((s != 0) << 0) | ((p != 0) << 1) | ((o != 0) << 2);

			Tuple v;
			ContainerTupleStoreT::iterator iter = tuplestore_.begin(&t, mask);
			do {
				iter = tuplestore_.erase(iter);
			} while(iter != tuplestore_.end());
		}

};

// <general wiselib boilerplate>
// {{{
	
	// Application Entry Point & Definiton of allocator
	//Allocator allocator_;
	//Allocator& get_allocator() { return allocator_; }
	wiselib::WiselibApplication<Os, App> app;
	void application_main(Os::AppMainParameter& amp) { app.init(amp); }
	
// }}}
// </general wiselib boilerplate>

// vim: set ts=4 sw=4 tw=78 noexpandtab foldmethod=marker foldenable :
