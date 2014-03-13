
#include <external_interface/external_interface.h>
#include <algorithms/semantic_entities/ss_mbf_tree.h>
#include <algorithms/neighbor_discovery/mock_neighborhood.h>
#include <external_interface/local_radio.h>
#include <algorithms/semantic_entities/se_scheduler.h>
#include <algorithms/semantic_entities/se_nd_token_ring.h>

using namespace wiselib;
typedef OSMODEL Os;

typedef LocalRadio<Os> Radio;

typedef MockNeighborhood<Os> Neighborhood;
typedef SsMbfTree<Os, Neighborhood, Radio> Tree;
typedef SeNdTokenRing<Os, Neighborhood, Tree> TokenRing;
typedef SeScheduler<Os, Neighborhood, Tree, TokenRing> Scheduler;

class ExampleApplication {
	public:
		Radio r;

		void init(Os::AppMainParameter& value) {
			//radio_ = &wiselib::FacetProvider<Os, Os::Radio>::get_facet( value );
			radio_ = &r;
			timer_ = &wiselib::FacetProvider<Os, Os::Timer>::get_facet( value );
			clock_ = &wiselib::FacetProvider<Os, Os::Clock>::get_facet(value);
			debug_ = &wiselib::FacetProvider<Os, Os::Debug>::get_facet( value );

			radio_->enable_radio();

			neighborhood_.init(debug_);
			tree_.init(radio_->id(), &neighborhood_, debug_);
			token_ring_.init(neighborhood_, tree_);
			scheduler_.init(neighborhood_, tree_, token_ring_, *clock_, *timer_, *debug_);

			//timer_->set_timer<ExampleApplication,
									//&ExampleApplication::start>( 5000, this, 0 );
		}
		void receive_radio_message(Os::Radio::node_id_t from, Os::Radio::size_t len, Os::Radio::block_data_t *buf) {
		}

	private:
		Neighborhood neighborhood_;
		Tree tree_;
		TokenRing token_ring_;
		Scheduler scheduler_;

		Radio::self_pointer_t radio_;
		Os::Timer::self_pointer_t timer_;
		Os::Debug::self_pointer_t debug_;
		Os::Clock::self_pointer_t clock_;
};
// --------------------------------------------------------------------------
wiselib::WiselibApplication<Os, ExampleApplication> example_app;
// --------------------------------------------------------------------------
void application_main( Os::AppMainParameter& value )
{
  example_app.init( value );
}
