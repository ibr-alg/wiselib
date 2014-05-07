
#if defined(TINYOS) || defined(CONTIKI)
	extern "C" {
#include <string.h>
#include <stdio.h>
	}
#endif

#if defined(ISENSE) || defined(TINYOS) || defined(CONTIKI) || defined(CONTIKI_TARGET_MICAZ)
	#warning "assertions messages not implemented for this platform, disabling"
	#define assert(X)
#endif

