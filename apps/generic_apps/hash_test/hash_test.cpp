

#define COMPILE_HASH_CAT 1
#define COMPILE_SPLITN3_CAT 0

#include <external_interface/external_interface.h>
using namespace wiselib;
typedef OSMODEL Os;
#include "util/allocators/malloc_free_allocator.h"
typedef MallocFreeAllocator<Os> Allocator;
Allocator& get_allocator();
typedef Os::block_data_t block_data_t;
typedef Os::size_t size_type;

#include <util/split_n3.h>

#ifdef PC
	#include <iostream>
#endif

#include <algorithms/hash/fnv.h>
#include <algorithms/hash/murmur.h>
#include <algorithms/hash/jenkins.h>
#include <algorithms/hash/larson.h>
//typedef Fnv32<Os> Hash;
//typedef Fnv64<Os> Hash;

class App {
	public:
		void init(Os::AppMainParameter& amp) {
			debug_ = &wiselib::FacetProvider<Os, Os::Debug>::get_facet(amp);
			
			//test_splitn3();
	#if COMPILE_SPLITN3_CAT
			splitn3_cat();
			
	#elif COMPILE_HASH_CAT
			
			if(strcmp(amp.argv[1], "fnv32") == 0) {
				hash_cat< Fnv32<Os> >();
			}
			else if(strcmp(amp.argv[1], "fnv64") == 0) {
				hash_cat< Fnv64<Os> >();
			}
			else if(strcmp(amp.argv[1], "murmur") == 0) {
				hash_cat< Murmur<Os> >();
			}
			else if(strcmp(amp.argv[1], "jenkins") == 0) {
				hash_cat< Jenkins<Os> >();
			}
			else if(strcmp(amp.argv[1], "larson") == 0) {
				hash_cat< Larson<Os> >();
			}
			
	#endif
		}
		
		
		void test_splitn3() {
			SplitN3<Os> splitter;
			splitter.parse_line((char*)
				" <http://foo.bar.baz/blah?x#blub>\t  foo:bar23    \" 12.34\"^^xsd::double .  ");
			
			debug_->debug("elements: %d", splitter.size());
			for(size_type i=0; i<splitter.size(); i++) {
				debug_->debug("%d: %s", i, splitter[i]);
			}
		}
		
		void splitn3_cat() {
			char line[20480];
			SplitN3<Os> splitter;
			while(std::cin) {
				std::cin.getline(line, 20480);
				splitter.parse_line(line);
				for(size_type i=0; i<splitter.size(); i++) {
					//debug_->debug("%s", splitter[i]);
					std::cout << splitter[i] << std::endl;
				}
			}
		}
		
		template<typename Hash>
		void hash_cat() {
			Hash h;
			
			char line[20480];
			while(std::cin) {
				std::cin.getline(line, 20480);
				if(strlen(line) > 0) {
					std::cout << h.hash((const block_data_t*)line, strlen(line)) << std::endl;
				}
			}
		}
		
	private:
		Os::Debug::self_pointer_t debug_;
		Os::Rand::self_pointer_t rand_;
		Os::Timer::self_pointer_t timer_;
		Os::Clock::self_pointer_t clock_;
		//Os::Radio::self_pointer_t radio_;
};

App app;
Allocator allocator_;
Allocator& get_allocator() { return allocator_; }

void application_main(Os::AppMainParameter& amp) {
	app.init(amp);
}

