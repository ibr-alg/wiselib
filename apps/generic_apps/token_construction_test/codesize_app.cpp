
#include "platform.h"

typedef Os::block_data_t block_data_t;
using namespace wiselib;

#if !defined(CODESIZE_EMPTY)

	#include <util/pstl/string_utils.h>
	#include <util/meta.h>
	#include <util/debugging.h>
	#include <util/pstl/map_static_vector.h>
	#include <util/pstl/priority_queue_dynamic.h>
	#include <util/pstl/list_dynamic.h>
	#include <util/pstl/unique_container.h>
	#include <util/tuple_store/tuplestore.h>
	#include "tuple.h"

	#include <algorithms/routing/flooding_nd/flooding_nd.h>
	#include <algorithms/protocols/packing_radio/packing_radio.h>
	//#include <algorithms/routing/tree_routing_ndis/tree_routing_ndis.h>
	#include <algorithms/routing/forward_on_directed_nd/forward_on_directed_nd.h>

	#include <algorithms/hash/fnv.h>
	typedef Fnv32<Os> Hash;

	#include <algorithms/semantic_entities/token_construction/token_construction.h>
	#include <algorithms/semantic_entities/token_construction/semantic_entity_id.h>


	//#include "semantics_office1.h"
	#include "semantics_uniform.h"

	typedef Tuple<Os> TupleT;
	typedef wiselib::list_dynamic<Os, TupleT> TupleList;
	typedef wiselib::UniqueContainer<TupleList> TupleContainer;
	
	#if USE_PRESCILLA
		#include <util/tuple_store/prescilla_dictionary.h>
		typedef wiselib::PrescillaDictionary<Os> Dictionary;
		typedef wiselib::TupleStore<Os, TupleContainer, Dictionary, Os::Debug, BIN(111), &TupleT::compare> TS;
	#elif USE_TREEDICT
		#include <util/pstl/unbalanced_tree_dictionary.h>
		typedef wiselib::UnbalancedTreeDictionary<Os> Dictionary;
		typedef wiselib::TupleStore<Os, TupleContainer, Dictionary, Os::Debug, BIN(111), &TupleT::compare> TS;
	#else
		#include <util/tuple_store/null_dictionary.h>
		typedef wiselib::TupleStore<Os, TupleContainer, NullDictionary<Os>, Os::Debug, 0, &TupleT::compare> TS;
	#endif

	typedef wiselib::TokenConstruction<Os, TS, Os::Radio, Os::Timer> TC;


	#if USE_INQP
		#include <algorithms/rdf/inqp/query_processor.h>
		#include <algorithms/semantic_entities/token_construction/inqp_rule_processor.h>
		typedef wiselib::INQPQueryProcessor<Os, TS> QueryProcessor;
		typedef wiselib::InqpRuleProcessor<Os, QueryProcessor, TC> RuleProcessor;
		//typedef QueryProcessor::Hash Hash;
		typedef QueryProcessor::Query Query;
		typedef QueryProcessor::GraphPatternSelectionT GPS;
		typedef QueryProcessor::CollectT Coll;
		typedef wiselib::ProjectionInfo<Os> Projection;
	#else 
		#include <algorithms/semantic_entities/token_construction/simple_rule_processor.h>
		typedef wiselib::SimpleRuleProcessor<Os, TS, TC, Hash> RuleProcessor;
	#endif
#endif // not def CODESIZE_EMPTY


#define STRHASH(X) Hash::hash((const block_data_t*)(X), strlen_compiletime(X))


class ExampleApplication
{
	public:
		
		void init( Os::AppMainParameter& value )
		{
			radio_ = &wiselib::FacetProvider<Os, Os::Radio>::get_facet( value );
			timer_ = &wiselib::FacetProvider<Os, Os::Timer>::get_facet( value );
			debug_ = &wiselib::FacetProvider<Os, Os::Debug>::get_facet( value );
			clock_ = &wiselib::FacetProvider<Os, Os::Clock>::get_facet( value );
			rand_ = &wiselib::FacetProvider<Os, Os::Rand>::get_facet(value);
			
	#if !defined(CODESIZE_EMPTY)
			radio_->enable_radio();
			
			#if USE_DICTIONARY
				#if USE_PRESCILLA
					dictionary.init(debug_);
				#elif USE_TREEDICT
					dictionary.init();
				#endif
				ts.init(&dictionary, &container, debug_);
			#else
				ts.init(0, &container, debug_);
			#endif
			token_construction_.init(radio_, timer_, clock_, debug_, rand_, &ts);
			token_construction_.set_end_activity_callback(
				TC::end_activity_callback_t::from_method<ExampleApplication, &ExampleApplication::on_end_activity>(this)
			);
			
			#if USE_INQP
				query_processor_.init(&ts, timer_);
				rule_processor_.init(&query_processor_, &token_construction_);
			#else
				rule_processor_.init(&ts, &token_construction_);
			#endif
			//initial_semantics(ts, radio_->id()); // included from semantics_XXX.h
			create_rules();
			rule_processor_.execute_all();
	#endif
		}
		
	#if !defined(CODESIZE_EMPTY)
		#if USE_INQP
			Query q1;
			Coll collect1;
			GPS gps1;
			
			void create_rules() {
				/*
				 * Add rule 1: (* <...#featureOfInterest> X)
				 */
				::uint8_t qid = 1;
				q1.init(&query_processor_, qid);
				q1.set_expected_operators(2);
				
				collect1.init(
						&q1, 2 /* opid */, 0 /* parent */, 0 /* parent port */,
						Projection((long)BIN(11))
						);
				q1.add_operator(&collect1);
				
				gps1.init(
						&q1, 1 /* op id */, 2 /* parent */, 0 /* parent port */,
						Projection((long)BIN(110000)),
						false, true, false,
						0, STRHASH("x"), 0
						);
				q1.add_operator<GPS>(&gps1);
				
				rule_processor_.add_rule(qid, &q1);
			}
		#else
			void create_rules() {
				/*
				 * Add rule 1: (* <...#featureOfInterest> X)
				 */
				::uint8_t qid = 1;
				const char *p = "x";
				TupleT t;
				t.set(1, (block_data_t*)p);
				rule_processor_.add_rule(qid, t, BIN(010), 2);
			}
		#endif
		
		typedef TC::SemanticEntityAggregatorT Aggregator;
		
		void on_end_activity(TC::SemanticEntityT& se, Aggregator& aggregator) {
			if(radio_->id() == se.root()) { aggregator.set_totals(se.id()); }
			aggregator.aggregate(se.id(), 123, 456, 10, Aggregator::INTEGER);
		}
		
		#if USE_DICTIONARY
			Dictionary dictionary;
		#endif
		TupleContainer container;
		TS ts;
	#endif
		
		
	private:
		Os::Radio::self_pointer_t radio_;
		Os::Timer::self_pointer_t timer_;
		Os::Debug::self_pointer_t debug_;
		Os::Clock::self_pointer_t clock_;
		Os::Rand::self_pointer_t rand_;
		
	#if !defined(CODESIZE_EMPTY)
		TC token_construction_;
		RuleProcessor rule_processor_;
		
		#if USE_INQP
			QueryProcessor query_processor_;
		#endif
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
