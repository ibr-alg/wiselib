
#include "platform.h"

typedef Os::block_data_t block_data_t;
using namespace wiselib;

#if defined(ARDUINO)
	#include <external_interface/arduino/arduino_monitor.h>
#endif


#if !defined(CODESIZE_EMPTY)

	#include <util/meta.h>
	#include <util/debugging.h>
	#include <util/tuple_store/tuplestore.h>
	#include "tuple.h"

	#include <algorithms/hash/sdbm.h>
	typedef Sdbm<Os> Hash;

	#include <algorithms/semantic_entities/token_construction/token_scheduler.h>
	#include <algorithms/semantic_entities/token_construction/semantic_entity_id.h>

	//#include "semantics_office1.h"
	//#include "semantics_uniform.h"
	#include "semantics_simple.h"

	typedef Tuple<Os> TupleT;
	
	//
	// Tuple Container
	// 
	
	#if (USE_BLOCK_CONTAINER || USE_BLOCK_DICTIONARY)
		#include <algorithms/block_memory/bitmap_chunk_allocator.h>
		#include <algorithms/block_memory/cached_block_memory.h>
		typedef CachedBlockMemory<Os, Os::BlockMemory, BLOCK_CACHE_SIZE, BLOCK_CACHE_SPECIAL, BLOCK_CACHE_WRITE_THROUGH> BlockMemory;
		typedef BitmapChunkAllocator<Os, BlockMemory, BLOCK_CHUNK_SIZE, BLOCK_CHUNK_ADDRESS_TYPE> BlockAllocator;
	#endif
	
	#if USE_LIST_CONTAINER
		#include <util/pstl/list_dynamic.h>
		#include <util/pstl/unique_container.h>
		typedef wiselib::list_dynamic<Os, TupleT> TupleList;
		typedef wiselib::UniqueContainer<TupleList> TupleContainer;
	#elif USE_BLOCK_CONTAINER
		#include <algorithms/block_memory/b_plus_hash_set.h>
		typedef BPlusHashSet<Os, BlockAllocator, Hash, TupleT, true> TupleContainer;
	#endif
		
	//
	// Dictionary
	// 
		
	#if USE_PRESCILLA_DICTIONARY
		#warning "Using PRESCILLA dictionary"
		#include <util/tuple_store/prescilla_dictionary.h>
		typedef wiselib::PrescillaDictionary<Os> Dictionary;
		typedef wiselib::TupleStore<Os, TupleContainer, Dictionary, Os::Debug, BIN(111), &TupleT::compare> TS;
	#elif USE_TREE_DICTIONARY
		#warning "Using TREE dictionary"
		#include <util/pstl/unbalanced_tree_dictionary.h>
		typedef wiselib::UnbalancedTreeDictionary<Os> Dictionary;
		typedef wiselib::TupleStore<Os, TupleContainer, Dictionary, Os::Debug, BIN(111), &TupleT::compare> TS;
	#elif USE_BLOCK_DICTIONARY
		#warning "Using BLOCK dictionary"
		#include <algorithms/block_memory/b_plus_dictionary.h>
		typedef BPlusDictionary<Os, BlockAllocator, Hash> Dictionary;
		typedef wiselib::TupleStore<Os, TupleContainer, Dictionary, Os::Debug, BIN(111), &TupleT::compare> TS;
	#else
		#warning "Using NULL dictionary"
		#include <util/tuple_store/null_dictionary.h>
		typedef wiselib::TupleStore<Os, TupleContainer, NullDictionary<Os>, Os::Debug, 0, &TupleT::compare> TS;
	#endif
	#define USE_DICTIONARY (USE_PRESCILLA_DICTIONARY || USE_TREE_DICTIONARY || USE_BLOCK_DICTIONARY)

	typedef wiselib::TokenScheduler<Os, TS, Os::Radio, Os::Timer, Os::Clock, Os::Debug, Os::Rand> TC;

	#if USE_INQP
		#warning "Using INQP rule processor"
		#include <algorithms/rdf/inqp/query_processor.h>
		#include <algorithms/rdf/inqp/communicator.h>
		#include <algorithms/semantic_entities/token_construction/inqp_rule_processor.h>
		typedef wiselib::INQPQueryProcessor<Os, TS, Hash, Dictionary> QueryProcessor;
		typedef wiselib::InqpRuleProcessor<Os, QueryProcessor, TC> RuleProcessor;
		//typedef QueryProcessor::Hash Hash;
		typedef QueryProcessor::Query Query;
		typedef QueryProcessor::GraphPatternSelectionT GPS;
		typedef QueryProcessor::CollectT Coll;
		typedef wiselib::ProjectionInfo<Os> Projection;
	#else 
		#warning "Using SIMPLE rule processor"
		#include <algorithms/semantic_entities/token_construction/simple_rule_processor.h>
		typedef wiselib::SimpleRuleProcessor<Os, TS, TC, Hash> RuleProcessor;
	#endif
#endif // not def CODESIZE_EMPTY


#define STRHASH(X) Hash::hash((const block_data_t*)(X), strlen_compiletime(X))


//static_print(sizeof(void*));

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
			pinMode(8, OUTPUT);
			pinMode(9, OUTPUT);
			pinMode(10, OUTPUT);
			pinMode(13, OUTPUT);
			
			
			radio_ = &wiselib::FacetProvider<Os, Os::Radio>::get_facet( value );
			timer_ = &wiselib::FacetProvider<Os, Os::Timer>::get_facet( value );
			debug_ = &wiselib::FacetProvider<Os, Os::Debug>::get_facet( value );
			clock_ = &wiselib::FacetProvider<Os, Os::Clock>::get_facet( value );
			rand_ = &wiselib::FacetProvider<Os, Os::Rand>::get_facet(value);
			
			debug_->debug("\nboot");
			/*
			debug_->debug("free %d sizes: TC %d EP %d ND %d Fwd %d",
					monitor_.free(),
					(int)sizeof(TC),
					(int)sizeof(TC::ReliableTransportT::Endpoint),
					(int)sizeof(TC::SemanticEntityNeighborhoodT),
					(int)sizeof(TC::SemanticEntityForwardingT));
			debug_->debug("TC::Aggr %d TC::Reg %d TC::Trans %d TC::Tree %d TC::Nap %d",
					(int)sizeof(TC::SemanticEntityAggregatorT),
					(int)sizeof(TC::SemanticEntityRegistryT),
					(int)sizeof(TC::ReliableTransportT),
					(int)sizeof(TC::GlobalTreeT),
					(int)sizeof(TC::NapControlT));
			debug_->debug(" Dct %d Cont %d TS %d RuleProc %d SE %d",
					(int)sizeof(Dictionary),
					(int)sizeof(TupleContainer), (int)sizeof(TS), (int)sizeof(RuleProcessor),
					(int)sizeof(TC::SemanticEntityT));
			
			debug_->debug("SE::TokFwd x SE::Evt %d SE::Tok %d SE::id %d",
					//(int)sizeof(TC::SemanticEntityT::TokenForwards),
					(int)sizeof(TC::SemanticEntityT::RegularEventT),
					(int)sizeof(TC::SemanticEntityT::TokenState),
					(int)sizeof(SemanticEntityId));
			debug_->debug("Tree:NeighEntr %d Tree::Neighs %d",
					(int)sizeof(TC::GlobalTreeT::NeighborEntries),
					(int)sizeof(TC::GlobalTreeT::Neighbors));
			
			debug_->debug("radio %d timer %d+ debug %d clock %d rand %d",
					(int)sizeof(Os::Radio),
					(int)sizeof(Os::Timer), //(int)sizeof(arduino_queue),
					(int)sizeof(Os::Debug),
					(int)sizeof(Os::Clock),
					(int)sizeof(Os::Rand));
			
			debug_->debug("App %d treestate %d treestatemsg %d", (int)sizeof(ExampleApplication),
					(int)sizeof(TC::GlobalTreeT::TreeStateT),
					(int)sizeof(TC::GlobalTreeT::TreeStateMessageT));
			*/
			
			#if ARDUINO
			radio_->set_pins(11, 12);
			#endif
			
			monitor_.init(debug_);
			
			radio_->enable_radio();
			
			//debug_->debug( "Hello World from Example Application! my id=%d\n", (int)radio_->id());
			#if USE_BLOCK_DICTIONARY || USE_BLOCK_CONTAINER
				block_memory_.physical().init();
				block_memory_.init();
				block_allocator_.init(&block_memory_, debug_);
			#endif
			
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
				
			monitor_.report("tc init");
			
			token_construction_.init(&ts, radio_, timer_, clock_, debug_, rand_);
			monitor_.report("tc i2");
			token_construction_.set_end_activity_callback(
				TC::end_activity_callback_t::from_method<ExampleApplication, &ExampleApplication::on_end_activity>(this)
			);
			
			monitor_.report("rp init");
			
			#if USE_INQP
				query_processor_.init(&ts, timer_);
				rule_processor_.init(&query_processor_, &token_construction_);
				
				//query_communicator_.init(query_processor_,
			#else
				rule_processor_.init(&ts, &token_construction_);
			#endif
				
			monitor_.report("rdf");
			
			// Insert some URIs we need for generating aggregation info into
			// the dictionary
			
			aggr_key_temp_ = dictionary.insert((::uint8_t*)"<http://spitfire-project.eu/property/Temperature>");
			aggr_key_centigrade_ = dictionary.insert((::uint8_t*)"<http://spitfire-project.eu/uom/Centigrade>");
			
			//debug_->debug("node %d // temp=%8lx centigrate=%8lx", (int)radio_->id(),
					//(long)aggr_key_temp_, (long)aggr_key_centigrade_);
			
			// end aggregation URIs
			
			
			monitor_.report("rdf2");
			initial_semantics(ts, radio_->id()); // included from semantics_XXX.h
			
			monitor_.report("rules");
			
			create_rules();
			rule_processor_.execute_all();
			
			
			monitor_.report("init end");
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
			if(radio_->id() == 0) { //se.root()) {
				//debug_->debug("node %d // aggr setting totals", radio_->id());
				aggregator.set_totals(se.id());
			}
			
			//debug_->debug("node %d // aggr local value", radio_->id());
			aggregator.aggregate(se.id(), aggr_key_temp_, aggr_key_centigrade_, (radio_->id() + 1) * 10, Aggregator::INTEGER);
			
			//debug_->debug("node %d // aggr begin list", (int)radio_->id());
			for(Aggregator::iterator iter = aggregator.begin(); iter != aggregator.end(); ++iter) {
				/*
				debug_->debug("node %d // aggr SE %2d.%08lx type %8lx uom %8lx datatype %d => current n %2d %2d/%2d/%2d total n %2d %2d/%2d/%2d",
						(int)radio_->id(), (int)se.id().rule(), (long)se.id().value(),
						(long)iter->first.type_key(), (long)iter->first.uom_key(), (long)iter->first.datatype(),
						(int)iter->second.count(), (int)iter->second.min(), (int)iter->second.max(), (int)iter->second.mean(),
						(int)iter->second.total_count(), (int)iter->second.total_min(), (int)iter->second.total_max(), (int)iter->second.total_mean());
				*/
			}
			//debug_->debug("node %d // aggr end list", (int)radio_->id());
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
		
		#if defined(ARDUINO)
			ArduinoMonitor<Os, Os::Debug> monitor_;
		#else
			NullMonitor monitor_;
		#endif
		
		TC token_construction_;
		RuleProcessor rule_processor_;
		
		#if USE_INQP
			QueryProcessor query_processor_;
			INQPCommunicator<Os, QueryProcessor> query_communicator_;
		#endif
			
		#if USE_DICTIONARY
			Dictionary::key_type aggr_key_temp_;
			Dictionary::key_type aggr_key_centigrade_;
		#endif
			
		#if USE_BLOCK_DICTIONARY || USE_BLOCK_CONTAINER
			BlockMemory block_memory_;
			BlockAllocator block_allocator_;
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
