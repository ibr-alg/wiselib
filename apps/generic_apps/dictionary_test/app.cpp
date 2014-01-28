
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


//#include <util/tuple_store/static_dictionary.h>
//enum { SLOTS = 10 };
//typedef StaticDictionary<Os, SLOTS, 15> Dictionary;

#define STATIC_DICTIONARY_OUTSOURCE 1
#include "precompiled_ts.cpp"
#include <util/tuple_store/static_dictionary.h>
enum { SLOTS = 200, SLOT_WIDTH = 9 };
typedef StaticDictionary<Os, SLOTS, SLOT_WIDTH> Dictionary;


#include <util/split_n3.h>

class App {
	public:
		Dictionary dictionary_;

		void init(Os::AppMainParameter& amp) {
			//radio_ = &wiselib::FacetProvider<Os, Os::Radio>::get_facet(amp);
			//timer_ = &wiselib::FacetProvider<Os, Os::Timer>::get_facet(amp);
			debug_ = &wiselib::FacetProvider<Os, Os::Debug>::get_facet(amp);
			rand_ = &wiselib::FacetProvider<Os, Os::Rand>::get_facet(amp);

			dictionary_.init(debug_);

		#if STATIC_DICTIONARY_OUTSOURCE
			dictionary_.set_data(dict_data_);
		#endif

			//test_insert();
	//		read_n3(0);

	//		for(int i = 0; i < 10; i++) { erase_random(); }
			dictionary_.debug();
			print_dictionary();
		}

		void read_n3(void*) {
			typedef SplitN3<Os> Splitter;
			Splitter sp;
			char buf[1025];

			while(std::cin) {

				//if(buf_empty) {
					std::cin.getline(buf, 1024);
					//buf_empty = false;
				//}

				if(buf[0] == 0) { break; }
				//debug_->debug("PARSE: [[[%s]]]", (char*)buf);

				sp.parse_line(buf);

				dictionary_.insert((block_data_t*)sp[0]);
				dictionary_.insert((block_data_t*)sp[1]);
				dictionary_.insert((block_data_t*)sp[2]);

			} // while cin

			dictionary_.debug();
		}

		void erase_random() {
			Dictionary::key_type k = rand_->operator()() % SLOTS;
			debug_->debug("erase %d.", (int)k);
			dictionary_.erase(k);
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

		void print_dictionary() {
			for(Dictionary::iterator iter = dictionary_.begin_keys(); iter != dictionary_.end_keys(); ++iter) {
				block_data_t *v = dictionary_.get_value(*iter);
				debug_->debug("%lu => %s", (unsigned long)*iter, (char*)v);
				dictionary_.free_value(v);
			}
		}


		//Os::Radio::self_pointer_t radio_;
		//Os::Timer::self_pointer_t timer_;
		Os::Debug::self_pointer_t debug_;
		Os::Rand::self_pointer_t rand_;
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

