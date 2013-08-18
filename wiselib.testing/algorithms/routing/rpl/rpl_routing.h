/***************************************************************************
 ** This file is part of the generic algorithm library Wiselib.           **
 ** Copyright (C) 2008,2009 by the Wisebed (www.wisebed.eu) project.      **
 **                                                                       **
 ** The Wiselib is free software: you can redistribute it and/or modify   **
 ** it under the terms of the GNU Lesser General Public License as        **
 ** published by the Free Software Foundation, either version 3 of the    **
 ** License, or (at your option) any later version.                       **
 **                                                                       **
 ** The Wiselib is distributed in the hope that it will be useful,        **
 ** but WITHOUT ANY WARRANTY; without even the implied warranty of        **
 ** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         **
 ** GNU Lesser General Public License for more details.                   **
 **                                                                       **
 ** You should have received a copy of the GNU Lesser General Public      **
 ** License along with the Wiselib.                                       **
 ** If not, see <http://www.gnu.org/licenses/>.                           **
 ***************************************************************************/
/*
* File: rpl_routing.h
* Author: Daniele Stirpe - Master Thesis
*/

#ifndef __ALGORITHMS_ROUTING_RPLROUTING_H__
#define __ALGORITHMS_ROUTING_RPLROUTING_H__

#include "util/base_classes/routing_base.h"
#include "algorithms/6lowpan/ipv6_packet_pool_manager.h"
#include "algorithms/routing/rpl/etx_computation.h"
#include "algorithms/routing/rpl/rpl_config.h"

#include "util/pstl/map_static_vector.h"

#include "config.h"

#define DEFAULT_DIO_INTERVAL_MIN 3	//Imin    2^3 (ms)
#define DEFAULT_DIO_INTERVAL_DOUBLINGS 20	//Imax   (2^20)*Imin (ms) c.ca 2,3 hours
#define DEFAULT_DIO_REDUNDANCY_CONSTANT 10	//k

#define DEFAULT_DAO_DELAY 1000

#define NO_PATH_DAO_COUNT 3


//Rank Constants
#define DEFAULT_MIN_HOP_RANK_INCREASE 256
#define DEFAULT_STEP_OF_RANK 3
#define DEFAULT_RANK_FACTOR 1
#define INFINITE_RANK 0xFFFF

//MRHOF Constants
#define MAX_LINK_METRIC 512
#define MAX_PATH_COST 32768
#define PARENT_SWITCH_THRESHOLD 256   //was 192
#define PARENT_SET_SIZE 5    //max sixe = PARENT_SET_SIZE( 3, according to RFC6719 ) + (PARENT_SET_SIZE - 1) = 5 (i.e. MAX Parent Set Size)
#define ALLOW_FLOATING_ROOT 0


#define DODAG_REPAIR_THRESHOLD 30

namespace wiselib
{

	/**
	 * \brief RPL routing implementation of \ref routing_concept "Routing Concept".
	 *
	 *  \ingroup routing_concept
	 *  \ingroup radio_concept
   	 *  \ingroup basic_algorithm_concept
    	 *  \ingroup routing_algorithm
   	 *
	 * RPL routing implementation of \ref routing_concept "Routing Concept" ...
	*/
   
	template<typename OsModel_P,
		typename Radio_IP_P,
		typename Radio_P,
		typename Debug_P,
		typename Timer_P/*,
		typename Clock_P*/>
	class RPLRouting
		: public RoutingBase<OsModel_P, Radio_IP_P>
	{
	public:
		typedef OsModel_P OsModel;
		typedef Radio_IP_P Radio_IP;
		typedef Radio_P Radio;
		typedef Debug_P Debug;
		typedef Timer_P Timer;
		//typedef Clock_P Clock;

		typedef RPLRouting<OsModel, Radio_IP, Radio, Debug, Timer/*, Clock*/> self_type;
		typedef self_type* self_pointer_t;

		typedef wiselib::IPv6PacketPoolManager<OsModel, Radio, Debug> Packet_Pool_Mgr_t;
		typedef typename Packet_Pool_Mgr_t::Packet IPv6Packet_t;

		typedef typename Radio_IP::node_id_t node_id_t;
		typedef typename Radio_IP::size_t size_t;
		typedef typename Radio_IP::block_data_t block_data_t;
		typedef typename Radio_IP::message_id_t message_id_t;

		//typedef typename Clock::time_t time_t;
		
		typedef typename Radio::node_id_t link_layer_node_id_t;

		typedef typename Timer::millis_t millis_t;

		struct Mapped_erase_node
		{
			node_id_t node;
		};

		typedef vector_static<OsModel, Mapped_erase_node, PARENT_SET_SIZE> Erase_parent_list;
		typedef typename wiselib::vector_static<OsModel, Mapped_erase_node, PARENT_SET_SIZE>::iterator Erase_parent_list_iterator;
		

		typedef MapStaticVector<OsModel, node_id_t, uint16_t, 8> NeighborSet; 
		typedef wiselib::pair<node_id_t, uint16_t> neigh_pair_t;
		typedef typename wiselib::MapStaticVector<OsModel , node_id_t, uint16_t, 8>::iterator NeighborSet_iterator;
		
		struct Mapped_parent_set
		{
			uint8_t current_version;
			uint8_t grounded;
			uint8_t metric_type;
			uint8_t dtsn;
			uint16_t rank;
			uint16_t path_cost;
		};
				
		typedef MapStaticVector<OsModel , node_id_t,  Mapped_parent_set, PARENT_SET_SIZE> ParentSet;
		typedef wiselib::pair<node_id_t,  Mapped_parent_set> pair_t;
		typedef typename wiselib::MapStaticVector<OsModel , node_id_t,  Mapped_parent_set, PARENT_SET_SIZE>::iterator ParentSet_iterator;
	
		typedef wiselib::ForwardingTableValue<Radio_IP> Forwarding_table_value;
		typedef wiselib::pair<node_id_t, Forwarding_table_value> ft_pair_t;

		typedef typename Radio_IP::Routing_t::ForwardingTable::iterator ForwardingTableIterator;

		typedef wiselib::ETX_computation<OsModel, Radio_IP, Radio, Debug, Timer> ETX_computation_t;
		typedef typename ETX_computation_t::ETX_values_iterator ETX_values_iterator;
		
		/**
		* Enumeration of the ICMPv6 message code types
		*/
		enum ICMPv6MessageCodes
		{
			/*DESTINATION_UNREACHABLE = 1,
			PACKET_TOO_BIG = 2,
			TIME_EXCEEDED = 3,
			PARAMETER_PROBLEM = 4,
			ECHO_REQUEST = 128,
			ECHO_REPLY = 129,
			ROUTER_SOLICITATION = 133,
			ROUTER_ADVERTISEMENT = 134,
			NEIGHBOR_SOLICITATION = 135,
			NEIGHBOR_ADVERTISEMENT = 136,*/
			RPL_CONTROL_MESSAGE = 155
			//DUPLICATE_ADDRESS_REQUEST = TBD4,
			//DUPLICATE_ADDRESS_CONFIRMATION = TBD5
		};

		// --------------------------------------------------------------------
		enum RPLMessageCodes
		{
			DODAG_INF_SOLICIT = 0x00,
			DODAG_INF_OBJECT = 0x01,
			DEST_ADVERT_OBJECT = 0x02,
			DEST_ADVERT_OBJECT_ACK = 0x03,
			SECURE_DODAG_INF_SOLICIT = 0x80,
			SECURE_DODAG_INF_OBJECT = 0x81,
			SECURE_DEST_ADVERT_OBJECT = 0x82,
			SECURE_DEST_ADVERT_OBJECT_ACK = 0x83,
			CONSISTENCY_CHECK = 0x8A,
			OTHERWISE = 0x07
		};
		// --------------------------------------------------------------------
		enum RPLOptions
		{
			PAD1 = 0x00,
			PADN = 0x01,
			DAG_METRIC_CONTAINER = 0x02,
			ROUTING_INFORMATION = 0x03,
			DODAG_CONFIGURATION = 0x04,
			RPL_TARGET = 0x05,
			TRANSIT_INFORMATION = 0x06,
			SOLICITED_INFORMATION = 0x07,
			PREFIX_INFORMATION = 0x08,
			RPL_TARGET_DESCRIPTOR = 0x09
		};
		// --------------------------------------------------------------------
		//OF0 (RFC 6552) , MRHOF (RFC 6719) 	
		enum ObjectiveFunctionTypes
		{
			OF0 = 0,  
			MRHOF = 1,  //Minimum Rank with Hysteresis Objective Function (only ETX avilable for the moment)
			OF2 = 2,
			OF3 = 3
		};
		
		// --------------------------------------------------------------------
		enum RoutingMetricTypes
		{
			/*
			NSA = 1,
			NODE_ENERGY = 2,
			HOP_COUNT = 3,
			LINK_THROUGHPUT = 4,
			LINK_LATENCY = 5,
			LINK_QUALITY_LEVEL = 6,
			*/			
			LINK_ETX = 7,
			//LINK_COLOR = 8
		};
		// --------------------------------------------------------------------
		enum ErrorCodes
		{
			SUCCESS = OsModel::SUCCESS,
			ERR_UNSPEC = OsModel::ERR_UNSPEC,
			ERR_NOTIMPL = OsModel::ERR_NOTIMPL,
			ERR_HOSTUNREACH = OsModel::ERR_HOSTUNREACH,
			ROUTING_CALLED = Radio_IP::ROUTING_CALLED
		};

			
		// --------------------------------------------------------------------
		///@name Construction / Destruction
		///@{debug().debug( "PARENT %s", from.get_address(str) );
		RPLRouting();
		~RPLRouting();
		///@}
		
		int init( Radio_IP& radio_ip, Radio& radio, Debug& debug, Timer& timer/*, Clock& clock*/, Packet_Pool_Mgr_t* p_mgr)
		{
			
			radio_ip_ = &radio_ip;
			radio_ = &radio;
			debug_ = &debug;
			timer_ = &timer;
			//clock_ = &clock; //uncomment if clock enabled
			packet_pool_mgr_ = p_mgr;
			etx_computation_.init( radio_ip, radio, debug, timer, *packet_pool_mgr_ );
			return SUCCESS;
		}

		inline int init();
		inline int destruct();

		///@name Routing Control
		///@{
		/** \brief Initialization/Start Routing
		 *
		 *  This methods does the initilaization that requires access to the OS
		 *  (and thus can not be done in the constructor). E.g., callbacks in 
		 *  task manager and radio are registered, and state variables regarding
		 *  acting as gateway or ordinary node are set.
		 *
		 *  At last, the network begins to build the routing tree. The gateway
		 *  periodically sends out flooding DIO messages. Every node that receives
		 *  such a message updates its parent set and then begins to send own
		 *  DIO messages.
		 *
		 *
		 *  \param os Reference to operating system
		 */
		int enable_radio( void );
		/** \brief Stop Routing
		 *
		 *  ...
		 */
		int disable_radio( void );
		/** \brief Set State
		 *
		 *  ...
		 */
		///@}

		///@name Radio Concept
		///@{
		/**
		 */
		int send( node_id_t receiver, uint16_t len, block_data_t *data );
		/** \brief Callback on Received Messages
		 *
		 *  Called if a message is received via the IP radio interface.
		 *  \sa \ref radio_concept "Radio concept"
		 */
		void receive( node_id_t from, size_t len, block_data_t *data );
		
		///@}

		///@name Methods called by Timer
		///@{
		/** \brief Periodic Tasks
		 *
		 *  This method is called periodically with intervals defined by
		 *  ::current_interval_. Each connected node (the root and nodes that have
		 *  a parent) broadcast a RPL DIO message with the Configuration Option, so that 
		 *  newly installed nodes can connect to the DODAG. If a node is not yet
		 *  connected, it prints out an appropriate debug message.
		 */
		void timer_elapsed( void *userdata );
		 ///@}

		int send_dis( node_id_t destination, uint16_t len, block_data_t *data );
 
		void dis_delay( void *userdata );
		
		void threshold_timer_elapsed( void *userdata );

		void leaf_timer_elapsed( void* userdata );

		void metric_timer_elapsed( void* userdata );
	
		void floating_timer_elapsed( void *userdata );

		void dao_timer_elapsed( void* userdata );

		void ETX_timer_elapsed( void *userdata );
		
		void no_path_timer_elapsed( void* userdata );

		void transient_parent_timer_elapsed( void* userdata );
	
		void more_dio_timer_elapsed( void* userdata );

		void delayed_restart_timer_elapsed( void* userdata );
			
		void first_dio( node_id_t from, block_data_t *data, uint16_t length );

		void set_dodag_root( bool root );

		uint8_t dio_packet_initialization( uint8_t position, bool grounded );

		uint8_t add_configuration_option( uint8_t position );

		uint8_t add_prefix_information( uint8_t position );

		void scan_configuration_option( block_data_t *data, uint16_t length_checked );

		void scan_prefix_information( block_data_t *data, uint16_t length_checked );
		
		uint8_t options_check( block_data_t *data, uint16_t length_checked, uint16_t length, node_id_t sender );
		
		uint8_t set_firsts_dio_fields( node_id_t from, block_data_t *data );
	
		void send_no_path_dao( node_id_t target );

		uint8_t start();
		
		void start2( void *userdata );

		bool is_reachable( node_id_t node );

		bool is_still_neighbor( node_id_t node );

		uint8_t prepare_dao();

		void update_dio( node_id_t parent, uint16_t path_cost );

		void find_worst_parent();

		int handle_TLV( uint8_t packet_number, uint8_t* data_pointer, bool only_usage );

		void print_parent_set();
		
		void print_neighbor_set();

		//uncomment if clock enabled
		/*
		time_t time()
		{
			return clock().time();
 		}

		uint32_t seconds( time_t t )
		{
			return clock().seconds( t );
 		}
		
		uint16_t milliseconds( time_t t )
		{
			return clock().milliseconds( t );
 		}
		*/
				
		void set_current_interval( uint8_t num )
		{				
			if (num == 0)
			{
				uint32_t t = 123456;  //comment if clock enabled
				//uint32_t t = milliseconds(time()) + 1000*seconds(time()); //uncomment if clock enabled
				current_interval_ = (t % (imax_ - imin_)) + imin_;
							
			}
			else if (num == 1)
				current_interval_ = max_interval_;
			else	
				current_interval_ = current_interval_*2;
		}
		
		
		void compute_sending_threshold()
		{
			uint32_t t = 123456; //comment if clock enabled			
			//uint32_t t = milliseconds(time()) + 1000*seconds(time()); //uncomment if clock enabled
			sending_threshold_ = ( t % current_interval_/2) + current_interval_/2;
		}

		
		uint16_t DAGRank( uint16_t rank )
		{			
			if( etx_ || ocp_ == 0 )
				return rank;
			float num = (float)(rank>>8)/(float)min_hop_rank_increase_;
			
			uint8_t int_part = (uint8_t)num;
			return int_part;
			
		}

		// -------------------------------------------------------------------------------------------------
		node_id_t my_global_address_;

	private:

		Radio_IP& radio_ip()
		{ return *radio_ip_; }		

		Radio& radio()
		{ return *radio_; }
		
		Timer& timer()
		{ return *timer_; }

		Debug& debug()
		{ return *debug_; }
		
		//uncomment if clock enabled
		/*
		Clock& clock()
        	{ return *clock_; }
		*/		

		typename Radio_IP::self_pointer_t radio_ip_;

		typename Radio::self_pointer_t radio_;
		typename Timer::self_pointer_t timer_;
		
		typename Debug::self_pointer_t debug_;
		//typename Clock::self_pointer_t clock_; //uncomment if clock enabled
				
		Packet_Pool_Mgr_t* packet_pool_mgr_;

		/**
		* Callback ID
		*/
		uint8_t callback_id_;
		
		uint8_t TLV_callback_id_;

		//To update
		enum RPLRoutingState
		{
			Dodag_root,
			Floating_Dodag_root,
			Unconnected,
			Connected, 
			Router,
			Leaf 
		};
		
		uint16_t ocp_;
		
		NeighborSet neighbor_set_;

		ETX_computation_t etx_computation_;

		Erase_parent_list erase_parent_list_;

		ParentSet parent_set_;

		IPv6Packet_t* dio_message_;
		IPv6Packet_t* dis_message_;
		IPv6Packet_t* dao_message_;
		IPv6Packet_t* no_path_dao_;
		
		link_layer_node_id_t my_link_layer_address_;		
	
		node_id_t my_address_; 
		

		node_id_t dodag_id_;

		node_id_t preferred_parent_;

		node_id_t transient_preferred_parent_;

		node_id_t old_preferred_parent_;

		node_id_t worst_parent_;
		 
		uint8_t dtsn_;

		bool stop_dio_timer_;

		bool stop_dao_timer_;

		uint8_t count_timer_;

		uint8_t no_path_count_;

		//timer variables
		uint32_t current_interval_;
		uint32_t sending_threshold_;

		uint16_t step_of_rank_;
		uint16_t rank_factor_;
		uint16_t rank_stretch_;
		uint16_t min_hop_rank_increase_;
		uint16_t DAGMaxRankIncrease_;
		uint16_t rank_;

		uint8_t mop_;
		bool mop_set_;

		uint8_t bcast_neigh_count_;

		bool etx_;

		bool prefix_present_;

		bool dao_ack_received_;

		bool no_path_ack_received_;

		bool neighbors_found_;

		bool dao_received_;

		uint8_t dis_count_;

		uint16_t cur_min_path_cost_; 
						
		uint8_t rpl_instance_id_; 
		uint8_t version_number_;
		uint8_t version_last_time_;

		RPLRoutingState state_;

		uint8_t dio_int_min_;
		uint8_t imin_; 
		uint8_t imax_; 
		
		uint8_t dio_count_;

		uint8_t more_dio_count_;

		uint8_t dao_sequence_;
		uint8_t path_sequence_;
				
		uint32_t max_interval_;
		uint8_t dio_redund_const_;

		uint8_t dio_reference_number_;
		uint8_t dis_reference_number_;
		uint8_t dao_reference_number_;
		uint8_t no_path_reference_number_;
		
	};
	// -----------------------------------------------------------------------
	
	// -----------------------------------------------------------------------
	template<typename OsModel_P,
		typename Radio_IP_P,
		typename Radio_P,
		typename Debug_P,
		typename Timer_P/*,
		typename Clock_P*/>
	RPLRouting<OsModel_P, Radio_IP_P, Radio_P, Debug_P, Timer_P/*, Clock_P*/>::
	RPLRouting()
		: rpl_instance_id_ (1),
		etx_ (true),
		rank_ (INFINITE_RANK),
		dao_ack_received_ (false),
		no_path_ack_received_ (false),
		neighbors_found_ (false),
		dao_received_ (false),
		state_ (Unconnected),
		dio_count_ (0),
		more_dio_count_ (2),
		dis_count_ (0),
		bcast_neigh_count_ (0),
		no_path_count_ (0),
		dtsn_ (0),
		dao_sequence_ (0),
		path_sequence_ (0),
		version_last_time_ (1),
		stop_dio_timer_ (false),
		stop_dao_timer_ (false),
		prefix_present_ (false),
		count_timer_ (0),
		mop_set_ (true),
		step_of_rank_ (DEFAULT_STEP_OF_RANK),
		rank_factor_ (DEFAULT_RANK_FACTOR),
		rank_stretch_ (0),
		dio_int_min_ (DEFAULT_DIO_INTERVAL_MIN),
		imax_ (DEFAULT_DIO_INTERVAL_DOUBLINGS),
		dio_redund_const_ (DEFAULT_DIO_REDUNDANCY_CONSTANT),
		min_hop_rank_increase_ (DEFAULT_MIN_HOP_RANK_INCREASE),
		DAGMaxRankIncrease_ ( 0 ), //0 means disabled
		preferred_parent_ ( Radio_IP::NULL_NODE_ID ),
		old_preferred_parent_ ( Radio_IP::NULL_NODE_ID ),
		transient_preferred_parent_ ( Radio_IP::NULL_NODE_ID ),
		worst_parent_ ( Radio_IP::NULL_NODE_ID ),
		cur_min_path_cost_ (0xFFFF)
	{}
	// -----------------------------------------------------------------------
	template<typename OsModel_P,
		typename Radio_IP_P,
		typename Radio_P,
		typename Debug_P,
		typename Timer_P/*,
		typename Clock_P*/>
	RPLRouting<OsModel_P, Radio_IP_P, Radio_P, Debug_P, Timer_P/*, Clock_P*/>::
	~RPLRouting()
	{
		#ifdef ROUTING_RPL_DEBUG
		debug().debug( "RPLRouting: Destroyed\n" );
		#endif
		
	}
	// -----------------------------------------------------------------------
	template<typename OsModel_P,
		typename Radio_IP_P,
		typename Radio_P,
		typename Debug_P,
		typename Timer_P/*,
		typename Clock_P*/>
	int
	RPLRouting<OsModel_P, Radio_IP_P, Radio_P, Debug_P, Timer_P/*, Clock_P*/>::
	destruct( void )
	{
		packet_pool_mgr_->clean_packet_with_number( dis_reference_number_ );
		packet_pool_mgr_->clean_packet_with_number( dio_reference_number_ );
		packet_pool_mgr_->clean_packet_with_number( dao_reference_number_ );
		packet_pool_mgr_->clean_packet_with_number( no_path_reference_number_ );
		stop_dio_timer_ = true;
		stop_dao_timer_ = true;
		return disable_radio();
		
	}
	// -----------------------------------------------------------------------
	template<typename OsModel_P,
		typename Radio_IP_P,
		typename Radio_P,
		typename Debug_P,
		typename Timer_P/*,
		typename Clock_P*/>
	int
	RPLRouting<OsModel_P, Radio_IP_P, Radio_P, Debug_P, Timer_P/*, Clock_P*/>::
	enable_radio( void )
	{
		#ifdef ROUTING_RPL_DEBUGS
		debug().debug( "\nRPL: Initialization\n" );
		#endif
			
		dio_reference_number_ = packet_pool_mgr_->get_unused_packet_with_number();
		if( dio_reference_number_ == Packet_Pool_Mgr_t::NO_FREE_PACKET )
		{
			#ifdef ROUTING_RPL_DEBUG
			debug().debug( "RPLRouting: NO FREE PACKET DIO\n" );
			#endif
			return ERR_UNSPEC;
		}
		dis_reference_number_ = packet_pool_mgr_->get_unused_packet_with_number();
		if( dis_reference_number_ == Packet_Pool_Mgr_t::NO_FREE_PACKET )
		{
			#ifdef ROUTING_RPL_DEBUG
			debug().debug( "RPLRouting: NO FREE PACKET DIS\n" );
			#endif
			return ERR_UNSPEC;
		}

		dao_reference_number_ = packet_pool_mgr_->get_unused_packet_with_number();
		if( dao_reference_number_ == Packet_Pool_Mgr_t::NO_FREE_PACKET )
		{
			#ifdef ROUTING_RPL_DEBUG
			debug().debug( "RPLRouting: NO FREE PACKET DAO\n" );
			#endif
			return ERR_UNSPEC;
		}

		no_path_reference_number_ = packet_pool_mgr_->get_unused_packet_with_number();
		if( no_path_reference_number_ == Packet_Pool_Mgr_t::NO_FREE_PACKET )
		{
			#ifdef ROUTING_RPL_DEBUG
			debug().debug( "RPLRouting: NO FREE PACKET NO_PATH\n" );
			#endif
			return ERR_UNSPEC;
		}

		dio_message_ = packet_pool_mgr_->get_packet_pointer( dio_reference_number_ );
		dis_message_ = packet_pool_mgr_->get_packet_pointer( dis_reference_number_ );
		dao_message_ = packet_pool_mgr_->get_packet_pointer( dao_reference_number_ );
		no_path_dao_ = packet_pool_mgr_->get_packet_pointer( no_path_reference_number_ );
		
		callback_id_ = radio_ip().template reg_recv_callback<self_type, &self_type::receive>( this );

		TLV_callback_id_ = radio_ip().template HOHO_reg_recv_callback<self_type, &self_type::handle_TLV>( this, 63, 4 );
		
		my_link_layer_address_ = radio().id();

		my_address_ = radio_ip().id();

		#ifdef ETX_METRIC
		etx_computation_.start();
		#endif
				
		return SUCCESS;
	}
	// -----------------------------------------------------------------------
	template<typename OsModel_P,
		typename Radio_IP_P,
		typename Radio_P,
		typename Debug_P,
		typename Timer_P/*,
		typename Clock_P*/>
	int
	RPLRouting<OsModel_P, Radio_IP_P, Radio_P, Debug_P, Timer_P/*, Clock_P*/>::
	disable_radio( void )
	{
		
		#ifdef ROUTING_RPL_DEBUG
		debug().debug( "RPLRouting: Should stop routing now...\n" );
		#endif

		if( radio_ip().disable_radio() != SUCCESS )
			return ERR_UNSPEC;
		radio_ip().template unreg_recv_callback(callback_id_);

		radio_ip().template unreg_recv_callback(TLV_callback_id_);

		#ifdef ETX_METRIC
		etx_computation_.stop();
		#endif

		return SUCCESS;
	}
	// -----------------------------------------------------------------------
	template<typename OsModel_P,
		typename Radio_IP_P,
		typename Radio_P,
		typename Debug_P,
		typename Timer_P/*,
		typename Clock_P*/>
	void
	RPLRouting<OsModel_P, Radio_IP_P, Radio_P, Debug_P, Timer_P/*, Clock_P*/>::
	set_dodag_root( bool root )
	{
		if ( root )
		{
			//to replace with the actual global prefix
			uint8_t global_prefix[8];
			global_prefix[0]=0xAA;
			global_prefix[1]=0xAA;
			memset(&(global_prefix[2]),0, 6);
						
			my_global_address_.set_prefix(global_prefix);
			my_global_address_.prefix_length = 64;

			my_global_address_.set_long_iid( &my_link_layer_address_, true );
			
			radio_ip().interface_manager_->set_prefix_for_interface( my_global_address_.addr ,0 ,64 );

			prefix_present_ = true;

			state_ = Dodag_root;
			
			#ifdef ETX_METRIC
			min_hop_rank_increase_ = 128;
			#endif	
					
			rank_ = min_hop_rank_increase_; //RFC6550 (pag.112 Section 17)

			#ifdef ROUTING_RPL_DEBUG
			debug().debug( "RPLRouting: Root, set rank:  %i\n", rank_ );
			#endif
			
			#ifdef OF0_ACTIVATED
			ocp_ = OF0;
			#endif
			#ifdef MRHOF_ACTIVATED
			ocp_ = MRHOF;
			#endif
			#ifdef STORING_MODE
			mop_ = 2;
			#endif
			#ifdef NONSTORING_MODE
			mop_ = 1
			#endif		
		}
		else
			state_ = Unconnected;
	}
	
	// -----------------------------------------------------------------------
	template<typename OsModel_P,
		typename Radio_IP_P,
		typename Radio_P,
		typename Debug_P,
		typename Timer_P/*,
		typename Clock_P*/>
	uint8_t
	RPLRouting<OsModel_P, Radio_IP_P, Radio_P, Debug_P, Timer_P/*, Clock_P*/>::
	start( void )
	{
		//NB: the set_payload function starts to fill the packet fields from the 40th byte (the ICMP header)
		uint8_t setter_byte = RPL_CONTROL_MESSAGE;
		dio_message_->template set_payload<uint8_t>( &setter_byte, 0, 1 );
		dis_message_->template set_payload<uint8_t>( &setter_byte, 0, 1 );
		dao_message_->template set_payload<uint8_t>( &setter_byte, 0, 1 );
		no_path_dao_->template set_payload<uint8_t>( &setter_byte, 0, 1 );
		setter_byte = DODAG_INF_OBJECT;
		dio_message_->template set_payload<uint8_t>( &setter_byte, 1, 1 );
		setter_byte = DODAG_INF_SOLICIT;
		dis_message_->template set_payload<uint8_t>( &setter_byte, 1, 1 );
		setter_byte = DEST_ADVERT_OBJECT;
		dao_message_->template set_payload<uint8_t>( &setter_byte, 1, 1 );
		no_path_dao_->template set_payload<uint8_t>( &setter_byte, 1, 1 );
		
		stop_dio_timer_ = false;
		dao_ack_received_ = false;
		no_path_ack_received_ = false;
		neighbors_found_ = false;
		dao_received_ = false;
		dis_count_ = 0;
			
		timer().template set_timer<self_type, &self_type::start2>( 20000, this, 0 );
		return SUCCESS;
	}

	// -----------------------------------------------------------------------
	template<typename OsModel_P,
		typename Radio_IP_P,
		typename Radio_P,
		typename Debug_P,
		typename Timer_P/*,
		typename Clock_P*/>
	void
	RPLRouting<OsModel_P, Radio_IP_P, Radio_P, Debug_P, Timer_P/*, Clock_P*/>::
	start2( void* userdata )
	{	
		for (ETX_values_iterator it = etx_computation_.etx_values_.begin(); it != etx_computation_.etx_values_.end(); it++) 
		{
			float forward = ((float)it->second.forward)/etx_computation_.timer_count_;
			float reverse = ((float)it->second.reverse)/etx_computation_.timer_count_;
			
			uint16_t current_metric = (uint16_t)( min_hop_rank_increase_ * (1/(forward * reverse)) );

			uint16_t remainder = (uint16_t)current_metric % 128;
			if( remainder != 0 )
			{
				//fix the approximation error
				if( remainder > 63 )
					current_metric = current_metric + (128 - remainder);
				else
					current_metric = current_metric - remainder;
			}
			
			if( current_metric != 0 )
				neighbor_set_.insert( neigh_pair_t( it->first, current_metric ) );
		}
		#ifdef ROUTING_RPL_DEBUG
		print_neighbor_set();
		#endif

		//update metric values
		timer().template set_timer<self_type, &self_type::metric_timer_elapsed>( 4000, this, 0 );
			
		
		if ( state_ == Dodag_root )
		{		
			stop_dao_timer_ = true;
			version_number_ = 1;
			imin_ = 2 << (dio_int_min_ - 1);
			//imax_ = DEFAULT_DIO_INTERVAL_DOUBLINGS;
			max_interval_ = (2 << (imax_ - 1)) * imin_;
			
			dodag_id_ = my_global_address_;

			preferred_parent_ = my_address_;
			
			uint8_t dio_current_position = 4;
						
			dio_current_position = dio_packet_initialization( dio_current_position, true );

			dio_current_position = add_configuration_option ( dio_current_position );

			dio_current_position = add_prefix_information ( dio_current_position );
				
			if (ocp_ != 0 && !etx_ )
			{
				//-----------------------------FILLING THE OPTIONS-----------------------------------------
				// TO USE IF FURTHER METRICS ARE CONSIDERED
				//NB: HOP COUNT Constraint and Metric must be placed in different Containers (RFC 6551)
			}
			
			dio_message_->set_transport_length( dio_current_position ); 
			
			set_current_interval(0);
			compute_sending_threshold();

			#ifdef ROUTING_RPL_DEBUG
			debug().debug( "\nRPL Routing: Start as root/gateway\n" );
			#endif

			timer().template set_timer<self_type, &self_type::timer_elapsed>( current_interval_, this, 0 );
					
			timer().template set_timer<self_type, &self_type::threshold_timer_elapsed>( sending_threshold_, this, 0 );
						
		}
		else
		{
			dodag_id_ = Radio_IP::NULL_NODE_ID;
			preferred_parent_ = Radio_IP::NULL_NODE_ID;
			
			uint8_t setter_byte = 0;
			dis_message_->template set_payload<uint8_t>( &setter_byte, 4, 1 );
			dis_message_->template set_payload<uint8_t>( &setter_byte, 5, 1 );
			dis_message_->set_transport_length( 6 );
			//timer().template set_timer<self_type, &self_type::dis_delay>( 10000, this, 0 ); //commented.. UNCOMMENT to create floating DODAGS	
			#ifdef ROUTING_RPL_DEBUG
			debug().debug( "RPLRouting: Start as ordinary node\n" );
			#endif
		}
		
		
	}
		
	// -----------------------------------------------------------------------
	template<typename OsModel_P,
		typename Radio_IP_P,
		typename Radio_P,
		typename Debug_P,
		typename Timer_P/*,
		typename Clock_P*/>
	int
	RPLRouting<OsModel_P, Radio_IP_P, Radio_P, Debug_P, Timer_P/*, Clock_P*/>::
	send_dis( node_id_t destination, uint16_t len, block_data_t *data )   
	{
		dis_message_->set_transport_next_header( Radio_IP::ICMPV6 );
		dis_message_->set_hop_limit(255);
		
		dis_message_->set_source_address(my_address_);

		dis_message_->set_destination_address(destination);
		dis_message_->set_flow_label(0);
		dis_message_->set_traffic_class(0);

		radio_ip().send( destination, len, data );
	
		return SUCCESS;
	}
	// -----------------------------------------------------------------------
	template<typename OsModel_P,
		typename Radio_IP_P,
		typename Radio_P,
		typename Debug_P,
		typename Timer_P/*,
		typename Clock_P*/>
	int
	RPLRouting<OsModel_P, Radio_IP_P, Radio_P, Debug_P, Timer_P/*, Clock_P*/>::
	send( node_id_t destination, uint16_t len, block_data_t *data )
	{
		//mainly used to forward DAO messages up the DODAG
		IPv6Packet_t* message = packet_pool_mgr_->get_packet_pointer( len );
		
		message->set_destination_address(destination);
		data = message->payload();
				
		uint8_t result = radio_ip().send( destination, len, data );
		
		if( result != ROUTING_CALLED )
			//packet_pool_mgr_->clean_packet( message );
		return result;
	}
	// -----------------------------------------------------------------------
	
	template<typename OsModel_P,
		typename Radio_IP_P,
		typename Radio_P,
		typename Debug_P,
		typename Timer_P/*,
		typename Clock_P*/>
	void
	RPLRouting<OsModel_P, Radio_IP_P, Radio_P, Debug_P, Timer_P/*, Clock_P*/>::
	dis_delay( void* userdata )
	{
		char str[43];
		if ( state_ == Unconnected )
		{
			//Unicast to a potential DODAG parent, see neighbor_set
			node_id_t dest = Radio_IP::NULL_NODE_ID;
		
			for (ETX_values_iterator it = etx_computation_.etx_values_.begin(); it != etx_computation_.etx_values_.end(); it++) 
			{
				//Check bidirectionality
				if( it->second.forward != 0 && it->second.reverse != 0 )
				{
					dest = it->first;
					break;
				}
				else
				{
					#ifdef ROUTING_RPL_DEBUG
					debug().debug( "\nRPLRouting: No bidirect connection to %s\n", it->first.get_address(str) );
					#endif
				}
			}
			if( dest != Radio_IP::NULL_NODE_ID )
			{
				#ifdef ROUTING_RPL_DEBUG
				debug().debug( "\nRPLRouting: This node seems isolated, try to send a Solicitation to neighbor %s\n", dest.get_address(str) );
				#endif
				send_dis( dest, dis_reference_number_, NULL );
				
				timer().template set_timer<self_type, &self_type::floating_timer_elapsed>( 9000, this, 0 );
			
			}
			else
			{
				#ifdef ROUTING_RPL_DEBUG
				debug().debug( "\nRPLRouting: This node seems isolated, find neighbors again\n" );
				#endif
				neighbors_found_ = false;
				start();
			}
		}
	}
	// -----------------------------------------------------------------------
	template<typename OsModel_P,
		typename Radio_IP_P,
		typename Radio_P,
		typename Debug_P,
		typename Timer_P/*,
		typename Clock_P*/>
	void
	RPLRouting<OsModel_P, Radio_IP_P, Radio_P, Debug_P, Timer_P/*, Clock_P*/>::
	timer_elapsed( void* userdata )
	{
		dio_count_ = 0;
		if( !stop_dio_timer_ && state_ != Leaf )
		{
		
			if ( 2 * current_interval_ < max_interval_ )
				set_current_interval(2);
			else
				set_current_interval(1); 

			compute_sending_threshold();
				
			timer().template set_timer<self_type, &self_type::timer_elapsed>( current_interval_, this, 0 );
			
			timer().template set_timer<self_type, &self_type::threshold_timer_elapsed>( sending_threshold_, this, 0 );	
			
			version_last_time_ = version_number_;
		}
		
		//Enter if a 'new version DIO' has been received (this happens when the timer of the new version expires before the old one)
		else if ( version_last_time_ != version_number_ && state_ != Leaf && state_ != Dodag_root )
		{
			if ( 2 * current_interval_ < max_interval_ )
				set_current_interval(2);
			else
				set_current_interval(1);
		
			compute_sending_threshold();
				
			timer().template set_timer<self_type, &self_type::timer_elapsed>( current_interval_, this, 0 );
			
			timer().template set_timer<self_type, &self_type::threshold_timer_elapsed>( sending_threshold_, this, 0 );	
			
			version_last_time_ = version_number_;
			stop_dio_timer_ = false;
		}
	}

	// -----------------------------------------------------------------------
	
	template<typename OsModel_P,
		typename Radio_IP_P,
		typename Radio_P,
		typename Debug_P,
		typename Timer_P/*,
		typename Clock_P*/>
	void
	RPLRouting<OsModel_P, Radio_IP_P, Radio_P, Debug_P, Timer_P/*, Clock_P*/>::
	more_dio_timer_elapsed( void* userdata )
	{
		if( more_dio_count_ > 0  )
		{
			radio_ip().send( Radio_IP::BROADCAST_ADDRESS, dio_reference_number_, NULL );
			timer().template set_timer<self_type, &self_type::more_dio_timer_elapsed>( 100, this, 0 );
			more_dio_count_ = more_dio_count_ - 1;
		}
		else
			more_dio_count_ = 2;
	}

	// -----------------------------------------------------------------------
	
	template<typename OsModel_P,
		typename Radio_IP_P,
		typename Radio_P,
		typename Debug_P,
		typename Timer_P/*,
		typename Clock_P*/>
	void
	RPLRouting<OsModel_P, Radio_IP_P, Radio_P, Debug_P, Timer_P/*, Clock_P*/>::
	metric_timer_elapsed( void* userdata )
	{
		
		for (ETX_values_iterator it = etx_computation_.etx_values_.begin(); it != etx_computation_.etx_values_.end(); it++) 
		{
			if( it->second.forward != 0 || it->second.reverse != 0 )
			{

				float forward = ((float)it->second.forward)/etx_computation_.timer_count_;
				float reverse = ((float)it->second.reverse)/etx_computation_.timer_count_;
				uint16_t current_metric = (uint16_t)( min_hop_rank_increase_ * (1/(forward * reverse)) );
				uint16_t remainder = (uint16_t)current_metric % 128;
				if( remainder != 0 )
				{
					//fix the approximation error
					if( remainder > 63 )
						current_metric = current_metric + (128 - remainder);
					else
						current_metric = current_metric - remainder;
				}

				NeighborSet_iterator neigh_it = neighbor_set_.find( it->first );
				
				if( neigh_it == neighbor_set_.end() )
					neighbor_set_.insert( neigh_pair_t( it->first, current_metric ) );
				else
					neigh_it->second = current_metric;				

			}
			else
				neighbor_set_.erase( it->first );
		}
		timer().template set_timer<self_type, &self_type::metric_timer_elapsed>( 5000, this, 0 );
	}

	// -----------------------------------------------------------------------
	
	template<typename OsModel_P,
		typename Radio_IP_P,
		typename Radio_P,
		typename Debug_P,
		typename Timer_P/*,
		typename Clock_P*/>
	void
	RPLRouting<OsModel_P, Radio_IP_P, Radio_P, Debug_P, Timer_P/*, Clock_P*/>::
	delayed_restart_timer_elapsed( void* userdata )
	{
		timer().template set_timer<self_type, &self_type::more_dio_timer_elapsed>( 300, this, 0 );
								
		//reset timer
		set_current_interval(0);
		compute_sending_threshold();
		if( state_ == Leaf )
		{
			dao_received_ = false;
			state_ = Connected;
			timer().template set_timer<self_type, &self_type::leaf_timer_elapsed>(  current_interval_ + 2500, this, 0 );
			timer().template set_timer<self_type, &self_type::timer_elapsed>( current_interval_, this, 0 );
			timer().template set_timer<self_type, &self_type::threshold_timer_elapsed>( sending_threshold_, this, 0 );
		}
	}

	// -----------------------------------------------------------------------
	
	template<typename OsModel_P,
		typename Radio_IP_P,
		typename Radio_P,
		typename Debug_P,
		typename Timer_P/*,
		typename Clock_P*/>
	void
	RPLRouting<OsModel_P, Radio_IP_P, Radio_P, Debug_P, Timer_P/*, Clock_P*/>::
	no_path_timer_elapsed( void* userdata )
	{
		radio_ip().send( old_preferred_parent_, no_path_reference_number_, NULL );
		if( no_path_count_ < 3 )
		{
			timer().template set_timer<self_type, &self_type::no_path_timer_elapsed>( 500, this, 0 );
			no_path_count_ = no_path_count_ + 1;
		}
		else
			no_path_count_ = 0;
	}

	// -----------------------------------------------------------------------
	
	template<typename OsModel_P,
		typename Radio_IP_P,
		typename Radio_P,
		typename Debug_P,
		typename Timer_P/*,
		typename Clock_P*/>
	void
	RPLRouting<OsModel_P, Radio_IP_P, Radio_P, Debug_P, Timer_P/*, Clock_P*/>::
	transient_parent_timer_elapsed( void* userdata )
	{
		transient_preferred_parent_ = Radio_IP::NULL_NODE_ID;
	}

	// -----------------------------------------------------------------------
	
	template<typename OsModel_P,
		typename Radio_IP_P,
		typename Radio_P,
		typename Debug_P,
		typename Timer_P/*,
		typename Clock_P*/>
	void
	RPLRouting<OsModel_P, Radio_IP_P, Radio_P, Debug_P, Timer_P/*, Clock_P*/>::
	leaf_timer_elapsed( void* userdata )
	{
		if(state_ != Dodag_root && state_ != Router && !dao_received_ )
		{
			#ifdef ROUTING_RPL_DEBUG
			debug().debug( "\nRPL Routing: I'm a Leaf\n" );
			#endif
			
			state_ = Leaf;
		}
		else if (state_ != Dodag_root && state_ != Router )
		{
			#ifdef ROUTING_RPL_DEBUG
			debug().debug( "\nRPL Routing: I'm a Router\n" );
			#endif
			state_ = Router;
		}
	}

	// -----------------------------------------------------------------------
	
	template<typename OsModel_P,
		typename Radio_IP_P,
		typename Radio_P,
		typename Debug_P,
		typename Timer_P/*,
		typename Clock_P*/>
	void
	RPLRouting<OsModel_P, Radio_IP_P, Radio_P, Debug_P, Timer_P/*, Clock_P*/>::
	dao_timer_elapsed( void* userdata )
	{
		//this timer must be directly proportional to the rank when aggregation is not supported (rank_*something +/- something_else??) 
		if( !dao_ack_received_ && !stop_dao_timer_ )
		{
			radio_ip().send( preferred_parent_, dao_reference_number_, NULL);

			timer().template set_timer<self_type, &self_type::dao_timer_elapsed>( ( rank_ * 3 ) - 100, this, 0 );
		}
		
	}

	// -----------------------------------------------------------------------
			
	template<typename OsModel_P,
		typename Radio_IP_P,
		typename Radio_P,
		typename Debug_P,
		typename Timer_P/*,
		typename Clock_P*/>
	void
	RPLRouting<OsModel_P, Radio_IP_P, Radio_P, Debug_P, Timer_P/*, Clock_P*/>::
	threshold_timer_elapsed( void* userdata )
	{
		
		if ( dio_count_ < dio_redund_const_ )
			radio_ip().send( Radio_IP::BROADCAST_ADDRESS, dio_reference_number_, NULL );
	}
	// -----------------------------------------------------------------------
	
	template<typename OsModel_P,
		typename Radio_IP_P,
		typename Radio_P,
		typename Debug_P,
		typename Timer_P/*,
		typename Clock_P*/>
	void
	RPLRouting<OsModel_P, Radio_IP_P, Radio_P, Debug_P, Timer_P/*, Clock_P*/>::
	floating_timer_elapsed( void* userdata )
	{
		//Before creating a Floating DODAG there's the need to understand how long it takes for a DIO to reach all the network
		if ( state_ == Unconnected && ALLOW_FLOATING_ROOT != 0 )
		{
			#ifdef ROUTING_RPL_DEBUG
			debug().debug( "\nRPL Routing: CREATE FLOATING DODAG\n" );
			#endif
						
			state_ = Floating_Dodag_root;
			version_number_ = 1;
			imin_ = 2 << (dio_int_min_ - 1);
			max_interval_ = (2 << (imax_ - 1)) * imin_; 
			
			dodag_id_ = my_address_;   
			preferred_parent_ = my_address_;
			
			uint8_t dio_current_position = 4;
			//False means not grounded (i.e. floating)... 
			dio_current_position = dio_packet_initialization( dio_current_position, false );

			ocp_ = 0; 
			
			dio_current_position = add_configuration_option ( dio_current_position );

			dio_message_->set_transport_length( dio_current_position ); 
			
			//initialize timers
			set_current_interval(0);
			compute_sending_threshold();

			#ifdef ROUTING_RPL_DEBUG
			debug().debug( "\nRPL Routing: Start as floating root\n" );
			#endif
			
			timer().template set_timer<self_type, &self_type::timer_elapsed>( current_interval_, this, 0 );
					
			timer().template set_timer<self_type, &self_type::threshold_timer_elapsed>( sending_threshold_, this, 0 );
		}
	}
	
	// -----------------------------------------------------------------------
	template<typename OsModel_P,
		typename Radio_IP_P,
		typename Radio_P,
		typename Debug_P,
		typename Timer_P/*,
		typename Clock_P*/>
	void
	RPLRouting<OsModel_P, Radio_IP_P, Radio_P, Debug_P, Timer_P/*, Clock_P*/>::
	receive( node_id_t from, size_t packet_number, block_data_t *data )
	{
		char str[43];
		char str2[43];
				
		IPv6Packet_t* message = packet_pool_mgr_->get_packet_pointer( packet_number ); 
		
		node_id_t sender;
		message->source_address(sender);
		
		//If it is not an ICMPv6 packet, just return...
		if( message->transport_next_header() != Radio_IP::ICMPV6 )
		{	
			#ifdef ROUTING_RPL_DEBUG
			debug().debug( "\nRPL Routing: DROP NON-ICMP MESSAGE: %i\n", message->transport_next_header());
			#endif
			
			//Don't drop it!
			return;
		}
		
		data = message->payload();
		
		uint8_t typecode = data[0];
				
		uint16_t checksum = ( data[2] << 8 ) | data[3];
		data[2] = 0;
		data[3] = 0;

		//RECEIVED CHECKSUM IS ALWAYS 0, don't check it
		
		//need to process only type 155, ignore the others for now... so terminate if type != 155
				
		if( typecode != RPL_CONTROL_MESSAGE )
			return;
				
		typecode = data[1];

		if( typecode == OTHERWISE )
		{
			return;
		}
		
		
		if ( typecode == DODAG_INF_OBJECT && mop_set_ )
		{
			
			uint8_t mop_check = data[8];
			mop_check = ( mop_check << 2 );
			mop_check = ( mop_check >> 5 );
			mop_ = mop_check;
						
			mop_set_ = false;
			
		}
		
		if( mop_ == 0 && (typecode != DODAG_INF_OBJECT && typecode != DODAG_INF_SOLICIT) )
		{
			packet_pool_mgr_->clean_packet( message );
			return;
		}
	
		if( ( mop_ == 1 || mop_ == 2 ) && (typecode != DODAG_INF_OBJECT && typecode != DODAG_INF_SOLICIT &&
						 typecode != DEST_ADVERT_OBJECT && typecode != DEST_ADVERT_OBJECT_ACK ) )
		{
			packet_pool_mgr_->clean_packet( message );
			return;
		}
				
		if ( typecode == DODAG_INF_OBJECT )
		{
			uint16_t length = message->transport_length();
			//If a floating Dodag root receive a DIO then it may connect to the real dodag---> to update
			if ( state_ == Dodag_root || state_ == Floating_Dodag_root )
			{
				packet_pool_mgr_->clean_packet( message );
				return;
			}

			if( state_ == Unconnected)	
			{	
				#ifdef ROUTING_RPL_DEBUG
				debug().debug( "RPL Routing: State = Unconnected... calling first_dio function\n" );
				#endif
				uint16_t check_rank = ( data[6] << 8 ) | data[7];
				if( check_rank == INFINITE_RANK )
				{
					#ifdef ROUTING_RPL_DEBUG
					debug().debug( "RPL Routing: Ignore INFINITE RANK\n" );
					#endif
					packet_pool_mgr_->clean_packet( message );
					return;
				}
								
				first_dio( sender, data, length );
			}

			else if ( state_ == Connected || state_ == Router || state_ == Leaf )
			{

				uint16_t check_rank = ( data[6] << 8 ) | data[7];
				if( check_rank == INFINITE_RANK || check_rank > MAX_PATH_COST )
				{
					#ifdef ROUTING_RPL_DEBUG
					debug().debug( "\n\nRPL Routing: Received INFINITE RANK from %s\n\n", sender.get_address(str) );
					#endif
					parent_set_.erase( sender );
					//ParentSet_iterator it = parent_set_.find( sender );
					if(sender != preferred_parent_ )
					{
						packet_pool_mgr_->clean_packet( message );
						return;
					}

					else
					{
						if ( parent_set_.empty() )
						{
							//NO MORE PARENTS!
							#ifdef ROUTING_RPL_DEBUG
							debug().debug( "\n\nRPL Routing: NO More Parents, stop timers and poison Sub-DODAG\n\n" );
							#endif
							
							ForwardingTableIterator default_route = radio_ip().routing_.forwarding_table_.find( Radio_IP::NULL_NODE_ID );
							radio_ip().routing_.forwarding_table_.erase( default_route );
							rank_ = INFINITE_RANK;
							state_ = Unconnected;
							preferred_parent_ = Radio_IP::NULL_NODE_ID;
							dio_message_->template set_payload<uint16_t>( &rank_, 6, 1 );
							
							radio_ip().send( Radio_IP::BROADCAST_ADDRESS, dio_reference_number_, NULL ); 

							stop_dio_timer_ = true;
							stop_dao_timer_ = true;
							
							timer().template set_timer<self_type, &self_type::dis_delay>( 1000, this, 0 );
						}
						else
						{
							send_dis( Radio_IP::BROADCAST_ADDRESS, dis_reference_number_, NULL );
							
							#ifdef ROUTING_RPL_DEBUG
							debug().debug( "\n\nRPL Routing: Finding new Parent...\n\n" );
							#endif
							node_id_t best = Radio_IP::NULL_NODE_ID;
							uint16_t current_best_path_cost = 0xFFFF;
			
							for (ParentSet_iterator it = parent_set_.begin(); it != parent_set_.end(); it++)
							{
								if ( it->second.path_cost < current_best_path_cost )
								{
									current_best_path_cost = it->second.path_cost;
									best = it->first;
								}
							}
									
							send_no_path_dao( my_global_address_ );
								
							update_dio( best, current_best_path_cost);
							
							if( state_ == Leaf )
							{
								dao_received_ = false;
								state_ = Connected;
			
								timer().template set_timer<self_type, &self_type::leaf_timer_elapsed>(  current_interval_ + 2500, this, 0 );
								timer().template set_timer<self_type, &self_type::timer_elapsed>( current_interval_, this, 0 );
								timer().template set_timer<self_type, &self_type::threshold_timer_elapsed>( sending_threshold_, this, 0 );
									
							}
															
							find_worst_parent();
												
							dao_sequence_ = dao_sequence_ + 1;
							dao_message_->template set_payload<uint8_t>( &dao_sequence_, 7, 1 );
							path_sequence_ = path_sequence_ + 1;
							dao_message_->template set_payload<uint8_t>( &path_sequence_, 48, 1 );

							if( dao_ack_received_ )
							{
								dao_ack_received_ = false;
								timer().template set_timer<self_type, &self_type::dao_timer_elapsed>( 300, this, 0 );
							}

							
							transient_preferred_parent_ = old_preferred_parent_;
							timer().template set_timer<self_type, &self_type::transient_parent_timer_elapsed>( 5000, this, 0 );
						}
					}				
					packet_pool_mgr_->clean_packet( message );
					return;
				}

				else if( check_rank > MAX_PATH_COST )
				{
					//if this is the only parent add it as preferred parent
					//this node is a leaf! So stop dio timer
				}
							
				if( version_number_ != data[5] )
				{
					#ifdef ROUTING_RPL_DEBUG
					debug().debug( "\n\nRPL Routing: Different version received!?\n\n" );
					#endif
					if( version_number_ > data[5] )
					{
						//The received DIO represents an older version => ignore the message
						packet_pool_mgr_->clean_packet( message );
						return;
					}
					
					if( !is_reachable( sender ) )
					{
						packet_pool_mgr_->clean_packet( message );
						return;				
					}
						
					//create a new message and restart the timer (RFC6550 pag.74)
					//RFC 6550 sect 8.2.2.1: Every element of a node's parent set MUST belong to the same version
					first_dio( sender, data, length );
				}
		
				//same version
				else
				{
					uint16_t parent_rank = ( data[6] << 8 ) | data[7];
					uint16_t parent_path_cost;
					float rank_inc;
					
					if (ocp_ == 0 )
					{
						rank_inc = ((rank_factor_ * step_of_rank_) + rank_stretch_ ) * min_hop_rank_increase_;
						parent_path_cost = parent_rank + rank_inc;
					}
					else
					{
						
						NeighborSet_iterator it = neighbor_set_.find( sender );
						if( it == neighbor_set_.end() )
						{
							//Is it Unreachable?
							#ifdef ROUTING_RPL_DEBUG
							//debug().debug( "\n\nRPL Routing: (ETX) I'm %s, same version received: ENTRY %s NOT PRESENT!?\n\n", my_address_.get_address(str), sender.get_address(str2) );
							#endif
							packet_pool_mgr_->clean_packet( message );
							return;
						}
				
						if( etx_ )
						{
							rank_inc = it->second;
							parent_path_cost = rank_inc + parent_rank;
						}
						else
						{
							rank_inc = step_of_rank_ * min_hop_rank_increase_; 
							parent_path_cost = parent_rank + rank_inc;
						}
					}
				
					Mapped_parent_set map;
					map.current_version = data[5];
					uint8_t grounded = data[8];
					grounded = (grounded >> 7);
					map.grounded = grounded;
					//map.dtsn = data[9];		
										
					dio_count_ = dio_count_ + 1;

					if (parent_rank == rank_ )
					{
						packet_pool_mgr_->clean_packet( message );
						return;	
					}
					if (parent_rank < rank_ )
					{
						if( !is_reachable( sender ) )
						{
							packet_pool_mgr_->clean_packet( message );
							return;				
						}

						ParentSet_iterator it = parent_set_.find(sender);
						if (it == parent_set_.end())
						{
							#ifdef ROUTING_RPL_DEBUG
							debug().debug( "\nRPLRouting: Parent %s not present in parent set. \n", sender.get_address(str) );
							#endif
							map.rank = parent_rank;
							map.path_cost = parent_path_cost;
							map.dtsn = data[9];
							
							if( parent_set_.size() == parent_set_.max_size() )
							{
								//delete the worst entry only if it is even worst than this parent...
								//... otherwise return
								ParentSet_iterator it_worst = parent_set_.find( worst_parent_ );
								if( it_worst->second.path_cost > parent_path_cost )
									parent_set_.erase( worst_parent_ );
								else
								{
									packet_pool_mgr_->clean_packet( message );
									return;	
								}
							}
							parent_set_.insert( pair_t( sender, map ) );

							find_worst_parent();
							
																					
							//change preferred parent only if this parent is at least 1.5-better (Hysteresis)
							//Update to support standard ETX without always changing the value
							
							if( parent_path_cost < cur_min_path_cost_ /1.5 )
							{	
								#ifdef ROUTING_RPL_DEBUG
								debug().debug( "\nRPLRouting: Received (NEW NODE) DIO with 1.5-better rank. Old preferred parent was %s, rank %i. New Preferred parent is %s, rank %i\n", preferred_parent_.get_address( str ), cur_min_path_cost_, sender.get_address(str2), parent_path_cost );
								#endif
								
								if( preferred_parent_ != Radio_IP::NULL_NODE_ID )
									send_no_path_dao( my_global_address_ );
								
								update_dio( sender, parent_path_cost); 
																
								if( state_ == Leaf || stop_dio_timer_ )
								{
									stop_dio_timer_ = false;
									dao_received_ = false;
									state_ = Connected;
			
									timer().template set_timer<self_type, &self_type::leaf_timer_elapsed>(  current_interval_ + 2500, this, 0 );
									timer().template set_timer<self_type, &self_type::timer_elapsed>( current_interval_, this, 0 );
									timer().template set_timer<self_type, &self_type::threshold_timer_elapsed>( sending_threshold_, this, 0 );
									
								}
								
								dao_sequence_ = dao_sequence_ + 1;
								dao_message_->template set_payload<uint8_t>( &dao_sequence_, 7, 1 );
								path_sequence_ = path_sequence_ + 1;
								dao_message_->template set_payload<uint8_t>( &path_sequence_, 48, 1 );		

								if( dao_ack_received_ )
								{
									dao_ack_received_ = false;
									timer().template set_timer<self_type, &self_type::dao_timer_elapsed>( 300, this, 0 );
								}
								transient_preferred_parent_ = old_preferred_parent_;
								timer().template set_timer<self_type, &self_type::transient_parent_timer_elapsed>( 5000, this, 0 );
							}
							else
							{
								//NOT SIGNIFICANT UPDATE
								packet_pool_mgr_->clean_packet( message );
								return;	
							}
							
						}
						
						else if( it->second.path_cost != parent_path_cost ) 
						{
							it->second.rank = parent_rank;
							it->second.path_cost = parent_path_cost;
							it->second.dtsn = data[9];
							if ( sender == preferred_parent_ )
							{	
								#ifdef ROUTING_RPL_DEBUG
								debug().debug( "\nRPLRouting: Path cost changed through Preferred parent %s, cost: %i. FINDING NEW PREFERRED PARENT\n", sender.get_address( str2 ), parent_path_cost );
								#endif
								node_id_t best = Radio_IP::NULL_NODE_ID;
								uint16_t current_best_path_cost = 0xFFFF;
		
								for( ParentSet_iterator it = parent_set_.begin(); it != parent_set_.end(); it++ )
								{
									if ( it->second.path_cost < current_best_path_cost )
									{
										current_best_path_cost = it->second.path_cost;
										best = it->first;
									}
								}
											
								if ( best != sender )
									send_no_path_dao( my_global_address_ );
								
								if( state_ == Leaf )
								{
									dao_received_ = false;
									state_ = Connected;
			
									timer().template set_timer<self_type, &self_type::leaf_timer_elapsed>(  current_interval_ + 2500, this, 0 );
									timer().template set_timer<self_type, &self_type::timer_elapsed>( current_interval_, this, 0 );
									timer().template set_timer<self_type, &self_type::threshold_timer_elapsed>( sending_threshold_, this, 0 );
									
								}
								update_dio( best, current_best_path_cost );
								
								#ifdef ROUTING_RPL_DEBUG
								debug().debug( "(CH) NEW PARENT %s, RANK %i",  preferred_parent_.get_address(str2), current_best_path_cost );
								#endif

								find_worst_parent();
										
								dao_ack_received_ = false;
								dao_sequence_ = dao_sequence_ + 1;
								dao_message_->template set_payload<uint8_t>( &dao_sequence_, 7, 1 );
								path_sequence_ = path_sequence_ + 1;
								dao_message_->template set_payload<uint8_t>( &path_sequence_, 48, 1 );
								
								transient_preferred_parent_ = old_preferred_parent_;
								timer().template set_timer<self_type, &self_type::transient_parent_timer_elapsed>( 5000, this, 0 );
								
							}
							else
							{
								it->second.rank = parent_rank;
								it->second.path_cost = parent_path_cost;
								
								if( parent_path_cost < cur_min_path_cost_ /1.5 )
								{
									#ifdef ROUTING_RPL_DEBUG
									debug().debug( "\nRPLRouting: Received DIO with 1.5-better rank. Old preferred parent was %s, rank %i. New Preferred parent is %s, rank %i\n", preferred_parent_.get_address( str ), cur_min_path_cost_, sender.get_address(str2), parent_path_cost );
									#endif
									
									send_no_path_dao( my_global_address_ );
									
									update_dio( sender, parent_path_cost); 
								
									if( state_ == Leaf || stop_dio_timer_ )
									{
										stop_dio_timer_ = false;
										dao_received_ = false;
										state_ = Connected;
			
										
										timer().template set_timer<self_type, &self_type::leaf_timer_elapsed>(  current_interval_ + 2500, this, 0 );
										timer().template set_timer<self_type, &self_type::timer_elapsed>( current_interval_, this, 0 );
										timer().template set_timer<self_type, &self_type::threshold_timer_elapsed>( sending_threshold_, this, 0 );
									
									}
									
									dao_sequence_ = dao_sequence_ + 1;
									dao_message_->template set_payload<uint8_t>( &dao_sequence_, 7, 1 );
									path_sequence_ = path_sequence_ + 1;
									dao_message_->template set_payload<uint8_t>( &path_sequence_, 48, 1 );

									if( dao_ack_received_ )
									{
										dao_ack_received_ = false;
										timer().template set_timer<self_type, &self_type::dao_timer_elapsed>( 300, this, 0 );
									}
									transient_preferred_parent_ = old_preferred_parent_;
									timer().template set_timer<self_type, &self_type::transient_parent_timer_elapsed>( 5000, this, 0 );
								}
				
								else if ( parent_rank > rank_ )
									parent_set_.erase( sender );
								
								find_worst_parent();
								packet_pool_mgr_->clean_packet( message );
								return;	
							}
						}
						else
						{
							uint8_t current_dtsn = it->second.dtsn;
							
							if( current_dtsn != data[9] )
							{
								it->second.dtsn = data[9];
								if( sender != preferred_parent_ )
								{
									packet_pool_mgr_->clean_packet( message );
									return;	
								}
								
								#ifdef ROUTING_RPL_DEBUG
								debug().debug( "\nRPLRouting: Preferred parent %s changed its dtsn.\n", sender.get_address( str ));
								#endif

								dao_sequence_ = dao_sequence_ + 1;
								dao_message_->template set_payload<uint8_t>( &dao_sequence_, 7, 1 );
								path_sequence_ = path_sequence_ + 1;
								dao_message_->template set_payload<uint8_t>( &path_sequence_, 48, 1 );

								if( dao_ack_received_ )
								{
									#ifdef ROUTING_RPL_DEBUG
									debug().debug( "\nRPLRouting: Activate DAO Timer.\n" );
									#endif
									//reactivate dao_timer
									dao_ack_received_ = false;
									timer().template set_timer<self_type, &self_type::dao_timer_elapsed>( 300, this, 0 );
								}
								dtsn_ = dtsn_ + 1;
								
								dio_message_->template set_payload<uint8_t>( &dtsn_, 9, 1 );

								set_current_interval(0);
								compute_sending_threshold();

							}
							packet_pool_mgr_->clean_packet( message );
							return;	
						}
					}

					else
					{	
						//parent_rank > rank 
						ParentSet_iterator it = parent_set_.find( sender );
						if ( sender == preferred_parent_ ) 
						{	
							it->second.rank = parent_rank;
							it->second.path_cost = parent_path_cost;
														
							#ifdef ROUTING_RPL_DEBUG
							debug().debug( "\nRPLRouting: Path Cost Changed through Preferred parent %s, cost: %i. FINDING NEW PREFERRED PARENT\n", sender.get_address(str), parent_path_cost );
							#endif
							node_id_t best = Radio_IP::NULL_NODE_ID;
							uint16_t current_best_path_cost = 0xFFFF;
		
							for (ParentSet_iterator it = parent_set_.begin(); it != parent_set_.end(); it++)
							{
								if ( it->second.path_cost < current_best_path_cost )
								{
									current_best_path_cost = it->second.path_cost;
									best = it->first;
								}
							}
								
							if( best != preferred_parent_ )
								send_no_path_dao( my_global_address_ );
							
							update_dio( best, current_best_path_cost);
							
							find_worst_parent();
					
							dao_ack_received_ = false;
							dao_sequence_ = dao_sequence_ + 1;
							dao_message_->template set_payload<uint8_t>( &dao_sequence_, 7, 1 );
							path_sequence_ = path_sequence_ + 1;
							dao_message_->template set_payload<uint8_t>( &path_sequence_, 48, 1 );
							
							transient_preferred_parent_ = old_preferred_parent_;
							timer().template set_timer<self_type, &self_type::transient_parent_timer_elapsed>( 5000, this, 0 );
						}
								
						else
						{
							if( it != parent_set_.end() )
								parent_set_.erase( sender );							
						}
						
						if( state_ == Leaf )
						{
							#ifdef ROUTING_RPL_DEBUG
							debug().debug( "\nRPL Routing: Leaf with Good Rank, Change state to Router\n" );
							#endif							
							state_ = Router;
							timer().template set_timer<self_type, &self_type::timer_elapsed>( current_interval_, this, 0 );
							timer().template set_timer<self_type, &self_type::threshold_timer_elapsed>( sending_threshold_, this, 0 );
						}
						find_worst_parent();
						
					}
				}
			}
		}
		else if( typecode == DODAG_INF_SOLICIT )
		{
			if( state_ == Dodag_root )
			{
				dis_count_ = dis_count_ + 1;
				if( dis_count_ > DODAG_REPAIR_THRESHOLD )
				{
					#ifdef ROUTING_RPL_DEBUG
					debug().debug( "\nRPL Routing: GLOBAL REPAIR\n" );
					#endif
					
					stop_dio_timer_ = true;
					version_number_ = version_number_ + 1;
						
					start();
				}
			}
			packet_pool_mgr_->clean_packet( message );
			if( state_ == Unconnected || rank_ == INFINITE_RANK )
				return;
			#ifdef ROUTING_RPL_DEBUG
			debug().debug( "RPLRouting: Received DIS from %s, Reset Trickle timer...\n", sender.get_address(str) );
			#endif
					
			if( sender != preferred_parent_ )
			{
				radio_ip().send( Radio_IP::BROADCAST_ADDRESS, dio_reference_number_, NULL );
				timer().template set_timer<self_type, &self_type::more_dio_timer_elapsed>( 100, this, 0 );
								
				set_current_interval(0);
				compute_sending_threshold();
				if( state_ == Leaf )
				{
					dao_received_ = false;
					state_ = Connected;
			
					timer().template set_timer<self_type, &self_type::leaf_timer_elapsed>(  current_interval_ + 2500, this, 0 );
					timer().template set_timer<self_type, &self_type::timer_elapsed>( current_interval_, this, 0 );
					timer().template set_timer<self_type, &self_type::threshold_timer_elapsed>( sending_threshold_, this, 0 );

				}
			}
			else
			{
				timer().template set_timer<self_type, &self_type::delayed_restart_timer_elapsed>( 2000, this, 0 );
			}


			
			if( state_ == Leaf )
			{
				dao_received_ = false;
				state_ = Connected;
			
				timer().template set_timer<self_type, &self_type::leaf_timer_elapsed>(  current_interval_ + 2500, this, 0 );
				timer().template set_timer<self_type, &self_type::timer_elapsed>( current_interval_, this, 0 );
				timer().template set_timer<self_type, &self_type::threshold_timer_elapsed>( sending_threshold_, this, 0 );

			}

			
		}
		else if( typecode == DEST_ADVERT_OBJECT )
		{
			
			//MOP = 1 is Non-storing mode
			if (mop_ == 1)
			{
				//Not supported for the moment
			}
			
			//Storing mode
			else if( mop_ == 2 )
			{	
				uint8_t addr[16];
				memcpy(addr, data + 28 ,16);
				
				#ifdef SHAWN
				uint8_t k = 0;
				for( uint8_t i = 15; i>7; i--)
				{
					uint8_t temp;
					temp = addr[i];
					addr[i] = addr[k];
					addr[k] = temp;
					k++;
				}
				#endif
					
				node_id_t target;
				target.set_address(addr);

				uint16_t seq_nr = (uint16_t)data[48];

				ForwardingTableIterator it = radio_ip().routing_.forwarding_table_.find( target );
				if( it != radio_ip().routing_.forwarding_table_.end() )
				{
					if ( data[49] == 0 )
					{
						#ifdef ROUTING_RPL_DEBUG
						debug().debug( "\nRPL Routing: Received NO-PATH with target %s, next hop %s\n", target.get_address(str), sender.get_address(str2) );
						#endif

						if( it->second.next_hop == sender )
							radio_ip().routing_.forwarding_table_.erase( it );
						else
						{
							#ifdef ROUTING_RPL_DEBUG
							debug().debug( "\nRPL Routing: Stop NO-PATH DAO process: reached the common ancestor of the old preferred parent and the new one of the target \n" );
							#endif
							packet_pool_mgr_->clean_packet( message );
							return;
						}
						message->remote_ll_address = Radio_P::NULL_NODE_ID;
						message->target_interface = NUMBER_OF_INTERFACES;
						message->set_source_address(my_address_);
						if( transient_preferred_parent_ != Radio_IP::NULL_NODE_ID )
							send( transient_preferred_parent_, packet_number, NULL ); 
						else
							send( preferred_parent_, packet_number, NULL ); 
						return;				
						
					}

							
					else if( seq_nr < it->second.seq_nr )
					{
						packet_pool_mgr_->clean_packet( message );
						return;
					}
					else
					{
						it->second.next_hop = sender;
						it->second.seq_nr = seq_nr;
						dao_received_ = true;
					}
				}
				else
				{
					if( target == my_global_address_ )
					{
						packet_pool_mgr_->clean_packet( message );
						return;
					}
					if ( data[49] == 0 )
					{
						#ifdef ROUTING_RPL_DEBUG
						debug().debug( "\nRPL Routing: Received NO-PATH with target %s, next hop %s\n", target.get_address(str), sender.get_address(str2) );
						#endif
						message->remote_ll_address = Radio_P::NULL_NODE_ID;
						message->target_interface = NUMBER_OF_INTERFACES;
						message->set_source_address(my_address_);
						if( transient_preferred_parent_ != Radio_IP::NULL_NODE_ID )
							send( transient_preferred_parent_, packet_number, NULL ); 
						else
							send( preferred_parent_, packet_number, NULL ); 
						return;	
					}
					else
					{
						dao_received_ = true;
						if( state_ != Dodag_root )
						{		
							if( state_ == Leaf)
							{
								timer().template set_timer<self_type, &self_type::timer_elapsed>( current_interval_, this, 0 );
								timer().template set_timer<self_type, &self_type::threshold_timer_elapsed>( sending_threshold_, this, 0 );
							}			
							state_ = Router;
						}
						stop_dio_timer_ = false;
						Forwarding_table_value entry( sender, 0, seq_nr, 0 );
						radio_ip().routing_.forwarding_table_.insert( ft_pair_t( target, entry ) );
												
					}
				}
								

				#ifdef ROUTING_RPL_DEBUG
				debug().debug( "\nRPL Routing: Received DAO with target: %s, next_hop: %s\n", target.get_address(str), sender.get_address(str2) );
				#endif

				if( state_ == Dodag_root || state_ == Floating_Dodag_root )
				{
					if( data[5] == 128 )
					{
						#ifdef ROUTING_RPL_DEBUG
						debug().debug( "\nRPL Routing: ROOT sends DAO-ACK to target: %s\n", target.get_address(str) );
						#endif
						uint8_t setter_byte = DEST_ADVERT_OBJECT_ACK;
						message->template set_payload<uint8_t>( &setter_byte, 1, 1 );
						setter_byte = data[7];
						message->template set_payload<uint8_t>( &setter_byte, 6, 1 );
						setter_byte = 0;
						message->template set_payload<uint8_t>( &setter_byte, 5, 1 );
						message->template set_payload<uint8_t>( &setter_byte, 7, 1 );
					
						message->set_transport_length( 8 );
						
						message->set_source_address(my_global_address_);

						send( target, packet_number, NULL ); 
						return;				
					}
					else
					{
						packet_pool_mgr_->clean_packet( message );
						return;
					}
				}			
				else
				{
					message->remote_ll_address = Radio_P::NULL_NODE_ID;
					message->target_interface = NUMBER_OF_INTERFACES;
					message->set_source_address(my_address_);

					send( preferred_parent_, packet_number, NULL );
				} 
				
				return;
				
			}
		}
		else if( typecode == DEST_ADVERT_OBJECT_ACK )
		{
			uint8_t rcvd_dao_sequence = data[6];
						
			if( rcvd_dao_sequence == dao_sequence_ )
			{
				#ifdef ROUTING_RPL_DEBUG
				debug().debug( "\nRPL Routing: Received DAO-ACK! STOP DAO TIMER...\n" );
				#endif
				dao_ack_received_ = true;
			}
			packet_pool_mgr_->clean_packet( message );
			return;
		}
		
		packet_pool_mgr_->clean_packet( message );
	}
	// -----------------------------------------------------------------------
	template<typename OsModel_P,
		typename Radio_IP_P,
		typename Radio_P,
		typename Debug_P,
		typename Timer_P/*,
		typename Clock_P*/>
	void
	RPLRouting<OsModel_P, Radio_IP_P, Radio_P, Debug_P, Timer_P/*, Clock_P*/>::
	first_dio( node_id_t from, block_data_t *data, uint16_t length )
	{
		char str[43];
				
		uint8_t option_type = data[28]; 
		uint8_t option_length = data[ 29 ];
		uint16_t length_checked = 28;
		
		bool config_present = false;
		while( length > length_checked )
		{
			if( option_type == PREFIX_INFORMATION )
				prefix_present_ = true;  
			else if( option_type == DODAG_CONFIGURATION )
				config_present = true;
			
			length_checked = length_checked + 2 + option_length;
			option_type = data[ length_checked ];
			option_length = data[ length_checked + 1 ];
			
		}

		if( !(config_present && prefix_present_) )
		{
			#ifdef ROUTING_RPL_DEBUG
			debug().debug( "\nRPL Routing: First DIO Message doesn't contain DODAG_CONFIGURATION_OPTION or PREFIX_INFORMATION.\n" );
			#endif
			return;
		}
		
		if ( ! is_reachable( from ) )
		{
			#ifdef ROUTING_RPL_DEBUG
			debug().debug( "\nRPL Routing: NEIGHBOR %s unreachable.\n", from.get_address(str) );
			#endif
			return;
		}

		if( version_number_ > 1)
		{
			stop_dio_timer_ = true;
			stop_dao_timer_ = true;
		}
		radio_ip().routing_.forwarding_table_.clear();
		
		dao_ack_received_ = false;
		no_path_ack_received_ = false;
		
		dao_received_ = false;

		preferred_parent_ = Radio_IP::NULL_NODE_ID;
		old_preferred_parent_ = Radio_IP::NULL_NODE_ID;
		worst_parent_ = Radio_IP::NULL_NODE_ID;

		length_checked = 28; 
		
		uint8_t ret = 2;
		
		ret = options_check( data, length_checked, length, from );
		
		if (ret <= 1 )
		{
			#ifdef ROUTING_RPL_DEBUG
			debug().debug( "\nRPL Routing:RETURN BEFORE BEING CONNECTED.\n" );
			#endif
			return;
		}
		#ifdef ROUTING_RPL_DEBUG
		debug().debug( "\nRPLRouting: CONNECTED \n" );
		#endif
		
		state_ = Connected;
		
		ret = set_firsts_dio_fields( from, data );
		if( ret == 3 )
		{
			state_ = Unconnected;
			return;
		}

		dio_message_->set_transport_length( length );
		
		set_current_interval(0);
		compute_sending_threshold();

		
		#ifdef ROUTING_RPL_DEBUG
		debug().debug( "\n\n\nRPL Routing: Starting the timers\n\n" );
		#endif
		
		timer().template set_timer<self_type, &self_type::leaf_timer_elapsed>( current_interval_ + 2500, this, 0 );

		timer().template set_timer<self_type, &self_type::timer_elapsed>( current_interval_, this, 0 );

		timer().template set_timer<self_type, &self_type::threshold_timer_elapsed>( sending_threshold_, this, 0 );
		
		if( mop_ == 2 )
		{
			stop_dao_timer_ = false;
			dao_ack_received_ = false;
			uint8_t dao_length;
			dao_sequence_ = dao_sequence_ + 1;
			dao_length = prepare_dao();
			dao_message_->set_transport_length( dao_length );
			timer().template set_timer<self_type, &self_type::dao_timer_elapsed>( 300 + current_interval_, this, 0 );
		}
	}
	
	// -----------------------------------------------------------------------
	
	template<typename OsModel_P,
		typename Radio_IP_P,
		typename Radio_P,
		typename Debug_P,
		typename Timer_P/*,
		typename Clock_P*/>
	uint8_t
	RPLRouting<OsModel_P, Radio_IP_P, Radio_P, Debug_P, Timer_P/*, Clock_P*/>::
	options_check( block_data_t *data, uint16_t length_checked, uint16_t length, node_id_t sender )
	{
		
		uint8_t option_type = data[ length_checked ]; 
		uint8_t option_length = data[ length_checked + 1 ];
		bool isMetric = false;
		bool constraint = false;
		bool satisfied = false;

		uint8_t return_value = 3;

		while ( length > length_checked )
		{
			if( option_type == DODAG_CONFIGURATION )
			{
				scan_configuration_option( data, length_checked );
			}
			else if( option_type == PREFIX_INFORMATION )
			{
				scan_prefix_information( data, length_checked );
			}

			else if( option_type == DAG_METRIC_CONTAINER )
			{
				uint8_t metric_type = data [ length_checked + 2 ];
				/*
				if( metric_type == HOP_COUNT )
				{
					
				}
				*/
				//else if (Other Metric types...)
			}

			//else if( option_type == ROUTING_INFORMATION )
				
			dio_message_->template set_payload<uint8_t>( &option_type, length_checked, 1 );
			dio_message_->template set_payload<uint8_t>( &option_length, length_checked + 1, 1 );

			if (!isMetric)  
			{
				for (int i = 0; i < option_length; i++ )
				{
					uint8_t setter_byte = data[ length_checked + i + 2 ];
					dio_message_->template set_payload<uint8_t>( &setter_byte, length_checked + i + 2, 1 );
				}
			}
		

			length_checked = length_checked + 2 + option_length;
			option_type = data[ length_checked ];
			option_length = data[ length_checked + 1 ];
		}
		
		if( ocp_ == 0 )
		{
			step_of_rank_ = DEFAULT_STEP_OF_RANK;
			return 2;
		}
		return return_value;
	}

	// -----------------------------------------------------------------------
	
	template<typename OsModel_P,
		typename Radio_IP_P,
		typename Radio_P,
		typename Debug_P,
		typename Timer_P/*,
		typename Clock_P*/>
	void
	RPLRouting<OsModel_P, Radio_IP_P, Radio_P, Debug_P, Timer_P/*, Clock_P*/>::
	scan_configuration_option( block_data_t *data, uint16_t length_checked )
	{
		uint8_t option_type = data[ length_checked ];
		uint8_t option_length = data[ length_checked + 1 ];
				
		#ifdef ROUTING_RPL_DEBUG
		debug().debug( "\nRPL Routing: Scanning DODAG_CONFIGURATION_OPTION!\n" );
		#endif

		imax_ = data[ length_checked + 3 ];
		dio_int_min_ = data[ length_checked + 4 ];
		dio_redund_const_ = data[ length_checked + 5 ];
		DAGMaxRankIncrease_ = ( data[ length_checked + 6 ] << 8 ) | data[ length_checked + 7 ];

		min_hop_rank_increase_ = ( data[ length_checked + 8 ] << 8 ) | data[ length_checked + 9 ];

		imin_ = 2 << (dio_int_min_ - 1);
		max_interval_ = (2 << (imax_ - 1)) *imin_;
		
		ocp_ = ( data[ length_checked + 10 ] << 8 ) | data[ length_checked + 11 ];	
		
	}

	// -----------------------------------------------------------------------
	
	template<typename OsModel_P,
		typename Radio_IP_P,
		typename Radio_P,
		typename Debug_P,
		typename Timer_P/*,
		typename Clock_P*/>
	void
	RPLRouting<OsModel_P, Radio_IP_P, Radio_P, Debug_P, Timer_P/*, Clock_P*/>::
	scan_prefix_information( block_data_t *data, uint16_t length_checked )
	{
		uint8_t prefix_len = data[ length_checked + 2 ];
		uint8_t flags = data[ length_checked + 3 ];
		uint8_t on_link = (flags >> 7);	
		uint8_t aut = (flags << 1);
		aut = (flags >> 7);			
		bool onlink_flag;
		if( on_link == 1 )
			onlink_flag = true;
		else
			onlink_flag = false;
	
		bool antonomous_flag;
		if( aut == 1 )
			antonomous_flag = true;
		else
			antonomous_flag = false;
		
		uint32_t valid_lifetime = ( data[ length_checked + 4 ] << 24 ) | ( data[ length_checked + 5 ] << 16 ) | ( data[ length_checked + 6 ] << 8 ) | data[ length_checked + 7 ];
						
		uint32_t prefered_lifetime = (  data[ length_checked + 8 ] << 24 ) | (  data[ length_checked + 9 ] << 16 ) | (  data[ length_checked + 10 ] << 8 ) | data[ length_checked + 11 ];
						
		radio_ip().interface_manager_->set_prefix_for_interface( data + length_checked + 16, Radio_IP::INTERFACE_RADIO, prefix_len, valid_lifetime, onlink_flag, prefered_lifetime, antonomous_flag );
				
		if( state_ != Dodag_root )
		{
			my_global_address_ = radio_ip().global_id();
		}
	}

	// -----------------------------------------------------------------------
	
	template<typename OsModel_P,
		typename Radio_IP_P,
		typename Radio_P,
		typename Debug_P,
		typename Timer_P/*,
		typename Clock_P*/>
	uint8_t
	RPLRouting<OsModel_P, Radio_IP_P, Radio_P, Debug_P, Timer_P/*, Clock_P*/>::
	set_firsts_dio_fields( node_id_t from, block_data_t *data )
	{		
		
		parent_set_.clear(); 
		dio_count_ = dio_count_ + 1; 
		rpl_instance_id_ = data[4];
		version_number_ = data[5];
	
		dio_message_->template set_payload<uint8_t>( &rpl_instance_id_, 4, 1 );
		dio_message_->template set_payload<uint8_t>( &version_number_, 5, 1 );
				
		
		uint16_t parent_rank = ( data[6] << 8 ) | data[7];
			
		Mapped_parent_set map;		
		map.current_version = data[5];
		uint8_t grounded = data[8];
		grounded = (grounded >> 7);
		map.grounded = grounded;
		map.dtsn = data[9];
		float rank_inc;
				
		if (ocp_ == 0 )
		{
			rank_inc = ((rank_factor_ * step_of_rank_) + rank_stretch_ ) * min_hop_rank_increase_;
			cur_min_path_cost_ = parent_rank + rank_inc;
			rank_ = cur_min_path_cost_;
			map.path_cost = cur_min_path_cost_;
			map.metric_type = 0;
			
		}
		else
		{
			if( etx_ )
			{
				NeighborSet_iterator it = neighbor_set_.find( from );
				if( it == neighbor_set_.end() )
				{
					#ifdef ROUTING_RPL_DEBUG
					char str[43];
										
					debug().debug( "\n\nRPL Routing: (ETX) First DIO: ENTRY %s NOT PRESENT!?\n\n", from.get_address(str) );
					#endif
					return 3;
				}
				
				rank_inc = it->second;

				#ifdef ROUTING_RPL_DEBUG
				debug().debug( "\n\nRPL Routing: Rank Increase is %f\n\n", rank_inc );
				#endif

				rank_ = rank_inc + parent_rank;

				cur_min_path_cost_ = rank_;
				map.rank = parent_rank;
				map.path_cost = cur_min_path_cost_;
				map.metric_type = LINK_ETX;
			
				
			}
			else
			{
				rank_inc = step_of_rank_ * min_hop_rank_increase_; 
				cur_min_path_cost_ = parent_rank + rank_inc;
				rank_ = cur_min_path_cost_;
				map.rank = parent_rank;
				map.path_cost = cur_min_path_cost_;
				map.metric_type = 0; //to verify the real metric type
			}
		}	
							
		preferred_parent_ = from; 
		worst_parent_ = from;
		parent_set_.insert( pair_t( from, map ) );
		//ADD default route
		Forwarding_table_value entry( preferred_parent_, 0, 0, 0 );
		radio_ip().routing_.forwarding_table_.insert( ft_pair_t( Radio_IP::NULL_NODE_ID, entry ) );
		
		#ifdef ROUTING_RPL_DEBUG
		char str[43];
		debug().debug( "\n\n\nRPL Routing: Preferred parent %s Set DIO New Rank is : RANK %i\n\n", preferred_parent_.get_address(str), rank_ );
		#endif
		

		dio_message_->set_transport_next_header( Radio_IP::ICMPV6 );
		dio_message_->set_hop_limit(255);
		
		dio_message_->set_source_address(my_address_);

		node_id_t destin = Radio_IP::BROADCAST_ADDRESS;
		dio_message_->set_destination_address( destin );
		
		dio_message_->set_flow_label(0);
		dio_message_->set_traffic_class(0);

		dio_message_->template set_payload<uint16_t>( &rank_, 6, 1 );
	
		uint8_t setter_byte = data[8];
		dio_message_->template set_payload<uint8_t>( &setter_byte, 8, 1 );
		setter_byte = data[9];
		dio_message_->template set_payload<uint8_t>( &dtsn_, 9, 1 );
		
		setter_byte = data[10];
		dio_message_->template set_payload<uint8_t>( &setter_byte, 10, 1 );
		setter_byte = data[11];
		dio_message_->template set_payload<uint8_t>( &setter_byte, 11, 1 );
		
		uint8_t addr[16];
		memcpy(addr, data + 12 ,16);
		
		#ifdef SHAWN		
		uint8_t k = 0;
		for( uint8_t i = 15; i>7; i--)
		{
			uint8_t temp;
			temp = addr[i];
			addr[i] = addr[k];
			addr[k] = temp;
			k++;
		}
		#endif
		
		dodag_id_.set_address(addr);
		
		dio_message_->template set_payload<uint8_t[16]>( &dodag_id_.addr, 12, 1 );
		return 0;
	
	}

	// -----------------------------------------------------------------------
	
	template<typename OsModel_P,
		typename Radio_IP_P,
		typename Radio_P,
		typename Debug_P,
		typename Timer_P/*,
		typename Clock_P*/>
	uint8_t
	RPLRouting<OsModel_P, Radio_IP_P, Radio_P, Debug_P, Timer_P/*, Clock_P*/>::
	dio_packet_initialization( uint8_t position, bool grounded )
	{
		dio_message_->set_transport_next_header( Radio_IP::ICMPV6 );
		dio_message_->set_hop_limit(255);
		
		dio_message_->set_source_address(my_address_);

		node_id_t destin = Radio_IP::BROADCAST_ADDRESS;
		dio_message_->set_destination_address(destin);
		dio_message_->set_flow_label(0);
		dio_message_->set_traffic_class(0);

		dio_message_->template set_payload<uint8_t>( &rpl_instance_id_, position, 1 );
		dio_message_->template set_payload<uint8_t>( &version_number_, position + 1, 1 );
		dio_message_->template set_payload<uint16_t>( &rank_, position + 2, 1 );
		uint8_t setter_byte = 0;
		if ( grounded ) 
			setter_byte = 128 + ( mop_ << 3 );
		else
			setter_byte = ( mop_ << 3 );
		dio_message_->template set_payload<uint8_t>( &setter_byte, position + 4, 1 );
		setter_byte = 0;
		
		dio_message_->template set_payload<uint8_t>( &dtsn_, position + 5, 1 );
		
		dio_message_->template set_payload<uint8_t>( &setter_byte, position + 6, 1 );
		dio_message_->template set_payload<uint8_t>( &setter_byte, position + 7, 1 );
			
		dio_message_->template set_payload<uint8_t[16]>( &my_global_address_.addr, position + 8, 1 ); //RIGHT WAY WITH 1 AS 3rd PARAM
		
		return position + 24;
	}

	
	// -----------------------------------------------------------------------
	
	template<typename OsModel_P,
		typename Radio_IP_P,
		typename Radio_P,
		typename Debug_P,
		typename Timer_P/*,
		typename Clock_P*/>
	uint8_t
	RPLRouting<OsModel_P, Radio_IP_P, Radio_P, Debug_P, Timer_P/*, Clock_P*/>::
	add_configuration_option( uint8_t position )
	{
		uint8_t	setter_byte = DODAG_CONFIGURATION;	
		dio_message_->template set_payload<uint8_t>( &setter_byte, position, 1 );
		
		setter_byte = 14;
		dio_message_->template set_payload<uint8_t>( &setter_byte, position + 1, 1 );

		setter_byte = 0;
		dio_message_->template set_payload<uint8_t>( &setter_byte, position + 2, 1 );
		dio_message_->template set_payload<uint8_t>( &imax_, position + 3, 1 );
		dio_message_->template set_payload<uint8_t>( &dio_int_min_, position + 4, 1 );
		dio_message_->template set_payload<uint8_t>( &dio_redund_const_, position + 5, 1 );

		uint16_t setter_byte_2 = DAGMaxRankIncrease_;
		dio_message_->template set_payload<uint16_t>( &setter_byte_2, position + 6, 1 );

		setter_byte_2 = min_hop_rank_increase_;
		
		dio_message_->template set_payload<uint16_t>( &setter_byte_2, position + 8, 1 );
		dio_message_->template set_payload<uint16_t>( &ocp_, position + 10, 1 );

		setter_byte = 0;
		dio_message_->template set_payload<uint8_t>( &setter_byte, position + 12, 1 );

		setter_byte = 5;
		dio_message_->template set_payload<uint8_t>( &setter_byte, position + 13, 1 );

		setter_byte_2 = 60000;
		dio_message_->template set_payload<uint16_t>( &setter_byte_2, position + 14, 1 );
		
		return position + 16;
	}
	//--------------------------------------------------------------------------------------------

	template<typename OsModel_P,
		typename Radio_IP_P,
		typename Radio_P,
		typename Debug_P,
		typename Timer_P/*,
		typename Clock_P*/>
	bool
	RPLRouting<OsModel_P, Radio_IP_P, Radio_P, Debug_P, Timer_P/*, Clock_P*/>::
	is_reachable( node_id_t node )
	{
		ETX_values_iterator it = etx_computation_.etx_values_.find( node );
		if( it->second.forward == 0 || it->second.reverse == 0 )
			return false;				
		return true;
			
	}

	// -----------------------------------------------------------------------
	
	template<typename OsModel_P,
		typename Radio_IP_P,
		typename Radio_P,
		typename Debug_P,
		typename Timer_P/*,
		typename Clock_P*/>
	void
	RPLRouting<OsModel_P, Radio_IP_P, Radio_P, Debug_P, Timer_P/*, Clock_P*/>::
	find_worst_parent()
	{
		uint16_t current_worst_path_cost = 0;
		for (ParentSet_iterator it = parent_set_.begin(); it != parent_set_.end(); it++)
		{
			if ( it->second.path_cost > current_worst_path_cost )
			{
				current_worst_path_cost = it->second.path_cost;
				worst_parent_ = it->first;
			}
		}
		if( current_worst_path_cost == 0 )
			worst_parent_ = Radio_IP::NULL_NODE_ID;
	}
	
	// -----------------------------------------------------------------------
	template<typename OsModel_P,
		typename Radio_IP_P,
		typename Radio_P,
		typename Debug_P,
		typename Timer_P/*,
		typename Clock_P*/>
	uint8_t
	RPLRouting<OsModel_P, Radio_IP_P, Radio_P, Debug_P, Timer_P/*, Clock_P*/>::
	prepare_dao()
	{
		dao_message_->set_transport_next_header( Radio_IP::ICMPV6 );
		dao_message_->set_hop_limit(255);
		
		if ( mop_ == 1 )
			dao_message_->set_source_address(my_global_address_);
		else if( mop_ == 2 )
			dao_message_->set_source_address(my_address_);
		
		dao_message_->set_destination_address(preferred_parent_);
		dao_message_->set_flow_label(0);
		dao_message_->set_traffic_class(0);
		dao_message_->remote_ll_address = Radio_P::NULL_NODE_ID;
		dao_message_->target_interface = NUMBER_OF_INTERFACES;

		dao_message_->template set_payload<uint8_t>( &rpl_instance_id_, 4, 1 );
		
		uint8_t setter_byte = 128;
		dao_message_->template set_payload<uint8_t>( &setter_byte, 5, 1 );
		dao_message_->template set_payload<uint8_t>( &setter_byte, 6, 1 );
			
		dao_message_->template set_payload<uint8_t>( &dao_sequence_, 7, 1 );
		
		setter_byte = RPL_TARGET;
		dao_message_->template set_payload<uint8_t>( &setter_byte, 24, 1 );
				
		setter_byte = 18;
		dao_message_->template set_payload<uint8_t>( &setter_byte, 25, 1 );
		
		setter_byte = 0;
		dao_message_->template set_payload<uint8_t>( &setter_byte, 26, 1 );
		dao_message_->template set_payload<uint8_t>( &setter_byte, 27, 1 );
		dao_message_->template set_payload<uint8_t[16]>( &my_global_address_.addr, 28, 1 );
				
		setter_byte = TRANSIT_INFORMATION;
		dao_message_->template set_payload<uint8_t>( &setter_byte, 44, 1 );
				
		if ( mop_ == 1 )
			setter_byte = 20;
		else if( mop_ == 2 )
			setter_byte = 4;
		dao_message_->template set_payload<uint8_t>( &setter_byte, 45, 1 );
		
		setter_byte = 0;
		dao_message_->template set_payload<uint8_t>( &setter_byte, 46, 1 );
		dao_message_->template set_payload<uint8_t>( &setter_byte, 47, 1 );
		dao_message_->template set_payload<uint8_t>( &path_sequence_, 48, 1 );
		
		setter_byte = 255;
		dao_message_->template set_payload<uint8_t>( &setter_byte, 49, 1 );

		if( mop_ == 1 )
		{
			uint8_t addr[16];
			addr[0] = 0xaa;
			addr[1] = 0xaa;
			for (uint8_t i = 2; i<16; i++)
				addr[i] = preferred_parent_.addr[i];
		
			dao_message_->template set_payload<uint8_t[16]>( &addr, 50, 1 );

			return 66;
		}
		else if( mop_ == 2 )
			return 51;
		return 50;
	}

	// -----------------------------------------------------------------------
	template<typename OsModel_P,
		typename Radio_IP_P,
		typename Radio_P,
		typename Debug_P,
		typename Timer_P/*,
		typename Clock_P*/>
	void
	RPLRouting<OsModel_P, Radio_IP_P, Radio_P, Debug_P, Timer_P/*, Clock_P*/>::
	send_no_path_dao( node_id_t target )
	{
		
		no_path_dao_->template set_payload<uint8_t>( &rpl_instance_id_, 4, 1 );
		uint8_t setter_byte = 128;
		no_path_dao_->template set_payload<uint8_t>( &setter_byte, 5, 1 );
		
		no_path_dao_->template set_payload<uint8_t>( &setter_byte, 6, 1 );
		
		dao_sequence_ = dao_sequence_ + 1;
		no_path_dao_->template set_payload<uint8_t>( &dao_sequence_, 7, 1 );

		setter_byte = RPL_TARGET;
		no_path_dao_->template set_payload<uint8_t>( &setter_byte, 24, 1 );
		
		setter_byte = 18;
		no_path_dao_->template set_payload<uint8_t>( &setter_byte, 25, 1 );
		setter_byte = 0;
		no_path_dao_->template set_payload<uint8_t>( &setter_byte, 26, 1 );
		no_path_dao_->template set_payload<uint8_t>( &setter_byte, 27, 1 );

		no_path_dao_->template set_payload<uint8_t[16]>( &target.addr, 28, 1 );
		setter_byte = TRANSIT_INFORMATION;
		no_path_dao_->template set_payload<uint8_t>( &setter_byte, 44, 1 );
		if ( mop_ == 1 )
			setter_byte = 20;
		else if( mop_ == 2 )
			setter_byte = 4;
		no_path_dao_->template set_payload<uint8_t>( &setter_byte, 45, 1 );
		setter_byte = 0;
		no_path_dao_->template set_payload<uint8_t>( &setter_byte, 46, 1 );
		no_path_dao_->template set_payload<uint8_t>( &setter_byte, 47, 1 );
		path_sequence_ = path_sequence_ + 1;
		no_path_dao_->template set_payload<uint8_t>( &path_sequence_, 48, 1 );
		setter_byte = 0;
		no_path_dao_->template set_payload<uint8_t>( &setter_byte, 49, 1 );
		
		no_path_dao_->set_transport_length( 50 );
	
		
		no_path_dao_->set_transport_next_header( Radio_IP::ICMPV6 );
		no_path_dao_->set_hop_limit(255);
		
		no_path_dao_->set_source_address(my_address_);
		no_path_dao_->set_destination_address(preferred_parent_);
		no_path_dao_->set_flow_label(0);
		no_path_dao_->set_traffic_class(0);

		old_preferred_parent_ = preferred_parent_;
		no_path_ack_received_ = false;
		
		#ifdef ROUTING_RPL_DEBUG
		char str[43];
		debug().debug( "\nRPL Routing: SENDING NO PATH DAO to %s\n", old_preferred_parent_.get_address(str) );
		#endif
		
		no_path_dao_->set_destination_address(old_preferred_parent_);
		
		radio_ip().send( old_preferred_parent_, no_path_reference_number_, NULL );

		timer().template set_timer<self_type, &self_type::no_path_timer_elapsed>( ( 500 ), this, 0 );

		
	}

	// -----------------------------------------------------------------------
	template<typename OsModel_P,
		typename Radio_IP_P,
		typename Radio_P,
		typename Debug_P,
		typename Timer_P/*,
		typename Clock_P*/>
	uint8_t
	RPLRouting<OsModel_P, Radio_IP_P, Radio_P, Debug_P, Timer_P/*, Clock_P*/>::
	add_prefix_information( uint8_t position )
	{
		uint8_t setter_byte = PREFIX_INFORMATION;
		dio_message_->template set_payload<uint8_t>( &setter_byte, position, 1);
		
		setter_byte = 30;
		dio_message_->template set_payload<uint8_t>( &setter_byte, position + 1, 1);
		
		dio_message_->template set_payload<uint8_t>( &(radio_ip().interface_manager_->prefix_list[Radio_IP::INTERFACE_RADIO][1]).ip_address.prefix_length, position + 2, 1 );
		
		setter_byte = 192;
		dio_message_->template set_payload<uint8_t>( &setter_byte, position + 3, 1 );
		
		dio_message_->template set_payload<uint32_t>( &(radio_ip().interface_manager_->prefix_list[Radio_IP::INTERFACE_RADIO][1].adv_valid_lifetime), position + 4, 1 );
		
		dio_message_->template set_payload<uint32_t>( &(radio_ip().interface_manager_->prefix_list[Radio_IP::INTERFACE_RADIO][1].adv_prefered_lifetime), position + 8, 1 );
		
		dio_message_->template set_payload<uint8_t>( radio_ip().interface_manager_->prefix_list[Radio_IP::INTERFACE_RADIO][1].ip_address.addr, position + 16, 16 );
		
		return position + 32;
		
	}

	// -----------------------------------------------------------------------
	template<typename OsModel_P,
		typename Radio_IP_P,
		typename Radio_P,
		typename Debug_P,
		typename Timer_P/*,
		typename Clock_P*/>
	void
	RPLRouting<OsModel_P, Radio_IP_P, Radio_P, Debug_P, Timer_P/*, Clock_P*/>::
	update_dio( node_id_t parent, uint16_t path_cost )
	{
		ParentSet_iterator it = parent_set_.find( parent );
		
		#ifdef ROUTING_RPL_DEBUG
		char str[43];
		char str2[43];
		debug().debug( "\nRPLRouting: Setting new preferred_parent %s, path cost %i\n, parent_rank: %i", parent.get_address( str2 ), path_cost, it->second.rank );
		#endif
		preferred_parent_ = parent; 

		dao_message_->set_destination_address( preferred_parent_ );

		//update default route
		
		radio_ip().routing_.forwarding_table_[Radio_IP::NULL_NODE_ID].next_hop = parent;

		cur_min_path_cost_ = path_cost;
		rank_ = path_cost;
	
		dio_message_->template set_payload<uint16_t>( &rank_, 6, 1 );

		erase_parent_list_.clear();
		
		for (ParentSet_iterator it = parent_set_.begin(); it != parent_set_.end(); it++)
		{
			if ( it->second.rank > rank_ )
			{
				Mapped_erase_node map;
				map.node = it->first;
				erase_parent_list_.push_back( map );
			}
		}
		
		for( Erase_parent_list_iterator it_er = erase_parent_list_.begin(); it_er != erase_parent_list_.end(); it_er++) 
		{
			parent_set_.erase( it_er->node );
		}

		set_current_interval(0);
		compute_sending_threshold();
		
	}

	// -----------------------------------------------------------------------		
	template<typename OsModel_P,
		typename Radio_IP_P,
		typename Radio_P,
		typename Debug_P,
		typename Timer_P/*,
		typename Clock_P*/>
	int
	RPLRouting<OsModel_P, Radio_IP_P, Radio_P, Debug_P, Timer_P/*, Clock_P*/>::
	handle_TLV( uint8_t packet_number, uint8_t* data_pointer, bool only_usage )
	{
		IPv6Packet_t* message = packet_pool_mgr_->get_packet_pointer( packet_number ); 

		if( only_usage )
		{
			if( message->transport_next_header() == Radio_IP::ICMPV6 )
				return Radio_IP::OUTOFUSE;
			else
				return Radio_IP::INUSE;
		}
		
		else
		{
			char str[43];
			char str2[43];
			
			#ifdef ROUTING_RPL_DEBUG
			debug_->debug( "TLV handler called: Type: %i Len: %i", data_pointer[0], data_pointer[1] );
			#endif

			node_id_t destination;
			message->destination_address( destination );
			
			if( destination == my_global_address_ )
			{
				#ifdef ROUTING_RPL_DEBUG
				debug().debug( "\nRPL Routing: FINAL DESTINATION!\n" );
				#endif
				uint8_t flags = data_pointer[2];
				uint8_t down = (flags >> 7);
				uint8_t rank_error = (flags << 1);
				rank_error = ( rank_error >> 7 );
				uint16_t sender_rank = ( data_pointer[4] << 8 ) | data_pointer[5];
				uint16_t compare_rank = DAGRank( sender_rank );
				
				if( compare_rank == 0 )
				{
					#ifdef ROUTING_RPL_DEBUG
					debug().debug( "\nRPL Routing: FINAL DESTINATION just one hop away from the sender\n" );
					#endif
					return Radio_IP::CORRECT;
				}
				if( ( down == 1 && compare_rank > DAGRank( rank_ ) ) || (down == 0 && compare_rank < DAGRank( rank_ ) ) )
				{
					//inconsistency
					if ( rank_error == 1 )
					{
						#ifdef ROUTING_RPL_DEBUG
						debug().debug( "\nRPL Routing: FINAL DESTINATION, 2nd Inconsistency ==> DROP\n" );
						#endif
						
						if( state_ != Dodag_root )
						{

							//Local Repair
							send_no_path_dao( my_global_address_ );

							stop_dio_timer_ = true;
				
							rank_ = INFINITE_RANK;
							update_dio( Radio_IP::NULL_NODE_ID ,rank_ );
							
							radio_ip().send( Radio_IP::BROADCAST_ADDRESS, dio_reference_number_, NULL );
							
							timer().template set_timer<self_type, &self_type::more_dio_timer_elapsed>( 100, this, 0 );
 	
							send_dis( Radio_IP::BROADCAST_ADDRESS, dis_reference_number_, NULL );
		
						}
						else{
							#ifdef ROUTING_RPL_DEBUG
							debug().debug( "\nRPL Routing: GLOBAL REPAIR\n" );
							#endif
							//GLOBAL REPAIR

							stop_dio_timer_ = true;
							version_number_ = version_number_ + 1;
						
							start();
						}
						
						//DON'T DROP IT, This is the destination... but anyway advertise nodes!
						return Radio_IP::CORRECT;
					}
					else
					{	
						#ifdef ROUTING_RPL_DEBUG
						debug().debug( "\nRPL Routing: FINAL DESTINATION, 1st Inconsistency\n" );
						#endif
						
						if ( down == 1 )
							flags = 192;
						else
							flags = 64;
						data_pointer[2] = flags;

					}

					send_dis( dodag_id_, dis_reference_number_, NULL );
					
				}
				
				#ifdef ROUTING_RPL_DEBUG
				print_neighbor_set();
				print_parent_set();
				radio_ip().routing_.print_forwarding_table();
				#endif
				#ifdef ROUTING_RPL_DEBUG
				debug().debug( "\nRPL Routing:THIS SHOULD BE PRINTED BY THE DESTINATION IF THE DODAG IS CONSISTENT.\n" );
				#endif
				return Radio_IP::CORRECT;
			}

			ForwardingTableIterator it = radio_ip().routing_.forwarding_table_.find( destination );
			
			if(data_pointer[3] == 0 ) //this means that the node is the source
			{	

				if( state_ == Unconnected )
				{
					#ifdef ROUTING_RPL_DEBUG
					debug().debug( "\nRPL Routing: NOT CONECTED TO THE DODAG!\n" );
					#endif
					return  Radio_IP::DROP_PACKET;
				}

				#ifdef ROUTING_RPL_DEBUG
				debug().debug( "\nRPL Routing: SOURCE NODE, my rank is %i\n", DAGRank( rank_ ) );
				#endif				
				
				data_pointer[3] = rpl_instance_id_;
				data_pointer[4] = 0;
				data_pointer[5] = 0;
																
				if( it != radio_ip().routing_.forwarding_table_.end() )
				{
					#ifdef ROUTING_RPL_DEBUG
					debug().debug( "\nRPL Routing: Source. FT contains destination %s, next hop is: %s\n", destination.get_address(str), it->second.next_hop.get_address(str2) );
					#endif

					if( !is_reachable( it->second.next_hop ) )
					{
						send_no_path_dao( destination );
						radio_ip().routing_.forwarding_table_.erase( it );
						return Radio_IP::DROP_PACKET;
					}
					
					data_pointer[2] = 128;
				}

				else
				{
					if( preferred_parent_ == my_address_ )
					{
						
						#ifdef ROUTING_RPL_DEBUG
						debug().debug( "\nRPL Routing: ROOT, cannot go up again: DAO update for destination %s not yet received\n", destination.get_address( str ) );
						#endif
			
						return  Radio_IP::DROP_PACKET;
					}
										
					if( !is_reachable( preferred_parent_ ) )
					{
						parent_set_.erase( preferred_parent_ );
						
						#ifdef ROUTING_RPL_DEBUG
						debug().debug( "\nRPLRouting: Preferred Parent Unreachable: FINDING NEW PREFERRED PARENT\n" );
						#endif

						node_id_t best = Radio_IP::NULL_NODE_ID;
						uint16_t current_best_path_cost = 0xFFFF;
						uint16_t best_parent_rank =  0xFFFF;
		
						for (ParentSet_iterator it = parent_set_.begin(); it != parent_set_.end(); it++)
						{
							if ( it->second.path_cost < current_best_path_cost )
							{
								current_best_path_cost = it->second.path_cost;
								best = it->first;
								best_parent_rank = it->second.rank;
							}
						}
								
						if( best == Radio_IP::NULL_NODE_ID )
						{
							#ifdef ROUTING_RPL_DEBUG
							debug().debug( "\nRPLRouting: NO MORE PARENTS, POISON SUB-DODAG\n" );
							#endif
							old_preferred_parent_ = preferred_parent_;
							transient_preferred_parent_ = old_preferred_parent_;
							timer().template set_timer<self_type, &self_type::transient_parent_timer_elapsed>( 5000, this, 0 );
							preferred_parent_ = Radio_IP::NULL_NODE_ID;
							cur_min_path_cost_ = 0xFFFF;
							
							rank_ = INFINITE_RANK;

							stop_dio_timer_ = true;
	
							dio_message_->template set_payload<uint16_t>( &rank_, 6, 1 );
								
							send_dis( Radio_IP::BROADCAST_ADDRESS, dis_reference_number_, NULL );
							radio_ip().send( Radio_IP::BROADCAST_ADDRESS, dio_reference_number_, NULL );
						}
						else
						{
							#ifdef ROUTING_RPL_DEBUG
							debug().debug( "\nRPLRouting: New Preferred Parent is %s, new path cost: %i, parent_rank: %i:\n", best.get_address( str ), current_best_path_cost, best_parent_rank );
							#endif
							dtsn_ = dtsn_ + 1;
							
							dio_message_->template set_payload<uint8_t>( &dtsn_, 9, 1 );
							
							update_dio( best, current_best_path_cost);
																					
							find_worst_parent();
							if( state_ == Leaf )
							{
								stop_dio_timer_ = false;
								dao_received_ = false;
								state_ = Connected;
			
								timer().template set_timer<self_type, &self_type::leaf_timer_elapsed>(  current_interval_ + 2500, this, 0 );
								timer().template set_timer<self_type, &self_type::timer_elapsed>( current_interval_, this, 0 );
								timer().template set_timer<self_type, &self_type::threshold_timer_elapsed>( sending_threshold_, this, 0 );
									
							}
						
							dao_sequence_ = dao_sequence_ + 1;
							dao_message_->template set_payload<uint8_t>( &dao_sequence_, 7, 1 );
							path_sequence_ = path_sequence_ + 1;
							dao_message_->template set_payload<uint8_t>( &path_sequence_, 48, 1 );		

							if( dao_ack_received_ )
							{
								dao_ack_received_ = false;
								timer().template set_timer<self_type, &self_type::dao_timer_elapsed>( 300, this, 0 );
							}		
							radio_ip().send( Radio_IP::BROADCAST_ADDRESS, dio_reference_number_, NULL );
							timer().template set_timer<self_type, &self_type::more_dio_timer_elapsed>( 100, this, 0 );
							return Radio_IP::CORRECT;
						}

						return Radio_IP::DROP_PACKET;
					}
			
					#ifdef ROUTING_RPL_DEBUG
					debug().debug( "\nRPL Routing: Source. Forwarding message to default route %s for destination %s \n", preferred_parent_.get_address(str), destination.get_address(str2));
					#endif
					
					data_pointer[2] = 0;
					
				}
				#ifdef ROUTING_RPL_DEBUG
				print_neighbor_set();
				print_parent_set();
				radio_ip().routing_.print_forwarding_table();
				#endif
				return Radio_IP::CORRECT;
			}
			
			else
			{
				
				//INTERMEDIATE NODE
				uint8_t flags = data_pointer[2];
				uint8_t down = (flags >> 7);
				uint8_t rank_error = (flags << 1);
				rank_error = ( rank_error >> 7 );
				uint8_t forwarding_error = (flags << 2 );
				forwarding_error = (forwarding_error << 7 );
	
				uint16_t sender_rank = ( data_pointer[4] << 8 ) | data_pointer[5];
				uint16_t compare_rank = DAGRank( sender_rank );
				#ifdef ROUTING_RPL_DEBUG
				debug().debug( "\nRPL Routing: INTERMEDIATE NODE, my rank is %i\n", DAGRank( rank_ ) );
				#endif
				if( compare_rank == 0 )
				{
					//This is the first router ==> add rank, of course don't check consistency
					data_pointer[4] = (uint8_t) (rank_ >> 8 );
					data_pointer[5] = (uint8_t) (rank_ );
					
					if( it != radio_ip().routing_.forwarding_table_.end() )
					{
						if( forwarding_error == 1 )
						{
							#ifdef ROUTING_RPL_DEBUG
							debug().debug( "\nRPL Routing: 1st INTERMEDIATE NODE, Packet returned with Forwarding error. Clean entry in FT (for destination %s) and send the packet to the default route %s\n", destination.get_address(str), preferred_parent_.get_address(str2) );
							#endif
							
							radio_ip().routing_.forwarding_table_.erase( it );
							data_pointer[2] = 0;
			
							data_pointer[4] = 0;
							data_pointer[5] = 0;

						}
						else
						{
							#ifdef ROUTING_RPL_DEBUG
							debug().debug( "\nRPL Routing: 1st INTERMEDIATE NODE, FT contains destination %s, next hop is %s\n", destination.get_address(str), it->second.next_hop.get_address(str2) );
							#endif
							if( !is_reachable( it->second.next_hop ) )
							{
								#ifdef ROUTING_RPL_DEBUG
								debug().debug( "\nRPL Routing: Destination unreachable 1. Sending No-Path DAO upwards.\n");
								#endif
								//send No-path DAO specifying the target!
								send_no_path_dao( destination );
								//delete entry
								radio_ip().routing_.forwarding_table_.erase( it );
								return Radio_IP::DROP_PACKET;
							}
					
							data_pointer[2] = 128;
						}
					
						#ifdef ROUTING_RPL_DEBUG
						print_neighbor_set();
						print_parent_set();
						radio_ip().routing_.print_forwarding_table();
						#endif
						return Radio_IP::CORRECT;
					}
					else
					{
						if ( preferred_parent_ == my_address_ )
						{
							#ifdef ROUTING_RPL_DEBUG
							debug().debug( "\nRPL Routing: ROOT, cannot go up again: DAO update for destination %s not yet received\n", destination.get_address( str ) );
							#endif
				
							//FIRST CHECK IF THE DESTINATION IS OUTSIDE THE DODAG, IF SO THE PACKET MUST BE FORWARDED OUTSIDE
							return  Radio_IP::DROP_PACKET;
						}
						else
						{	
							if( down == 1 )
							{
								#ifdef ROUTING_RPL_DEBUG
								debug().debug( "\nRPL Routing: INTERMEDIATE NODE 1st, Forwarding Error, send back to the preferred parent (source) \n" );
								#endif
								
								data_pointer[4] = 0;
								data_pointer[5] = 0;
			
								data_pointer[2] = 32;
							}
							else
							{
								#ifdef ROUTING_RPL_DEBUG
								debug().debug( "\nRPL Routing: 1st Interm Node. Up again. Forward to default route %s for destination %s \n", preferred_parent_.get_address(str), destination.get_address(str2) );
								#endif	
				
								data_pointer[2] = 0;
								
							}

							if( !is_reachable( preferred_parent_ ) )
							{
								parent_set_.erase( preferred_parent_ );
								
								#ifdef ROUTING_RPL_DEBUG
								debug().debug( "\nRPLRouting: Preferred Parent Unreachable: FINDING NEW PREFERRED PARENT\n" );
								#endif
								node_id_t best = Radio_IP::NULL_NODE_ID;
								uint16_t current_best_path_cost = 0xFFFF;
								uint16_t best_parent_rank = 0xFFFF;
		
								for (ParentSet_iterator it = parent_set_.begin(); it != parent_set_.end(); it++)
								{
									if ( it->second.path_cost < current_best_path_cost )
									{
										current_best_path_cost = it->second.path_cost;
										best = it->first;
										best_parent_rank = it->second.rank;
									}
								}
								
								if( best == Radio_IP::NULL_NODE_ID )
								{

									#ifdef ROUTING_RPL_DEBUG
									debug().debug( "\nRPLRouting: NO MORE PARENTS, POISON SUB-DODAG\n" );
									#endif
									old_preferred_parent_ = preferred_parent_;
									transient_preferred_parent_ = old_preferred_parent_;
									timer().template set_timer<self_type, &self_type::transient_parent_timer_elapsed>( 5000, this, 0 );
									preferred_parent_ = Radio_IP::NULL_NODE_ID;
									cur_min_path_cost_ = 0xFFFF;
									
									rank_ = INFINITE_RANK;

									stop_dio_timer_ = true;
	
									dio_message_->template set_payload<uint16_t>( &rank_, 6, 1 );
									
									//solicit neighborhood
									send_dis( Radio_IP::BROADCAST_ADDRESS, dis_reference_number_, NULL );
									radio_ip().send( Radio_IP::BROADCAST_ADDRESS, dio_reference_number_, NULL );
								}
								else
								{

									#ifdef ROUTING_RPL_DEBUG
									debug().debug( "\nRPLRouting: DATA-path New Preferred Parent is %s, new path cost: %i, parent_rank: %i\n", best.get_address( str ), current_best_path_cost, best_parent_rank );
									#endif
									dtsn_ = dtsn_ + 1;
									
									dio_message_->template set_payload<uint8_t>( &dtsn_, 9, 1 );

									block_data_t *data = dio_message_->payload();
								
									update_dio( best, current_best_path_cost);
																	
									find_worst_parent();

									if( state_ == Leaf )
									{
										
										dao_received_ = false;
										state_ = Connected;
										#ifdef ROUTING_RPL_DEBUG
										debug().debug( "\nRPLRouting: REACTIVATE TIMERS\n" );
										#endif
										timer().template set_timer<self_type, &self_type::leaf_timer_elapsed>(  current_interval_ + 2500, this, 0 );
										timer().template set_timer<self_type, &self_type::timer_elapsed>( current_interval_, this, 0 );
										timer().template set_timer<self_type, &self_type::threshold_timer_elapsed>( sending_threshold_, this, 0 );
									
									}
									
									
									#ifdef ROUTING_RPL_DEBUG
									debug().debug( "\nRPLRouting: dio timer is %i, sending_threshold is %i\n", current_interval_, sending_threshold_ );
									#endif									


									dao_sequence_ = dao_sequence_ + 1;
									dao_message_->template set_payload<uint8_t>( &dao_sequence_, 7, 1 );
									path_sequence_ = path_sequence_ + 1;
									dao_message_->template set_payload<uint8_t>( &path_sequence_, 48, 1 );

									if( dao_ack_received_ )
									{
										dao_ack_received_ = false;
										timer().template set_timer<self_type, &self_type::dao_timer_elapsed>( 300, this, 0 );
									}
									radio_ip().send( Radio_IP::BROADCAST_ADDRESS, dio_reference_number_, NULL );
									timer().template set_timer<self_type, &self_type::more_dio_timer_elapsed>( 100, this, 0 );
									return Radio_IP::CORRECT;
								}


								return Radio_IP::DROP_PACKET;
							}


							#ifdef ROUTING_RPL_DEBUG
							print_neighbor_set();
							print_parent_set();
							radio_ip().routing_.print_forwarding_table();
							#endif
							return Radio_IP::CORRECT;	
						}
					}
					
				}
				else
				{
					//This is not the first router ==> check rank
					
					if( ( down == 1 && compare_rank > DAGRank( rank_ ) ) || (down == 0 && compare_rank < DAGRank( rank_ ) ) ) 
					{
						//inconsistency
						if ( rank_error == 1 )
						{
							#ifdef ROUTING_RPL_DEBUG
							debug().debug( "\nRPL Routing: INTERMEDIATE NODE, 2nd Inconsistency ==> DROP\n" );
							#endif
							if( state_ != Dodag_root )
							{
								
								//Local Repair
								send_no_path_dao( my_global_address_ );

								stop_dio_timer_ = true;

								rank_ = INFINITE_RANK;
								update_dio( Radio_IP::NULL_NODE_ID ,rank_ );

								radio_ip().send( Radio_IP::BROADCAST_ADDRESS, dio_reference_number_, NULL );
								
								
								timer().template set_timer<self_type, &self_type::more_dio_timer_elapsed>( 100, this, 0 );

								send_dis( Radio_IP::BROADCAST_ADDRESS, dis_reference_number_, NULL );
		
							}
							else{
								#ifdef ROUTING_RPL_DEBUG
								debug().debug( "\nRPL Routing: GLOBAL REPAIR\n" );
								#endif
								//GLOBAL REPAIR
								version_number_ = version_number_ + 1;
						
								stop_dio_timer_ = true;
		
								start();
							}
								
							return Radio_IP::DROP_PACKET;
						}
						else
						{	
							#ifdef ROUTING_RPL_DEBUG
							debug().debug( "\nRPL Routing: INTERMEDIATE NODE, 1st Inconsistency\n" );
							#endif
							if ( down == 1 )
								flags = 192;
							else
								flags = 64;
							data_pointer[2] = flags;
						
						}

						send_dis( dodag_id_, dis_reference_number_, NULL );
					}
					
					if( it != radio_ip().routing_.forwarding_table_.end() )
					{
						//GO DOWN
						if( down == 1 )
						{ 
							//DOWN AGAIN
							#ifdef ROUTING_RPL_DEBUG
							debug().debug( "\nRPL Routing: INTERMEDIATE NODE, FT contains destination %s, next hop is %s\n", destination.get_address(str), it->second.next_hop.get_address(str2) );
							#endif
							if( !is_reachable( it->second.next_hop ) )
							{
								#ifdef ROUTING_RPL_DEBUG
								debug().debug( "\nRPL Routing: Destination unreachable 2. Sending No-Path DAO upwards.\n");
								#endif
								send_no_path_dao( destination );
								
								radio_ip().routing_.forwarding_table_.erase( it );
								return Radio_IP::DROP_PACKET;

							}
					
							if( rank_error == 1 )
								data_pointer[2] = 192;
							else
								data_pointer[2] = 128;
						}
						else    
						{
							//WAS UP
							if( forwarding_error == 1 )
							{
								#ifdef ROUTING_RPL_DEBUG
								debug().debug( "\nRPL Routing: INTERMEDIATE NODE, Packet returned with Forwarding error. Clean entry in FT (for destination %s) and send the packet to the default route %s\n", destination.get_address(str), preferred_parent_.get_address(str2) );
								#endif
							
								radio_ip().routing_.forwarding_table_.erase( it );
								if( rank_error == 1 )
									data_pointer[2] = 64;
								else
									data_pointer[2] = 0;

							}
							else
							{
								#ifdef ROUTING_RPL_DEBUG
								debug().debug( "\nRPL Routing: INTERMEDIATE NODE, FT contains destination %s, next hop is %s, CHANGE DIRECTION\n", destination.get_address(str), it->second.next_hop.get_address(str2) );
								#endif
								if( !is_reachable( it->second.next_hop ) )
								{
									#ifdef ROUTING_RPL_DEBUG
									debug().debug( "\nRPL Routing: Destination unreachable 3. Sending No-Path DAO upwards.\n");
									#endif
									
									send_no_path_dao( destination );
									
									radio_ip().routing_.forwarding_table_.erase( it );
									return Radio_IP::DROP_PACKET;
								}
					
								if( rank_error == 1 )
									data_pointer[2] = 192;
								else
									data_pointer[2] = 128;
							
							}
						}
						data_pointer[4] = (uint8_t) (rank_ >> 8 );
						data_pointer[5] = (uint8_t) (rank_ );
						#ifdef ROUTING_RPL_DEBUG
						print_neighbor_set();
						print_parent_set();
						radio_ip().routing_.print_forwarding_table();
						#endif
						return Radio_IP::CORRECT;
					}
					else
					{
						if ( preferred_parent_ == my_address_ )
						{
							#ifdef ROUTING_RPL_DEBUG
							debug().debug( "\nRPL Routing: ROOT, cannot go up again: DAO update for destination %s not yet received\n", destination.get_address( str ) );
							#endif
							#ifdef ROUTING_RPL_DEBUG
							print_neighbor_set();
							print_parent_set();
							radio_ip().routing_.print_forwarding_table();
							#endif
							return  Radio_IP::DROP_PACKET;
						}
		
						if( down == 1 )
						{
							//WAS DOWN
							#ifdef ROUTING_RPL_DEBUG
							debug().debug( "\nRPL Routing: INTERMEDIATE NODE, going up again again. Forward packet to default route %s for destination %s, \n", preferred_parent_.get_address(str2), destination.get_address(str));
							#endif
							
							if( rank_error == 1 )
								data_pointer[2] = 96;
							else
								data_pointer[2] = 32;
						}
			
						else
						{
							//WAS UP
							#ifdef ROUTING_RPL_DEBUG
							debug().debug( "\nRPL Routing: INTERMEDIATE NODE, going up again again. Forward packet to default route %s for destination %s, \n", preferred_parent_.get_address(str2), destination.get_address(str));
							#endif
						}
						

						if( !is_reachable( preferred_parent_ ) )
						{
							parent_set_.erase( preferred_parent_ );
							#ifdef ROUTING_RPL_DEBUG
							debug().debug( "\nRPLRouting: Preferred Parent Unreachable: FINDING NEW PREFERRED PARENT\n" );
							#endif
							node_id_t best = Radio_IP::NULL_NODE_ID;
							uint16_t current_best_path_cost = 0xFFFF;
							uint16_t best_parent_rank = 0xFFFF;
		
							for (ParentSet_iterator it = parent_set_.begin(); it != parent_set_.end(); it++)
							{
								if ( it->second.path_cost < current_best_path_cost )
								{
									current_best_path_cost = it->second.path_cost;
									best = it->first;
									best_parent_rank = it->second.rank;
								}
							}
								
							if( best == Radio_IP::NULL_NODE_ID )
							{
								#ifdef ROUTING_RPL_DEBUG
								debug().debug( "\nRPLRouting: NO MORE PARENTS, POISON SUB-DODAG\n" );
								#endif
								old_preferred_parent_ = preferred_parent_;
								transient_preferred_parent_ = old_preferred_parent_;
								timer().template set_timer<self_type, &self_type::transient_parent_timer_elapsed>( 5000, this, 0 );
								preferred_parent_ = Radio_IP::NULL_NODE_ID;
								cur_min_path_cost_ = 0xFFFF;
								
								rank_ = INFINITE_RANK;

								stop_dio_timer_ = true;
	
								dio_message_->template set_payload<uint16_t>( &rank_, 6, 1 );
								
								send_dis( Radio_IP::BROADCAST_ADDRESS, dis_reference_number_, NULL );
								
								radio_ip().send( Radio_IP::BROADCAST_ADDRESS, dio_reference_number_, NULL );
							}
							else
							{
								#ifdef ROUTING_RPL_DEBUG
								debug().debug( "\nRPLRouting: New Preferred Parent is %s, new path cost: %i, parent_rank: %i:\n", best.get_address( str ), current_best_path_cost, best_parent_rank );
								#endif
								dtsn_ = dtsn_ + 1;
								
								dio_message_->template set_payload<uint8_t>( &dtsn_, 9, 1 );
								
								update_dio( best, current_best_path_cost);
								
								find_worst_parent();
						
								if( state_ == Leaf )
								{
									dao_received_ = false;
									state_ = Connected;
			
									timer().template set_timer<self_type, &self_type::leaf_timer_elapsed>(  current_interval_ + 2500, this, 0 );
									timer().template set_timer<self_type, &self_type::timer_elapsed>( current_interval_, this, 0 );
									timer().template set_timer<self_type, &self_type::threshold_timer_elapsed>( sending_threshold_, this, 0 );
									
								}
							
								dao_sequence_ = dao_sequence_ + 1;
								dao_message_->template set_payload<uint8_t>( &dao_sequence_, 7, 1 );
								path_sequence_ = path_sequence_ + 1;
								dao_message_->template set_payload<uint8_t>( &path_sequence_, 48, 1 );		

								if( dao_ack_received_ )
								{
									dao_ack_received_ = false;
									timer().template set_timer<self_type, &self_type::dao_timer_elapsed>( 300, this, 0 );
								}
		
								radio_ip().send( Radio_IP::BROADCAST_ADDRESS, dio_reference_number_, NULL );
								timer().template set_timer<self_type, &self_type::more_dio_timer_elapsed>( 100, this, 0 );
								
								return Radio_IP::CORRECT;
							}

							return Radio_IP::DROP_PACKET;
						}
						
						data_pointer[4] = (uint8_t) (rank_ >> 8 );
						data_pointer[5] = (uint8_t) (rank_ );
						
						#ifdef ROUTING_RPL_DEBUG
						print_neighbor_set();
						print_parent_set();
						radio_ip().routing_.print_forwarding_table();
						#endif
						return Radio_IP::CORRECT;
					}
				}
			}
		}
	}
	
	// -----------------------------------------------------------------------
	template<typename OsModel_P,
		typename Radio_IP_P,
		typename Radio_P,
		typename Debug_P,
		typename Timer_P/*,
		typename Clock_P*/>
	void
	RPLRouting<OsModel_P, Radio_IP_P, Radio_P, Debug_P, Timer_P/*, Clock_P*/>::
	print_parent_set()
	{
		debug().debug( "\nRPL Routing: Parent Set with relative rank and path cost: \n" );
		int i = 0;
		for( ParentSet_iterator it = parent_set_.begin(); it != parent_set_.end(); it++ )
		{
			char str[43];
			debug().debug( "\n %i: %s, %i, %i ", i, it->first.get_address(str), it->second.rank, it->second.path_cost );
			i = i + 1;
		}
		debug().debug( "\n\n" );
	}

	// -----------------------------------------------------------------------
	template<typename OsModel_P,
		typename Radio_IP_P,
		typename Radio_P,
		typename Debug_P,
		typename Timer_P/*,
		typename Clock_P*/>
	void
	RPLRouting<OsModel_P, Radio_IP_P, Radio_P, Debug_P, Timer_P/*, Clock_P*/>::
	print_neighbor_set()
	{
		char str[43];
		
		debug().debug( "\nRPL Routing: Neighbor Set: \n");
	
		int i = 0;

		for (NeighborSet_iterator it = neighbor_set_.begin(); it != neighbor_set_.end(); it++) 
		{
			char str[43];
		
			debug().debug( "\n %i: %s, %i", i, it->first.get_address(str), it->second);
			
			i = i + 1;
		}
		debug().debug( "\n\n" );
	}

}
#endif
