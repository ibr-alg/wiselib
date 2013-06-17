/*
* File: DPS_test.cpp
* Author: Daniel Gehberger - GSoC 2013 - DPS project
*/

#ifdef ISENSE
//Needed for iSense in order to avoid linker error with the MapStaticVector class
extern "C" void assert(int) { }
#endif

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
	
	typedef DPS_Radio_t::node_id_t DPS_node_id_t;
	
	void init( Os::AppMainParameter& value )
	{
		radio_ = &wiselib::FacetProvider<Os, Os::Radio>::get_facet( value );
		timer_ = &wiselib::FacetProvider<Os, Os::Timer>::get_facet( value );
		debug_ = &wiselib::FacetProvider<Os, Os::Debug>::get_facet( value );
		rand_ = &wiselib::FacetProvider<Os, Os::Rand>::get_facet( value );
	
		debug_->debug( "Booting with ID: %llx %i\n", (long long unsigned)(radio_->id()), sizeof(node_id_t));
		
		DPS_Radio_.init(*radio_, *debug_, *timer_, *rand_);
		DPS_Radio_.enable_radio();
		
		//Register
		if( radio_->id() == 0x1 )
			DPS_Radio_.reg_recv_callback<DPSApp,&DPSApp::RPC_handler, &DPSApp::manage_buffer>( this, DPS_Radio_t::TEST_PID, true );
		else if( radio_->id() == 0x2 )
			DPS_Radio_.reg_recv_callback<DPSApp,&DPSApp::RPC_handler, &DPSApp::manage_buffer>( this, DPS_Radio_t::TEST_PID, false );
		
		if( radio_->id() == 0x2 )
		{
			timer_->set_timer<DPSApp, &DPSApp::send>( 2000, this, NULL );
			
		}
		
// 		if( radio_->id() == 0x203c )
// 			DPS_Radio_.reg_recv_callback<DPSApp,&DPSApp::RPC_handler, &DPSApp::manage_buffer>( this, DPS_Radio_t::TEST_PID, true );
// 		else if( radio_->id() == 0x203d )
// 			DPS_Radio_.reg_recv_callback<DPSApp,&DPSApp::RPC_handler, &DPSApp::manage_buffer>( this, DPS_Radio_t::TEST_PID, false );
// 		
// 		if( radio_->id() == 0x203d )
// 		{
// 			timer_->set_timer<DPSApp, &DPSApp::send>( 2000, this, NULL );
// 			
// 		}
		//--------------------------------------------------------------------------------------
		//					Testing part
		//--------------------------------------------------------------------------------------
		
		
	}
	
	int RPC_handler( DPS_node_id_t IDs, uint16_t length, block_data_t* buffer )
	{
		debug_->debug( "RPC_handler is called at: %llx (%i/%i)\n", (long long unsigned)(radio_->id()), IDs.Pid, IDs.Fid);
		
		//Free up the buffer, if there are many buffers, then the right one should be found
		test_buffer_inuse = false;
		
		//Call the function which is associated with the F_id
		if( IDs.Fid == 1 )
			add_numbers( buffer[0], buffer[1] );
		else if (IDs.Fid == 2 )
			print_text( buffer );
		
		return 1;
	}
	
	block_data_t* manage_buffer( block_data_t* buffer, uint16_t length, bool get_buffer )
	{
		if( get_buffer )
		{
			test_buffer_inuse = true;
			return test_buffer;
		}
		else
		{
			debug_->debug( "RPC buffer has been freed up" );
			test_buffer_inuse = false;
			return NULL;
		}
	}
	
	void add_numbers( uint8_t a, uint8_t b)
	{
		debug_->debug( "%i + %i = %i", a, b, a + b );
	}
	
	void print_text ( block_data_t* buffer )
	{
		debug_->debug( "PRINT: %s", buffer );
	}
	
	void send( void* )
	{
		DPS_node_id_t dest;
		dest.Pid = DPS_Radio_t::TEST_PID;
		dest.Fid = 2;
		dest.ack_required = 1;
		
// 		test_buffer[0] = 5;
// 		test_buffer[1] = 10;
		
		uint8_t mypayload[] = "This is a test message. This is a test message. This is a test message. This is a test message. This is a test message.  This is a test message.  This is a test message.  This is a test message.  This is a test message.  This is a test message.  This is a test message.  This is a test message.  This is a test message.  This is a test message.  This is a test message. ";
		memcpy( test_buffer, mypayload, sizeof(mypayload) ); 
		
		test_buffer_inuse = true;
		
		if( DPS_Radio_.send(dest, sizeof(mypayload), test_buffer ) == DPS_Radio_t::NO_CONNECTION )
		{
			test_buffer_inuse = false;
			debug_->debug( "No connection at %llx, call is postponed", (long long unsigned)(radio_->id()));
			timer_->set_timer<DPSApp, &DPSApp::send>( 2000, this, NULL );
		}
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
	
	block_data_t test_buffer[500];
	bool test_buffer_inuse;

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
