
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
//typedef ForwardOnDirectedNd<Os, Neighborhood, Neighborhood::Neighbor::IN_EDGE> OneShotQueryRadio;

typedef INQPCommunicator<Os, Processor, Os::Timer, Os::Debug, OneShotQueryRadio, StaticResultRadio, Neighborhood> Communicator;

enum { ROOT = 53851 }; // 29
//enum { ROOT = 18045 }; // blue node without battery box

template<int NNNN> struct inode_to_addr { };

template<> struct inode_to_addr<  1> { enum { ADDR = 55778 }; };
template<> struct inode_to_addr<  4> { enum { ADDR = 49185 }; };
template<> struct inode_to_addr<  5> { enum { ADDR = 55323 }; };
template<> struct inode_to_addr<  7> { enum { ADDR = 53440 }; };
template<> struct inode_to_addr<  8> { enum { ADDR = 53952 }; };
template<> struct inode_to_addr<  9> { enum { ADDR = 55546 }; };
template<> struct inode_to_addr< 10> { enum { ADDR = 56648 }; };
template<> struct inode_to_addr< 11> { enum { ADDR = 49765 }; };
template<> struct inode_to_addr< 12> { enum { ADDR = 48389 }; };
template<> struct inode_to_addr< 13> { enum { ADDR = 49279 }; };
template<> struct inode_to_addr< 14> { enum { ADDR = 49272 }; };
template<> struct inode_to_addr< 15> { enum { ADDR = 56188 }; };
template<> struct inode_to_addr< 17> { enum { ADDR = 49900 }; };
template<> struct inode_to_addr< 18> { enum { ADDR = 56265 }; };
template<> struct inode_to_addr< 19> { enum { ADDR = 50053 }; };
template<> struct inode_to_addr< 20> { enum { ADDR = 52027 }; };
template<> struct inode_to_addr< 21> { enum { ADDR = 51271 }; };
template<> struct inode_to_addr< 23> { enum { ADDR = 50502 }; };
template<> struct inode_to_addr< 24> { enum { ADDR = 49292 }; };
template<> struct inode_to_addr< 25> { enum { ADDR = 49268 }; };
template<> struct inode_to_addr< 27> { enum { ADDR = 51243 }; };
template<> struct inode_to_addr< 28> { enum { ADDR = 56345 }; };
template<> struct inode_to_addr< 29> { enum { ADDR = 53851 }; };
template<> struct inode_to_addr< 30> { enum { ADDR = 48445 }; };
template<> struct inode_to_addr< 31> { enum { ADDR = 55054 }; };
template<> struct inode_to_addr< 33> { enum { ADDR = 48570 }; };
template<> struct inode_to_addr< 35> { enum { ADDR = 49898 }; };
template<> struct inode_to_addr< 36> { enum { ADDR = 55421 }; };
template<> struct inode_to_addr< 37> { enum { ADDR = 56038 }; };
template<> struct inode_to_addr< 38> { enum { ADDR = 54286 }; };
template<> struct inode_to_addr< 39> { enum { ADDR = 52154 }; };
template<> struct inode_to_addr< 40> { enum { ADDR = 54493 }; };
template<> struct inode_to_addr< 41> { enum { ADDR = 56375 }; };
template<> struct inode_to_addr< 42> { enum { ADDR = 56458 }; };
template<> struct inode_to_addr< 43> { enum { ADDR = 54171 }; };
template<> struct inode_to_addr< 44> { enum { ADDR = 56445 }; };
template<> struct inode_to_addr< 46> { enum { ADDR = 52434 }; };
template<> struct inode_to_addr< 47> { enum { ADDR = 49819 }; };
template<> struct inode_to_addr< 50> { enum { ADDR = 52174 }; };
template<> struct inode_to_addr< 51> { enum { ADDR = 51765 }; };
template<> struct inode_to_addr< 52> { enum { ADDR = 48994 }; };
template<> struct inode_to_addr< 54> { enum { ADDR = 51419 }; };
template<> struct inode_to_addr< 55> { enum { ADDR = 52793 }; };
template<> struct inode_to_addr< 56> { enum { ADDR = 53431 }; };
template<> struct inode_to_addr<199> { enum { ADDR = 47742 }; };
template<> struct inode_to_addr<200> { enum { ADDR = 50223 }; };

struct addr_room {
	Os::Radio::node_id_t addr;
	char room[3];
};

addr_room rooms_[] = {
	{ inode_to_addr<  1>::ADDR, "01" },
	{ inode_to_addr<  4>::ADDR, "01" },
	//{ inode_to_addr<  2>::ADDR, "02" },
	//{ inode_to_addr<  3>::ADDR, "02" },
	{ inode_to_addr<  5>::ADDR, "02" },
	//{ inode_to_addr<  6>::ADDR, "02" },
	{ inode_to_addr<  8>::ADDR, "03" },
	{ inode_to_addr<  9>::ADDR, "03" },
	{ inode_to_addr<  7>::ADDR, "04" },
	{ inode_to_addr< 10>::ADDR, "04" },
	{ inode_to_addr< 11>::ADDR, "04" },
	{ inode_to_addr< 14>::ADDR, "04" },
	{ inode_to_addr< 12>::ADDR, "05" },
	{ inode_to_addr< 13>::ADDR, "05" },
	{ inode_to_addr< 15>::ADDR, "06" },
	//{ inode_to_addr< 16>::ADDR, "06" },
	{ inode_to_addr< 17>::ADDR, "07" },
	{ inode_to_addr< 20>::ADDR, "07" },
	{ inode_to_addr< 18>::ADDR, "08" },
	{ inode_to_addr< 19>::ADDR, "08" },
	{ inode_to_addr< 21>::ADDR, "08" },
	//{ inode_to_addr< 22>::ADDR, "08" },
	{ inode_to_addr< 23>::ADDR, "08" },
	{ inode_to_addr< 24>::ADDR, "09" },
	{ inode_to_addr< 25>::ADDR, "09" },
	//{ inode_to_addr< 26>::ADDR, "09" },
	{ inode_to_addr< 27>::ADDR, "09" },
	{ inode_to_addr< 28>::ADDR, "09" },
	{ inode_to_addr< 29>::ADDR, "09" },
	{ inode_to_addr< 30>::ADDR, "09" },
	{ inode_to_addr< 31>::ADDR, "09" },
	{ inode_to_addr< 39>::ADDR, "10" },
	{ inode_to_addr< 42>::ADDR, "10" },
	{ inode_to_addr< 46>::ADDR, "10" },
	{ inode_to_addr< 50>::ADDR, "10" },
	{ inode_to_addr< 51>::ADDR, "10" },
	{ inode_to_addr< 54>::ADDR, "10" },
	{ inode_to_addr< 41>::ADDR, "11" },
	//{ inode_to_addr< 45>::ADDR, "11" },
	//{ inode_to_addr< 49>::ADDR, "11" },
	//{ inode_to_addr< 53>::ADDR, "11" },
	{ inode_to_addr< 55>::ADDR, "12" },
	{ inode_to_addr< 33>::ADDR, "12" },
	{ inode_to_addr< 36>::ADDR, "12" },
	{ inode_to_addr< 56>::ADDR, "12" },
	{ inode_to_addr< 47>::ADDR, "12" },
	{ inode_to_addr< 43>::ADDR, "12" },
	{ inode_to_addr<199>::ADDR, "13" },
	{ inode_to_addr<200>::ADDR, "13" },
	//{ inode_to_addr< 34>::ADDR, "13" },
	{ inode_to_addr< 37>::ADDR, "13" },
	{ inode_to_addr< 35>::ADDR, "13" },
	{ inode_to_addr< 38>::ADDR, "13" },

	{ 18045, "42" },
	{ Radio::NULL_NODE_ID, "\0\0" },
};


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

		void insert_special_tuples() {

			Radio::node_id_t me = this->radio_->id();
			for(addr_room *ar = rooms_; ar->addr != Radio::NULL_NODE_ID; ar++) {
				if(ar->addr == me) {
					//printf("yay\n");
					char room[] = "<http://example.org/roomXX>";
					enum { offs = 24 };
					memcpy(room + offs, ar->room, 2);

					const char *sensor1 = "<http://example.org/sensor_042_1>";
					const char *sensor2 = "<http://example.org/sensor_042_2>";
					const char *foi = "<http://purl.oclc.org/NET/ssnx/ssn#featureOfInterest>";
					const char *lat = "<http://www.w3.org/2003/01/geo/wgs84_pos#lat>";
					const char *latv = "\"53.2730550\"";
					const char *lon = "<http://www.w3.org/2003/01/geo/wgs84_pos#long>";
					const char *lonv = "\"-9.2479960\"";
					insert_tuple(sensor1, foi, room);
					insert_tuple(sensor2, foi, room);
					insert_tuple(room, lat, latv);
					insert_tuple(room, lon, lonv);
					break;
				}
				else {
					//printf("%lu != %lu\n", (unsigned long)ar->addr, (unsigned long)me);
				}

			}

		} // insert_special_tuples

		void init_neighborhood() {
			neighborhood_.init();
			
			for(Os::Radio::node_id_t (*p)[2] = tree_; (*p)[0] != Radio::NULL_NODE_ID; ++p) {
				if((*p)[0] == radio_->id()) {
					neighborhood_.add_neighbor((*p)[1], Neighborhood::Neighbor::OUT_EDGE);
					break;
				}
				//else if((*p)[1] == radio_->id()) {
					//neighborhood_.add_neighbor((*p)[0], Neighborhood::Neighbor::IN_EDGE);
				//}
			}
		}

		void init_communicator() {
			//query_radio_.init(radio_);
			//query_radio_.enable_radio();

			result_radio_.init(neighborhood_, *radio_, *timer_, *rand_, *debug_);
			result_radio_.enable_radio();

			//debug_->debug("init com rt=%lu", (unsigned long)ROOT);
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






