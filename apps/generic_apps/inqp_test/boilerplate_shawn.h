
#include "boilerplate_base.h"

#include <algorithms/neighbor_discovery/static_neighborhood.h>

typedef StaticNeighborhood<Os> Neighborhood;
typedef ForwardOnDirectedNd<Os, Neighborhood> StaticResultRadio;

typedef INQPCommunicator<Os, Processor, Os::Timer, Os::Debug, OneShotQueryRadio, StaticResultRadio, Neighborhood> Communicator;

class AppBoilerplate : public AppBase {
	public:

		typedef StaticResultRadio ResultRadio;
		enum { SINK = 1 };

		void init(Os::AppMainParameter& v) {
			AppBase::init(v);
			
			init_neighborhood();
			init_communicator();
		}

		void init_neighborhood() {
			neighborhood_.init();
			switch(radio_->id()) {
				case 1:
					neighborhood_.add_neighbor(1, Neighborhood::Neighbor::OUT_EDGE);
					break;
				case 2:
				case 3:
				case 4:
					neighborhood_.add_neighbor(1, Neighborhood::Neighbor::OUT_EDGE);
					break;
				case 5:
				case 6:
					neighborhood_.add_neighbor(3, Neighborhood::Neighbor::OUT_EDGE);
					break;
				case 12:
					neighborhood_.add_neighbor(6, Neighborhood::Neighbor::OUT_EDGE);
					break;
				default:
					neighborhood_.add_neighbor(4, Neighborhood::Neighbor::OUT_EDGE);
					break;
			}
		}

		void init_communicator() {
			query_radio_.init(radio_);
			query_radio_.enable_radio();

			result_radio_.init(neighborhood_, *radio_, *timer_, *debug_);
			result_radio_.enable_radio();

			communicator_.init(query_processor_, query_radio_, result_radio_, neighborhood_, *timer_, *debug_);
			communicator_.set_sink(SINK);
		}

		ResultRadio& result_radio() { return result_radio_; }

	protected:
		Neighborhood neighborhood_;
		StaticResultRadio result_radio_;
		OneShotQueryRadio query_radio_;
		Communicator communicator_;
		Os::Radio::node_id_t root_;

};





