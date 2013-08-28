
#define USE_FILE_BLOCK_MEMORY 1
/*
#include <external_interface/external_interface.h>
#include <external_interface/external_interface_testing.h>
using namespace wiselib;

typedef OSMODEL Os;
#include <util/allocators/malloc_free_allocator.h>
typedef MallocFreeAllocator<Os> Allocator;
Allocator allocator_;
Allocator& get_allocator() { return allocator_; }

#include <util/meta.h>
#include <util/debugging.h>
*/
#include "platform.h"
using namespace wiselib;
#include <algorithms/block_memory/bitmap_chunk_allocator.h>
#include <algorithms/block_memory/cached_block_memory.h>
typedef CachedBlockMemory<Os, Os::BlockMemory, 100, 20, true> BlockMemory;
typedef BitmapChunkAllocator<Os, BlockMemory, BLOCK_CHUNK_SIZE, BLOCK_CHUNK_ADDRESS_TYPE> BlockAllocator;

class App {
	public:
		void init(Os::AppMainParameter& value) {
			debug_ = &wiselib::FacetProvider<Os, Os::Debug>::get_facet( value );
			
			//Os::BlockMemory mem;
			BlockMemory cache;
			BlockAllocator alloc;
			
			assert(value.argc == 2);
			
			//mem.init(value.argv[1]);
			cache.physical().init(value.argv[1]);
			cache.init();
			alloc.init(&cache, debug_);
			
			alloc.format();
			debug_->debug("done.");
		}
		
		Os::Debug *debug_;
};

wiselib::WiselibApplication<Os, App> example_app;
void application_main( Os::AppMainParameter& value )
{
	example_app.init( value );
}

