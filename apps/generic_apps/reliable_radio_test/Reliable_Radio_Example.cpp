/*
 * Simple Wiselib Example
 */
#include "external_interface/external_interface_testing.h"
#include "radio/reliable/reliableradio.h"

typedef wiselib::OSMODEL Os;
typedef wiselib::ReliableRadio<Os, Os::Radio, Os::Timer, Os::Debug> reliable_radio_t;

//typedef typename Os::Radio::node_id_t node_id_t;

typedef Os::Radio::node_id_t node_id_t;
typedef Os::Radio::block_data_t block_data_t;


class ReliableRadioExample
{

    typedef reliable_radio_t::size_t size_t;

   public:
      void init( Os::AppMainParameter& value )
      {
         radio_ = &wiselib::FacetProvider<Os, Os::Radio>::get_facet( value );
         timer_ = &wiselib::FacetProvider<Os, Os::Timer>::get_facet( value );
         debug_ = &wiselib::FacetProvider<Os, Os::Debug>::get_facet( value );

         debug_->debug( "Hello World from Example Application!\n" );

         reliable_radio_.init( *radio_, *timer_, *debug_ );
         reliable_radio_.enable_radio();

         node_id_t destination = 0;
         node_id_t source = 0;
         #ifdef SHAWN
            source = 0;
            destination = 2;
         #else
            source = 0xFFFF;
            destination = 0xFFFF;
         #endif

         size_t len=2;
         block_data_t data[2];
         data[0] = 4;
         data[1] = 2;

         if ( radio_->id() == source )
             reliable_radio_.send_callback<ReliableRadioExample,&ReliableRadioExample::callback>(destination,len,data,this);

         timer_->set_timer<ReliableRadioExample,
                           &ReliableRadioExample::start>( 5000, this, 0 );
      }
      // --------------------------------------------------------------------
      void callback( uint8_t event, node_id_t from, size_t len, block_data_t* data)
      {
          if ( reliable_radio_t::MSG_ACK_RCVD == event ) {
              
              debug_->debug( "MSG_ACK_RCVD\n" );
          }
          else if ( reliable_radio_t::MSG_DROPPED == event ) {
              
              debug_->debug( "MSG_DROPPED\n" );
          }
      }
      // --------------------------------------------------------------------
      void start( void* )
      {

         // following can be used for periodic messages to sink
         // timer_->set_timer<ExampleApplication,
         //                  &ExampleApplication::start>( 5000, this, 0 );
      }
   private:
      reliable_radio_t reliable_radio_;
      Os::Radio::self_pointer_t radio_;
      Os::Timer::self_pointer_t timer_;
      Os::Debug::self_pointer_t debug_;
};
// --------------------------------------------------------------------------
wiselib::WiselibApplication<Os, ReliableRadioExample> example_app;
// --------------------------------------------------------------------------
void application_main( Os::AppMainParameter& value )
{
   example_app.init( value );
}
