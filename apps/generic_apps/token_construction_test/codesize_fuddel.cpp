
#include "platform.h"

typedef Os::block_data_t block_data_t;
using namespace wiselib;

//#include <util/allocators/malloc_free_allocator.h>
//typedef MallocFreeAllocator<Os> Allocator;
//Allocator& get_allocator();

#if defined(ARDUINO)
	#include <external_interface/arduino/arduino_monitor.h>
#endif


#if !defined(CODESIZE_EMPTY)

	#include <util/meta.h>
	#include <util/debugging.h>
	//#include <util/pstl/map_static_vector.h>
	//#include <util/pstl/priority_queue_dynamic.h>
	#include <util/pstl/list_dynamic.h>
	#include <util/pstl/unique_container.h>
	#include <util/tuple_store/tuplestore.h>
	#include "tuple.h"

	//#include <algorithms/routing/flooding_nd/flooding_nd.h>
	//#include <algorithms/protocols/packing_radio/packing_radio.h>
	//#include <algorithms/routing/tree_routing_ndis/tree_routing_ndis.h>
	//#include <algorithms/routing/forward_on_directed_nd/forward_on_directed_nd.h>

	#include <algorithms/hash/sdbm.h>
	typedef Sdbm<Os> Hash;

	#include <algorithms/semantic_entities/token_construction/token_scheduler.h>
	#include <algorithms/semantic_entities/token_construction/semantic_entity_id.h>


	//#include "semantics_office1.h"
	//#include "semantics_uniform.h"
	#include "semantics_simple.h"

	typedef Tuple<Os> TupleT;
	typedef wiselib::list_dynamic<Os, TupleT> TupleList;
	typedef wiselib::UniqueContainer<TupleList> TupleContainer;
	
	// 6867
	
	#if USE_PRESCILLA
		#warning "Using PRESCILLA dictionary"
		#include <util/tuple_store/prescilla_dictionary.h>
		typedef wiselib::PrescillaDictionary<Os> Dictionary;
		typedef wiselib::TupleStore<Os, TupleContainer, Dictionary, Os::Debug, BIN(111), &TupleT::compare> TS;
	#elif USE_TREEDICT
		//#warning "Using TREE dictionary"
		//#include <util/pstl/unbalanced_tree_dictionary.h>
		//typedef wiselib::UnbalancedTreeDictionary<Os> Dictionary;
		//typedef wiselib::TupleStore<Os, TupleContainer, Dictionary, Os::Debug, BIN(111), &TupleT::compare> TS;
	#else
		#warning "Using NULL dictionary"
		#include <util/tuple_store/null_dictionary.h>
		typedef wiselib::TupleStore<Os, TupleContainer, NullDictionary<Os>, Os::Debug, 0, &TupleT::compare> TS;
	#endif
		
	// 6855 (-12)

	//typedef wiselib::TokenScheduler<Os, TS, Os::Radio, Os::Timer, Os::Clock, Os::Debug, Os::Rand> TC;

	// 5100 (-1755)
	
	#if USE_INQP
	/*
		#warning "-------------- Using INQP rule processor"
		#include <algorithms/rdf/inqp/query_processor.h>
		#include <algorithms/semantic_entities/token_construction/inqp_rule_processor.h>
		typedef wiselib::InqpRuleProcessor<Os, QueryProcessor, TC> RuleProcessor;
		typedef wiselib::INQPQueryProcessor<Os, TS> QueryProcessor;
		//typedef QueryProcessor::Hash Hash;
		typedef QueryProcessor::Query Query;
		typedef QueryProcessor::GraphPatternSelectionT GPS;
		typedef QueryProcessor::CollectT Coll;
		typedef wiselib::ProjectionInfo<Os> Projection;
		*/
	#else 
	/*
		#warning "Using SIMPLE rule processor"
		#include <algorithms/semantic_entities/token_construction/simple_rule_processor.h>
		typedef wiselib::SimpleRuleProcessor<Os, TS, TC, Hash> RuleProcessor;
		*/
	#endif
	
	// 4974 (-126)
	
#endif // not def CODESIZE_EMPTY


#define STRHASH(X) Hash::hash((const block_data_t*)(X), strlen_compiletime(X))


class ExampleApplication
{
	public:
		
	#if !defined(CODESIZE_EMPTY)
		
		
		void init( Os::AppMainParameter& value )
		{
			radio_ = &wiselib::FacetProvider<Os, Os::Radio>::get_facet( value );
			timer_ = &wiselib::FacetProvider<Os, Os::Timer>::get_facet( value );
			debug_ = &wiselib::FacetProvider<Os, Os::Debug>::get_facet( value );
			clock_ = &wiselib::FacetProvider<Os, Os::Clock>::get_facet( value );
			rand_ = &wiselib::FacetProvider<Os, Os::Rand>::get_facet(value);
			
			monitor_.init(debug_);
			monitor_.report();
		}
		
		//Dictionary dictionary;
		TupleContainer container;
		//TS ts;
		
	private:
		Os::Radio::self_pointer_t radio_;
		Os::Timer::self_pointer_t timer_;
		Os::Debug::self_pointer_t debug_;
		Os::Clock::self_pointer_t clock_;
		Os::Rand::self_pointer_t rand_;
		
		ArduinoMonitor<Os, Os::Debug> monitor_;
		
		//TC token_construction_;
		//RuleProcessor rule_processor_;
		
		//Dictionary::key_type aggr_key_temp_;
		//Dictionary::key_type aggr_key_centigrade_;
		
	#endif // not def CODESIZE_EMPTY
};

//Allocator allocator_;
//Allocator& get_allocator() { return allocator_; }
// --------------------------------------------------------------------------
wiselib::WiselibApplication<Os, ExampleApplication> example_app;
// --------------------------------------------------------------------------
void application_main( Os::AppMainParameter& value )
{
	#if !defined(CODESIZE_EMPTY)
		example_app.init( value );
	#endif // CODESIZE_EMPTY
}
