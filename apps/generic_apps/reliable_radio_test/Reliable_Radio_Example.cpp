/**
 * Reliable Radio Benchmark Application
 * defines a source and a destination address
 * The source sends TOTAL messages to the destination
 * At a rate of 1 message every INTERVAL milis
 * Destination Shows the total # of messages received
 * And source the # of messages sent
 */
#include "external_interface/external_interface_testing.h"
#include "radio/reliable/reliableradio.h"

typedef wiselib::OSMODEL Os;
typedef wiselib::ReliableRadio<Os, Os::Radio, Os::Timer, Os::Debug> reliable_radio_t;

typedef Os::Radio::node_id_t node_id_t;
typedef Os::Radio::block_data_t block_data_t;

#define INTERVAL 15
#define TOTAL 30000

class ReliableRadioExample {
    typedef reliable_radio_t::size_t size_t;

public:

    void init(Os::AppMainParameter& value) {
        debug_ = &wiselib::FacetProvider<Os, Os::Debug>::get_facet(value);
        debug_->debug("Hello World from Example Application!");


        radio_ = &wiselib::FacetProvider<Os, Os::Radio>::get_facet(value);
        timer_ = &wiselib::FacetProvider<Os, Os::Timer>::get_facet(value);
        radio_->enable_radio();

        reliable_radio_.init(*radio_, *timer_, *debug_);
        reliable_radio_.enable_radio();

#ifdef SHAWN
        source = 0;
        destination = 2;
#else
        source = 0x296;
        destination = 0xcaa;
#endif

        len = 2;

        data[0] = 4;
        data[1] = 2;

        count = 0;
        count_normal = 0;
        count_reliable = 0;


        radio_->reg_recv_callback<ReliableRadioExample,
                &ReliableRadioExample::receive_callback > (this);
        reliable_radio_.reg_recv_callback<ReliableRadioExample,
                &ReliableRadioExample::receive_callback > (this);

        if (radio_->id() == source) {
            timer_->set_timer<ReliableRadioExample,
                    &ReliableRadioExample::send_normal > (INTERVAL, this, 0);
        }
        if (radio_->id() == destination) {
            timer_->set_timer<ReliableRadioExample,
                    &ReliableRadioExample::result > (2 * (INTERVAL * TOTAL + 2000), this, 0);
        }
    }
    // --------------------------------------------------------------------

    void receive_callback(node_id_t from, size_t len, block_data_t* data) {

        if ((radio_->id() == destination) && (len == 2) && (data[0] == 4) && (data[1] == 2)) {
            ++count_normal;
        }
        if ((radio_->id() == destination) && (len == 2) && (data[0] == 5) && (data[1] == 2)) {
            ++count_reliable;
        }
    }
    // --------------------------------------------------------------------

    void result(void*) {
        debug_->debug("received so far normal %d ", count_normal);
        debug_->debug("received so far reliable %d ", count_reliable);
    }
    // --------------------------------------------------------------------

    void send_reliable(void*) {
        // following can be used for periodic messages to sink
        if (count < TOTAL) {
            reliable_radio_.send(destination, len, data);

            count++;
            if (count % 1000 == 0)
                debug_->debug("Sent-reliable %d", count);
            timer_->set_timer<ReliableRadioExample,
                    &ReliableRadioExample::send_reliable > (INTERVAL, this, 0);
        } else {
            debug_->debug("Totally Sent-reliable %d", count);
        }
    }
    // --------------------------------------------------------------------

    void send_normal(void*) {
        // following can be used for periodic messages to sink
        if (count < TOTAL) {
            radio_->send(destination, len, data);

            count++;

            if (count % 1000 == 0)
                debug_->debug("Sent-normal %d", count);
            timer_->set_timer<ReliableRadioExample,
                    &ReliableRadioExample::send_normal > (INTERVAL, this, 0);
        } else {
            debug_->debug("Totally Sent-normal %d", count);
            data[0] = 5;
            count = 0;
            timer_->set_timer<ReliableRadioExample,
                    &ReliableRadioExample::send_reliable > (INTERVAL, this, 0);
        }
    }
    // --------------------------------------------------------------------

private:
    reliable_radio_t reliable_radio_;
    Os::Radio::self_pointer_t radio_;
    Os::Timer::self_pointer_t timer_;
    Os::Debug::self_pointer_t debug_;
    int count;
    int count_normal;
    int count_reliable;
    node_id_t source, destination;
    size_t len;
    block_data_t data[2];
};
// --------------------------------------------------------------------------
wiselib::WiselibApplication<Os, ReliableRadioExample> example_app;
// --------------------------------------------------------------------------

void application_main(Os::AppMainParameter& value) {
    example_app.init(value);
}

