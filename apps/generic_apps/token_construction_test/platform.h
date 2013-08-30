
#if CODESIZE
	#warning "COMPILING FOR CODESIZE TEST"

	// running code size tests, leave out debug messages!
	#define CHECK_INVARIANTS 0
	#define WISELIB_DISABLE_DEBUG 1

	//#if CODESIZE_INQP
		//#define USE_INQP 1
	//#endif

	//#if CODESIZE_PRESCILLA
		//#define USE_PRESCILLA 1
	//#else
		//#define USE_TREEDICT 1
	//#endif

#else
	#define CHECK_INVARIANTS 0
	#define WISELIB_DISABLE_DEBUG (!defined(PC))
	#define WISELIB_DISABLE_DEBUG_MESSAGES (!defined(PC))
	
	#define USE_LIST_CONTAINER   0
    #define USE_VECTOR_CONTAINER 1
	#define USE_BLOCK_CONTAINER  0

	#define BLOCK_CACHE_SIZE          2
	#define BLOCK_CACHE_SPECIAL       1
	#define BLOCK_CACHE_WRITE_THROUGH 1
	#define BLOCK_CHUNK_SIZE          8
	#define BLOCK_CHUNK_ADDRESS_TYPE ::uint32_t
	
	#define USE_PRESCILLA_DICTIONARY   0
	#define USE_TREE_DICTIONARY        0
	#define USE_BLOCK_DICTIONARY       0
	#define USE_NULL_DICTIONARY        1

	#define USE_INQP 0

	#define INSE_USE_AGGREGATOR 0


	#define BITMAP_ALLOCATOR_RAM_SIZE 1024
#endif


// Allocator

#define NEED_ALLOCATOR (defined(TINYOS) || defined(CONTIKI) || defined(CONTIKI_TARGET_MICAZ))

#include <external_interface/external_interface.h>
#include <external_interface/external_interface_testing.h>
typedef wiselib::OSMODEL Os;

#if NEED_ALLOCATOR
	#warning "Using BITMAP allocator"

	#include <util/allocators/bitmap_allocator.h>
	typedef wiselib::BitmapAllocator<Os, BITMAP_ALLOCATOR_RAM_SIZE> Allocator;
	Allocator allocator_;
	Allocator& get_allocator() { return allocator_; }
#else
	#include <util/allocators/malloc_free_allocator.h>
	typedef wiselib::MallocFreeAllocator<Os> Allocator;
	Allocator allocator_;
	Allocator& get_allocator() { return allocator_; }
#endif


// OS quirks

#if defined(TINYOS) || defined(CONTIKI_TARGET_MSB430)
	int strcmp(const char* a, const char* b) {
		for( ; *a && *b; a++, b++) {
			if(a != b) { return b - a; }
		}
		return b - a;
	}
#endif

#if (defined(TINYOS) || defined(CONTIKI)) && !defined(memset)
	#include <string.h>

	void* memset(void* p, int v, unsigned int n) {
		char *p2 = (char*)p;
		for(unsigned int i = 0; i < n; i++) { *p2++ = (char)v; }
		return p;
	}

	int memcmp(const void* s1, const void* s2, size_t n) {
		const char *p1 = (const char*)s1;
		const char *p2 = (const char*)s2;
		for(unsigned int i = 0; i < n; i++) {
			if(p1[i] != p2[i]) { return p1[i] - p2[i]; }
		}
		return 0;
	}

/*
	char *strncpy(char *dest, const char *src, size_t n) {
		const char *end = src + n;
		for( ; src < end && *src; src++, dest++) {
			*dest = *src;
		}
		if(src < end) { *dest = '\0'; }
		return dest;
	}
	*/
#endif

#if defined(ISENSE)
	void* malloc(size_t n) { return isense::malloc(n); }
	void free(void* p) { isense::free(p); }
#endif
	
#if defined(ISENSE) || defined(TINYOS) || defined(CONTIKI) || defined(CONTIKI_TARGET_MICAZ)
	#warning "assertions messages not implemented for this platform, disabling"
	//extern "C" void assert(int) { }
	#define assert(X)
#endif
	
#ifndef DBG
	#warning "debug messages not implemented for this platform, disabling"
	#define DBG(...)
#endif
	
#if defined(SHAWN)
	#define WISELIB_TIME_FACTOR 100
#endif
	
// Time scaling
	
#if !defined(WISELIB_TIME_FACTOR)
	#define WISELIB_TIME_FACTOR 1
#endif

template<typename OsModel_P>
class NullMonitor {
	public:
		void init(typename OsModel_P::Debug* d) { debug_ = d; }
		
		void report() { }
		void report(const char *remark) { debug_->debug(remark); }
		int free() { return 666; }
		
		typename OsModel_P::Debug* debug_;
};
	
/* vim: set ts=4 sw=4 tw=78 noexpandtab :*/
