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

The project uses t files
1. ftl_test.cpp : the implementation of the test application for FTL
2. ftl.h : the implementation of flash translation layer for Flash memory
3. cached_block_memory.h : the implementation of caching
4. flash_interface.h : acts as an interface between block memory interface and ISense/Shawn flash interface

any one of the below
5. ram_flash_memory.h : implementation of simulator for Flash memory (Used for Shawn)
6. isense_flash.h : used for calling methods for using ISense flash (Used for ISense)


*/

#include <external_interface/external_interface.h>

typedef wiselib::OSMODEL Os;


/* Modelling a Flash Device with 4 sectors and 512 pages each
	The memory is organized as: 
	524,288 bytes (8 bits each)
	4 sectors (131,072 bytes each),
	Sector 0:  00000h 1FFFFh,
	Sector 1:  20000h 3FFFFh,
	Sector 2:  40000h 5FFFFh,
	Sector 3:  60000h 7FFFFh 
	512 pages (256 bytes each)
*/
	
#include <algorithms/block_memory/ram_flash_memory.h>					// For Flash Simulation
typedef wiselib::RAMFlashMemory<Os,4,256,512> RAMFlashMemory_;

#include <algorithms/ftl/flash_interface.h>						// Interface between FLash and Block Memory
typedef wiselib::FlashInterface<Os,RAMFlashMemory_> FlashInterface_;			

#include <algorithms/block_memory/cached_block_memory.h>				// For Caching
typedef wiselib::CachedBlockMemory<Os,FlashInterface_,16,16> CachedBlockMemory_;	

#include <algorithms/ftl/ftl.h>								// For flash translation layer
typedef wiselib::FTL<Os,CachedBlockMemory_> Flash;		


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
