
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
	typedef wiselib::BitmapAllocator<Os, 3000> Allocator;
	Allocator& get_allocator();

#include <util/pstl/static_dictionary.h>
typedef StaticDictionary<Os, 100, 15> Dictionary;

class App {
	public:
		Dictionary dictionary_;

		void init(Os::AppMainParameter& amp) {
			//radio_ = &wiselib::FacetProvider<Os, Os::Radio>::get_facet(amp);
			//timer_ = &wiselib::FacetProvider<Os, Os::Timer>::get_facet(amp);
			debug_ = &wiselib::FacetProvider<Os, Os::Debug>::get_facet(amp);

			dictionary_.init(debug_);
			test_insert();
		}

		void test_insert() {
			dictionary_.insert((block_data_t*)"<http://www.spitfire-project.eu/ontologies/blah.owl>");
			dictionary_.insert((block_data_t*)"<http://www.spitfire-project.eu/ontologies/blub.owl>");
			dictionary_.insert((block_data_t*)"<http://www.spitfire-project.eu/ontologies/blah.owl>");
			dictionary_.insert((block_data_t*)"<http://www.spitfire-project.eu/ontologies/blah.owl>");

			debug_->debug("%d", (int)
			dictionary_.find((block_data_t*)"<http://www.spitfire-project.eu/ontologies/blah.owl>"));

			dictionary_.debug();
		}

		//Os::Radio::self_pointer_t radio_;
		//Os::Timer::self_pointer_t timer_;
		Os::Debug::self_pointer_t debug_;
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

