/***************************************************************************
 ** This file is part of the generic algorithm library Wiselib.           **
 ** Copyright (C) 2008,2009 by the Wisebed (www.wisebed.eu) project.      **
 **                                                                       **
 ** The Wiselib is free software: you can redistribute it and/or modify   **
 ** it under the terms of the GNU Lesser General Public License as        **
 ** published by the Free Software Foundation, either version 3 of the    **
 ** License, or (at your option) any later version.                       **
 **                                                                       **
 ** The Wiselib is distributed in the hope that it will be useful,        **
 ** but WITHOUT ANY WARRANTY; without even the implied warranty of        **
 ** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         **
 ** GNU Lesser General Public License for more details.                   **
 **                                                                       **
 ** You should have received a copy of the GNU Lesser General Public      **
 ** License along with the Wiselib.                                       **
 ** If not, see <http://www.gnu.org/licenses/>.                           **
 ***************************************************************************/


/*

Flash translation Layer Testing

The output on running with Shawn or iSense would be


start of operation 

Write at block 2 successfully

Write at block 4 successfully

Write at block 3 successfully

Write at block 2 successfully

Write at block 2 successfully

Write at block 2 successfully

Data at block 3 offset 0 : 5 

Data at block 3 offset 1 : 2 

Data at block 3 offset 2 : 4 

Data at block 3 offset 3 : 2 

Data at block 3 offset 4 : 1 

Data at block 3 offset 5 : 1 

Data at block 2 offset 0 : 1 

Data at block 2 offset 1 : 2 

Data at block 2 offset 2 : 3 

Data at block 2 offset 3 : 4 

Data at block 2 offset 4 : 5 

Data at block 2 offset 5 : 6 

end of operation 

.*
.*
.*
.*
.*
.*
.*
.*
.*
.*
.*


Due to FTL, we can write data to a particular block any number of times and the way data is stored is internally managed by FTL.

*/

#include <external_interface/external_interface.h>

typedef wiselib::OSMODEL Os;

#include <algorithms/block_memory/ram_flash_memory.h>				// For RAM Flash Memory Simulator
typedef wiselib::RAMFlashMemory<Os,64,512,4> RAMFlashMemory_;

#include <algorithms/block_memory/cached_block_memory.h>			// For Caching
typedef wiselib::CachedBlockMemory<Os,RAMFlashMemory_,4,0> CachedBlockMemory_;

#include <algorithms/ftl/ftl.h>							// For Flash translation Layer
typedef wiselib::Flash<Os,CachedBlockMemory_> Flash;


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
			
			flash_.init(Flash::NEWFLASH);

			debug_->debug("start of operation \n");
			test_ftl();
			debug_->debug("end of operation \n");
			
			flash_.uninit();
		}

		void test_ftl() {

			block_data_t buf1[512]={1,2,3,4,5,6};
			block_data_t buf2[512]={5,2,4,2,1,1};
			int i;
			int r = flash_.write(buf1,2,1);
			if(r!=SUCCESS)
				debug_->debug("Unable to Write at block 2 \n");
			else
				debug_->debug("Write at block 2 successfully\n");
	
			r = flash_.write(buf1,4,1);
			if(r!=SUCCESS)
				debug_->debug("Unable to Write at block 4 \n");
			else
				debug_->debug("Write at block 4 successfully\n");

			r = flash_.write(buf2,3,1);
			if(r!=SUCCESS)
				debug_->debug("Unable to Write at block 3 \n");
			else
				debug_->debug("Write at block 3 successfully\n");

			r = flash_.write(buf1,2,1);
			if(r!=SUCCESS)
				debug_->debug("Unable to Write at block 2 \n");
			else
				debug_->debug("Write at block 2 successfully\n");

			r = flash_.write(buf1,2,1);
			if(r!=SUCCESS)
				debug_->debug("Unable to Write at block 2 \n");
			else
				debug_->debug("Write at block 2 successfully\n");
			r = flash_.write(buf1,2,1);			
			if(r!=SUCCESS)
				debug_->debug("Unable to Write at block 2 \n");
			else
				debug_->debug("Write at block 2 successfully\n");
			r = flash_.read(buf1,3,1);
			if(r!=SUCCESS)
				debug_->debug("Unable to read at block 3 \n");
			else
				for(i=0;i<6;i++)
					debug_->debug("Data at block 3 offset %d : %d \n",i,buf1[i]);
				
			r = flash_.read(buf1,2,1);
			if(r!=SUCCESS)
				debug_->debug("Unable to read at block 2 \n");
			else
				for(i=0;i<6;i++)
					debug_->debug("Data at block 2 offset %d : %d \n",i,buf1[i]);
				
		}


	private:
		Os::Debug::self_pointer_t debug_;
		Flash flash_;
	
};

wiselib::WiselibApplication<Os, App> app;

void application_main( Os::AppMainParameter& value ) {
	app.init(value);
}
