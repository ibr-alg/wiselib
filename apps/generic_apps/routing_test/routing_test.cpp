/*
 * Test different routing algorithms
 */

/*
1. Tree Routing
2. Tree Routing with Neighbor Discovery
3. DSDV Routing
4. DSDV Routing with Neighbor Discovery
5. DSR Routing
6. Flooding
*/



#include "external_interface/external_interface.h"
#include "internal_interface/routing_table/routing_table_static_array.h"
#include "util/pstl/vector_static.h"
#include "algorithms/routing/tree/tree_routing.h"
#include "algorithms/routing/dsdv/dsdv_routing.h"
#include "algorithms/routing/dsr/dsr_routing.h"
#include "algorithms/routing/flooding/flooding_algorithm.h"
#include "algorithms/routing/dsdv_routing_ndis/dsdv_routing_ndis.h"
#include "algorithms/routing/tree_routing_ndis/tree_routing_ndis.h"
#include "algorithms/neighbor_discovery/echo.h"


typedef wiselib::OSMODEL Os;

// --------------------------------------------------------------------------
typedef wiselib::TreeRouting<Os, Os::Radio, Os::Timer, Os::Debug> tree_routing_t;

class TreeRoutingApplication
{
   public:
      void init( Os::AppMainParameter& value )
      {
         radio_ = &wiselib::FacetProvider<Os, Os::Radio>::get_facet( value );
         timer_ = &wiselib::FacetProvider<Os, Os::Timer>::get_facet( value );
         debug_ = &wiselib::FacetProvider<Os, Os::Debug>::get_facet( value );
         debug_->debug( "INIT TREEROUTING over app_main in Wiselib\n" );

         tree_routing.init( *radio_, *timer_, *debug_ );
         tree_routing.set_sink( false );
         tree_routing.enable_radio();

         timer_->set_timer<TreeRoutingApplication,
                         &TreeRoutingApplication::start>( 500, this, 0 );
      }
      // --------------------------------------------------------------------
      void start( void* )
      {
         tree_routing.send( 1, 0, 0 );
         timer_->set_timer<TreeRoutingApplication,
                         &TreeRoutingApplication::start>( 2000, this, 0 );
      }
   private:
      tree_routing_t tree_routing;
      Os::Radio::self_pointer_t radio_;
      Os::Timer::self_pointer_t timer_;
      Os::Debug::self_pointer_t debug_;
};

// --------------------------------------------------------------------------
// --------------------------------------------------------------------------
// --------------------------------------------------------------------------

typedef wiselib::Echo<Os, Os::Radio, Os::Timer, Os::Debug> echo_t;
typedef wiselib::TreeRoutingNdis<Os, Os::Radio, Os::Clock, Os::Timer, echo_t, Os::Debug> tree_routing_ndis_t;

class TreeRoutingNdisApplication
{
	public:
	void init( Os::AppMainParameter& value )
		{
		clock_ = &wiselib::FacetProvider<Os, Os::Clock>::get_facet( value );
		radio_ = &wiselib::FacetProvider<Os, Os::Radio>::get_facet( value );
         	timer_ = &wiselib::FacetProvider<Os, Os::Timer>::get_facet( value );
         	debug_ = &wiselib::FacetProvider<Os, Os::Debug>::get_facet( value );
         	debug_->debug( "INIT TREEROUTING with Neighborhood Discovery over app_main in Wiselib\n" );
		
         	echo_test.init(*radio_, *clock_, *timer_, *debug_); 
         	tree_routing_ndis.init(*radio_, *clock_, *timer_, echo_test, *debug_);
         	tree_routing_ndis.set_sink( false );
         	tree_routing_ndis.enable_radio();
       		timer_->set_timer<TreeRoutingNdisApplication, &TreeRoutingNdisApplication::start>( 500, this, 0 );
       		tree_routing_ndis.reg_recv_callback<TreeRoutingNdisApplication, &TreeRoutingNdisApplication::receive_tree>( this );
       		}
       		// .....................................................................................................................
       		void start( void* )
      		{
		tree_routing_ndis.send( 1, 0, 0 );
		echo_test.register_debug_callback(0);
         	timer_->set_timer<TreeRoutingNdisApplication, &TreeRoutingNdisApplication::start>( 2000, this, 0 );
		}
		//......................................................................................................................
		void receive_tree( Os::Radio::node_id_t from, Os::Radio::size_t len, Os::Radio::block_data_t *data ) 
      		{
      		debug_->debug("Received TREE Message at %u from %u and data = %s\n", radio_->id(), from, data);
      		}
      		
      		//......................................................................................................................
      	private:
		echo_t echo_test;
		tree_routing_ndis_t tree_routing_ndis;
		Os::Clock::self_pointer_t clock_;
		Os::Radio::self_pointer_t radio_;
		Os::Timer::self_pointer_t timer_;
      		Os::Debug::self_pointer_t debug_;


};
// --------------------------------------------------------------------------

typedef wiselib::StaticArrayRoutingTable<Os, Os::Radio, 8,wiselib::DsdvRoutingTableValue<Os, Os::Radio> > RoutingTable;
typedef wiselib::DsdvRouting<Os, RoutingTable> DsdvRouting;
// --------------------------------------------------------------------------
class DsdvRoutingApplication
{
   public:
      void init( Os::AppMainParameter& value )
      {
         radio_ = &wiselib::FacetProvider<Os, Os::Radio>::get_facet( value );
         timer_ = &wiselib::FacetProvider<Os, Os::Timer>::get_facet( value );
         debug_ = &wiselib::FacetProvider<Os, Os::Debug>::get_facet( value );

         debug_->debug( "init dsdv application at %u\n", radio_->id() );

         routing_.init( *radio_, *timer_, *debug_ );
         routing_.enable_radio();
         routing_.reg_recv_callback<DsdvRoutingApplication, &DsdvRoutingApplication::receive_dsdv_message>( this );
         timer_->set_timer<DsdvRoutingApplication, &DsdvRoutingApplication::start>( 500, this, 0 );
      }
      // --------------------------------------------------------------------
      void start( void* )
      {
         Os::Radio::block_data_t message[] = "test dsdv\0";
         routing_.send( 1, sizeof(message), message );

         timer_->set_timer<DsdvRoutingApplication, &DsdvRoutingApplication::start>( 20000, this, 0 );
      }
      // --------------------------------------------------------------------
      void receive_dsdv_message( Os::Radio::node_id_t from, Os::Radio::size_t len, Os::Radio::block_data_t *buf )
      {
         debug_->debug( "received dsdv message at %u from %u and data = %s\n", radio_->id(), from, buf );
      }

   private:
      DsdvRouting routing_;
      Os::Radio::self_pointer_t radio_;
      Os::Timer::self_pointer_t timer_;
      Os::Debug::self_pointer_t debug_;
};

// --------------------------------------------------------------------------
// --------------------------------------------------------------------------
typedef wiselib::Echo<Os, Os::Radio, Os::Timer, Os::Debug> echo_t;
typedef wiselib::StaticArrayRoutingTable<Os, Os::Radio, 8,wiselib::DsdvRoutingTableValue<Os, Os::Radio> > RoutingTable;
typedef wiselib::DsdvRoutingNdis<Os, RoutingTable, Os::Radio, Os::Clock, Os::Timer, echo_t, Os::Debug> DsdvRoutingNdis;
// --------------------------------------------------------------------------
class DsdvRoutingNdisApplication
{
   public:
      void init( Os::AppMainParameter& value )
      {
         clock_ = &wiselib::FacetProvider<Os, Os::Clock>::get_facet( value );
         radio_ = &wiselib::FacetProvider<Os, Os::Radio>::get_facet( value );
         timer_ = &wiselib::FacetProvider<Os, Os::Timer>::get_facet( value );
         debug_ = &wiselib::FacetProvider<Os, Os::Debug>::get_facet( value );

         debug_->debug( "init dsdv application at %u\n", radio_->id() );
         echo_test.init(*radio_, *clock_, *timer_, *debug_);
         routing_.init(*radio_, *clock_, *timer_, echo_test, *debug_);
         routing_.enable_radio();
         routing_.reg_recv_callback<DsdvRoutingNdisApplication, &DsdvRoutingNdisApplication::receive_dsdv_message>( this );
         timer_->set_timer<DsdvRoutingNdisApplication, &DsdvRoutingNdisApplication::start>( 500, this, 0 );
      }
      // --------------------------------------------------------------------
      void start( void* )
      {
         Os::Radio::block_data_t message[] = "test dsdv\0";
         routing_.send( 1, sizeof(message), message );
         echo_test.register_debug_callback(0);
         timer_->set_timer<DsdvRoutingNdisApplication, &DsdvRoutingNdisApplication::start>( 20000, this, 0 );
      }
      // --------------------------------------------------------------------
      void receive_dsdv_message( Os::Radio::node_id_t from, Os::Radio::size_t len, Os::Radio::block_data_t *buf )
      {
         debug_->debug( "received dsdv message at %u from %u and data = %s\n", radio_->id(), from, buf );
      }

   private:
      echo_t echo_test;
      DsdvRoutingNdis routing_;
      Os::Clock::self_pointer_t clock_;
      Os::Radio::self_pointer_t radio_;
      Os::Timer::self_pointer_t timer_;
      Os::Debug::self_pointer_t debug_;
};

// --------------------------------------------------------------------------
// --------------------------------------------------------------------------

// --------------------------------------------------------------------------
// --------------------------------------------------------------------------

typedef wiselib::vector_static<Os, Os::Radio::node_id_t, 8> NodeVector;
typedef wiselib::StaticArrayRoutingTable<Os, Os::Radio, 10, wiselib::DsrRoutingTableValue<Os::Radio, NodeVector> > DsrRoutingTable;
typedef wiselib::DsrRouting<Os, DsrRoutingTable> DsrRouting;
// --------------------------------------------------------------------------
class DsrRoutingApplication
{
   public:
      void init( Os::AppMainParameter& value )
      {
         radio_ = &wiselib::FacetProvider<Os, Os::Radio>::get_facet( value );
         timer_ = &wiselib::FacetProvider<Os, Os::Timer>::get_facet( value );
         debug_ = &wiselib::FacetProvider<Os, Os::Debug>::get_facet( value );

         debug_->debug( "init dsr application at %u\n", radio_->id() );

         routing_.init( *radio_, *timer_, *debug_ );
         routing_.enable_radio();
         int r1 = routing_.reg_recv_callback<DsrRoutingApplication, &DsrRoutingApplication::receive_dsr_message>( this );
        // int r2 = routing_.reg_recv_callback<DsrRoutingApplication, &DsrRoutingApplication::receive_dsr_message2>( this );
         int r3 = routing_.reg_recv_callback<DsrRoutingApplication, &DsrRoutingApplication::receive_dsr_message3>( this );

         routing_.unreg_recv_callback( r1 );
//          routing_.unreg_recv_callback( r2 );
         routing_.unreg_recv_callback( r3 );
         timer_->set_timer<DsrRoutingApplication, &DsrRoutingApplication::start>( 500, this, 0 );
      }
      // --------------------------------------------------------------------
      void start( void* )
      {
         if ( radio_->id() >= 3 && radio_->id() <= 6 )
         {
            Os::Radio::block_data_t message[] = "hi via dsr!\0";
            routing_.send( 4, sizeof(message), message );
         }
         timer_->set_timer<DsrRoutingApplication, &DsrRoutingApplication::start>( 5000, this, 0 );
      }
      // --------------------------------------------------------------------
      void receive_dsr_message( Os::Radio::node_id_t from, Os::Radio::size_t len, Os::Radio::block_data_t *buf )
      {
         debug_->debug( "1: received flooding at %d from %d with %d bytes: '%s'\n", radio_->id(), from, len, buf );
      }
      // --------------------------------------------------------------------
      void receive_dsr_message2( Os::Radio::node_id_t from, Os::Radio::size_t len, Os::Radio::block_data_t *buf )
      {
         debug_->debug( "2: received flooding at %d from %d with %d bytes: '%s'\n", radio_->id(), from, len, buf );
      }
      // --------------------------------------------------------------------
      void receive_dsr_message3( Os::Radio::node_id_t from, Os::Radio::size_t len, Os::Radio::block_data_t *buf )
      {
         debug_->debug( "3: received flooding at %d from %d with %d bytes: '%s'\n", radio_->id(), from, len, buf );
      }

   private:
      DsrRouting routing_;
      Os::Radio::self_pointer_t radio_;
      Os::Timer::self_pointer_t timer_;
      Os::Debug::self_pointer_t debug_;
};

// --------------------------------------------------------------------------
// --------------------------------------------------------------------------
// --------------------------------------------------------------------------

typedef wiselib::StaticArrayRoutingTable<Os, Os::Radio, 64> FloodingStaticMap;
typedef wiselib::FloodingAlgorithm<Os, FloodingStaticMap, Os::Radio, Os::Debug> flooding_algorithm_t;

// --------------------------------------------------------------------------
class FloodingApplication
{
   public:
      void init( Os::AppMainParameter& value )
      {
         radio_ = &wiselib::FacetProvider<Os, Os::Radio>::get_facet( value );
         timer_ = &wiselib::FacetProvider<Os, Os::Timer>::get_facet( value );
         debug_ = &wiselib::FacetProvider<Os, Os::Debug>::get_facet( value );

         debug_->debug( "Init flooding application at %u\n", radio_->id() );

         flooding_.init( *radio_, *debug_ );
         flooding_.enable_radio();
         flooding_.reg_recv_callback<FloodingApplication, &FloodingApplication::receive_flooding_message>( this );
         //radio_->reg_recv_callback<FloodingApplication, &FloodingApplication::receive_radio_message>( this );

         timer_->set_timer<FloodingApplication, &FloodingApplication::start>( 5000, this, 0 );
         //timer_->set_timer<FloodingApplication, &FloodingApplication::reset_flooding>( 20000, this, (void*)0 );
      }
      // --------------------------------------------------------------------
      void start( void* )
      {
         debug_->debug( "Send flooding message at %u\n", radio_->id() );
         Os::Radio::block_data_t message[] = "Test\0";
         flooding_.send( Os::Radio::BROADCAST_ADDRESS, sizeof(message), message );

         timer_->set_timer<FloodingApplication, &FloodingApplication::start>( 20000, this, 0 );
      }
      // --------------------------------------------------------------------
      void reset_flooding( void* )
      {
         debug_->debug( "\nRESET flooding at %d \n", radio_->id() );
         flooding_.init();
      }
      // --------------------------------------------------------------------
      void receive_flooding_message( Os::Radio::node_id_t from, Os::Radio::size_t len, Os::Radio::block_data_t *buf )
      {
         debug_->debug( "received flooding at %u from %u and data = %s\n", radio_->id(), from, buf );
      }
      // --------------------------------------------------------------------
      void receive_radio_message( Os::Radio::node_id_t from, Os::Radio::size_t len, Os::Radio::block_data_t *buf )
      {
         printf( "received radio at %u from %u\n", radio_->id(), from );
         printf( "msg len %u, first byte %d\n", len, buf[0] );
      }

   private:
      flooding_algorithm_t flooding_;
      Os::Radio::self_pointer_t radio_;
      Os::Timer::self_pointer_t timer_;
      Os::Debug::self_pointer_t debug_;
};
// --------------------------------------------------------------------------
// --------------------------------------------------------------------------
// --------------------------------------------------------------------------
//wiselib::WiselibApplication<Os, DsrRoutingApplication> routing_app;
//wiselib::WiselibApplication<Os, DsdvRoutingApplication> routing_app;
//wiselib::WiselibApplication<Os, TreeRoutingNdisApplication> routing_app;
wiselib::WiselibApplication<Os, DsdvRoutingNdisApplication> routing_app;
//wiselib::WiselibApplication<Os, FloodingApplication> routing_app;
// --------------------------------------------------------------------------
void application_main( Os::AppMainParameter& value )
{
   routing_app.init( value );
}

// #include "external_interface/external_interface.h"
// #include "internal_interface/routing_table/routing_table_static_array.h"
// #include "algorithms/routing/flooding_algorithm.h"
// 
// typedef wiselib::OSMODEL Os;
// typedef wiselib::StaticArrayRoutingTable<Os, Os::Radio, 8> FloodingStaticMap;
// typedef wiselib::FloodingAlgorithm<Os, FloodingStaticMap, Os::Radio, Os::Debug> flooding_algorithm_t;
// 
// flooding_algorithm_t flooding;
// // --------------------------------------------------------------------------
// void application_main( void )
// {
//    flooding.set_os( Os::get_firmware_instance() );
//    flooding.enable();
//    flooding.send( 1, 0, 0 );
// }




// #include "external_interface/external_interface.h"
// #include "internal_interface/routing_table/routing_table_static_array.h"
// #include "util/pstl/list_static.h"
// #include "algorithms/routing/aodv_routing.h"
// 
// typedef uint16 uint16_t;
// // --------------------------------------------------------------------------
// typedef wiselib::OSMODEL Os;
// typedef wiselib::StaticArrayRoutingTable<Os, Os::Radio, 8, wiselib::AODVRouteDiscoveryMessage<Os, Os::Radio, int> >
//    RoutingTable;
// typedef wiselib::StaticArrayRoutingTable<Os, Os::Radio, 8, uint16_t> IntIntTable;
// typedef wiselib::StaticArrayRoutingTable<Os, Os::Radio, 8, wiselib::retry_info > IntRetryTable;
// typedef wiselib::list_static<Os, wiselib::rreq_info, 10> RreqList;
// 
// typedef wiselib::AODVRouting<Os, RoutingTable, IntIntTable, IntRetryTable, RreqList> aodv_routing_t;
// 
// aodv_routing_t aodv_routing;
// // --------------------------------------------------------------------------
// void application_main( void )
// {
//    aodv_routing.set_os( Os::get_firmware_instance() );
//    aodv_routing.enable();
// //    aodv_routing.send( 1, 0, 0 );
// }
