
int assert(int) { }

extern "C" {
	#include <string.h>
	#include <stdio.h>
}

#include "boilerplate_base.h"
#include <algorithms/neighbor_discovery/static_neighborhood.h>

typedef StaticNeighborhood<Os> Neighborhood;
typedef ForwardOnDirectedNd<Os, Neighborhood> StaticResultRadio;

typedef INQPCommunicator<Os, Processor, Os::Timer, Os::Debug, OneShotQueryRadio, StaticResultRadio, Neighborhood> Communicator;

// Convert mote IDs to node addresses at compile time

template<int> struct M { };

// 3rd floor, Minds w-iLab.t office

template<> struct M<  1> { enum { v = 55778 }; };
template<> struct M<  4> { enum { v = 49185 }; };
template<> struct M<  5> { enum { v = 55323 }; };
template<> struct M<  7> { enum { v = 53440 }; };
template<> struct M<  9> { enum { v = 55546 }; };
template<> struct M< 10> { enum { v = 56648 }; };
template<> struct M< 11> { enum { v = 49765 }; };
template<> struct M< 12> { enum { v = 48389 }; };
template<> struct M< 13> { enum { v = 49279 }; };
template<> struct M< 14> { enum { v = 49272 }; };
template<> struct M< 15> { enum { v = 56188 }; };
template<> struct M< 17> { enum { v = 49900 }; };
template<> struct M< 18> { enum { v = 56265 }; };
template<> struct M< 23> { enum { v = 50502 }; };
template<> struct M< 24> { enum { v = 49292 }; };
template<> struct M< 25> { enum { v = 49268 }; };
template<> struct M< 27> { enum { v = 51243 }; };
template<> struct M< 28> { enum { v = 56345 }; };
template<> struct M< 29> { enum { v = 53851 }; };
template<> struct M< 30> { enum { v = 48445 }; };
template<> struct M< 31> { enum { v = 55054 }; };
template<> struct M< 33> { enum { v = 48570 }; };
template<> struct M< 35> { enum { v = 49898 }; };
template<> struct M< 36> { enum { v = 55421 }; };
template<> struct M< 37> { enum { v = 56038 }; };
template<> struct M< 38> { enum { v = 54286 }; };
template<> struct M< 39> { enum { v = 52154 }; };
template<> struct M< 40> { enum { v = 54493 }; };
template<> struct M< 41> { enum { v = 56375 }; };
template<> struct M< 42> { enum { v = 56458 }; };
template<> struct M< 43> { enum { v = 54171 }; };
template<> struct M< 44> { enum { v = 56445 }; };
template<> struct M< 46> { enum { v = 52434 }; };
template<> struct M< 47> { enum { v = 49819 }; };
template<> struct M< 51> { enum { v = 51765 }; };
template<> struct M< 52> { enum { v = 48994 }; };
template<> struct M< 54> { enum { v = 51419 }; };
template<> struct M< 55> { enum { v = 52793 }; };
template<> struct M< 56> { enum { v = 53431 }; };
template<> struct M<199> { enum { v = 47742 }; };
template<> struct M<200> { enum { v = 50223 }; };

//enum { ROOT = 50053 }; // 19
enum { ROOT = 53851 }; // 29

Os::Radio::node_id_t tree_[][2] = {
	/* Run1: 3, INSE with 19 as root, manual fixing
	{ 47742, 52154 },
	{ 48389, 49279 },
	{ 48445, 49292 },
	{ 48570, 49898 },
	{ 48994, 55054 },
	{ 49185, 49765 },
	{ 49268, 49292 },
	{ 49272, 49765 },
	{ 49279, 55546 },
	{ 49292, 56265 },
	{ 49765, 50053 },
	{ 49819, 56038 },
	{ 49898, 51765 },
	{ 49900, 55546 },
	{ 50223, 49898 },
	{ 50502, 49268 },
	{ 51243, 49292 },
	{ 51419, 48445 },
	{ 51765, 51419 },
	{ 52154, 49898 },
	{ 52174, 51419 },
	{ 52434, 54286 },
	{ 52793, 53431 },
	{ 53431, 49898 },
	{ 53440, 50053 },
	{ 53851, 51243 },
	{ 54171, 49898 },
	{ 54286, 49898 },
	{ 54493, 48570 },
	{ 55054, 48445 },
	{ 55323, 48389 },
	{ 55421, 56038 },
	{ 55546, 56648 },
	{ 55778, 49765 },
	{ 56038, 51765 },
	{ 56188, 48389 },
	{ 56265, 49765 },
	{ 56345, 55054 },
	{ 56375, 55421 },
	{ 56445, 48994 },
	{ 56458, 49898 },
	{ 56648, 53440 },
	*/
	// Run2: 3, INSE with 19 as root, manual fixing

	//{ 55323, 53952 },
	//{ 52154, 56375 },
	//{ 54493, 56375 },

	//{ 50502, 49268 },
	//{ 56445, 55054 },
	//{ 56375, 48445 },

	//{ 51765, 55054 },
	//{ 49900, 52027 },
	//{ 50223, 48570 },
	//{ 55546, 49279 },

	//{ 52027, 51271 },
	//{ 55778, 49185 },
	//{ 49898, 49819 },
	//{ 56458, 49898 },
	//{ 51271, 49268 },
	//{ 54286, 50223 },
	//{ 47742, 50223 },
	//{ 48389, 51271 },
	//{ 53440, 49272 },
	//{ 49292, 51243 },
	//{ 56038, 49898 },
	//{ 55054, 48445 },
	//{ 56265, 51243 },
	//{ 49185, 49272 },
	//{ 49344, 53851 },

	//{ 49819, 56375 },
	//{ 48445, 49344 },
	//{ 49272, 49765 },
	//{ 56188, 48389 },
	//{ 52174, 51419 },
	//{ 49268, 53851 },
	//{ 54171, 49819 },
	//{ 55421, 48570 },
	//{ 52793, 48570 },
	//{ 52434, 51765 },
	//{ 53431, 50223 },
	//{ 48994, 48570 },
	//{ 49765, 50053 },
	//{ 48570, 56375 },
	//{ 51419, 52434 },
	//{ 53952, 56188 },
	//{ 56648, 49765 },
	//{ 51243, 49268 },

	//{ 50053, 50502 },
	//{ 56345, 55054 },
	//{ 49279, 56188 },

	// Run 2 with more improvements

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

};






