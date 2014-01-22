
#include "external_interface/external_interface.h"
#include "external_interface/external_interface_testing.h"
using namespace wiselib;
typedef OSMODEL Os;
typedef OSMODEL OsModel;
typedef Os::block_data_t block_data_t;
typedef Os::size_t size_type;
	// Enable dynamic memory allocation using malloc() & free()
	//#include "util/allocators/malloc_free_allocator.h"
	//typedef MallocFreeAllocator<Os> Allocator;
	//Allocator& get_allocator();

	#include <util/allocators/bitmap_allocator.h>
	typedef wiselib::BitmapAllocator<Os, 4000> Allocator;
	Allocator& get_allocator();
	//Allocator allocator_;
	//Allocator& get_allocator() { return allocator_; }

#include "platform.h"


#define APP_DATABASE_DEBUG 0


#undef TS_USE_BLOCK_MEMORY

//#define TS_CONTAINER_STATIC_VECTOR 0
//#define TS_CONTAINER_LIST 1

//#define TS_DICT_UNBAL 1

#define TS_CODEC_NONE 1
#define TS_CODEC_HUFFMAN 0

#include "setup_tuplestore.h"

#include <util/serialization/serialization.h>
#include <util/meta.h>

#if defined(CONTIKI)
	#include <contiki.h>
	#include <netstack.h>
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


};

// <general wiselib boilerplate>
// {{{
	
	// Application Entry Point & Definiton of allocator
	Allocator allocator_;
	Allocator& get_allocator() { return allocator_; }
	wiselib::WiselibApplication<Os, App> app;
	void application_main(Os::AppMainParameter& amp) { app.init(amp); }
	
// }}}
// </general wiselib boilerplate>

// vim: set ts=4 sw=4 tw=78 noexpandtab foldmethod=marker foldenable :
