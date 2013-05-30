
#ifndef STRING_UTILS_H
#define STRING_UTILS_H

namespace wiselib {
	
	/**
	 * Allocate memory large enough for the concatenation
	 * of s1 and s2 and copy that concatenation to it.
	 * Return address of the resulting string.
	 * Caller is responsible for freeing result
	 * eg by calling get_allocator().free_array<char>(returnvalue).
	 */
	char *alloc_strcat(char* s1, char* s2) {
		size_t l1 = strlen(s1), l2 = strlen(s2);
			
		char *r = get_allocator().allocate_array<char>(l1 + l2 + 1) .raw();
		memcpy((void*)r, (void*)s1, l1);
		memcpy((void*)(r + l1), (void*)s2, l2);
		r[l1 + l2] = '\0';
		return r;
	}
	
	long atol(char *s) {
		long r = 0;
		for( ; *s != '\0'; s++) {
			r *= 10;
			r += (*s - '0');
		}
		return r;
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
	
	size_t prefix_length(char *a, char *b) {
		size_t r = 0;
		for( ; *a && (*a == *b); ++a, ++b) {
			++r;
		}
		return r;
	}
}

#endif // STRING_UTILS_H

