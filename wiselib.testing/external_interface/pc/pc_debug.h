// vim: set noexpandtab ts=4 sw=4:

#ifndef PC_DEBUG_H
#define PC_DEBUG_H

#include <iostream>
#include <cstdarg>
#include <cstdio>

namespace wiselib {
	template<typename OsModel_P>
	class PCDebug {
		public:
			typedef OsModel_P OsModel;
			
			typedef PCDebug<OsModel> self_type;
			typedef self_type* self_pointer_t;
			
			PCDebug() {
			}
						
			void debug(const char* msg, ...) {
				va_list fmtargs;
				char buffer[1024];
				va_start(fmtargs, msg);
				vsnprintf(buffer, sizeof(buffer) - 1, msg, fmtargs);
				va_end(fmtargs);
				std::cout << buffer;
				std::cout.flush();
			}
	};
}

#endif // PC_DEBUG_H

