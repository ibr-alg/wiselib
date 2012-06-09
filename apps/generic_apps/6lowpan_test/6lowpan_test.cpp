
#include "external_interface/external_interface.h"
#include "algorithms/6lowpan/ipv6_address.h"
#include "algorithms/6lowpan/ipv6_packet.h"
#include "algorithms/6lowpan/ipv6.h"
#include "algorithms/6lowpan/lowpan.h"
#include "algorithms/6lowpan/ipv6_stack.h"

//#include "util/serialization/bitwise_serialization.h"

typedef wiselib::OSMODEL Os;
typedef Os::Radio Radio;
typedef Os::Radio::node_id_t node_id_t;



typedef wiselib::IPv6Stack<Os, Radio, Os::Debug> IPv6_stack_t;
typedef wiselib::LoWPAN<Os, Radio, Os::Debug> LoWPAN_t;
typedef wiselib::IPv6<Os, LoWPAN_t, Os::Debug> IPv6_t;
typedef wiselib::IPv6Address<LoWPAN_t, Os::Debug> IPv6Address_t;
typedef wiselib::IPv6Packet<Os, IPv6_t, LoWPAN_t, Os::Debug> IPv6Packet_t;

class lowpanApp
{
  public:
    typedef Radio::block_data_t block_data_t;
    typedef Radio::node_id_t node_id_t;
    typedef Radio::size_t size_t;
    typedef Radio::message_id_t message_id_t;
    
      void init( Os::AppMainParameter& value )
      {
         radio_ = &wiselib::FacetProvider<Os, Os::Radio>::get_facet( value );
         timer_ = &wiselib::FacetProvider<Os, Os::Timer>::get_facet( value );
         debug_ = &wiselib::FacetProvider<Os, Os::Debug>::get_facet( value );

         debug_->debug( "Booting with ID: %d\n", radio_->id());
	 
	 ipv6_stack_.init(*radio_, *debug_);
	 callback_id = ipv6_stack_.ipv6.reg_recv_callback<lowpanApp,&lowpanApp::receive_radio_message>( this );
	 
	 //NOTE Test IP packet
	 IPv6Address_t sourceaddr;
	 IPv6Address_t destinationaddr;
	 IPv6Packet_t message;
	 
	 destinationaddr.set_debug( *debug_ );
	 destinationaddr.make_it_link_local();
	
	 
	 node_id_t ll_id = 1;
	 destinationaddr.set_long_iid(&ll_id, false);
	 
	 uint8_t mypayload[8];
	 mypayload[0]='h';
	 mypayload[1]='e';
	 mypayload[2]='l';
	 mypayload[3]='l';
	 mypayload[4]='o';
	 mypayload[5]=' ';
	 mypayload[6]=':';
	 mypayload[7]=')';	
	 
	 //destinationaddr.print_address();
	 
	 //These will be in the UDP layer!
	 //Next header = 17 UDP
	 //TODO hop limit?
	 sourceaddr = ipv6_.id();
	 message.init(17,100,8,mypayload,sourceaddr,destinationaddr,0,0);
	 
	 //It will be routed: 0 --> 2 ---> 1
	 if(radio_->id() == 0)
	 	ipv6_stack_.ipv6.send(destinationaddr,message.get_content_size(),message.get_content());
	//NOTE END
	 

	 
         //timer_->set_timer<lowpanApp,&lowpanApp::broadcast_loop>( 3000, this, 0 );
      }
      // --------------------------------------------------------------------
      void broadcast_loop( void* )
      {
	 /*debug_->debug( "broadcasting message at %x\n", radio_->id() );
         Os::Radio::block_data_t message[] = "ID-collision test\0";
         radio_->send( Os::Radio::BROADCAST_ADDRESS, sizeof(message), message );*/

	 //timer_->set_timer<lowpanApp,&lowpanApp::broadcast_loop>( 3000, this, 0 );
      }
      // --------------------------------------------------------------------
      void receive_radio_message( IPv6Address_t from, Os::Radio::size_t len, Os::Radio::block_data_t *buf )
      {

         debug_->debug( "Application layer received msg at %x from ", radio_->id() );
	 from.print_address();
	 debug_->debug( " Content: " );
	 for(int i = 0; i < len; i++)
	 	debug_->debug( "%c", buf[i]);
	 debug_->debug( "\n" );
	 

      }
   private:
      int callback_id;
      IPv6_t ipv6_;
      LoWPAN_t lowpan_;
      IPv6_stack_t ipv6_stack_;
      Radio::self_pointer_t radio_;
      Os::Timer::self_pointer_t timer_;
      Os::Debug::self_pointer_t debug_;
      Os::Rand::self_pointer_t rand_;
};
// --------------------------------------------------------------------------
wiselib::WiselibApplication<Os, lowpanApp> example_app;
// --------------------------------------------------------------------------
void application_main( Os::AppMainParameter& value )
{
  example_app.init( value );
}
