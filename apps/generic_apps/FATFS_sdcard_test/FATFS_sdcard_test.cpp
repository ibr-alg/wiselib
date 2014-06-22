#define USE_FILE_BLOCK_MEMORY 1
//#define USE_RAM_BLOCK_MEMORY 1
#include <external_interface/external_interface.h>

typedef wiselib::OSMODEL Os;

#include <algorithms/rand/kiss.h>
typedef wiselib::Kiss<Os> Rand;

#include <util/filesystems/fat32.h>
typedef wiselib::Fat32<Os> Fat32;

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

			debug_->debug( "SD Card test application running, fs = %d", a );

			address_ = 0;
			//progress(0);

			if(fs.init(*debug_, sd_)==FR_OK)
                debug_->debug( "File System Mount successful" );
            else
                debug_->debug( "Could not mount file System" );

//            FIL *fp;
//            a = fs.wf_open("MYDIR/MYFILE01.TXT");
            a = fs.wf_open("FOOBAR02.TXT");
//            a = fs.wf_open(fp,"FOOBAR01.TXT", 0);
            debug_->debug(" Opening file %d", a);

            WORD btr = 150;
            BYTE buf[btr];
//            BYTE buf2[10] = {'a','b','c','d','e','f','g','h','i'};
//            BYTE *buf2 = "abcdefghi";
//            BYTE buf2[10];
//            buf2 = "abcdefghi";
            WORD br = 0;
            WORD bw = 0;
            a = fs.wf_read(buf, btr, &br);
            debug_->debug(" Reading file %c \n\nREAD - %d", buf[1498], br);

            a = fs.wf_open("FOOBAR01.TXT");
            debug_->debug(" Opening file %d", a);

            a = fs.wf_lseek(2000);
            debug_->debug(" Seeking file %d", a);

            a = fs.wf_write(buf, btr, &bw);
            debug_->debug(" Writing file %d \n\nWROTE - %d", a, bw);
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

		Fat32 fs;
};

//Os::Debug App::dbg = Os::Debug();

wiselib::WiselibApplication<Os, App> app;
void application_main( Os::AppMainParameter& value ) {
	app.init(value);
}
