#if ISENSE
	extern "C" void assert(int) { }
#endif

#include <external_interface/external_interface.h>
#include <external_interface/external_interface_testing.h>
#ifdef ISENSE
	void* malloc(size_t n) { return isense::malloc(n); }
	void free(void* p) { isense::free(p); }
#endif

	
typedef wiselib::OSMODEL Os;
typedef Os::block_data_t block_data_t;
using namespace wiselib;

#include <util/allocators/malloc_free_allocator.h>
typedef MallocFreeAllocator<Os> Allocator;
Allocator& get_allocator();

#include <util/pstl/string_utils.h>
#include <util/meta.h>
#include <util/debugging.h>
#include <util/pstl/map_static_vector.h>
#include <util/pstl/priority_queue_dynamic.h>
#include <util/pstl/list_dynamic.h>
#include <util/pstl/unique_container.h>
#include <util/tuple_store/tuplestore.h>
#include <util/tuple_store/prescilla_dictionary.h>
#include "tuple.h"

#include <algorithms/routing/flooding_nd/flooding_nd.h>
#include <algorithms/protocols/packing_radio/packing_radio.h>
//#include <algorithms/routing/tree_routing_ndis/tree_routing_ndis.h>
#include <algorithms/routing/forward_on_directed_nd/forward_on_directed_nd.h>
#include <algorithms/hash/fnv.h>

#include <algorithms/semantic_entities/token_construction/token_construction.h>
#include <algorithms/semantic_entities/token_construction/semantic_entity_id.h>


typedef Tuple<Os> TupleT;
typedef wiselib::list_dynamic<Os, TupleT> TupleList;
typedef wiselib::UniqueContainer<TupleList> TupleContainer;
typedef wiselib::PrescillaDictionary<Os> Dictionary;
typedef wiselib::TupleStore<Os, TupleContainer, Dictionary, Os::Debug, BIN(111), &TupleT::compare> TS;

typedef wiselib::TokenConstruction<Os, Os::Radio, Os::Timer> TC;


class ExampleApplication
{
	public:
		
		template<typename TS>
		void ins(TS& ts, char* s, char* p, char* o) {
			TupleT t;
			t.set(0, (block_data_t*)s);
			t.set(1, (block_data_t*)p);
			t.set(2, (block_data_t*)o);
			ts.insert(t);
		}
		
		void init( Os::AppMainParameter& value )
		{
			radio_ = &wiselib::FacetProvider<Os, Os::Radio>::get_facet( value );
			timer_ = &wiselib::FacetProvider<Os, Os::Timer>::get_facet( value );
			debug_ = &wiselib::FacetProvider<Os, Os::Debug>::get_facet( value );
			clock_ = &wiselib::FacetProvider<Os, Os::Clock>::get_facet( value );
			
			debug_->debug( "Hello World from Example Application! my id=%d app=%p\n", radio_->id(), this );
			
			//init_ts();
			init_tc();
			
			
			/*
			if(radio_->id() == 0) {
				debug_->debug("---- I AM THE SINK! ----\n");
				timer_->set_timer<ExampleApplication, &ExampleApplication::be_sink>(1000, this, 0);
			}
			else {
				be();
			}
			*/
		}
		
		Dictionary dictionary;
		TupleContainer container;
		TS ts;
		
		#pragma GCC diagnostic push
		#pragma GCC diagnostic ignored "-Wwrite-strings"
		void init_ts() {
			dictionary.init(debug_);
			ts.init(&dictionary, &container, debug_);
			
			if(radio_->id() & 0x01) { ins(ts, "node", "foi", "f0"); }
			if((radio_->id() >> 1) & 0x01) { ins(ts, "node", "foi", "f1"); }
			if((radio_->id() >> 2) & 0x01) { ins(ts, "node", "foi", "f2"); }
			if((radio_->id() >> 3) & 0x01) { ins(ts, "node", "foi", "f3"); }
		}
		#pragma GCC diagnostic pop
		
		void init_tc() {
			token_construction_.init(radio_, timer_, clock_, debug_);
			
			for(size_t i = 0; i < 4; i++) {
				if((radio_->id() >> i) & 0x01) {
					token_construction_.add_entity(SemanticEntityId(1, i));
				}
			}
		}
		
		void hashes() {
			typedef Fnv32<Os> Hash;
			const char *strings[] = {
				"A", "measures", "m1", "m2", "has_value", "12", "14"
			};
			for(const char **s = strings; *s; s++) {
				DBG("%-20s %08x", *s, Hash::hash((block_data_t*)*s, strlen(*s)));
			}
		}
		
		
		
	private:
		Os::Radio::self_pointer_t radio_;
		Os::Timer::self_pointer_t timer_;
		Os::Debug::self_pointer_t debug_;
		Os::Clock::self_pointer_t clock_;
		
		TC token_construction_;
};

Allocator allocator_;
Allocator& get_allocator() { return allocator_; }
// --------------------------------------------------------------------------
wiselib::WiselibApplication<Os, ExampleApplication> example_app;
// --------------------------------------------------------------------------
void application_main( Os::AppMainParameter& value )
{
  example_app.init( value );
}
