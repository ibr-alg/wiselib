/*
 * Simple Neighbor Discovery Example
 */
//#define ISENSE_RADIO_ADDR_TYPE

#include "external_interface/external_interface_testing.h"
//#include "external_interface/external_interface.h"
#include "algorithms/neighbor_discovery/echo.h"
#include "util/delegates/delegate.hpp"
#include "util/pstl/vector_static.h"
#include "util/pstl/pair.h"
#include "util/serialization/serialization.h"

//#define TEST_PIGGYBACKING
//#define ECHO_BENCHMARK

typedef wiselib::OSMODEL Os;
typedef Os::TxRadio Radio;
typedef Radio::block_data_t block_data_t;

typedef wiselib::Echo<Os,Radio,Os::Timer,Os::Debug> nb_t;
typedef Os::Radio::node_id_t node_id_t;
typedef Os::Uart::size_t uart_size_t;

class ExampleApplication
{
   public:
      void init( Os::AppMainParameter& value )
      {

//         os_ = &wiselib::FacetProvider<Os, Os>::get_facet( value );

         radio_ = &wiselib::FacetProvider<Os, Radio>::get_facet( value );
         timer_ = &wiselib::FacetProvider<Os, Os::Timer>::get_facet( value );
         debug_ = &wiselib::FacetProvider<Os, Os::Debug>::get_facet( value );
         clock_ = &wiselib::FacetProvider<Os, Os::Clock>::get_facet( value );
         rand_ = &wiselib::FacetProvider<Os, Os::Rand>::get_facet(value);
         uart_msg_handler_ = &wiselib::FacetProvider<Os, Os::Uart>::get_facet(value);

//         radio_->hardware_radio().set_channel(12);
//         os_.radio().hardware_radio().set_channel(12);

         debug_->debug( "Hello World from Neighbor Discovery Test Application!\n" );

         neighbor_discovery.init( *radio_, *clock_, *timer_, *debug_
                    , 1000, 5000
                    , 160, 200);
         enabled = false;
	 disable = false;
         rand_->srand(radio_->id());

         uint8_t flags = nb_t::NEW_NB | nb_t::NEW_NB_BIDI | nb_t::DROPPED_NB | nb_t::NEW_PAYLOAD_BIDI | nb_t::LOST_NB_BIDI;
         //neighbor_discovery.register_debug_callback(0);

         neighbor_discovery.reg_event_callback<ExampleApplication,&ExampleApplication::callback>( 6, flags, this );

           //        print periodically the neighborhood list

        uart_msg_handler_->reg_read_callback<ExampleApplication, &ExampleApplication::handle_uart_msg > (this);
        uart_msg_handler_->enable_serial_comm();

        neighbor_discovery.enable();

#ifdef SHAWN
//        start(0);
#endif
      }

      /*
       * The callback function that is called by the the neighbor discovery
       * module when a event is generated. The arguments are: the event ID,
       * the node ID that generated the event, the len of the payload ( 0 if
       * this is not a NEW_PAYLOAD event ), the piggybacked payload data.
       */
      // --------------------------------------------------------------------
      void callback( uint8_t event, node_id_t from, uint8_t len, uint8_t* data)
      {
          if ( nb_t::NEW_PAYLOAD == event ) {
              debug_->debug( "event NEW_PAYLOAD!! \n" );
              debug_->debug( "NODE %d: new payload from %d with size %d ",
              radio_->id(), from, len );

              //print payload
              debug_->debug( " [");
              for (uint8_t j = 0; j < len; j++) {
                debug_->debug( "%d ", *(data + j) );
              }
              debug_->debug( "]\n");
          }
          else if ( nb_t::NEW_PAYLOAD_BIDI == event ) {
              debug_->debug( "event NEW_PAYLOAD_BIDI!! \n" );
              debug_->debug( "NODE %d: new payload from %d (bidi) with size %d ",
              radio_->id(), from, len );

              //print payload
              debug_->debug( " [");
              for (uint8_t j = 0; j < len; j++) {
                debug_->debug( "%d ", *(data + j) );
              }
              debug_->debug( "]\n");
          }
          /*
           * +====+====+====+====++====+====++====+====++====+====+
           *  CMD  NODE TIME TIME  NODE NODE  SNBH SNBH
           */
          else if ( nb_t::NEW_NB == event ) {
              debug_->debug( "NEW_NB;%x;Time;%d;Node;%x;neighbors;%d;bd_neighbors;%d;stability;%d" , from, clock_->seconds(clock_->time())+delay, radio_->id(),
neighbor_discovery.stable_nb_size(),neighbor_discovery.bidi_nb_size(),neighbor_discovery.get_node_stability());
//              debug_->debug( "NODE %d: event NEW_NB!! new neighbor added %d \n",radio_->id(), from);
          }
          else if ( nb_t::NEW_NB_BIDI == event ) {
              debug_->debug( "NEW_NB_BIDI;%x;Time;%d;Node;%x;neighbors;%d;bd_neighbors;%d;stability;%d" , from, clock_->seconds(clock_->time())+delay, radio_->id(),
neighbor_discovery.stable_nb_size(),neighbor_discovery.bidi_nb_size(),neighbor_discovery.get_node_stability());
//              debug_->debug( "NODE %d: event NEW_NB_BIDI!! new neighbor added %d \n",radio_->id(), from);
          }
          else if ( nb_t::DROPPED_NB == event ) {
              debug_->debug( "DROPPED_NB;%x;Time;%d;Node;%x;neighbors;%d;bd_neighbors;%d;stability;%d" , from, clock_->seconds(clock_->time())+delay, radio_->id(),
neighbor_discovery.stable_nb_size(),neighbor_discovery.bidi_nb_size(),neighbor_discovery.get_node_stability());
//              debug_->debug( "NODE %d: event DROPED_NB!! neighbor dropped %d \n",radio_->id(), from);
          }
          else if ( nb_t::LOST_NB_BIDI == event ) {
              debug_->debug( "LOST_NB_BIDI;%x;Time;%d;Node;%x;neighbors;%d;bd_neighbors;%d;stability;%d" , from, clock_->seconds(clock_->time())+delay, radio_->id(),
neighbor_discovery.stable_nb_size(),neighbor_discovery.bidi_nb_size(),neighbor_discovery.get_node_stability());
//              debug_->debug( "NODE %d: event LOST_NB_BIDI!! neighbor bidi com lost %d \n",radio_->id(), from);
          }

      }

      void stop ( void* ) {
    	  neighbor_discovery.disable();
      }
      // --------------------------------------------------------------------
      void start( void* ) {

//         debug_->debug( "stability;%d;%x;%d;" ,clock_->seconds(clock_->time()),radio_->id(),neighbor_discovery.get_node_stability());

//         debug_->debug( "Node %x neighbors are:[ " ,radio_->id());


	  if ( disable ) {
              neighbor_discovery.disable();
              return;
          }

          if(clock_->seconds(clock_->time())>350) {
              debug_->debug( "NB_DISABLED;%x;Time;%d;Node;%x;neighbors;%d;bd_neighbors;%d;stability;%d" , radio_->id(), clock_->seconds(clock_->time())+delay, radio_->id(),
neighbor_discovery.stable_nb_size(),neighbor_discovery.bidi_nb_size(),neighbor_discovery.get_node_stability());
              neighbor_discovery.unreg_event_callback(6);
              disable =true;
              timer_->set_timer<ExampleApplication,
                             &ExampleApplication::start>( 12000, this, 0 );
		return;
          }

//          if ( radio_->id() == 0x9700) { //unige
//          if ( radio_->id() == 0x15) {   //cti
//                 for ( nb_t::iterator_t it = neighbor_discovery.neighborhood.begin();
//                         it!= neighbor_discovery.neighborhood.end(); ++it)
//                 {
                     // Print only the neighbors we have bidi links
        //             if (neighbor_discovery.is_neighbor_bidi(it->id)) {
                    // if (it->stable) {
//                         debug_->debug("STATUS;%d;%x;%x;%d\n" ,
  //                               radio_->id(), it->id,
    //                             neighbor_discovery.get_nb_stability(it->id),
      //                           neighbor_discovery.get_node_stability() );
//                     }
//                 }
//          }
//         debug_->debug( " ]\n");

/**
 *   simulate a node failure
 */
//         if (radio_->id()==4 )//&& clock_->seconds(clock_->time())==3)
//                 neighbor_discovery.disable();
//        neighbor_discovery.show_nearby();


//        print periodically the neighborhood list
          timer_->set_timer<ExampleApplication,
                           &ExampleApplication::start>( 1000, this, 0 );

      }
/*
    void handle_uart_msg(uart_size_t len, Os::Uart::block_data_t *data) {
        if (data[0] == 0x1) {
            //if an enable message
            if (data[1] == 0x10) {

                max_threshold = 200;//data[2];
                min_threshold = 160;//data[3];

                beacon_period = 3000; //data[4] * 100;
                timeout_period = 15000; //data[5] * 100;

            	debug_->debug("Node %x enabled TIME [%d] \
            			MAX_THOLD [%d] MIN_THOLD [%d] \
            			TIMEOUT [%d] BEACON [%d]",
            			radio_->id(),clock_->seconds(clock_->time()),
            			max_threshold,min_threshold,
            			beacon_period,timeout_period);

                neighbor_discovery.init( *radio_, *clock_, *timer_, *debug_
                           , beacon_period, timeout_period
                           , min_threshold, max_threshold);

                //Adding a random delay before starting to avoid collisions
               delay = (*rand_)()%10;
               timer_->set_timer<ExampleApplication,
                    &ExampleApplication::start>( 2000+(delay*100), this, 0 );
//         	  return read<OsModel, block_data_t, uint8_t > (buffer + PAYLOAD_POS);
            }
            if (data[1] == 0x20) {
                debug_->debug("Node %x received disable from uart", radio_->id());
                disable = true;
            }
            if (data[1] == 0x30) {
                  if ((radio_->id())%100 > 50) {
                      debug_->debug("Node %x received reset from uart", radio_->id() );
                      neighbor_discovery.disable();
                      neighbor_discovery.enable();
                  }
            }

        }
    }
*/


    void handle_uart_msg(uart_size_t len, Os::Uart::block_data_t *data) {
        if (data[0] == 0x1) {
            //if an enable message
            if (data[1] == 0x1) {
                    debug_->debug("Node %x received enable from uart at ;%d", radio_->id(),clock_->seconds(clock_->time()));

               delay = 0;//(*rand_)()%4;
               timer_->set_timer<ExampleApplication,
                    &ExampleApplication::start>( 2000+(delay*1000), this, 0 );

            }
            if (data[1] == 0x2) {
                debug_->debug("Node %x received disable from uart", radio_->id());
                neighbor_discovery.disable();
            }
        }
    }

   private:
      bool enabled,disable;
      uint8_t delay;
//      uint16_t offset;
      nb_t neighbor_discovery;
      Os os_;
      Radio::self_pointer_t radio_;
      Os::Timer::self_pointer_t timer_;
      Os::Debug::self_pointer_t debug_;
      Os::Clock::self_pointer_t clock_;
      Os::Rand::self_pointer_t rand_;
      Os::Uart::self_pointer_t uart_msg_handler_;
};
// --------------------------------------------------------------------------
wiselib::WiselibApplication<Os, ExampleApplication> example_app;
// --------------------------------------------------------------------------
void application_main( Os::AppMainParameter& value )
{
   example_app.init( value );
}
