
#include <external_interface/external_interface.h>
#include <algorithms/semantic_entities/ss_mbf_tree.h>
#include <external_interface/local_radio.h>
#include <algorithms/semantic_entities/se_scheduler.h>
#include <algorithms/semantic_entities/se_nd_token_ring.h>

using namespace wiselib;
typedef OSMODEL Os;

#if defined(PC)
	typedef LocalRadio<Os> Radio;
#else
	typedef Os::Radio Radio;
#endif

#if defined(PC)
	#include <algorithms/neighbor_discovery/mock_neighborhood.h>
	typedef MockNeighborhood<Os> Neighborhood;
#else
	#include <algorithms/neighbor_discovery/adaptive2/echo.h>
	typedef Echo<Os, Radio, Os::Timer, Os::Debug> Neighborhood;
#endif

typedef SsMbfTree<Os, Neighborhood> Tree;
typedef SeNdTokenRing<Os, Neighborhood, Tree> TokenRing;
typedef SeScheduler<Os, Neighborhood, Tree, TokenRing> Scheduler;

class ExampleApplication {
	public:
		void init(Os::AppMainParameter& value) {
			#if defined(PC)
				radio_ = &r;
			#else
				radio_ = &wiselib::FacetProvider<Os, Os::Radio>::get_facet( value );
			#endif
			timer_ = &wiselib::FacetProvider<Os, Os::Timer>::get_facet( value );
			clock_ = &wiselib::FacetProvider<Os, Os::Clock>::get_facet(value);
			debug_ = &wiselib::FacetProvider<Os, Os::Debug>::get_facet( value );

			if(radio_->id() == 0) {
				debug_->debug("My id is 0. Thats too weird, ill just sit here and do nothing. And freak everybody out.");
				return;
			}

			radio_->enable_radio();

			#if defined(PC)
				neighborhood_.init(debug_);
			#else
				neighborhood_.init(*radio_, *clock_, *timer_, *debug_, 200, 20000, 50000, 50000);
				neighborhood_.enable();
			#endif

			tree_.init(radio_->id(), &neighborhood_, debug_);
			token_ring_.init(neighborhood_, tree_, *debug_);
			scheduler_.init(neighborhood_, tree_, token_ring_, *clock_, *timer_, *debug_);


			#if defined(PC)
				scenario_01();
			#endif

			//timer_->set_timer<ExampleApplication,
									//&ExampleApplication::fake_beacon>(100, this, 0);
		}

	/*
	 * Functions for faking events with mock ND, fake (local) radio, etc...
	 */
	#if defined(PC)
		Radio r;

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
	#endif // defined(PC)

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
