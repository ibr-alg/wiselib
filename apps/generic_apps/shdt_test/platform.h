#include <external_interface/external_interface.h>
#include <external_interface/external_interface_testing.h>
typedef wiselib::OSMODEL Os;

#include <util/allocators/malloc_free_allocator.h>
typedef wiselib::MallocFreeAllocator<Os> Allocator;
Allocator allocator_;
Allocator& get_allocator() { return allocator_; }


