
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
	typedef wiselib::BitmapAllocator<Os, 1000> Allocator;
	Allocator& get_allocator();
	//Allocator allocator_;
	//Allocator& get_allocator() { return allocator_; }

#include "platform.h"


#define APP_DATABASE_DEBUG 1


#undef TS_USE_BLOCK_MEMORY

//#define TS_CONTAINER_STATIC_VECTOR 0
//#define TS_CONTAINER_LIST 1

//#define TS_DICT_UNBAL 1

#define TS_CODEC_NONE 1
#define TS_CODEC_HUFFMAN 0

//#include "setup_tuplestore.h"

#include <util/serialization/serialization.h>
#include <util/meta.h>

class App {
	public:

		block_data_t rdf_buffer_[1024];

		Os::Radio::self_pointer_t radio_;
		Os::Timer::self_pointer_t timer_;
		Os::Debug::self_pointer_t debug_;

		//CodecTupleStoreT::Dictionary dictionary_;
		//CodecTupleStoreT::TupleContainer container_;
		//CodecTupleStoreT tuplestore_;

		void init(Os::AppMainParameter& amp) {
			radio_ = &wiselib::FacetProvider<Os, Os::Radio>::get_facet(amp);
			timer_ = &wiselib::FacetProvider<Os, Os::Timer>::get_facet(amp);
			debug_ = &wiselib::FacetProvider<Os, Os::Debug>::get_facet(amp);

			radio_->enable_radio();
			radio_->reg_recv_callback<App, &App::on_receive>(this);

			//initialize_tuplestore();
		#if APP_DATABASE_DEBUG
			debug_->debug("db boot %lu", (unsigned long)radio_->id());
		#endif

			//block_data_t x;
			//radio_->send(Os::Radio::BROADCAST_ADDRESS, 1, &x);

		}

		/**
		 * Do all necessary runtime initialization for the Wiselib TupleStore.
		 */
		void initialize_tuplestore() {
			/*
			#if TS_USE_BLOCK_MEMORY // Only needed & useful for block memory
				
				#ifdef PC
					block_cache_.physical().init("block_memory.img");
				#else
					block_cache_.physical().init();
				#endif
				block_cache_.init();
				block_allocator_.init(&block_cache_, debug_);
				
				// WARNING: This formats your block memory.
				// (Necessary, the block allocator won't work on a plain
				// blanked disc!)
				block_allocator_.wipe();
				
				// Cache can count the number of i/o's for us. Reset counter
				// here.
				block_cache_.reset_stats();
				
				dictionary_.init(&block_allocator_, debug_);
				container_.init(&block_allocator_, debug_);
			#else
			*/
				// RAM
				
//				dictionary_.init(debug_);
			//#endif
				
//			tuplestore_.init(&dictionary_, &container_, debug_);
		}


		void on_receive(Os::Radio::node_id_t from, Os::Radio::size_t len, Os::Radio::block_data_t *data) {

			if(data[0] == 0x99) {
				::uint16_t pos = wiselib::read<Os, block_data_t, ::uint16_t>(data + 1);

			#if APP_DATABASE_DEBUG
				debug_->debug("recv %x %x %x p=%d", (int)data[0], (int)data[1], (int)data[2], (int)pos);
			#endif

				if(len == 3) {
					#if APP_DATABASE_DEBUG
						debug_->debug("recv end");
					#endif
	//				timer_->set_timer<App, &App::start_insert>(5000, this, 0);
				}
				else {
					#if APP_DATABASE_DEBUG
						debug_->debug("recv l=%d", (int)len);
					#endif
					memcpy(rdf_buffer_ + pos, data + 3, len - 3);
				}

				block_data_t ack[] = { 0xAA, 0, 0 };
				wiselib::write<Os, block_data_t, ::uint16_t>(ack + 1, pos);
				radio_->send(from, 3, ack); 
			}
		}

		/*
		void start_insert(void*) {
			block_data_t *e = rdf_buffer_;

			while(*e) {
				Tuple t;
				t.set(0, e); e += strlen((char*)e);
				t.set(1, e); e += strlen((char*)e);
				t.set(2, e); e += strlen((char*)e);

					#if APP_DATABASE_DEBUG
						debug_->debug("ins %s %s %s", (char*)t.get(0), (char*)t.get(1), (char*)t.get(2));
					#endif
				tuplestore_.insert(t);
			}
		}
		*/

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
