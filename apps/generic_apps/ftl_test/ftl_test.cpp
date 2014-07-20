/*

Flash translation Layer Testing

The output on running with Shawn or iSense would be

start of operation 

Write at location 0x20000 successfully

Write at location 0x21000 successfully

Write at location 0x20000 successfully

Unable to Write at location 0x20001 

Data at location 0x20000 : 5 

Data at location 0x20001 : 2 

Data at location 0x20002 : 4 

Data at location 0x20003 : 2 

Data at location 0x20004 : 1 

Data at location 0x20005 : 1 

Unable to read at location 0x30000 

end of operation 

Due to FTL, we can write data to a particular sector any number of times and the way data is stored is internally managed by FTL.

*/

#include <external_interface/external_interface.h>

typedef wiselib::OSMODEL Os;

#include <algorithms/block_memory/ram_flash_memory.h>
typedef wiselib::RAMFlashMemory<Os,4,256,512> RAMFlashMemory_;

#include <algorithms/ftl/ftl.h>
typedef wiselib::Flash<Os,RAMFlashMemory_> Flash;


typedef Os::block_data_t block_data_t;

enum {
	SUCCESS = Os::SUCCESS, 
	ERR_UNSPEC = Os::ERR_UNSPEC,
};


class App {
	public:
		App(){}


		void init(Os::AppMainParameter& value) {
			debug_ = &wiselib::FacetProvider<Os, Os::Debug>::get_facet( value );
			//clock_ = &wiselib::FacetProvider<Os, Os::Clock>::get_facet(value);
			//timer_ = &wiselib::FacetProvider<Os, Os::Timer>::get_facet(value);

			flash_.init();

			debug_->debug("start of operation \n");
			test_ftl();
			debug_->debug("end of operation \n");
			
			flash_.uninit();
		}

		void test_ftl() {

			block_data_t buf1[512]={1,2,3,4,5,6};
			block_data_t buf2[512]={5,2,4,2,1,1};
			int i;
			buf2[259]=129;
			int r = flash_.write(buf1,0x20000,1);
			if(r!=SUCCESS)
				debug_->debug("Unable to Write at location 0x20000 \n");
			else
				debug_->debug("Write at location 0x20000 successfully\n");
	
			r = flash_.write(buf1,0x21000,1);
			if(r!=SUCCESS)
				debug_->debug("Unable to Write at location 0x21000 \n");
			else
				debug_->debug("Write at location 0x21000 successfully\n");

			r = flash_.write(buf2,0x20000,1);
			if(r!=SUCCESS)
				debug_->debug("Unable to Write at location 0x20000 \n");
			else
				debug_->debug("Write at location 0x20000 successfully\n");

			r = flash_.write(buf1,0x20001,1);
			if(r!=SUCCESS)
				debug_->debug("Unable to Write at location 0x20001 \n");
			else
				debug_->debug("Write at location 0x20001 successfully\n");

			r = flash_.read(buf1,0x20000,1);
			if(r!=SUCCESS)
				debug_->debug("Unable to read at location 0x20001 \n");
			else
				for(i=0;i<6;i++)
					debug_->debug("Data at location 0x2000%d : %d \n",i,buf1[i]);
				
			r = flash_.read(buf1,0x30000,1);
			if(r!=SUCCESS)
				debug_->debug("Unable to read at location 0x30000 \n");
			else
				for(i=0;i<6;i++)
					debug_->debug("Data at location 0x3000%d : %d \n",i,buf1[i]);
				
			
		/*	//is_.erase((size_t)3);
			uint8 data[2]={3,4};
			uint32 address =0x70000;
			uint16 size = 2;
			//uint8 sector=0x7;
			//os_->flash().erase(sector);
			//os_->flash().write(0x20000,256,data);
			//data[0]=0;
			//data[1]=0;
			os_->flash().write(address,size,data);
			//debug_->debug("Data %d %d \n",data[0],data[1]);			*/
		}


	private:
		//static Os::Debug dbg;
		Os::Debug::self_pointer_t debug_;
		//wiselib::iSenseInternalFlash<Os> is_;		
		//Os::Clock::self_pointer_t clock_;
		//Os::Timer::self_pointer_t timer_;
		//Os::BlockMemory sd_;
		Flash flash_;
		//Os::BlockMemory::address_t address_;
		//isense::Os *os_;
};

wiselib::WiselibApplication<Os, App> app;

void application_main( Os::AppMainParameter& value ) {
	app.init(value);
}
