
#include <cassert>

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
typedef wiselib::BitmapAllocator<Os, 400, 16> Allocator;
Allocator& get_allocator();

#include <util/pstl/vector_static.h>
typedef wiselib::vector_static<Os, block_data_t*, 200> Pointers;

class App {
	public:

		enum { MAX_ELEMENT_SIZE = 64 };

		void init(Os::AppMainParameter& amp) {
			debug_ = &wiselib::FacetProvider<Os, Os::Debug>::get_facet(amp);
			rand_ = &wiselib::FacetProvider<Os, Os::Rand>::get_facet(amp);

			test_merge();
			//for(int i = 0; i < 100; i++) {
				//allocate_one();
				//allocate_one();
				//free_one();
			//}
		}

		void test_merge() {
			int c = 16;
			block_data_t *p0 = allocate(3 * c);
			block_data_t *p1 = allocate(4 * c);
			block_data_t *p2 = allocate(3 * c);
			free(p1);
			free(p0);
			allocate(7 * c);
		}


		block_data_t* allocate(size_type n) {
			return reinterpret_cast<block_data_t*>(::get_allocator().allocate_array<block_data_t>(n) .raw());
		}

		void free(block_data_t *p) {
			::get_allocator().free_array(p);
		}

		void allocate_one() {
			size_type n = rand_->operator()() % MAX_ELEMENT_SIZE;
			block_data_t *p = reinterpret_cast<block_data_t*>(::get_allocator().allocate_array<block_data_t>(n) .raw());
			pointers.push_back(p);
		}

		void free_one() {
			int i = rand_->operator()() % pointers.size();
			block_data_t* p = pointers[i];
			::get_allocator().free_array(p);
			pointers[i] = pointers.back();
			pointers.pop_back();
		}



		//Os::Radio::self_pointer_t radio_;
		//Os::Timer::self_pointer_t timer_;
		Os::Debug::self_pointer_t debug_;
		Os::Rand::self_pointer_t rand_;

		Pointers pointers;
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

