/*
 * Example application using MQTT-SN protocol
 */

#include "external_interface/external_interface.h"
#include "algorithms/protocols/mqttsn/mqttsn.h"

typedef wiselib::OSMODEL Os;
typedef wiselib::MqttSn<Os, Os::Radio> MqttSnClient;
typedef Os::Radio::block_data_t block_data_t;


class MqttsnExampleApplication
{
   public:

      typedef MqttsnExampleApplication self_type;

      void init( Os::AppMainParameter& value )
      {
         radio_ = &wiselib::FacetProvider<Os, Os::Radio>::get_facet( value );
         timer_ = &wiselib::FacetProvider<Os, Os::Timer>::get_facet( value );
         debug_ = &wiselib::FacetProvider<Os, Os::Debug>::get_facet( value );
         rand_ = &wiselib::FacetProvider<Os, Os::Rand>::get_facet( value );

         debug_->debug( "Hello World from MqttSn Client!\n" );
         timer_->set_timer<MqttsnExampleApplication, &MqttsnExampleApplication::start>( 1000, this, 0 );
      }

      void registerTopic( void* )
      {
          mqttsn_client.send_register( "Light" );
      }

      void subscribeTopic( void* )
      {
          mqttsn_client.send_subscribe( "Light" );
      }

      void sendPublish( void* )
      {
#if defined(ARDUINO)
          uint8_t photoresistor_pin = 0;
          uint16_t photoresistor_value = 0;

          photoresistor_value = analogRead( photoresistor_pin );

          block_data_t message[2];
          wiselib::write<Os, block_data_t, uint16_t>( message, photoresistor_value );

          mqttsn_client.send_publish( "Light", message, sizeof( photoresistor_value ) );
#else
          block_data_t message[] = {'T','e','s','t','\0'};
          mqttsn_client.send_publish( "Light", message, sizeof( message ) );
#endif
      }

      void myRecvPublish( const char* topic_name, block_data_t *data )
      {
#if defined(ARDUINO)
          uint16_t photoresistor_value = wiselib::read<Os, block_data_t, uint16_t>( data );
          debug_->debug("Light: %d", photoresistor_value);

          //repeat publish
          timer_->set_timer<MqttsnExampleApplication, &MqttsnExampleApplication::sendPublish>( 2000, this, 0 );
#else
          debug_->debug("Default");
#endif
      }

      // --------------------------------------------------------------------

      void start( void* )
      {
         // INITIALIZATION
         mqttsn_client.init( *radio_, *debug_, *timer_, *rand_ );

         //CALLBACK REGISTRATION
         mqttsn_client.reg_recv_publish<self_type, &self_type::myRecvPublish>( this );

         //some time to setup connection
         timer_->set_timer<MqttsnExampleApplication, &MqttsnExampleApplication::registerTopic>( 4000, this, 0 );

         //wait 1 sec to subscribe
         timer_->set_timer<MqttsnExampleApplication, &MqttsnExampleApplication::subscribeTopic>( 6000, this, 0 );

         //start publishing after 2 seconds
         timer_->set_timer<MqttsnExampleApplication, &MqttsnExampleApplication::sendPublish>( 8000, this, 0 );
      }

      // --------------------------------------------------------------------

   private:
      Os::Radio::self_pointer_t radio_;
      Os::Timer::self_pointer_t timer_;
      Os::Debug::self_pointer_t debug_;
      Os::Rand::self_pointer_t rand_;
      MqttSnClient mqttsn_client;
};
// --------------------------------------------------------------------------
wiselib::WiselibApplication<Os, MqttsnExampleApplication> example_app;
// --------------------------------------------------------------------------
void application_main( Os::AppMainParameter& value )
{
  example_app.init( value );
}

