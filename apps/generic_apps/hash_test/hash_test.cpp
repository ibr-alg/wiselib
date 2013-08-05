

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

#include <algorithms/hash/bernstein.h>
#include <algorithms/hash/elf.h>
#include <algorithms/hash/firstchar.h>
#include <algorithms/hash/fletcher.h>
#include <algorithms/hash/fnv.h>
#include <algorithms/hash/jenkins_lookup2.h>
#include <algorithms/hash/jenkins_lookup3.h>
#include <algorithms/hash/jenkins_one_at_a_time.h>
#include <algorithms/hash/kr.h>
#include <algorithms/hash/larson.h>
#include <algorithms/hash/modified_bernstein.h>
#include <algorithms/hash/murmur.h>
#include <algorithms/hash/novak.h>
#include <algorithms/hash/sdbm.h>

class App {
	public:
		void init(Os::AppMainParameter& amp) {
			debug_ = &wiselib::FacetProvider<Os, Os::Debug>::get_facet(amp);
			
			//test_splitn3();
	#if COMPILE_SPLITN3_CAT
			splitn3_cat();
			
	#elif COMPILE_HASH_CAT
			
			if(strcmp(amp.argv[1], "bernstein8") == 0) { hash_cat< Bernstein<Os, ::uint8_t> >(); }
			else if(strcmp(amp.argv[1], "bernstein16") == 0) { hash_cat< Bernstein<Os, ::uint16_t> >(); }
			else if(strcmp(amp.argv[1], "bernstein32") == 0) { hash_cat< Bernstein<Os, ::uint32_t> >(); }
			else if(strcmp(amp.argv[1], "bernstein64") == 0) { hash_cat< Bernstein<Os, ::uint64_t> >(); }
			else if(strcmp(amp.argv[1], "bernstein2_8") == 0) { hash_cat< ModifiedBernstein<Os, ::uint8_t> >(); }
			else if(strcmp(amp.argv[1], "bernstein2_16") == 0) { hash_cat< ModifiedBernstein<Os, ::uint16_t> >(); }
			else if(strcmp(amp.argv[1], "bernstein2_32") == 0) { hash_cat< ModifiedBernstein<Os, ::uint32_t> >(); }
			else if(strcmp(amp.argv[1], "bernstein2_64") == 0) { hash_cat< ModifiedBernstein<Os, ::uint64_t> >(); }
			else if(strcmp(amp.argv[1], "elf32") == 0) { hash_cat< Elf<Os> >(); }
			else if(strcmp(amp.argv[1], "firstchar8") == 0) { hash_cat< Firstchar<Os> >(); }
			else if(strcmp(amp.argv[1], "fletcher16") == 0) { hash_cat< Fletcher<Os, ::uint16_t> >(); }
			
			else if(strcmp(amp.argv[1], "fnv1_16") == 0) { hash_cat< Fnv1<Os, ::uint16_t> >(); }
			else if(strcmp(amp.argv[1], "fnv1_32") == 0) { hash_cat< Fnv1<Os, ::uint32_t> >(); }
			else if(strcmp(amp.argv[1], "fnv1_64") == 0) { hash_cat< Fnv1<Os, ::uint64_t> >(); }
			else if(strcmp(amp.argv[1], "fnv1a_16") == 0) { hash_cat< Fnv1a<Os, ::uint16_t> >(); }
			else if(strcmp(amp.argv[1], "fnv1a_32") == 0) { hash_cat< Fnv1a<Os, ::uint32_t> >(); }
			else if(strcmp(amp.argv[1], "fnv1a_64") == 0) { hash_cat< Fnv1a<Os, ::uint64_t> >(); }
			else if(strcmp(amp.argv[1], "lookup2_32") == 0) { hash_cat< JenkinsLookup2<Os> >(); }
			else if(strcmp(amp.argv[1], "lookup3_32") == 0) { hash_cat< JenkinsLookup3<Os> >(); }
			else if(strcmp(amp.argv[1], "oneatatime_32") == 0) { hash_cat< JenkinsOneAtATime<Os> >(); }
			else if(strcmp(amp.argv[1], "kr8") == 0) { hash_cat< Kr<Os, ::uint8_t> >(); }
			else if(strcmp(amp.argv[1], "kr16") == 0) { hash_cat< Kr<Os, ::uint16_t> >(); }
			else if(strcmp(amp.argv[1], "kr32") == 0) { hash_cat< Kr<Os, ::uint32_t> >(); }
			else if(strcmp(amp.argv[1], "kr64") == 0) { hash_cat< Kr<Os, ::uint64_t> >(); }
			else if(strcmp(amp.argv[1], "larson8") == 0) { hash_cat< Larson<Os, ::uint8_t> >(); }
			else if(strcmp(amp.argv[1], "larson16") == 0) { hash_cat< Larson<Os, ::uint16_t> >(); }
			else if(strcmp(amp.argv[1], "larson32") == 0) { hash_cat< Larson<Os, ::uint32_t> >(); }
			else if(strcmp(amp.argv[1], "larson64") == 0) { hash_cat< Larson<Os, ::uint64_t> >(); }
			else if(strcmp(amp.argv[1], "novak8") == 0) { hash_cat< Novak<Os, ::uint8_t> >(); }
			else if(strcmp(amp.argv[1], "novak16") == 0) { hash_cat< Novak<Os, ::uint16_t> >(); }
			else if(strcmp(amp.argv[1], "novak32") == 0) { hash_cat< Novak<Os, ::uint32_t> >(); }
			else if(strcmp(amp.argv[1], "novak64") == 0) { hash_cat< Novak<Os, ::uint64_t> >(); }
			else if(strcmp(amp.argv[1], "sdbm8") == 0) { hash_cat< Sdbm<Os, ::uint8_t> >(); }
			else if(strcmp(amp.argv[1], "sdbm16") == 0) { hash_cat< Sdbm<Os, ::uint16_t> >(); }
			else if(strcmp(amp.argv[1], "sdbm32") == 0) { hash_cat< Sdbm<Os, ::uint32_t> >(); }
			else if(strcmp(amp.argv[1], "sdbm64") == 0) { hash_cat< Sdbm<Os, ::uint64_t> >(); }
			else {
				debug_->debug("ALART! hash function '%s' not found!", amp.argv[1]);
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
			
			char line[20480];
			while(std::cin) {
				std::cin.getline(line, 20480);
				if(strlen(line) > 0) {
					std::cout << (unsigned long long)Hash::hash((const block_data_t*)line, strlen(line)) << std::endl;
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

