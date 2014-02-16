
#include "defs.h"

////
//#define APP_DATABASE_DEBUG 1
//#define APP_DATABASE_ERASE 1
//#define MODE_ERASE 1
////

#if APP_DATABASE_DEBUG
	#define APP_HEARTBEAT 1
#endif

#if (TS_USE_TREE_DICT || TS_USE_PRESCILLA_DICT || TS_USE_AVL_DICT)
	#define TS_USE_ALLOCATOR 1
#endif
#undef TS_USE_BLOCK_MEMORY
#define TS_CODEC_NONE 1
#define TS_CODEC_HUFFMAN 0

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

#if TS_USE_ALLOCATOR
	#warning "Using BITMAP allocator"

	#include <util/allocators/bitmap_allocator.h>

	#if TS_USE_PRESCILLA_DICT
		typedef wiselib::BitmapAllocator<Os, 3072, 4> Allocator;
	#else
		typedef wiselib::BitmapAllocator<Os, 3072, 16> Allocator;
	#endif
	Allocator allocator_;
	Allocator& get_allocator() { return allocator_; }
#endif

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

		/*
		void find_erase(block_data_t* s, block_data_t* p, block_data_t* o) {
			Tuple t;
			t.set(0, s);
			t.set(1, p);
			t.set(2, o);

			CodecTupleStoreT::column_mask_t mask =
				((s != 0) << 0) | ((p != 0) << 1) | ((o != 0) << 2);

			Tuple v;
			CodecTupleStoreT::iterator iter = tuplestore_.begin(&t, mask);
			do {
				iter = tuplestore_.erase(iter);
			} while(iter != tuplestore_.end());
		}
		*/

		void erase(block_data_t*s, block_data_t* p, block_data_t* o) {
			Tuple t;
			t.set(0, s);
			t.set(1, p);
			t.set(2, o);
			CodecTupleStoreT::column_mask_t mask =
				((*s != 0) << 0) | ((*p != 0) << 1) | ((*o != 0) << 2);

			#if APP_DATABASE_DEBUG
				debug_->debug("erase mask %d", (int)mask);
				debug_->debug("erasing (%s,%s,%s) sz=%d", (char*)s, (char*)p, (char*)o, (int)tuplestore_.size());
			#endif
			//Tuple v;
			CodecTupleStoreT::iterator iter = tuplestore_.begin(&t, mask);
			//do {
			tuplestore_.erase(iter);
			//} while(iter != tuplestore_.end());
			#if APP_DATABASE_DEBUG
				debug_->debug("post erase (%s,%s,%s) sz=%d", (char*)s, (char*)p, (char*)o, (int)tuplestore_.size());
			#endif
		}

		void prepare_erase(block_data_t*s, block_data_t* p, block_data_t* o) {

			RandomChoice c(tuplestore_.size());
			*s = '\0';
			*p = '\0';
			*o = '\0';
			CodecTupleStoreT::iterator iter;
			for(iter = tuplestore_.begin(); iter != tuplestore_.end(); ++iter, ++c) {
				if(c.choose()) {
					int spo = (unsigned)rand() % 3;
					strcpy(reinterpret_cast<char*>(s), reinterpret_cast<char*>(iter->get(0)));
					strcpy(reinterpret_cast<char*>(p), reinterpret_cast<char*>(iter->get(1)));
					strcpy(reinterpret_cast<char*>(o), reinterpret_cast<char*>(iter->get(2)));
					if(spo == 0) { *s = '\0'; }
					else if(spo == 1) { *p = '\0'; }
					else { *o = '\0'; }
					break;
				}
			}
			if(*s == '\0' && *p == '\0') {
				iter = tuplestore_.begin();
					int spo = (unsigned)rand() % 3;
					strcpy(reinterpret_cast<char*>(s), reinterpret_cast<char*>(iter->get(0)));
					strcpy(reinterpret_cast<char*>(p), reinterpret_cast<char*>(iter->get(1)));
					strcpy(reinterpret_cast<char*>(o), reinterpret_cast<char*>(iter->get(2)));
					if(spo == 0) { *s = '\0'; }
					else if(spo == 1) { *p = '\0'; }
					else { *o = '\0'; }
			}
			#if APP_DATABASE_DEBUG
				debug_->debug("will erase (%s,%s,%s)", (char*)s, (char*)p, (char*)o);
			#endif
		}

};

// <general wiselib boilerplate>
// {{{
	
	// Application Entry Point & Definiton of allocator
	wiselib::WiselibApplication<Os, App> app;
	void application_main(Os::AppMainParameter& amp) { app.init(amp); }
	
// }}}
// </general wiselib boilerplate>

// vim: set ts=4 sw=4 tw=78 noexpandtab foldmethod=marker foldenable :
