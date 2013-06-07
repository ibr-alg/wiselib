/*
* File: DPS_test.cpp
* Author: Daniel Gehberger - GSoC 2013 - DPS project
*/

//Needed for iSense in order to avoid linker error with the MapStaticVector class
extern "C" void assert(int) { }

#include "external_interface/external_interface.h"
#include "radio/DPS/DPS_radio.h"

typedef wiselib::OSMODEL Os;
typedef Os::Radio Radio;
typedef Os::Radio::node_id_t node_id_t;

typedef wiselib::DPS_Radio<Os, Radio, Os::Debug, Os::Timer, Os::Rand> DPS_Radio_t;

class DPSApp
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
		rand_ = &wiselib::FacetProvider<Os, Os::Rand>::get_facet( value );
	
		debug_->debug( "Booting with ID: %llx\n", (long long unsigned)(radio_->id()));
		
		DPS_Radio_.init(*radio_, *debug_, *timer_, *rand_);
		DPS_Radio_.enable_radio();
		
		//Register
		
		if( radio_->id() == 0x0 || radio_->id() == 0x2 )
			DPS_Radio_.reg_recv_callback<DPSApp,&DPSApp::RPC_handler>( this, DPS_Radio_t::TEST_PID, true );
		else if( radio_->id() == 0x1 )
			DPS_Radio_.reg_recv_callback<DPSApp,&DPSApp::RPC_handler>( this, DPS_Radio_t::TEST_PID, false );

		//--------------------------------------------------------------------------------------
		//					Testing part
		//--------------------------------------------------------------------------------------
		
		
	}
	
	int RPC_handler( node_id_t source, uint8_t Fild, uint16_t length, block_data_t* buffer )
	{
		debug_->debug( "RPC_handler is called at: %llx\n", (long long unsigned)(radio_->id()));
		return 1;
	}
	
	// --------------------------------------------------------------------
// 	void receive_radio_message( UDPSocket_t socket, uint16_t len, Os::Radio::block_data_t *buf )
// 	{
// 		//Drop the messages from this node --> only the IP addresses are compared
// 		if( socket != ipv6_stack_.udp.id() )
// 		{
// 			char str[43];
// 			debug_->debug( "Application layer received msg at %llx from %s", (long long unsigned)(radio_->id()), socket.remote_host.get_address(str) );
// 			debug_->debug( "    Size: %i Content: %s ", len, buf);
// 		}
// 	}

private:
	int callback_id;

	DPS_Radio_t DPS_Radio_;
	Radio::self_pointer_t radio_;
	Os::Timer::self_pointer_t timer_;
	Os::Debug::self_pointer_t debug_;
	Os::Rand::self_pointer_t rand_;
};
// --------------------------------------------------------------------------
wiselib::WiselibApplication<Os, DPSApp> example_app;
// --------------------------------------------------------------------------
void application_main( Os::AppMainParameter& value )
{
	example_app.init( value );
}
