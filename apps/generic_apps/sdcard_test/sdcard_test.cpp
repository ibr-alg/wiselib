#include <external_interface/external_interface.h>
#include <external_interface/arduino/arduino_sdcard.h>
#include <external_interface/arduino/arduino_debug.h>

typedef wiselib::OSMODEL Os;

class App {
	public:
		
		// NOTE: this uses up a *lot* of RAM, way too much for uno!
		enum { TEST_BUFFER_SIZE = 512 * 3 };
		
		void init(Os::AppMainParameter& value) {
			debug_ = &wiselib::FacetProvider<Os, Os::Debug>::get_facet( value );
			
			debug_->debug( "SD Card test application running" );
			test_sd();
			
		}
		
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
		
	private:
		//static Os::Debug dbg;
		Os::Debug::self_pointer_t debug_;
		
		Os::block_data_t test_buffer_[TEST_BUFFER_SIZE];
		Os::block_data_t verify_buffer_[TEST_BUFFER_SIZE];
		
		wiselib::ArduinoSdCard<Os> sd_;
};

//Os::Debug App::dbg = Os::Debug();

wiselib::WiselibApplication<Os, App> app;
void application_main( Os::AppMainParameter& value ) {
	app.initx(value);
}
