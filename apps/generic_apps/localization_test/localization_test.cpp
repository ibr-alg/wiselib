/*
 * Simple Wiselib Example
 */
#include "external_interface/external_interface_testing.h"
#include "algorithms/routing/tree/tree_routing.h"

#include "util/pstl/vector_static.h"
#include "util/pstl/pair.h"
#include "util/pstl/map_static_vector.h"
#include "util/pstl/set_static.h"
#include "util/pstl/list_static.h"

#include "algorithms/localization/distance_based/distance_based_localization_base.h"
#include "algorithms/localization/distance_based/modules/localization_nop_module.h"
#include "algorithms/localization/distance_based/modules/distance/localization_dv_hop_module.h"
#include "algorithms/localization/distance_based/modules/distance/localization_sum_dist_module.h"
//#include "algorithms/localization/distance_based/modules/distance/localization_euclidean_module.h"
#include "algorithms/localization/distance_based/modules/position/localization_minmax_module.h"
#include "algorithms/localization/distance_based/modules/position/localization_lateration_module.h"

#include "algorithms/localization/distance_based/util/localization_shared_data.h"

#include "algorithms/localization/distance_based/math/vec.h"
#ifndef SHAWN
#include "algorithms/localization/distance_based/util/lqi_distance.h"
#include "util/pstl/map_static_vector.h"
#endif
#ifdef SHAWN

#include "sys/node.h"
#include "sys/vec.h"
#define GATEWAY_RADIO_ID 0
#define SHAWN_VIS
#endif

typedef wiselib::OSMODEL Os;
//----- Routing
typedef wiselib::TreeRouting<Os, Os::ExtendedRadio, Os::Timer, Os::Debug> tree_routing_t;

//// ------------ Configure Shared Data - especially the used containers ------
#define MAX_SIZE 12

//typedef Arithmatic Arithmatic;
typedef double Arithmatic;

typedef wiselib::set_static<Os, Os::ExtendedRadio::node_id_t, MAX_SIZE> NodeSet;
typedef wiselib::list_static<Os, Os::ExtendedRadio::node_id_t, MAX_SIZE> NodeList;
typedef wiselib::MapStaticVector<Os, Os::ExtendedRadio::node_id_t, Arithmatic, MAX_SIZE> DistanceMap;

typedef wiselib::LocalizationNeighborInfo<Os, Os::ExtendedRadio::node_id_t, NodeSet, DistanceMap, Arithmatic> NeighborInfo;
typedef wiselib::list_static<Os, NeighborInfo*, MAX_SIZE> NeighborInfoList;
typedef wiselib::MapStaticVector<Os, Os::ExtendedRadio::node_id_t, NeighborInfo, MAX_SIZE> NeighborInfoMap;
typedef wiselib::LocalizationNeighborhood<Os, Os::ExtendedRadio::node_id_t, NeighborInfo, NeighborInfoMap, Arithmatic> Neighborhood;

typedef wiselib::MapStaticVector<Os, int, wiselib::Vec<Arithmatic>, MAX_SIZE> LocationMap;

typedef wiselib::LocalizationSharedData<Os, Os::ExtendedRadio, Os::Clock, Neighborhood, NeighborInfoList, NodeSet, NodeList, DistanceMap, LocationMap, Arithmatic>
		SharedData;

typedef wiselib::pair<Arithmatic, Arithmatic> DistancePair;

// ------------ Configure LQI Values ------------------------------
#ifndef SHAWN
typedef uint8_t lqi_t;
typedef wiselib::MapStaticVector<Os, lqi_t , Arithmatic, LQI_TABLE_SIZE> LQIMap;
typedef LQIMap::iterator LQIMapIterator;

//typedef wiselib::pair<int, Arithmatic> DistanceLQIPair;

typedef wiselib::LQIDistanceModel<Os, Os::Debug, Arithmatic, Os::ExtendedRadio, LQI_TABLE_SIZE > Distance;


#endif


// ------------ Configure Localization Modules ------------------------------
// // no operation - dummy module
typedef wiselib::LocalizationNopModule<Os, Os::ExtendedRadio, SharedData> NopModule;
// // distance modules
typedef wiselib::LocalizationDvHopModule<Os, Os::ExtendedRadio, Os::Clock, Os::Debug, SharedData, NodeSet, Arithmatic> DvHopModule;
#ifdef SHAWN
typedef wiselib::LocalizationSumDistModule<Os, Os::ExtendedRadio, Os::Clock, Os::Distance, Os::Debug, SharedData, Arithmatic> SumDistModule;
#else
typedef wiselib::LocalizationSumDistModule<Os, Os::ExtendedRadio, Os::Clock, Distance, Os::Debug, SharedData, Arithmatic> SumDistModule;
#endif
 //typedef wiselib::LocalizationEuclideanModule<Os, Os::ExtendedRadio, Os::Clock, Distance, Os::Debug, SharedData, DistancePair> EuclideanModule;



// // position modules
typedef wiselib::LocalizationMinMaxModule<Os, Os::ExtendedRadio, Os::Debug, SharedData, Arithmatic> MinMaxModule;
typedef wiselib::LocalizationLaterationModule<Os, Os::ExtendedRadio, Os::Debug, SharedData, Arithmatic> LaterationModule;

#define MINMAX
//#define LATERATION

#define DVHOP
//#define SUMDIST



//#define USE_TREEROUTING

#ifdef MINMAX
#undef LATERATION
#endif

#ifdef LATERATION
#undef MINMAX
#endif



#ifdef DVHOP
#undef SUMDIST
#endif
#ifdef SUMDIST
#undef DVHOP
#endif

#ifdef DVHOP

#ifdef LATERATION
typedef wiselib::DistanceBasedLocalization<Os, Os::ExtendedRadio, Os::Timer, SharedData, DvHopModule, LaterationModule, NopModule, Arithmatic> LocalizationBase;
#elif defined MINMAX
typedef wiselib::DistanceBasedLocalization<Os, Os::ExtendedRadio, Os::Timer, SharedData, DvHopModule, MinMaxModule, NopModule, Arithmatic> LocalizationBase;
#endif
#endif


#ifdef SUMDIST
#ifdef LATERATION
typedef wiselib::DistanceBasedLocalization<Os, Os::ExtendedRadio, Os::Timer, SharedData, SumDistModule, LaterationModule, NopModule, Arithmatic> LocalizationBase;

#elif defined MINMAX
typedef wiselib::DistanceBasedLocalization<Os, Os::ExtendedRadio, Os::Timer, SharedData, SumDistModule, MinMaxModule, NopModule, Arithmatic> LocalizationBase;

#endif
#endif

#ifndef SHAWN
#define GATEWAY_RADIO_ID 0xCBE5
#endif



const uint8_t DEBUG_MESSAGE_ID = 36;


class LocalizationTestApplication {
public:
	void init(Os::AppMainParameter& value) {
		radio_ = &wiselib::FacetProvider<Os, Os::ExtendedRadio>::get_facet(value);

		timer_ = &wiselib::FacetProvider<Os, Os::Timer>::get_facet(value);
		debug_ = &wiselib::FacetProvider<Os, Os::Debug>::get_facet(value);
		position_ = &wiselib::FacetProvider<Os, Os::Position>::get_facet(value);
		clock_ = &wiselib::FacetProvider<Os, Os::Clock>::get_facet(value);

#ifdef SHAWN
		distance_ = &wiselib::FacetProvider<Os, Os::Distance>::get_facet(value);

		node_ = &value.proc->owner_w();
#endif



		//if (radio_->id() == GATEWAY_RADIO_ID)
			debug_->debug("Start Localization Test Application!\n");
#ifdef SIZEDEBUG
		debug_->debug("NodeSet %d\n", sizeof(NodeSet));
		debug_->debug("LQIMap %d\n", sizeof(LQIMap));
		debug_->debug("NodeSet %d\n", sizeof(NodeSet));
		debug_->debug("NodeList %d\n", sizeof(NodeList));
		debug_->debug("DistanceMap %d\n", sizeof(DistanceMap));
		debug_->debug("NeighborInfo %d\n", sizeof(NeighborInfo));
		debug_->debug("NeighborInfoList %d\n", sizeof(NeighborInfoList));
		debug_->debug("NeighborInfoMap %d\n", sizeof(NeighborInfoMap));
		debug_->debug("Neighborhood %d\n", sizeof(Neighborhood));
		debug_->debug("LocationMap %d\n", sizeof(LocationMap));
		debug_->debug("SharedData %d\n", sizeof(SharedData));
#endif
		round_ = 1;

		// configure routing

#ifdef USE_TREEROUTING
		tree_routing.init(*radio_, *timer_, *debug_);

		if (radio_->id() == GATEWAY_RADIO_ID)
			tree_routing.set_sink(true);
		else
			tree_routing.set_sink(false);

		tree_routing.enable_radio();

		tree_routing.reg_recv_callback<LocalizationTestApplication, &LocalizationTestApplication::receive_tree_message> (this);
#endif

// 		radio_->enable_radio();
// 		radio_->reg_recv_callback<LocalizationTestApplication, &LocalizationTestApplication::receive> (this);


//SUMDIST
#ifdef DVHOP
		if (shared_data_.is_anchor())
			//start_time = 700;
			start_time = 700;
		else
			//start_time = 720;//750;
			start_time = 665;//750;
#elif defined SUMDEST
		if (shared_data_.is_anchor())
			//start_time = 700;
			start_time = 700;
		else
			start_time = 720;//750;


#endif

#ifndef SHAWN



		//('0dB Keramikantenne, Freifeld, 20cm ueber dem Boden

#ifndef DVHOP

		lqi_distance_map.insert(wiselib::pair<lqi_t,Arithmatic>(151,2.0));

		//added
		//lqi_distance_map.insert(wiselib::pair<lqi_t,Arithmatic>(140,3.0));


		lqi_distance_map.insert(wiselib::pair<lqi_t,Arithmatic>(119,4.0));

		//added
		//lqi_distance_map.insert(wiselib::pair<lqi_t,Arithmatic>(110,5.0));

		lqi_distance_map.insert(wiselib::pair<lqi_t,Arithmatic>(104,6.0));

		//added
		//lqi_distance_map.insert(wiselib::pair<lqi_t,Arithmatic>(95,7.0));

		lqi_distance_map.insert(wiselib::pair<lqi_t,Arithmatic>(89,8.0));

		//added
		//lqi_distance_map.insert(wiselib::pair<lqi_t,Arithmatic>(83,9.0));

		lqi_distance_map.insert(wiselib::pair<lqi_t,Arithmatic>(77,10.0));

		//added
		//lqi_distance_map.insert(wiselib::pair<lqi_t,Arithmatic>(75,11.0));

		lqi_distance_map.insert(wiselib::pair<lqi_t,Arithmatic>(73,12.0));

		//added
		//lqi_distance_map.insert(wiselib::pair<lqi_t,Arithmatic>(64,13.0));

		lqi_distance_map.insert(wiselib::pair<lqi_t,Arithmatic>(59,14.0));

		//added
		//lqi_distance_map.insert(wiselib::pair<lqi_t,Arithmatic>(57,15.0));

		lqi_distance_map.insert(wiselib::pair<lqi_t,Arithmatic>(55,16.0));

		//added
		//lqi_distance_map.insert(wiselib::pair<lqi_t,Arithmatic>(50,17.0));

/*		lqi_distance_map.insert(wiselib::pair<lqi_t,Arithmatic>(46,18.0));

		lqi_distance_map.insert(wiselib::pair<lqi_t,Arithmatic>(42,20.0));

		lqi_distance_map.insert(wiselib::pair<lqi_t,Arithmatic>(51,22.0));

		lqi_distance_map.insert(wiselib::pair<lqi_t,Arithmatic>(42,24.0));

		lqi_distance_map.insert(wiselib::pair<lqi_t,Arithmatic>(44,26.0));

		lqi_distance_map.insert(wiselib::pair<lqi_t,Arithmatic>(42,28.0));

		lqi_distance_map.insert(wiselib::pair<lqi_t,Arithmatic>(33,30.0));

		lqi_distance_map.insert(wiselib::pair<lqi_t,Arithmatic>(31,32.0));

		lqi_distance_map.insert(wiselib::pair<lqi_t,Arithmatic>(25,34.0));

		lqi_distance_map.insert(wiselib::pair<lqi_t,Arithmatic>(24,36.0));

		lqi_distance_map.insert(wiselib::pair<lqi_t,Arithmatic>(12,38.0));

		lqi_distance_map.insert(wiselib::pair<lqi_t,Arithmatic>(23,40.0));

		lqi_distance_map.insert(wiselib::pair<lqi_t,Arithmatic>(24,42.0));*/
//to be sure that a some distance will be found
		lqi_distance_map.insert(wiselib::pair<lqi_t,Arithmatic>(5,52.0));

#endif


#endif

//if(!shared_data_.is_anchor())
		timer_->set_timer<LocalizationTestApplication, &LocalizationTestApplication::start_localization_algorithm> (start_time , this, 0);
//else
//	timer_->set_timer<LocalizationTestApplication, &LocalizationTestApplication::start_localization_algorithm> (start_time + (radio_->id()%10)*2, this, 0);

//		start_localization_algorithm(0);

	}

	void start_localization_algorithm(void*) {

		//-- configure localization


		configure_localization_algorithm();





#ifndef SHAWN
#ifndef DVHOP
		 distance_.init(radio_, debug_ ,&lqi_distance_map);
#endif
#endif

#ifdef DVHOP
		dv_hop_module_.init(*radio_, *clock_, *debug_, shared_data_);
#endif
#ifdef SUMDIST

#ifndef SHAWN
		sum_dist_module_.init(*radio_, *clock_, *debug_, shared_data_, distance_);
#else
		sum_dist_module_.init(*radio_, *clock_, *debug_, shared_data_, *distance_);

#endif

		//euclidean_module_.init( *radio_, *clock_, *debug_, shared_data_, distance_ );
#endif

		//
#ifdef LATERATION
		lateration_module_.init( *radio_, *debug_, shared_data_ );
#elif defined MINMAX
		min_max_module_.init(*radio_, *debug_, shared_data_);
#endif


#ifdef DVHOP
#ifdef LATERATION
		localization_.init(*radio_, *timer_, *debug_, shared_data_, dv_hop_module_, lateration_module_, nop_module_);
#elif defined MINMAX
		localization_.init(*radio_, *timer_, *debug_, shared_data_, dv_hop_module_, min_max_module_, nop_module_);
#endif

#endif


#ifdef SUMDIST
#ifdef LATERATION
		localization_.init(*radio_, *timer_, *debug_, shared_data_, sum_dist_module_, lateration_module_, nop_module_);
#elif defined MINMAX
		localization_.init(*radio_, *timer_, *debug_, shared_data_, sum_dist_module_, min_max_module_, nop_module_);
#endif

#endif

		//if (shared_data_.is_anchor())
			timer_->set_timer<LocalizationTestApplication, &LocalizationTestApplication::evaluate> (74000 + radio_->id() / 100, this, 0);
		//else
			//timer_->set_timer<LocalizationTestApplication, &LocalizationTestApplication::evaluate> (start_time + radio_->id() / 10000, this, 0);


	}

	// --------------------------------------------------------------------
	void configure_localization_algorithm() {

		//Anker Knoten
		if ((radio_->id() == 0x5A35) || (radio_->id() == 0)) {
			shared_data_.set_anchor(true);
			shared_data_.set_confidence(1.0);
			wiselib::Vec<Arithmatic> v(1, 11, 2);

#ifdef SHAWN

			shawn::Vec v2(1, 11, 2);
			node_->set_real_position(v2);
#endif
			shared_data_.set_position(v);

		} else if ((radio_->id() == 0x971e) || (radio_->id() == 1)) {

			shared_data_.set_anchor(true);
			shared_data_.set_confidence(1.0);
			wiselib::Vec<Arithmatic> v(10.5, 6, 1);
#ifdef SHAWN
			shawn::Vec v2(10.5, 6, 1);
			node_->set_real_position(v2);
#endif
			shared_data_.set_position(v);

		} else if ((radio_->id() == 0xcbe5) || (radio_->id() == 2)) {

			shared_data_.set_anchor(true);
			shared_data_.set_confidence(1.0);
			wiselib::Vec<Arithmatic> v(23, 10, 1);

#ifdef SHAWN
			shawn::Vec v2(23, 10, 1);
			node_->set_real_position(v2);
#endif
			shared_data_.set_position(v);

		} else if ((radio_->id() == 0x1bb0) || (radio_->id() == 3)) {

			shared_data_.set_anchor(true);
			shared_data_.set_confidence(1.0);
			wiselib::Vec<Arithmatic> v(28.5, 1, 1);

#ifdef SHAWN
			shawn::Vec v2(28.5, 1, 1);
			node_->set_real_position(v2);
#endif
			shared_data_.set_position(v);

		} else if ((radio_->id() == 0x61E1) || (radio_->id() == 4)) {

			shared_data_.set_anchor(true);
			shared_data_.set_confidence(1.0);
			wiselib::Vec<Arithmatic> v(1, 6, 2);

#ifdef SHAWN
			shawn::Vec v2(1, 6, 2);
			node_->set_real_position(v2);
#endif
/*			shared_data_.set_position(v);
//id DVHOP and Lateration

			} else if ((radio_->id() == 0x970B) || (radio_->id() == 5)) {

			shared_data_.set_anchor(true);
			shared_data_.set_confidence(1.0);
			wiselib::Vec<Arithmatic> v(34.5, 11.5, 2);

#ifdef SHAWN
			shawn::Vec v2(34.5, 11.5, 2);
			node_->set_real_position(v2);
#endif
			shared_data_.set_position(v);

		} else if ((radio_->id() == 0xCC33) || (radio_->id() == 6)) {

			shared_data_.set_anchor(true);
			shared_data_.set_confidence(1.0);
			wiselib::Vec<Arithmatic> v(17, 2.5, 0);

#ifdef SHAWN
			shawn::Vec v2(17, 2.5, 0);
			node_->set_real_position(v2);
#endif
			shared_data_.set_position(v);

		}

		else if ((radio_->id() == 0x96F0) || (radio_->id() == 7)) {

					shared_data_.set_anchor(true);
					shared_data_.set_confidence(1.0);
					wiselib::Vec<Arithmatic> v(28.5, 10.5, 0);

		#ifdef SHAWN
					shawn::Vec v2(28.5, 10.5, 0);
					node_->set_real_position(v2);
		#endif
					shared_data_.set_position(v);

				}


		else if ((radio_->id() == 0x5980) || (radio_->id() == 8)) {

							shared_data_.set_anchor(true);
							shared_data_.set_confidence(1.0);
							wiselib::Vec<Arithmatic> v(14.5, 12, 0);

				#ifdef SHAWN
							shawn::Vec v2(14.5, 12, 0);
							node_->set_real_position(v2);
				#endif
							shared_data_.set_position(v);
*/
//end if DVHOP and Lateration
						}

		//Nicht lokaliesierte knoten

		else {
#ifdef SHAWN
			node_->add_tag(new shawn::DoubleTag("VisBattery", -1));
#endif
			shared_data_.set_anchor(false);
			shared_data_.set_confidence(0.1);
			shared_data_.set_position(Os::Position::UNKNOWN_POSITION);

		}

#ifdef SHAWN
		if (shared_data_.is_anchor()) {
			node_->add_tag(new shawn::DoubleTag("VisBattery", 0));
		}
#endif
#ifndef SHAWN
		//SUMDIST
		//minmax
//		shared_data_.set_idle_time(9000);
		//Lateration
		shared_data_.set_idle_time(20000);//in test

		//DVHOP
		//shared_data_.set_idle_time(5);

#else
		shared_data_.set_idle_time(5);
#endif
		shared_data_.set_floodlimit(4);
		shared_data_.set_communication_range(100);
		shared_data_.set_check_residue(false);
		shared_data_.reset_neighborhood_();
	}
	// --------------------------------------------------------------------
	void evaluate(void*) {

/*if(radio_->id()==GATEWAY_RADIO_ID)
		debug_->debug(" ------------------");*/


		if (radio_->id() != GATEWAY_RADIO_ID) {

			if (!shared_data_.is_anchor()) {

			const uint16_t debug_message_length = sizeof(uint8_t) + sizeof(Arithmatic) + sizeof(Arithmatic) + sizeof(uint16_t) + sizeof(uint16_t);
			uint8_t debug_message_[debug_message_length] = { 0 };
			uint8_t buffer = 0;
			debug_message_[buffer] = DEBUG_MESSAGE_ID;
			buffer += sizeof(uint8_t);
			Arithmatic x_pos = shared_data_.position().x();
			memcpy(&(debug_message_[buffer]), &x_pos, sizeof(Arithmatic));
			buffer += sizeof(Arithmatic);
			Arithmatic y_pos = shared_data_.position().y();
			memcpy(&(debug_message_[buffer]), &y_pos, sizeof(Arithmatic));
			buffer += sizeof(Arithmatic);
			memcpy(&(debug_message_[buffer]), &round_, sizeof(uint16_t));
			buffer += sizeof(uint16_t);
			uint16_t send_id = radio_->id();
			memcpy(&(debug_message_[buffer]), &send_id, sizeof(uint16_t));
			buffer += sizeof(uint16_t);

			//send an gateway
			//tree_routing.send(GATEWAY_RADIO_ID, debug_message_length, (Os::ExtendedRadio::block_data_t*) debug_message_);
			//radio_->send(GATEWAY_RADIO_ID, debug_message_length, (Os::ExtendedRadio::block_data_t*) debug_message_);
			//if(radio_->id()==0x9999)

				debug_->debug("%d, %x, %f, %f;\n",round_, radio_->id(), x_pos, y_pos);

			round_++;

			}

		}

/*

		//dv_hop_module_.rollback();
		sum_dist_module_.rollback();
		//euclidean_module_.rollback();
		//
#ifdef LATERATION
		lateration_module_.rollback();
#else
		min_max_module_.rollback();
#endif



*/
		//configure_localization_algorithm();
		//localization_.roll_back();


		/*uint8_t delay = 1;
			if((round_%2)==0)
				delay = -1;*/
		//timer_->set_timer<LocalizationTestApplication, &LocalizationTestApplication::start_localization_algorithm> (24000 /*+ ((radio_->id())%10)*delay*/, this, 0);
		//timer_->set_timer<LocalizationTestApplication, &LocalizationTestApplication::evaluate> (24000 /*+ ((radio_->id())%10)*delay*/, this, 0);

		timer_->set_timer<LocalizationTestApplication, &LocalizationTestApplication::evaluate> (24000, this, 0);
	}

#ifdef USE_TREEROUTING
	void receive_tree_message(Os::ExtendedRadio::node_id_t from, Os::ExtendedRadio::size_t len, Os::ExtendedRadio::block_data_t *buf) {



		if ((buf[0] == DEBUG_MESSAGE_ID) && (radio_->id() == GATEWAY_RADIO_ID)) {

			uint8_t buffer = 0;
			buffer += sizeof(uint8_t);
			Arithmatic pos_x = 0;
			memcpy(&pos_x, &(buf[buffer]), sizeof(Arithmatic));
			buffer += sizeof(Arithmatic);
			Arithmatic pos_y = 0;
			memcpy(&pos_y, &(buf[buffer]), sizeof(Arithmatic));
			buffer += sizeof(Arithmatic);
			uint16_t round = 0;
			memcpy(&round, &(buf[buffer]), sizeof(uint16_t));
			buffer += sizeof(uint16_t);
			uint16_t node_id = 0;
			memcpy(&(node_id), &(buf[buffer]), sizeof(uint16_t));
			buffer += sizeof(uint16_t);

			debug_->debug("Message Received from %x at round %d est position (%f,%f) \n", node_id, round, pos_x, pos_y);



		}
	}
#endif

	/*void receive(Os::ExtendedRadio::node_id_t from, Os::ExtendedRadio::size_t len, Os::ExtendedRadio::block_data_t *buf) {



	 if ((const uint8_t)buf[0] == RESTART_MESSAGE_ID) {
	 round_ = 0;
	 start_localization_algorithm(0);

	 } else

	 if ((buf[0] == DEBUG_MESSAGE_ID)&& (radio_->id() == GATEWAY_RADIO_ID)) {

	 uint8_t buffer = 0;
	 buffer += sizeof(uint8_t);
	 Arithmatic pos_x = 0;
	 memcpy(&pos_x, &(buf[buffer]), sizeof(Arithmatic));
	 buffer += sizeof(Arithmatic);
	 Arithmatic pos_y = 0;
	 memcpy(&pos_y, &(buf[buffer]), sizeof(Arithmatic));
	 buffer += sizeof(Arithmatic);
	 uint16_t round = 0;
	 memcpy(&round, &(buf[buffer]), sizeof(uint16_t));
	 buffer += sizeof(uint16_t);
	 uint16_t node_id = 0;
	 memcpy(&(node_id), &(buf[buffer]), sizeof(uint16_t));
	 buffer += sizeof(uint16_t);

	 debug_->debug("Message Received from %x at round %d est position (%f,%f) \n", node_id, round, pos_x, pos_y);

	 }
	 }*/

private:
	LocalizationBase localization_;
	SharedData shared_data_;
	NopModule nop_module_;

	DvHopModule dv_hop_module_;
 	SumDistModule sum_dist_module_;
	//EuclideanModule euclidean_module_;

#ifdef LATERATION
   LaterationModule lateration_module_;
#else
	MinMaxModule min_max_module_;
#endif

	Os::ExtendedRadio::self_pointer_t radio_;
	Os::Timer::self_pointer_t timer_;
	Os::Debug::self_pointer_t debug_;
	Os::Position::self_pointer_t position_;
	Os::Clock::self_pointer_t clock_;
#ifdef SHAWN
	Os::Distance::self_pointer_t distance_;
#endif

#ifdef USE_TREEROUTING
	tree_routing_t tree_routing;
#endif
	int start_time;
	uint16_t round_;
#ifndef SHAWN
	Distance distance_;
	LQIMap lqi_distance_map;
#endif
#ifdef SHAWN

	shawn::Node *node_;

#endif

};
// --------------------------------------------------------------------------
wiselib::WiselibApplication<Os, LocalizationTestApplication> loc_test;
// --------------------------------------------------------------------------
void application_main(Os::AppMainParameter& value) {
	loc_test.init(value);
}
