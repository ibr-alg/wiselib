
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


//#include <util/tuple_store/static_dictionary.h>
//enum { SLOTS = 10 };
//typedef StaticDictionary<Os, SLOTS, 15> Dictionary;

#define STATIC_DICTIONARY_OUTSOURCE 0
#include "precompiled_ts.cpp"
#include <util/tuple_store/static_dictionary.h>

// incontextsensing:
//
// for w in $(< ../tuplestore_test/incontextsensing.rdf); do echo $w; done|sort|uniq|wc
//    67      67    2214
// --> total (unique) element size: 2214
// 67 * 6 = 402 bytes overhead with AVL
// + allocator overhead
//
//  171 * (10 + 4) = 2394
//  149 * (14 + 4) = 2682
//
enum { SLOTS = 149, SLOT_WIDTH = 14 };
typedef StaticDictionary<Os, SLOTS, SLOT_WIDTH> Dictionary;


#include <util/split_n3.h>

typedef wiselib::vector_static<Os, char*, 200> Inserts;
typedef wiselib::vector_static<Os, Dictionary::key_type, 200> UsedKeys;

class App {
	public:
		Dictionary dictionary_;
		UsedKeys used_keys;
		Inserts inserts;

		void init(Os::AppMainParameter& amp) {
			//radio_ = &wiselib::FacetProvider<Os, Os::Radio>::get_facet(amp);
			//timer_ = &wiselib::FacetProvider<Os, Os::Timer>::get_facet(amp);
			debug_ = &wiselib::FacetProvider<Os, Os::Debug>::get_facet(amp);
			rand_ = &wiselib::FacetProvider<Os, Os::Rand>::get_facet(amp);

			dictionary_.init(debug_);

		#if STATIC_DICTIONARY_OUTSOURCE
			dictionary_.set_data(dict_data_);
		#else
			read_n3(0);
		#endif
			//test_insert();

	//		for(int i = 0; i < 10; i++) { erase_random(); }
			//dictionary_.debug();
			//print_dictionary();
			verify_inserts();

			dictionary_.debug_precompile();
			erase_some();

			verify_inserts();
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

				insert(sp[0]);
				insert(sp[1]);
				insert(sp[2]);

			} // while cin

			//dictionary_.debug();
		}

		/**
		 * Insert into dictionary, track in `inserts`.
		 */
		void insert(char* s) {
			Dictionary::key_type k = dictionary_.insert((block_data_t*)s);
			int l = strlen(s) + 1;
			int pos = 0;
			for(Inserts::iterator iter = inserts.begin(); iter != inserts.end(); ++iter, pos++) {
				if(memcmp(*iter, s, l - 1) == 0) {
					assert(used_keys[pos] == k);
					return;
				}
			}

			char *s2 = new char[l];
			memcpy(s2, s, l);
			used_keys.push_back(k);
			inserts.push_back(s2);
		}

		/**
		 * Check that all inserts are present in the dictionary and nothing
		 * else is.
		 */
		void verify_inserts() {
			bool fail = false;
			debug_->debug("Verifying inserts using find()...");
			for(Inserts::iterator iter = inserts.begin(); iter != inserts.end(); ++iter) {
				Dictionary::key_type k = dictionary_.find((block_data_t*)*iter);
				if(k == Dictionary::NULL_KEY) {
					debug_->debug("  %s not findable.", *iter);
					fail = true;
				}
				else {
					block_data_t *ds = dictionary_.get_value(k);
					int c = strcmp(*iter, (char*)ds);
					if(c != 0) {
						debug_->debug("  %s found with key %d which resolves to %s",
								*iter, (int)k, (char*)ds);
						fail = true;
					}
					dictionary_.free_value(ds);
				}
			}
			if(!fail) {
				debug_->debug("  all inserts are findable.");
			}


			debug_->debug("Verifying inserts using iteration...");
			fail = false;
			size_type dictionary_entries = 0;
			for(Dictionary::iterator iter = dictionary_.begin_keys(); iter != dictionary_.end_keys(); ++iter) {
				Dictionary::key_type k = *iter;
				block_data_t *ds = dictionary_.get_value(k);
				//debug_->debug("  %s", (char*)ds);
				// Did we even insert this?
				bool found = false;
				for(Inserts::iterator jter = inserts.begin(); jter != inserts.end(); ++jter) {
					if(strcmp(*jter, (char*)ds) == 0) { found = true; break; }
				}
				if(!found) {
					debug_->debug("  key %d resolves to %s which has not been inserted",
							(int)k, (char*)ds);
					fail = true;
				}
				dictionary_.free_value(ds);
				dictionary_entries++;
			}
			if(!fail) {
				debug_->debug("  all dictionary keys were actually inserted.");
			}

			debug_->debug("Verifying using keys returned by insert()...");
			fail = false;
			int pos = 0;
			for(UsedKeys::iterator iter = used_keys.begin(); iter != used_keys.end(); ++iter, pos++) {
				block_data_t *ds = dictionary_.get_value(*iter);
				debug_->debug("[%d] -> '%s'", (int)*iter, (char*)ds);
				if(memcmp(inserts[pos], ds, strlen((char*)ds)) != 0) {
					debug_->debug("  key %d (%dth distinct insert) resolves to '%s' instead of '%s'",
							(int)*iter, (int)pos, (char*)ds, (char*)inserts[pos]);
					fail = true;
				}
			}


			if(dictionary_entries != inserts.size()) {
				debug_->debug("  number of dictionary entries %d doesnt match insertions %d",
						(int)dictionary_entries, (int)inserts.size());
			}

			if(dictionary_entries != dictionary_.size()) {
				debug_->debug("  number of dictionary entries %d doesnt match dictionary size %d",
						(int)dictionary_entries, (int)dictionary_.size());
			}
		}


		void erase_some() {
			debug_->debug("deleting some entries...");
			for(int i = 0; i<40; i++) {
				int p = rand_->operator()() % inserts.size();
				char *v = inserts[p];
				Dictionary::key_type k = dictionary_.find((block_data_t*)v);
				assert(k != Dictionary::NULL_KEY);
				int count = dictionary_.count(k);
				debug_->debug("  deleting %s (key=%d,count=%d)", v, (int)k, (int)count);
				int sz = dictionary_.size();
				dictionary_.erase(k);
				assert((count == 1) <= (dictionary_.size() == sz - 1));
				assert((count != 1) <= (dictionary_.size() == sz));
				if(dictionary_.size() == sz - 1) {
					debug_->debug("  entry %s cleared completely", v);
					inserts[p] = inserts.back();
					inserts.pop_back();

					used_keys[p] = used_keys.back();
					used_keys.pop_back();
				}
			}
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

