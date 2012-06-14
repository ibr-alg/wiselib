
#include "malloc_free_allocator.h"

void* operator new(size_t size, void* ptr, bool _) {
	return ptr;
}


