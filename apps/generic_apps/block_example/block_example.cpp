

// On PC Os this will control whether Os::BlockMemory will be a RAM-based
// implementation or a file-based one. In the latter case we can provide
// the file name in init(), defaulting to block_memory.img
#define USE_RAM_BLOCK_MEMORY 1
#define USE_FILE_BLOCK_MEMORY (!(USE_RAM_BLOCK_MEMORY))

#include <external_interface/external_interface.h>
#include <util/debugging.h>
typedef wiselib::OSMODEL Os;

typedef Os::BlockMemory BlockMemory;

class App {
	public:
		void init(Os::AppMainParameter& amp) {
			debug_ = &wiselib::FacetProvider<Os, Os::Debug>::get_facet(amp);

			block_memory_.init();
			test_block_memory();
		}

		void test_block_memory() {
			debug_->debug("block memory has size %lu", (unsigned long)block_memory_.size());

			// read and print first block (index 0)
			block_memory_.read(block_, 0);
			debug_->debug("block 0:");
			wiselib::debug_buffer<Os, 16>(debug_, block_, BlockMemory::BLOCK_SIZE);

			// Now overwrite a part of the block with this string
			strcpy((char*)block_ + 10, "Hello, World!");
			// And write the block back to storage
			block_memory_.write(block_, 0);

			// clear our RAM-block to prove we actually wrote to storage
			memset(block_, 0, BlockMemory::BUFFER_SIZE);

			// read block back in from storage
			block_memory_.read(block_, 0);
			debug_->debug("block 0 now:");
			wiselib::debug_buffer<Os, 16>(debug_, block_, BlockMemory::BLOCK_SIZE);
		}

	private:
		Os::Debug::self_pointer_t debug_;
		BlockMemory block_memory_;

		Os::block_data_t block_[BlockMemory::BUFFER_SIZE];

};

wiselib::WiselibApplication<Os, App> app;
void application_main(Os::AppMainParameter& amp) { app.init(amp); }

