#define USE_FILE_BLOCK_MEMORY 1
//#define USE_RAM_BLOCK_MEMORY 1
#include <external_interface/external_interface.h>

typedef wiselib::OSMODEL Os;

#include <algorithms/rand/kiss.h>
typedef wiselib::Kiss<Os> Rand;

#include <util/filesystems/fat.h>
typedef wiselib::Fat<Os> Fat;

class App {
	public:

		// NOTE: this uses up a *lot* of RAM, way too much for uno!

		#ifdef PC
		enum { TEST_BUFFER_SIZE = 4 };
		#else
		enum { TEST_BUFFER_SIZE = 1 };
		#endif

		void init(Os::AppMainParameter& value) {
			debug_ = &wiselib::FacetProvider<Os, Os::Debug>::get_facet( value );
			//clock_ = &wiselib::FacetProvider<Os, Os::Clock>::get_facet(value);
			//timer_ = &wiselib::FacetProvider<Os, Os::Timer>::get_facet(value);
			rand_ = &wiselib::FacetProvider<Os, Rand>::get_facet(value);

			int a = sd_.init("myfile.img");
//            int a = sd_.init();

			debug_->debug( "SD Card test application running, fs = %d", a );

			address_ = 0;
			//progress(0);

			if(!fs.init(*debug_, sd_))
                debug_->debug( "File System Mount successful" );
            else
                debug_->debug( "Could not mount file System" );

//            a = fs.open("MYFILE");
//            a = fs.open("FOOBAR05.TXT");
//            debug_->debug(" Opening file %d", a);

            ::uint16_t btr = 10;
            Os::block_data_t buf[btr];
//            block_data_t buf2[10] = {'a','b','c','d','e','f','g','h','i'};
            Os::block_data_t buf2[10] = {'1','2','3','4','5','6','7','8','9'};
            buf2[9] = 0x0A;
//
            ::uint16_t br = 0;
            ::uint16_t bw = 0;
            ::uint16_t t = 110;
//            a = fs.read(buf, btr, &br);
//            debug_->debug(" Reading file %c \n\nREAD - %d", buf[btr-1], br);

//            a = fs.open("minefile.TXT");
//            debug_->debug(" Opening file %d", a);
//
//            a = fs.lseek(0);
//            debug_->debug(" Seeking file %d", a);
//
//            while(t--) {
//                a = fs.write(buf2, btr, &bw);
//                debug_->debug(" Writing file %d \n\nWROTE - %d, t = %d", a, bw, t);
//            }
            debug_->debug("t = %d", t);
//
//
//            a = fs.lseek(0);
//            a = fs.read(buf, btr, &br);
//            debug_->debug(" Reading file %s \n\nREAD - %d", buf, br);
            a = fs.erase_obj("DIR01");
            debug_->debug(" Deleted file %d", a);
//			test_sd();
		}

		Os::BlockMemory::address_t address_;
		void test_sd() {
			debug_->debug("SD read/write test, %d block(s) at a time.", TEST_BUFFER_SIZE);

			Os::block_data_t buf1[TEST_BUFFER_SIZE * Os::BlockMemory::BLOCK_SIZE];
			Os::block_data_t buf2[TEST_BUFFER_SIZE * Os::BlockMemory::BLOCK_SIZE];

			for(address_ = 0; address_ < sd_.size(); address_++) {

				if(address_ % 1000 == 0) {
					debug_->debug("tested %7lu / %7lu blocks.", (unsigned long)address_, (unsigned long)sd_.size());
				}

				// fill buf1 with random numbers
				Rand::value_t *v1 = reinterpret_cast<Rand::value_t*>(buf1);
				Rand::value_t *v1_end = reinterpret_cast<Rand::value_t*>(buf1 + sizeof(buf1));
				for( ; v1 < v1_end; v1++) { *v1 = rand_->operator()(); }

				// int r = sd_.write(buf1, address_, TEST_BUFFER_SIZE);
				int r = sd_.write(buf1, address_);
				if(r != Os::SUCCESS) {
					debug_->debug("error writing %d blocks at %d code %d", TEST_BUFFER_SIZE, address_, r);
				}

				// r = sd_.read(buf2, address_, TEST_BUFFER_SIZE);
				r = sd_.read(buf2, address_);
				if(r != Os::SUCCESS) {
					debug_->debug("error reading %d blocks at %d code %d", TEST_BUFFER_SIZE, address_, r);
				}

				r = memcmp(buf1, buf2, TEST_BUFFER_SIZE * Os::BlockMemory::BLOCK_SIZE);
				if(r != 0) {
					debug_->debug("read blocks differ from written blocks at %d memcmp result %d", address_, r);
				}
			}
		}

		/*
		void progress(void*) {
			debug_->debug("tested %6d / %6d blocks.");
			timer_->set_timer<App, &App::progress>(10000, this, 0);
		}
		*/

	private:
		//static Os::Debug dbg;
		Os::Debug::self_pointer_t debug_;
		//Os::Clock::self_pointer_t clock_;
		Rand::self_pointer_t rand_;
		//Os::Timer::self_pointer_t timer_;
		Os::BlockMemory sd_;

		Fat fs;
};

//Os::Debug App::dbg = Os::Debug();

wiselib::WiselibApplication<Os, App> app;
void application_main( Os::AppMainParameter& value ) {
	app.init(value);
}
