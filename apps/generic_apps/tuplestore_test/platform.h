
#if defined(TINYOS) || defined(CONTIKI)
	int strcmp(const char* a, const char* b) {
		for( ; *a && *b; a++, b++) {
			if(*a != *b) { return *b - *a; }
		}
		return *b - *a;
	}

	int mymemcmp(void const* a, void const* b, size_t l) { 
		const char* p1 = reinterpret_cast<const char*>(a);
		const char* p2 = reinterpret_cast<const char*>(b);
		const char* end = p1 + l;
		for( ; p1 < end; p1++, p2++) {
			if(*p1 != *p2) { return *p2 - *p1; }
		}
		return *p2 - *p1;
	}

	void mymemset(void *p, int v, unsigned l) {
		char *p1 = reinterpret_cast<char*>(p);
		for(unsigned i = 0; i < l; i++) {
			p1[i] = v;
		}
	}

	void mystrncpy(char* to, char const* from, unsigned l) {
		unsigned i = 0;
		for(; from[i] && i<l; i++) {
			to[i] = from[i];
		}
		if(i < l) {
			to[i] = from[i];
		}
	}

#endif

#if defined(ISENSE) || defined(TINYOS) || defined(CONTIKI) || defined(CONTIKI_TARGET_MICAZ)
	#warning "assertions messages not implemented for this platform, disabling"
	//extern "C" void assert(int) { }
	#define assert(X)
#endif

