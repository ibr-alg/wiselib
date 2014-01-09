

#include "platform.h"

typedef Os::block_data_t block_data_t;
using namespace wiselib;

#if defined(ARDUINO)
	#include <external_interface/arduino/arduino_monitor.h>
#endif

#include <util/meta.h>
#include <util/debugging.h>
#include <util/tuple_store/tuplestore.h>
#include "tuple.h"
#include <algorithms/hash/sdbm.h>
#include <algorithms/hash/crc16.h>
#include <algorithms/hash/checksum_radio.h>
#include <algorithms/semantic_entities/token_scheduler.h>

//#include "semantics_office1.h"
#include "semantics_uniform.h"
//#include "semantics_simple.h"

typedef Sdbm<Os> Hash;
typedef Crc16<Os> ChecksumHash;

typedef Tuple<Os> TupleT;

#if USE_CHECKSUM_RADIO
	typedef ChecksumRadio<Os, Os::Radio, ChecksumHash> Radio;
#else
	typedef Os::Radio Radio;
#endif
//typedef ChecksumRadio<Os, Os::Radio, ChecksumHash> Radio;


//#define SINK_ID 57

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
#elif USE_VECTOR_CONTAINER
	#include <util/pstl/vector_static.h>
	#include <util/pstl/unique_container.h>
	typedef wiselib::vector_static<Os, TupleT, TUPLE_CONTAINER_SIZE> TupleList;
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
	#warning "++++++ Using NULL dictionary (by fallthrough) ++++++"
	#include <util/tuple_store/null_dictionary.h>
	typedef wiselib::TupleStore<Os, TupleContainer, NullDictionary<Os>, Os::Debug, 0, &TupleT::compare> TS;
#endif
#define USE_DICTIONARY (USE_PRESCILLA_DICTIONARY || USE_TREE_DICTIONARY || USE_BLOCK_DICTIONARY)

typedef wiselib::TokenScheduler<Os, TS, Radio, Os::Timer, Os::Clock, Os::Debug, Os::Rand> TC;

#if USE_INQP
	#warning "Using INQP rule processor"
	#include <algorithms/rdf/inqp/query_processor.h>
	#include <algorithms/semantic_entities/token_construction/inqp_rule_processor.h>
	typedef wiselib::INQPQueryProcessor<Os, TS, Hash, INSE_MAX_QUERIES, INSE_MAX_NEIGHBORS, Dictionary> QueryProcessor;
	typedef QueryProcessor::Value Value;
	typedef wiselib::InqpRuleProcessor<Os, QueryProcessor, TC> RuleProcessor;
	//typedef QueryProcessor::Hash Hash;
	typedef QueryProcessor::Query Query;
	typedef QueryProcessor::GraphPatternSelectionT GPS;
	typedef QueryProcessor::AggregateT AggregateT;
	//typedef QueryProcessor::CollectT Coll;
	typedef QueryProcessor::ConstructionRuleT Cons;
	typedef wiselib::ProjectionInfo<Os> Projection;
	
	//
	// Query Distributor
	// 
	
	#include <algorithms/semantic_entities/token_construction/opportunistic_distributor.h>
	typedef OpportunisticDistributor<Os, TC::GlobalTreeT, TC::NapControlT, TC::SemanticEntityRegistryT, QueryProcessor, Radio> Distributor;
	
	//
	// SE Anycast Radio
	//
	
	#include <algorithms/semantic_entities/token_construction/semantic_entity_anycast_radio.h>
	
	

	#include <algorithms/protocols/packing_radio/packing_radio.h>
	#include <algorithms/semantic_entities/token_construction/row_collector.h>

	typedef SemanticEntityAnycastRadio<Os, TC::SemanticEntityRegistryT,
		TC::SemanticEntityNeighborhoodT, INSE_MESSAGE_TYPE_ROW_ANYCAST, Radio> RowAnycastRadio;
	
	typedef PackingRadio<Os, RowAnycastRadio> PackingAnycastRadio;
	typedef PackingAnycastRadio RowRadio;
	
	#if USE_STRING_INQUIRY
		#include <algorithms/semantic_entities/token_construction/string_inquiry.h>
		
		typedef SemanticEntityAnycastRadio<Os, TC::SemanticEntityRegistryT,
			TC::SemanticEntityNeighborhoodT, INSE_MESSAGE_TYPE_STRING_ANYCAST, Radio> StringAnycastRadio;

		// TODO: add packer here as well?
		typedef StringInquiry<Os, StringAnycastRadio, QueryProcessor> StringInquiryT;	
	#endif
		
	//typedef PackingRadio<Os, Os::Radio> PackingOsRadio;
	//typedef SemanticEntityAnycastRadio<Os, TC::SemanticEntityRegistryT, TC::SemanticEntityNeighborhoodT,
		//INSE_MESSAGE_TYPE_ANYCAST, PackingOsRadio> AnycastPackingRadio;
	//typedef AnycastPackingRadio RowRadio;
	
	typedef RowCollector<Os, RowRadio, QueryProcessor, TC::GlobalTreeT, Os::Debug> RowCollectorT;
	
#else 
	#warning "Using SIMPLE rule processor"
	#include <algorithms/semantic_entities/token_construction/simple_rule_processor.h>
	typedef wiselib::SimpleRuleProcessor<Os, TS, TC, Hash> RuleProcessor;
#endif

#define STRHASH(X) Hash::hash((const block_data_t*)(X), strlen_compiletime(X))

//typedef Os::Clock::time_t time_t;
typedef ::uint32_t abs_millis_t;

#if INSE_USE_AGGREGATOR
	typedef TC::SemanticEntityAggregatorT Aggregator;
#endif

