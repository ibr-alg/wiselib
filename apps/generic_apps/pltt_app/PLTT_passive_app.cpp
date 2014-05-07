#include "external_interface/external_interface_testing.h"
#include "util/serialization/simple_types.h"
#include "util/pstl/vector_static.h"
#include "internal_interface/position/position_new.h"
#include "internal_interface/node/node_new.h"
#include "algorithms/tracking/PLTT_node.h"
#include "algorithms/tracking/PLTT_node_target.h"
#include "algorithms/tracking/PLTT_trace.h"
#include "algorithms/tracking/PLTT_passive.h"
#include "algorithms/tracking/PLTT_agent.h"
#include "algorithms/neighbor_discovery/neighbor_discovery.h"
#include "radio/reliable/reliable_radio.h"
#include "PLTT_app_config.h"

#ifdef CONFIG_PLTT_SECURE
#include "algorithms/tracking/PLTT_secure_trace.h"
#endif
#ifdef UNIGE_TESTBED
#include "../topologies/UNIGE_ISENSE_topology.h"
#endif
#ifdef CTI_TESTBED
#include "../topologies/CTI_ISENSE_topology.h"
#endif

typedef wiselib::OSMODEL Os;
typedef Os::TxRadio Radio;
typedef Radio::node_id_t node_id_t;
typedef uint16 CoordinatesNumber;
typedef uint8 IntensityNumber;
typedef Radio::ExtendedData ExtendedData;
typedef Os::Debug Debug;
typedef Os::Rand Rand;
typedef Os::Timer Timer;
typedef Os::Clock Clock;
typedef uint8 TimesNumber;
typedef uint8 SecondsNumber;
typedef uint32 Integer;
typedef uint16_t AgentID;
typedef wiselib::NeighborDiscovery_Type<Os, Radio, Clock, Timer, Rand, Debug> NeighborDiscovery;
typedef wiselib::Position2DType<Os, Radio, CoordinatesNumber, Debug> Position;
typedef wiselib::NodeType<Os, Radio, node_id_t, Position, Debug> Node;
typedef wiselib::ReliableRadio_Type<Os, Radio, Clock, Timer, Rand, Debug> ReliableRadio;
#ifdef CONFIG_PLTT_SECURE
typedef wiselib::PLTT_SecureTraceType<Os, Radio, TimesNumber, SecondsNumber, IntensityNumber, Node, node_id_t, Debug> PLTT_SecureTrace;
typedef wiselib::vector_static<Os, PLTT_SecureTrace, PLTT_MAX_SECURE_TRACES_SUPPORTED> PLTT_SecureTraceList;
#endif
typedef wiselib::PLTT_TraceType<Os, Radio, TimesNumber, SecondsNumber, IntensityNumber, Node, node_id_t, Debug> PLTT_Trace;
typedef wiselib::vector_static<Os, PLTT_Trace, PLTT_MAX_TARGETS_SUPPORTED> PLTT_TraceList;
typedef wiselib::PLTT_NodeTargetType<Os, Radio, node_id_t, IntensityNumber, Debug > PLTT_NodeTarget;
typedef wiselib::vector_static<Os, PLTT_NodeTarget, PLTT_MAX_TARGETS_SUPPORTED> PLTT_NodeTargetList;
typedef wiselib::PLTT_NodeType<Os, Radio, Node, PLTT_NodeTarget, PLTT_NodeTargetList, PLTT_TraceList, Debug> PLTT_Node;
typedef wiselib::vector_static<Os, PLTT_Node, PLTT_MAX_NEIGHBORS_SUPPORTED> PLTT_NodeList;
typedef wiselib::PLTT_AgentType< Os, Radio, AgentID, IntensityNumber, Debug> PLTT_Agent;
#ifdef CONFIG_PLTT_SECURE
typedef wiselib::PLTT_PassiveType<Os, Node, PLTT_Node, PLTT_NodeList, PLTT_Trace, PLTT_TraceList, PLTT_SecureTrace, PLTT_SecureTraceList, PLTT_Agent, NeighborDiscovery, Timer, Radio, ReliableRadio, Rand, Clock, Debug> PLTT_Passive;
#else
typedef wiselib::PLTT_PassiveType<Os, Node, PLTT_Node, PLTT_NodeList, PLTT_Trace, PLTT_TraceList, PLTT_Agent, NeighborDiscovery, Timer, Radio, ReliableRadio, Rand, Clock, Debug> PLTT_Passive;
#endif

NeighborDiscovery neighbor_discovery;
PLTT_Passive passive;
ReliableRadio reliable_radio;

void application_main( Os::AppMainParameter& value )
{
	Radio *wiselib_radio_ = &wiselib::FacetProvider<Os, Radio>::get_facet( value );
	Timer *wiselib_timer_ = &wiselib::FacetProvider<Os, Timer>::get_facet( value );
	Debug *wiselib_debug_ = &wiselib::FacetProvider<Os, Debug>::get_facet( value );
	Rand *wiselib_rand_ = &wiselib::FacetProvider<Os, Rand>::get_facet( value );
	Clock *wiselib_clock_ = &wiselib::FacetProvider<Os, Clock>::get_facet( value );
	wiselib_rand_->srand( wiselib_radio_->id() );
	wiselib_radio_->set_channel( PLTT_CHANNEL );
	neighbor_discovery.init( *wiselib_radio_, *wiselib_timer_, *wiselib_debug_, *wiselib_clock_, *wiselib_rand_ );
	reliable_radio.init(  *wiselib_radio_, *wiselib_timer_, *wiselib_debug_, *wiselib_clock_, *wiselib_rand_ );
	passive.init( *wiselib_radio_, reliable_radio, *wiselib_timer_, *wiselib_debug_, *wiselib_rand_, *wiselib_clock_, neighbor_discovery );
	passive.set_self( PLTT_Node( Node( wiselib_radio_->id(), get_node_info<Position, Radio>( wiselib_radio_ ) ) ) );
	passive.enable();
}
