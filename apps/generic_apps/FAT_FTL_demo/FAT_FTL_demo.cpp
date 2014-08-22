//#define USE_FILE_BLOCK_MEMORY 1
#define USE_RAM_BLOCK_MEMORY 1
#include <external_interface/external_interface.h>
typedef wiselib::OSMODEL Os;

#include <algorithms/block_memory/ram_flash_memory.h>					/* For Flash Simulation */
typedef wiselib::RAMFlashMemory<Os,4,256,512> RAMFlashMemory_;

#include <algorithms/ftl/flash_interface.h>		/* Interface between FLash and Block Memory */
typedef wiselib::FlashInterface<Os,RAMFlashMemory_> FlashInterface_;

#include <algorithms/block_memory/cached_block_memory.h>				/* For Caching */
typedef wiselib::CachedBlockMemory<Os,FlashInterface_,16,16> CachedBlockMemory_;

#include <algorithms/ftl/ftl.h>								/* For flash translation layer */
typedef wiselib::Flash<Os,CachedBlockMemory_> Flash;

#include <util/filesystems/fat.h>
typedef wiselib::Fat<Os, Flash> Fat;

class App {
	public:

		void init(Os::AppMainParameter& value) {
			debug_ = &wiselib::FacetProvider<Os, Os::Debug>::get_facet( value );
			int res;

//			res = sd_.init("myfile.img");
			flash_.init(Flash::NEWFLASH);
			debug_->debug("size %d",flash_.size());

			if(res == Os::SUCCESS)
                debug_->debug( "SD Card test application running, Block memory initialized " );
            else
                debug_->debug( "Block memory not initialized " );

//            res=fs.init(*debug_, sd_);
            res=fs.init(*debug_, flash_);
            if(res==0)
                debug_->debug( "File System init successful" );
            else
                debug_->debug( "Could not init file System, error %d", res );

            res=fs.mkfs(1);
            if(res==0)
                debug_->debug( "File System Format successful" );
            else
                debug_->debug( "Could not format file System, error %d", res );

//            res = flash_.init(Flash::NEWFLASH);
//            if(res == Os::SUCCESS)
//                debug_->debug( "SD Card test application running, Flash memory initialized " );
//            else
//                debug_->debug( "Block memory not initialized " );

//			test_fs();
		}

		Os::BlockMemory::address_t address_;
		void test_fs() {
            int res;
            ::uint32_t offset;
            ::uint16_t bytes_to_read = 10, bytes_read = 0;
            ::uint16_t bytes_to_write = 10, bytes_written = 0;
            const char *file_name = "MYFILE";
            Os::block_data_t read_buf[bytes_to_read];
            Os::block_data_t write_buf[10] = {'a','b','c','d','e','f','g','h','i'};
            Os::block_data_t write_buf2[10] = {'1','2','3','4','5','6','7','8','9'};
            write_buf[9] = 0x0A;
            write_buf2[9] = 0x0A;
            int count = 110;

            /**
             *  mount() is used to mount a FAT file system existing on the initialized memory
             *  if FAT file system is not found, mount() will fail and return error
             */
            if(!fs.mount())
                debug_->debug( "File System mount successful" );
            else
                debug_->debug( "Could not mount file System" );

            /**
             *  open(const char *path) is used to open or create a file.
             *  path is the absolute path of file to be opened.
             *  There is no need to close a file explicitly, open other file and
             *  previous file is closed, multiple files cannot be opened for same FileSystem object.
             */
            res = fs.open(file_name);

            if(res == 0)
                debug_->debug("File %s opened successfully", file_name);
            else
                debug_->debug("Could not open %s error %d", file_name, res);

            /**
             *  lseek(::uint32_t byte_offset) moves the file read/write pointer of an open file.
             *  If the offset is larger than file size, it is automatically moved to end of file.
             *  If data exists on the file to the location pointed by R/W pointer,
             *  it is over-written without any warning!
             */
            offset = 0;
            res = fs.lseek(offset);
            if(res == 0)
                debug_->debug("Offset to %d Bytes successful", offset);
            else
                debug_->debug("Could not set offset to file %s", file_name);

            /**
             *  read(block_data_t *buffer, ::uint32_t bytes_to_read, ::uint32_t *bytes_read)
             *  is used to read data from open file into the buffer.
             */
            res = fs.read(read_buf, bytes_to_read, &bytes_read);

            if(res == 0)
                debug_->debug("Read %d Bytes successfully", bytes_read);
            else
                debug_->debug("Could not read file %s, error %d", file_name, res);

            /**
             *  write(block_data_t *buffer, ::uint32_t bytes_to_write, ::uint32_t *bytes_written)
             *  is used to write data from the buffer into the open file.
             */
            res = fs.write(write_buf, bytes_to_write, &bytes_written);

            if(res == 0)
                debug_->debug("Wrote %d Bytes successfully", bytes_written);
            else
                debug_->debug("Could not write to file %s, error %d", file_name, res);

            /**
             *  erase(const char *path) erases the file or directory defined by the absolute path.
             *  Even if the file is open, erase deletes the file anyway.
             */
////            file_name = "MYDIR";
//            res = fs.erase(file_name);
//            if(res == 0)
//                debug_->debug("File %s deletion successful", file_name);
//            else
//                debug_->debug("Could not delete file %s, error %d", file_name, res);

            /**
             *  mkdir(const char* path) creates a directory in path specified as the parameter.
             *  Name of directory must be different from any filename in same directory.
             */
            file_name = "MYDIR";
            res = fs.mkdir(file_name);
            if(res == 0)
                debug_->debug("Directory %s created successfully", file_name);
            else
                debug_->debug("Could not create directory %s", file_name);

//            while(t--) {
//                a = fs.write(buf2, bytes_to_read, &bw);
//                debug_->debug(" Writing file %d \n\nWROTE - %d, t = %d", a, bw, t);
//            }
		}

	private:
		Os::Debug::self_pointer_t debug_;
		Os::BlockMemory sd_;
		Flash flash_;

		Fat fs;
};

wiselib::WiselibApplication<Os, App> app;
void application_main( Os::AppMainParameter& value ) {
	app.init(value);
}
