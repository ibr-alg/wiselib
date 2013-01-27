#include "PLTT_app_config.h"
#include "external_interface/external_interface_testing.h"
#include "util/serialization/simple_types.h"
#include "internal_interface/position/position_new.h"
#include "internal_interface/node/node_new.h"
#include "algorithms/tracking/PLTT_trace_revision.h"
#ifdef PLTT_TARGET
#include "algorithms/tracking/PLTT_target.h"
#endif
#ifdef PLTT_TRACKER
#include "algorithms/tracking/PLTT_agent.h"
#include "algorithms/tracking/PLTT_tracker.h"
#endif
#ifdef PLTT_SECURE
#include "algorithms/tracking/PLTT_secure_trace_revision.h"
#include "algorithms/privacy/privacy.h"
#include "algorithms/privacy/privacy_message.h"
#endif

typedef wiselib::OSMODEL Os;
typedef Os::TxRadio Radio;
typedef uint16 CoordinatesNumber;
typedef uint8 IntensityNumber;
typedef uint32 AgentID;
typedef uint32 Integer;
typedef uint8 TimesNumber;
typedef uint8 SecondsNumber;
typedef Radio::node_id_t node_id_t;
typedef Radio::block_data_t block_data_t;
typedef Radio::size_t r_size_t;
typedef Os::Debug Debug;
typedef Os::Rand Rand;
typedef Os::Timer Timer;
typedef Os::Uart Uart;
typedef Os::Clock Clock;
typedef wiselib::Position2DType<Os, Radio, CoordinatesNumber, Debug> Position;
typedef wiselib::NodeType<Os, Radio, node_id_t, Position, Debug> Node;
#ifdef PLTT_TRACKER
typedef wiselib::PLTT_AgentType<Os, Radio, AgentID, Node, IntensityNumber, Clock, Debug> PLTT_Agent;
typedef wiselib::PLTT_TrackerType<Os, PLTT_Agent, Node, Position, IntensityNumber, Timer, Radio, Rand, Clock, Debug> PLTT_Tracker;
#endif
typedef wiselib::PLTT_TraceType<Os, Radio, uint8, uint8, IntensityNumber, Node, node_id_t, Debug> PLTT_Trace;
#ifdef PLTT_SECURE
typedef wiselib::PLTT_SecureTraceType<Os, Radio, TimesNumber, SecondsNumber, IntensityNumber, Node, node_id_t, Debug> PLTT_SecureTrace;
typedef wiselib::PrivacyMessageType<Os, Radio> PrivacyMessage;
typedef wiselib::vector_static<Os, PrivacyMessage, 100> PrivacyMessageList;
typedef wiselib::PrivacyType<Os, Radio, Timer, Uart, PrivacyMessage, PrivacyMessageList, Debug> Privacy;
typedef wiselib::PLTT_TargetType<Os, PLTT_SecureTrace, Node, Timer, Radio, PrivacyMessage, Clock, Debug> PLTT_Target;
#else
typedef wiselib::PLTT_TargetType<Os, PLTT_Trace, Node, Timer, Radio, Clock, Debug> PLTT_Target;
#endif
#ifdef PLTT_TARGET
#ifdef PLTT_SECURE
PLTT_Target target( PLTT_SecureTrace( TRACE_DIMINISH_SECONDS, TRACE_DIMINISH_AMOUNT, TRACE_SPREAD_PENALTY, TRACE_START_INTENSITY, 0 ), TARGET_SPREAD_MILIS, TARGET_TRANSMISSION_POWER );
#else
PLTT_Target target( PLTT_Trace(TRACE_DIMINISH_SECONDS, TRACE_DIMINISH_AMOUNT, TRACE_SPREAD_PENALTY, TRACE_START_INTENSITY, 0), TARGET_SPREAD_MILIS, TARGET_TRANSMISSION_POWER );
#endif
#endif
#ifdef PLTT_TRACKER
PLTT_Tracker tracker( TARGET_ID_TO_TRACK1, TARGET_TO_TRACK_MAX_INTENSITY1, TRACKER_SEND_MILIS1, TRACKER_TRANSMISSION_POWER1 );
#endif
#ifdef PLTT_SECURE
Privacy privacy;
#endif

void application_main( Os::AppMainParameter& value )
{
	Radio *wiselib_radio_ = &wiselib::FacetProvider<Os, Radio>::get_facet( value );
	Timer *wiselib_timer_ = &wiselib::FacetProvider<Os, Os::Timer>::get_facet( value );
	Debug *wiselib_debug_ = &wiselib::FacetProvider<Os, Os::Debug>::get_facet( value );
	Rand *wiselib_rand_ = &wiselib::FacetProvider<Os, Os::Rand>::get_facet( value );
	Clock *wiselib_clock_ = &wiselib::FacetProvider<Os, Os::Clock>::get_facet( value );
	wiselib_radio_->set_channel(20);
	wiselib_rand_->srand( wiselib_radio_->id() );
#ifdef PLTT_TRACKER
	tracker.init(*wiselib_radio_, *wiselib_timer_, *wiselib_rand_, *wiselib_clock_, *wiselib_debug_);
	tracker.set_self( Node( wiselib_radio_->id(), Position( 0, 0, 0 ) ) );
#ifdef PLTT_TRACKING_METRICS
	tracker.set_metrics_timer( TRACKER_TRACKING_METRICS_TIMER );
#endif
	tracker.enable();
#endif
#ifdef PLTT_TARGET
	target.init( *wiselib_radio_, *wiselib_timer_, *wiselib_clock_, *wiselib_debug_ );
	target.set_self( Node( wiselib_radio_->id(), Position( 0, 0, 0 ) ) );
#ifdef PLTT_TARGET_SPREAD_METRICS
	target.set_metrics_timer( TARGET_SPREAD_METRICS_TIMER );
#endif
#ifdef PLTT_TARGET_MINI_RUN
	target.set_mini_run_times( TARGET_MINI_RUN_TIMES );
#endif
#ifdef PLTT_SECURE
	target.set_request_id( TARGET_SECURE_REQUEST_ID_1 );
	Os::Uart *wiselib_uart_ = &wiselib::FacetProvider<Os, Os::Uart>::get_facet( value );
	privacy.set_randomization();
	privacy.init( *wiselib_radio_, *wiselib_debug_, *wiselib_uart_, *wiselib_timer_ );
	target.reg_privacy_radio_callback<Privacy, &Privacy::radio_receive>( &privacy );
	privacy.reg_privacy_callback<PLTT_Target, &PLTT_Target::randomize_callback>( 999, &target );
	privacy.enable();
#endif
	target.enable();
#endif
}
