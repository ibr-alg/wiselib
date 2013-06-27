
#if CODESIZE
	// running code size tests, leave out debug messages!
	#define CHECK_INVARIANTS 0
	#define WISELIB_DISABLE_DEBUG_MESSAGES 1

	#if CODESIZE_INQP
		#define USE_INQP 1
	#endif

	#if CODESIZE_PRESCILLA
		#define USE_PRESCILLA 1
	#else
		#define USE_TREEDICT 1
	#endif

#else
	#define USE_PRESCILLA 1
	//#define USE_TREEDICT 1

#endif

#define USE_DICTIONARY (USE_PRESCILLA || USE_TREEDICT)

#define NEED_ALLOCATOR (defined(TINYOS) || defined(CONTIKI) || defined(CONTIKI_TARGET_MICAZ))


#include <external_interface/external_interface.h>
#include <external_interface/external_interface_testing.h>
typedef wiselib::OSMODEL Os;

#if NEED_ALLOCATOR
	#include <util/allocators/bitmap_allocator.h>
	typedef wiselib::BitmapAllocator<Os, 1024> Allocator;
	Allocator allocator_;
	Allocator& get_allocator() { return allocator_; }
#else
	#include <util/allocators/malloc_free_allocator.h>
	typedef wiselib::MallocFreeAllocator<Os> Allocator;
	Allocator allocator_;
	Allocator& get_allocator() { return allocator_; }
#endif
	
#ifdef TINYOS
	int strcmp(char* a, char* b) {
		for( ; a && b; a++, b++) {
			if(a != b) { return b - a; }
		}
		return b - a;
	}
#endif

#ifdef ISENSE
	void* malloc(size_t n) { return isense::malloc(n); }
	void free(void* p) { isense::free(p); }
#endif
	
#if defined(ISENSE) || defined(TINYOS) || defined(CONTIKI) || defined(CONTIKI_TARGET_MICAZ)
	#warning "assertions messages not implemented for this platform, disabling"
	extern "C" void assert(int) { }
#endif
	
#ifndef DBG
	#warning "debug messages not implemented for this platform, disabling"
	#define DBG(...)
#endif

