#include "external_interface/external_interface_testing.h"
#include "util/serialization/simple_types.h"
#include "util/pstl/vector_static.h"
#include "algorithms/topology/atp/ATP.h"
#include "algorithms/neighbor_discovery/neighbor_discovery.h"
#include "algorithms/neighbor_discovery/adaptive/adaptiveMessaging.h"
#include "SCLD_ATP_app_config.h"

typedef wiselib::OSMODEL Os;
typedef Os::TxRadio Radio;
typedef Radio::node_id_t node_id_t;
typedef uint16 CoordinatesNumber;
typedef Radio::ExtendedData ExtendedData;
typedef Os::Debug Debug;
typedef Os::Rand Rand;
typedef Os::Timer Timer;
typedef Os::Clock Clock;
typedef uint16_t TimesNumber;
typedef uint8 SecondsNumber;
typedef uint32 AgentID;
typedef wiselib::NeighborDiscovery_Type<Os, Radio, Clock, Timer, Rand, Debug> NeighborDiscovery;
typedef wiselib::AdaptiveMessaging<Os, Radio, Timer, Debug, Rand, NeighborDiscovery> AdaptiveMessaging_t;
typedef wiselib::ATP_Type<Os, Radio, NeighborDiscovery, Timer, Rand, Clock, Debug/*, AdaptiveMessaging_t*/> ATP;

NeighborDiscovery* neighbor_discovery;
ATP atp;
AdaptiveMessaging_t *adm;

void application_main(Os::AppMainParameter& value) {
    Radio *wiselib_radio_ = &wiselib::FacetProvider<Os, Radio>::get_facet(value);
    Timer *wiselib_timer_ = &wiselib::FacetProvider<Os, Timer>::get_facet(value);
    Debug *wiselib_debug_ = &wiselib::FacetProvider<Os, Debug>::get_facet(value);
    Rand *wiselib_rand_ = &wiselib::FacetProvider<Os, Rand>::get_facet(value);
    Clock *wiselib_clock_ = &wiselib::FacetProvider<Os, Clock>::get_facet(value);
    wiselib_rand_->srand(wiselib_radio_->id());
    wiselib_radio_->set_channel(SCLD_ATP_CHANNEL);
    neighbor_discovery = new NeighborDiscovery();
    //adm = new AdaptiveMessaging_t();
    neighbor_discovery->init(*wiselib_radio_, *wiselib_timer_, *wiselib_debug_, *wiselib_clock_, *wiselib_rand_);
    adm->init(*wiselib_radio_, *wiselib_timer_, *wiselib_debug_, *wiselib_rand_, *neighbor_discovery);
    atp.init(*wiselib_radio_, *wiselib_timer_, *wiselib_debug_, *wiselib_rand_, *wiselib_clock_, *neighbor_discovery/* *adm*/);
    atp.enable();
}
