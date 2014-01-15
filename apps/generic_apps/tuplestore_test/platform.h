
#if defined(TINYOS) || defined(CONTIKI)
	int strcmp(const char* a, const char* b) {
		for( ; *a && *b; a++, b++) {
			if(*a != *b) { return *b - *a; }
		}
		return *b - *a;
	}
#endif

#if defined(ISENSE) || defined(TINYOS) || defined(CONTIKI) || defined(CONTIKI_TARGET_MICAZ)
	#warning "assertions messages not implemented for this platform, disabling"
	//extern "C" void assert(int) { }
	#define assert(X)
#endif

