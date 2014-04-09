
// <general wiselib boilerplate>
// {{{

	#include "external_interface/external_interface.h"
	#include "external_interface/external_interface_testing.h"
	using namespace wiselib;
	typedef OSMODEL Os;
	typedef Os::block_data_t block_data_t;
	typedef Os::size_t size_type;

	// Enable dynamic memory allocation using malloc() & free()
	#include "util/allocators/malloc_free_allocator.h"
	typedef MallocFreeAllocator<Os> Allocator;
	Allocator& get_allocator();

// }}}
// </general wiselib boilerplate>

#define TS_USE_BLOCK_MEMORY 0
#define APP_GATEWAY 0


#include "setup_tuplestore.h"


// Our Application

class App {
	// {{{
	public:
		block_data_t rdf_buffer_[1024];
		size_type bytes_received_;

		void init(Os::AppMainParameter& amp) {
			debug_ = &wiselib::FacetProvider<Os, Os::Debug>::get_facet(amp);
			debug_->debug("App booting!");

			bytes_received_ = 0;
			
			initialize_tuplestore();
			insert_some_tuples();
			show_tuplestore();
		}

	#if APP_GATEWAY

	#endif // APP_GATEWAY


		// ----- SWEEEEP ----

		
		/**
		 * Do all necessary runtime initialization for the Wiselib TupleStore.
		 */
		void initialize_tuplestore() {
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
				// RAM
				
				dictionary_.init(debug_);
			#endif
				
			tuplestore_.init(&dictionary_, &container_, debug_);
		}
		
		void insert_some_tuples() {
			Tuple t;
			
			t.set("this", "is_a", "test");
			tuplestore_.insert(t);
			
			t.set("one", "two", "three");
			tuplestore_.insert(t);
		}
		
		void show_tuplestore() {
			for(CodecTupleStoreT::iterator iter = tuplestore_.begin();
					iter != tuplestore_.end();
					++iter) {
				debug_->debug("( %s %s %s )", iter->get(0), iter->get(1), iter->get(2));
			}
		}
		
	private:
		#if TS_USE_BLOCK_MEMORY
			BlockCache block_cache_;
			BlockAllocator block_allocator_;
		#endif
			
			CodecTupleStoreT::Dictionary dictionary_;
			CodecTupleStoreT::TupleContainer container_;
			CodecTupleStoreT tuplestore_;
			
			Os::Debug::self_pointer_t debug_;
	// }}}
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

