
#include "platform.h"

typedef Os::block_data_t block_data_t;
using namespace wiselib;

//#include <util/allocators/malloc_free_allocator.h>
//typedef MallocFreeAllocator<Os> Allocator;
//Allocator& get_allocator();


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
	#else
		#include <util/tuple_store/null_dictionary.h>
		typedef wiselib::TupleStore<Os, TupleContainer, NullDictionary<Os>, Os::Debug, 0, &TupleT::compare> TS;
	#endif

	typedef wiselib::TokenConstruction<Os, TS, Os::Radio, Os::Timer> TC;


	#if USE_INQP
		#include <algorithms/rdf/inqp/query_processor.h>
		#include <algorithms/semantic_entities/token_construction/inqp_rule_processor.h>
		typedef wiselib::InqpRuleProcessor<Os, QueryProcessor, TC> RuleProcessor;
		typedef wiselib::INQPQueryProcessor<Os, TS> QueryProcessor;
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
		
	#if !defined(CODESIZE_EMPTY)
		
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
			rand_ = &wiselib::FacetProvider<Os, Os::Rand>::get_facet(value);
			
			radio_->enable_radio();
			
			debug_->debug( "Hello World from Example Application! my id=%d app=%p\n", radio_->id(), this );
			
			#if USE_DICTIONARY
				dictionary.init(debug_);
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
				
			
			// Insert some URIs we need for generating aggregation info into
			// the dictionary
			
			aggr_key_temp_ = dictionary.insert((uint8_t*)"<http://spitfire-project.eu/property/Temperature>");
			aggr_key_centigrade_ = dictionary.insert((uint8_t*)"<http://spitfire-project.eu/uom/Centigrade>");
			
			// end aggregation URIs
			
			
			initial_semantics(ts, radio_->id()); // included from semantics_XXX.h
			create_rules();
			rule_processor_.execute_all();
		}
		
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
						0, STRHASH("<http://purl.oclc.org/NET/ssnx/ssn#featureOfInterest>"), 0
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
				const char *p = "<http://purl.oclc.org/NET/ssnx/ssn#featureOfInterest>";
				TupleT t;
				t.set(1, (block_data_t*)p);
				rule_processor_.add_rule(qid, t, BIN(010), 2);
			}
		#endif
		
		typedef TC::SemanticEntityAggregatorT Aggregator;
		
		void on_end_activity(TC::SemanticEntityT& se, Aggregator& aggregator) {
			if(radio_->id() == se.root()) {
				debug_->debug("node %d // aggr setting totals", radio_->id());
				aggregator.set_totals(se.id());
			}
			
			debug_->debug("node %d // aggr local value", radio_->id());
			aggregator.aggregate(se.id(), aggr_key_temp_, aggr_key_centigrade_, (radio_->id() + 1) * 10, Aggregator::INTEGER);
			
			for(Aggregator::iterator iter = aggregator.begin(); iter != aggregator.end(); ++iter) {
				debug_->debug("node %d // aggr SE %2d.%08lx type %8lx uom %8lx datatype %d => current n %2d %2d/%2d/%2d total n %2d %2d/%2d/%2d",
						(int)radio_->id(), (int)se.id().rule(), (long)se.id().value(),
						(long)iter->first.type_key(), (long)iter->first.uom_key(), (long)iter->first.datatype(),
						(int)iter->second.count(), (int)iter->second.min(), (int)iter->second.max(), (int)iter->second.mean(),
						(int)iter->second.total_count(), (int)iter->second.total_min(), (int)iter->second.total_max(), (int)iter->second.total_mean());
			}
		}
		
		#if USE_DICTIONARY
			Dictionary dictionary;
		#endif
		TupleContainer container;
		TS ts;
		
		
	private:
		Os::Radio::self_pointer_t radio_;
		Os::Timer::self_pointer_t timer_;
		Os::Debug::self_pointer_t debug_;
		Os::Clock::self_pointer_t clock_;
		Os::Rand::self_pointer_t rand_;
		
		TC token_construction_;
		RuleProcessor rule_processor_;
		
		#if USE_INQP
			QueryProcessor query_processor_;
		#endif
			
		#if USE_DICTIONARY
			Dictionary::key_type aggr_key_temp_;
			Dictionary::key_type aggr_key_centigrade_;
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
