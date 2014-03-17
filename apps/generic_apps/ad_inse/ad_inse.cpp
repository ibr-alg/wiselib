/*
 * Simple Neighbor Discovery Example
 */
//#define ISENSE_RADIO_ADDR_TYPE

#include "external_interface/external_interface_testing.h"
//#include "external_interface/external_interface.h"
#include "algorithms/neighbor_discovery/adaptive2/echo.h"
#include "util/delegates/delegate.hpp"
#include "util/pstl/vector_static.h"
#include "util/pstl/pair.h"
#include "util/serialization/serialization.h"

//#define ECHO_BENCHMARK

typedef wiselib::OSMODEL Os;
typedef Os::ExtendedRadio Radio;
typedef Radio::block_data_t block_data_t;

typedef wiselib::Echo<Os, Radio, Os::Timer, Os::Debug> nb_t;
typedef Os::Radio::node_id_t node_id_t;
typedef Os::Uart::size_t uart_size_t;

class ExampleApplication {
public:

    void init(Os::AppMainParameter& value) {

	radio_ = &wiselib::FacetProvider<Os, Radio>::get_facet(value);
	timer_ = &wiselib::FacetProvider<Os, Os::Timer>::get_facet(value);
	debug_ = &wiselib::FacetProvider<Os, Os::Debug>::get_facet(value);
	clock_ = &wiselib::FacetProvider<Os, Os::Clock>::get_facet(value);
	rand_ = &wiselib::FacetProvider<Os, Os::Rand>::get_facet(value);
	uart_msg_handler_ = &wiselib::FacetProvider<Os, Os::Uart>::get_facet(value);

	//radio_->hardware_radio().set_channel(12);

	debug_->debug("Hello World from Neighbor Discovery Test Application!\n");

	neighbor_discovery.init(*radio_, *clock_, *timer_, *debug_, 200, 20000, 50000, 50000);
	enabled = false;
	disable = false;
	rand_->srand(radio_->id());

	uint8_t flags = nb_t::NEW_NB | nb_t::NEW_NB_BIDI | nb_t::DROPPED_NB | nb_t::NEW_PAYLOAD_BIDI | nb_t::LOST_NB_BIDI;
	//neighbor_discovery.register_debug_callback(0);

	neighbor_discovery.reg_event_callback<ExampleApplication, &ExampleApplication::callback>(6, flags, this);

	neighbor_discovery.enable();

	
#ifdef SHAWN
	//        start(0);
#endif
	
	start(0);
    }

    // --------------------------------------------------------------------

    /*
     * The callback function that is called by the the neighbor discovery
     * module when a event is generated. The arguments are: the event ID,
     * the node ID that generated the event, the len of the payload ( 0 if
     * this is not a NEW_PAYLOAD event ), the piggybacked payload data.
     */
    void callback(uint8_t event, node_id_t from, uint8_t len, uint8_t* data) {
	if (nb_t::NEW_NB == event) {
	    //new single direction neighbor
	    debug_->debug("NB;%x;%x", from, radio_->id());
	} else if (nb_t::NEW_NB_BIDI == event) {
	    //new bidi neighbor
	    debug_->debug("NBB;%x;%x", from, radio_->id());
	} else if (nb_t::DROPPED_NB == event) {
	    //dropped nb
	    debug_->debug("NBD;%x;%x", from, radio_->id());
	} else if (nb_t::LOST_NB_BIDI == event) {
	    //lost nb 
	    debug_->debug("NBL;%x;%x", from, radio_->id());
	}
    }

    void stop(void*) {
	neighbor_discovery.disable();
    }

    // --------------------------------------------------------------------

    void start(void* ) {

	if (disable) {
	    neighbor_discovery.disable();
	    return;
	}

/*
	debug_->debug("EVENT=NB_PRINT_INFO;NODE=%x;Time=%d;NB_SIZE=%d;NB_BIDI_SIZE=%d",
		radio_->id(),
		clock_->seconds(clock_->time()) + delay,
		neighbor_discovery.stable_nb_size(),
		neighbor_discovery.bidi_nb_size()
		);
*/
	if (interval%10==0){
		neighbor_discovery.leave_token_phase();
		neighbor_discovery.enter_sync_phase();
	}
	else if (interval%10==1){
		neighbor_discovery.leave_sync_phase();
		neighbor_discovery.enter_token_phase();
	}
	else{
		//nothing to do here yet
	}
	//move to next interval
	interval++;
	
	//        run this periodically
	timer_->set_timer<ExampleApplication,
		&ExampleApplication::start>(1000, this, 0);
    }


private:
    bool enabled, disable;
    int interval;
    uint8_t delay;
    nb_t neighbor_discovery;
    Os os_;
    Radio::self_pointer_t radio_;
    Os::Timer::self_pointer_t timer_;
    Os::Debug::self_pointer_t debug_;
    Os::Clock::self_pointer_t clock_;
    Os::Rand::self_pointer_t rand_;
    Os::Uart::self_pointer_t uart_msg_handler_;
};
// --------------------------------------------------------------------------
wiselib::WiselibApplication<Os, ExampleApplication> example_app;
// --------------------------------------------------------------------------

void application_main(Os::AppMainParameter& value) {
    example_app.init(value);
}
