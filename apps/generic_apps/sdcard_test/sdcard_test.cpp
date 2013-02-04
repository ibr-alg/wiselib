#include <external_interface/external_interface.h>

typedef wiselib::OSMODEL Os;

class App {
	public:
		
		// NOTE: this uses up a *lot* of RAM, way too much for uno!
		enum { TEST_BUFFER_SIZE = 512 * 4 };
		
		void init(Os::AppMainParameter& value) {
			debug_ = &wiselib::FacetProvider<Os, Os::Debug>::get_facet( value );
			clock_ = &wiselib::FacetProvider<Os, Os::Clock>::get_facet(value);
			timer_ = &wiselib::FacetProvider<Os, Os::Timer>::get_facet(value);
			
			sd_.init();
			
			debug_->debug( "SD Card test application running" );
			//test_sd();
			
			state_ = 0;
			
			on_time(0);
		}
		
		/*
		void test_sd() {
			// initialize test buffer
			
			for(Os::size_t i=0; i<TEST_BUFFER_SIZE; i++) {
				test_buffer_[i] = (i & 0xff) ^ (i >> 8);
				verify_buffer_[i] = 0;
			}
			
			int r = Os::SUCCESS;
			
			sd_.init();
			
			debug_->debug("writing...");
			
			r = sd_.write(test_buffer_, 0, TEST_BUFFER_SIZE / 512);
			if(r != Os::SUCCESS) { debug_->debug("Error %d", r); }
			
			debug_->debug("reading...");
			
			r = sd_.read(verify_buffer_, 0, TEST_BUFFER_SIZE / 512);
			if(r != Os::SUCCESS) { debug_->debug("Error %d", r); }
			
			debug_->debug("verifying...");
			
			Os::size_t i;
			for(i=0; i<TEST_BUFFER_SIZE; i++) {
				if(verify_buffer_[i] != test_buffer_[i]) {
					debug_->debug("buffers differ at position %d (orig=%d after transfer=%d).", i, test_buffer_[i], verify_buffer_[i]);
					break;
				}
			}
			debug_->debug("%d elements verified.", i);
		}
		*/
		
		
		int state_;
		void on_time(void*) {
			enum { STATES = 2 };
			
			Os::BlockMemory::address_t max_addr = 128;
			
			switch(state_) {
				case 0: {
					// write a few (identical) blocks
					
					for(Os::size_t i=0; i<TEST_BUFFER_SIZE; i++) { test_buffer_[i] = (i & 0xff) ^ (i >> 8); }
					int r = Os::SUCCESS;
					
					for(Os::BlockMemory::address_t addr = 0; addr < max_addr; addr += TEST_BUFFER_SIZE / 512) {
						r = sd_.write(test_buffer_, addr, TEST_BUFFER_SIZE / 512);
						if(r != Os::SUCCESS) { debug_->debug("Error %d", r); }
					}
					break;
				}
				case 1: {
					// read dem blocks
					
					for(Os::BlockMemory::address_t addr = 0; addr < max_addr; addr += TEST_BUFFER_SIZE / 512) {
						r = sd_.read(test_buffer_, 0, TEST_BUFFER_SIZE / 512);
						if(r != Os::SUCCESS) { debug_->debug("Error %d", r); }
					}
					break;
				}
			} // switch
			
			unsigned long delay = 1000;
			state_++;
			if(state_ == STATES) {
				state_ = 0;
				delay = 3000;
			}
			
			timer_->set_timer<App, &App::on_time>(delay, this, 0);
		}
		
	private:
		//static Os::Debug dbg;
		Os::Debug::self_pointer_t debug_;
		Os::Clock::self_pointer_t clock_;
		
		Os::block_data_t test_buffer_[TEST_BUFFER_SIZE];
		//Os::block_data_t verify_buffer_[TEST_BUFFER_SIZE];
		
		//wiselib::ArduinoSdCard<Os> sd_;
		Os::BlockMemory sd_;
};

//Os::Debug App::dbg = Os::Debug();

wiselib::WiselibApplication<Os, App> app;
void application_main( Os::AppMainParameter& value ) {
	app.init(value);
}
