/***************************************************************************
 ** This file is part of the generic algorithm library Wiselib.           **
 ** Copyright (C) 2008,2009 by the Wisebed (www.wisebed.eu) project.      **
 **                                                                       **
 ** The Wiselib is free software: you can redistribute it and/or modify   **
 ** it under the terms of the GNU Lesser General Public License as        **
 ** published by the Free Software Foundation, either version 3 of the    **
 ** License, or (at your option) any later version.                       **
 **                                                                       **
 ** The Wiselib is distributed in the hope that it will be useful,        **
 ** but WITHOUT ANY WARRANTY; without even the implied warranty of        **
 ** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         **
 ** GNU Lesser General Public License for more details.                   **
 **                                                                       **
 ** You should have received a copy of the GNU Lesser General Public      **
 ** License along with the Wiselib.                                       **
 ** If not, see <http://www.gnu.org/licenses/>.                           **
 ***************************************************************************/

#ifndef STRING_UTIL_H
#define STRING_UTIL_H

namespace wiselib {
	
	bool is_whitespace(char c) {
		return (c == ' ') || (c == '\t') || (c == '\x0a') || (c == '\x0d');
	}
	
	char* skip_whitespace(char* p) {
		for( ; is_whitespace(*p); p++) { }
		return p;
	}
	
	bool is_printable(char c) {
		unsigned char uc = (unsigned char)c;
		return (uc >= 0x20) && (uc <= 0x7e);
	}
	
	char hexchar(::uint8_t n) {
		assert(n < 16);
		return (n < 10) ? ('0' + n) : ('a' + n - 10);
	}
	
#if STRING_UTIL_USE_ALLOC
	/**
	 * Allocate memory large enough for the concatenation
	 * of s1 and s2 and copy that concatenation to it.
	 * Return address of the resulting string.
	 * Caller is responsible for freeing result
	 * eg by calling get_allocator().free_array<char>(returnvalue).
	 */
	char *alloc_strcat(char* s1, char* s2) {
		size_t l1 = strlen(s1), l2 = strlen(s2);
			
		char *r = ::get_allocator().allocate_array<char>(l1 + l2 + 1) .raw();
		memcpy((void*)r, (void*)s1, l1);
		memcpy((void*)(r + l1), (void*)s2, l2);
		r[l1 + l2] = '\0';
		return r;
	}
#endif
	
	long atol(char *s) {
		long r = 0;
		for( ; *s != '\0'; s++) {
			r *= 10;
			r += (*s - '0');
		}
		return r;
	}
	
	template<typename Value>
	int ltoa(unsigned long buflen, char* buffer, Value v, int base = 10) {
		int digits = 0;
		Value v2 = v;
		//int base = 10;
		for( ; v2; digits++) { v2 /= base; }
		if((unsigned long)digits >= buflen) { return 0; }
		
		buffer[digits + 1] = '\0';
		for( ; digits; digits--) {
			buffer[digits] = v % 10 + '0';
			v /= 10;
		}
		return digits;
	}
	
	float atof(char *s) {
		float r = 0;
		float f = 1.0;
		for( ; *s != '\0' && *s != '.'; s++) {
			r *= 10;
			r += (*s - '0');
		}
		if(*s == '.') { s++; }
		for( ; *s != '\0'; s++) {
			f /= 10;
			r += (*s - '0') * f;
		}
		return r;
	}
	
	template<typename Value>
	int ftoa(unsigned long buflen, char* buffer, Value v, unsigned long prec) {
		int digits = 0; // digits before decimal point
		Value v2 = v;
		int base = 10;
		for( ; v2 >= 1; digits++) { v2 /= base; }
		if(digits == 0) { digits = 1; }
		if(digits + 1 + prec >= buflen) {
			return -1;
		}
		
		v2 = v;
		v2 -= (long)v; // TODO: use standalone math to improve this
		int l = digits + 1 + prec + 1;
		buffer[l] = '\0';
		buffer[digits] = '.';
		long b = base;
		for(unsigned long i = 0; i < prec; i++) {
			Value x = (long)(v * b) % base;
			buffer[digits + 1 + i] = (char)x + '0';
			b *= 10;
		}
		
		long vl = v;
		for( ; digits; digits--) {
			buffer[digits - 1] = '0' + (char)vl % 10;
			vl /= 10;
		}
		return l;
	}
	
	size_t prefix_length(char *a, char *b) {
		size_t r = 0;
		for( ; *a && (*a == *b); ++a, ++b) {
			++r;
		}
		return r;
	}
	
	/**
	 * In contrast to @a prefix_length, this will ignore 0-termination!
	 */
	template<typename S>
	S prefix_length_n(S n, ::uint8_t *a, ::uint8_t *b) {
		S r = 0;
		for( ; (r < n) && (*a == *b); ++a, ++b) {
			++r;
		}
		return r;
	}
	
	
}

#endif // STRING_UTIL_H

