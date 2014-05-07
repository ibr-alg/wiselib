
int assert(int) { }

extern "C" {
	#include <string.h>
	#include <stdio.h>
}

#include "boilerplate_base.h"
#include <algorithms/neighbor_discovery/static_neighborhood.h>
#include <util/broker/sky_light_data_provider.h>

typedef StaticNeighborhood<Os> Neighborhood;
typedef ForwardOnDirectedNd<Os, Neighborhood> StaticResultRadio;

typedef INQPCommunicator<Os, Processor, Os::Timer, Os::Debug, OneShotQueryRadio, StaticResultRadio, Neighborhood> Communicator;

enum { ROOT = 53851 }; // 29

Os::Radio::node_id_t tree_[][2] = {
	{ 55323, 53952 },
	{ 52154, 56458 },
	{ 54493, 56445 },
	{ 50502, 49268 },
	{ 56445, 55054 },
	{ 56375, 48445 },
	{ 51765, 55054 },
	{ 49900, 52027 },
	{ 50223, 48570 },
	{ 55546, 49279 },
	{ 52027, 50502 },
	{ 55778, 49185 },
	{ 49898, 49819 },
	{ 56458, 49898 },
	{ 51271, 49268 },
	{ 54286, 50223 },
	{ 47742, 50223 },
	{ 48389, 50502 },
	{ 53440, 49272 },
	{ 49292, 51243 },
	{ 56038, 49898 },
	{ 55054, 48445 },
	{ 56265, 51243 },
	{ 49185, 49272 },
	{ 49819, 56375 },
	{ 48445, 53851 },
	{ 49272, 49765 },
	{ 56188, 48389 },
	{ 52174, 51419 },
	{ 49268, 53851 },
	{ 54171, 49819 },
	{ 55421, 48570 },
	{ 52793, 48570 },
	{ 52434, 51765 },
	{ 53431, 50223 },
	{ 48994, 48570 },
	{ 49765, 50053 },
	{ 48570, 56375 },
	{ 51419, 52434 },
	{ 53952, 56188 },
	{ 56648, 49765 },
	{ 51243, 49268 },
	{ 50053, 50502 },
	{ 56345, 55054 },
	{ 49279, 56188 },
	{ ROOT, ROOT },
	{ Radio::NULL_NODE_ID, Radio::NULL_NODE_ID }
};


class AppBoilerplate : public AppBase {
	public:
		typedef StaticResultRadio ResultRadio;
		enum { SINK = ROOT };

		void init(Os::AppMainParameter& v) {
			AppBase::init(v);
			
			init_neighborhood();

			init_communicator();

			sensor_data_provider_.init("_:b2", this->tuplestore_, *this->timer_, *this->debug_);
		}

		void init_neighborhood() {
			neighborhood_.init();
			
			for(Os::Radio::node_id_t (*p)[2] = tree_; (*p)[0] != Radio::NULL_NODE_ID; ++p) {
				if((*p)[0] == radio_->id()) {
					neighborhood_.add_neighbor((*p)[1], Neighborhood::Neighbor::OUT_EDGE);
					break;
				}
			}
		}

		void init_communicator() {
			query_radio_.init(radio_);
			query_radio_.enable_radio();

			result_radio_.init(neighborhood_, *radio_, *timer_, *rand_, *debug_);
			result_radio_.enable_radio();

			communicator_.init(query_processor_, query_radio_, result_radio_, neighborhood_, *timer_, *debug_);
			communicator_.set_sink(ROOT);
		}

		ResultRadio& result_radio() { return result_radio_; }

	protected:
		Neighborhood neighborhood_;
		StaticResultRadio result_radio_;
		OneShotQueryRadio query_radio_;
		Communicator communicator_;

		SkyLightDataProvider<Os, TS> sensor_data_provider_;

};






