
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
typedef SsMbfTree<Os, Neighborhood> Tree;
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
			token_ring_.init(neighborhood_, tree_, *debug_);
			scheduler_.init(neighborhood_, tree_, token_ring_, *clock_, *timer_, *debug_);


			scenario_01();

			//timer_->set_timer<ExampleApplication,
									//&ExampleApplication::fake_beacon>(100, this, 0);
		}

		// fake a new neighbor event
		void mock_new_neighbor(::uint16_t id, ::uint16_t distance, ::uint16_t parent) {
			Tree::TreeMessageT tree_message;
			tree_message.set_distance(distance);
			tree_message.set_parent(parent);
			neighborhood_.mock_insert_event(Neighborhood::NEW_NB_BIDI, Tree::PAYLOAD_ID, id, tree_message.size(), tree_message.data());
			neighborhood_.mock_insert_event(Neighborhood::NEW_PAYLOAD_BIDI, Tree::PAYLOAD_ID, id, tree_message.size(), tree_message.data());
		}

		void mock_parent_beacon(void*) {
			debug_->debug("AP:parent beacon");
			Tree::TreeMessageT tree_message;
			tree_message.set_distance(8);
			tree_message.set_parent(101);
			//neighborhood_.mock_insert_event(Neighborhood::NEW_NB_BIDI, Tree::PAYLOAD_ID, id, tree_message.size(), tree_message.data());
			neighborhood_.mock_insert_event(Neighborhood::NEW_PAYLOAD_BIDI, Tree::PAYLOAD_ID, 101, tree_message.size(), tree_message.data());
			//if(is_sync_beacon) {
				neighborhood_.mock_insert_event(Neighborhood::NEW_SYNC_PAYLOAD, Tree::PAYLOAD_ID, 101, tree_message.size(), tree_message.data());
			//}

			timer_->set_timer<ExampleApplication, &ExampleApplication::mock_parent_beacon>(10000, this, 0);
		}

		void scenario_01() {
			// Nodes:
			// 1 = this
			// 100, 101, 102,... = potential parents 
			// 10, 11, 12,... = potential childs

			mock_new_neighbor(100, 10, 2100);
			mock_new_neighbor(101, 8, 2101); // should become selected parent
			mock_new_neighbor(102, 22, 2102);

			mock_new_neighbor(10, -1, Radio::NULL_NODE_ID); // no parent yet
			mock_new_neighbor(11, -1, Radio::NULL_NODE_ID); // no parent yet
			mock_new_neighbor(12, 10, 2199); // parent 2199, dist 10

			mock_parent_beacon(0);
		}

		//void fake_beacon(void*) {
			//neighborhood_.mock_insert_event

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
