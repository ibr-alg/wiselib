/*
* File: 6lowpan_test.cpp
* Author: Daniel Gehberger - GSoC 2012 - 6LoWPAN project
*
* Test application for the IPv6 stack
*
*/


#include "external_interface/external_interface.h"
#include "algorithms/6lowpan/ipv6_address.h"

#include "algorithms/6lowpan/ipv6_stack.h"

#ifdef SHAWN
#include "external_interface/shawn/shawn_testbedservice_uart.h"
#endif

typedef wiselib::OSMODEL Os;
typedef Os::Radio Radio;
typedef Os::Radio::node_id_t node_id_t;

#ifdef SHAWN
typedef wiselib::ShawnTestbedserviceUartModel<wiselib::OSMODEL> Uart;
#else
typedef Os::Uart Uart;
#endif

typedef wiselib::IPv6Stack<Os, Radio, Os::Debug, Os::Timer, Uart> IPv6_stack_t;

typedef wiselib::IPv6Address<Radio, Os::Debug> IPv6Address_t;

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
		uart_ = &wiselib::FacetProvider<Os, Uart>::get_facet( value );

		debug_->debug( "Booting with ID: %x\n", radio_->id());
		
		ipv6_stack_.init(*radio_, *debug_, *timer_, *uart_);
		
		callback_id = ipv6_stack_.icmpv6.reg_recv_callback<lowpanApp,&lowpanApp::receive_echo_reply>( this );
		callback_id = ipv6_stack_.udp.reg_recv_callback<lowpanApp,&lowpanApp::receive_radio_message>( this );
		
		/*
			Test destination
		*/
		uint8_t my_prefix[8];
		my_prefix[0] = 0x20;
		my_prefix[1] = 0x01;
		my_prefix[2] = 0x63;
		my_prefix[3] = 0x80;
		my_prefix[4] = 0x70;
		my_prefix[5] = 0xA0;
		my_prefix[6] = 0xB0;
		my_prefix[7] = 0x69;

		IPv6Address_t destinationaddr;

		//destinationaddr.make_it_link_local();
		destinationaddr.set_prefix( my_prefix, 64 );

		node_id_t ll_id = 0x2140;
		destinationaddr.set_long_iid(&ll_id, true);
		
		/*
			UDP test
		*/
		/* if( radio_->id() == 0x2140 )
		{
			int my_number = ipv6_stack_.udp.add_socket( 10, 10, destinationaddr, callback_id );
			//ipv6_stack_.udp.print_sockets();
			uint8_t mypayload[] = "hello :) This is a test message."
			ipv6_stack_.udp.send(my_number,sizeof(mypayload),mypayload);
		}
		if( radio_->id() == 0x2144 )
		{
			node_id_t ll_id = 1;
			destinationaddr.set_long_iid(&ll_id, false);
			ipv6_stack_.udp.add_socket( 10, 10, destinationaddr, callback_id );
		}*/
		
		/*
			ICMPv6 Ping test
		*/
		/*
		if( radio_->id() == 0x2144 )
		{
			debug_->debug("Application layer: sending echo request");
			ipv6_stack_.icmpv6.ping(destinationaddr);
			//ipv6_stack_.icmpv6.ping(ipv6_stack_.ipv6.BROADCAST_ADDRESS);
		}
		*/
	}

	// --------------------------------------------------------------------
	void receive_radio_message( IPv6Address_t from, uint16_t len, Os::Radio::block_data_t *buf )
	{
		char str[43];
		debug_->debug( "Application layer received msg at %x from ", radio_->id(), from.get_address(str) );
		debug_->debug( " Size: %i Content: %s ", len, buf);
	}

	// --------------------------------------------------------------------
	void receive_echo_reply( IPv6Address_t from, size_t len, Os::Radio::block_data_t *buf )
	{
		if( len == 1 && buf[0] == ipv6_stack_.icmpv6.ECHO_REPLY )
		{
			char str[43];
			debug_->debug( "Application layer received echo reply at %x from ", radio_->id(), from.get_address(str) );
		}
	}

private:
	int callback_id;

	IPv6_stack_t ipv6_stack_;
	Radio::self_pointer_t radio_;
	Os::Timer::self_pointer_t timer_;
	Os::Debug::self_pointer_t debug_;
	Uart::self_pointer_t uart_;
};
// --------------------------------------------------------------------------
wiselib::WiselibApplication<Os, lowpanApp> example_app;
// --------------------------------------------------------------------------
void application_main( Os::AppMainParameter& value )
{
	example_app.init( value );
}
