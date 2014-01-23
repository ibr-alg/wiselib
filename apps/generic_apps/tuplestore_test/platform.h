
#if defined(TINYOS) || defined(CONTIKI)
	//int strcmp(const char* a, const char* b) {
		//for( ; *a && *b; a++, b++) {
			//if(*a != *b) { return *b - *a; }
		//}
		//return *b - *a;
	//}
/*

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

	*/
	extern "C" {
#include <string.h>
#include <stdio.h>
	}

/*
linking...
msp430-g++ out/contiki-sky/app_database.o -mmcu=msp430x1611 -Wl,-Map=contiki-sky.map -Wl,--gc-sections,--undefined=_reset_vector__,--undefined=InterruptVectors,--undefined=_copy_data_init__,--undefined=_clear_bss_init__,--undefined=_end_of_init__ -L. -L/home/henning/repos/wiselib/applications/lib obj_sky/contiki-sky-main.o contiki-sky.a -o out/contiki-sky/app_database.elf  \
		out/contiki-sky/contiki_os.o out/contiki-sky/contiki_timer.o out/contiki-sky/contiki_radio.o \
		out/contiki-sky/contiki_ext_radio.o out/contiki-sky/contiki_uart.o out/contiki-sky/contiki_byte_uart.o \
		out/contiki-sky/contiki_sky_button_listener.o  obj_sky/sht11-sensor.o obj_sky/sht11.o obj_sky/battery-sensor.o 
app_database.o: In function `StaticDictionary<>::find_slot(unsigned int, unsigned char*, bool&)':
static_dictionary.h:239: undefined reference to `memcmp(void const*, void const*, unsigned int)'
app_database.o: In function `StaticDictionary<>::make_meta(StaticDictionary<>::Slot&)':
static_dictionary.h:223: undefined reference to `memset(void*, int, unsigned int)'
app_database.o: In function `StaticDictionary<>::insert(unsigned char*)':
static_dictionary.h:111: undefined reference to `strncpy(char*, char const*, unsigned int)'
collect2: ld returned 1 exit status
 *
 */

#endif

#if defined(ISENSE) || defined(TINYOS) || defined(CONTIKI) || defined(CONTIKI_TARGET_MICAZ)
	#warning "assertions messages not implemented for this platform, disabling"
	//extern "C" void assert(int) { }
	#define assert(X)
#endif

