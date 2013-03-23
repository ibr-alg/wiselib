
#ifndef DEBUGGING_H
#define DEBUGGING_H

namespace wiselib {
	
	template<typename OsModel_P, size_t COLUMNS_P, typename Debug_P>
	void debug_buffer(Debug_P* debug_,
			typename OsModel_P::block_data_t *buffer,
			typename OsModel_P::size_t length) {
		typedef typename OsModel_P::size_t size_t;
		typedef typename OsModel_P::block_data_t block_data_t;
		
		enum { SZ = COLUMNS_P * 8 + 10 };
		char line[SZ];
		char *l = line;
		size_t i = 0;
		size_t line_start = 0;
		
		debug_->debug("buf @%x sz=%u", (void*)buffer, length);
		while(i < length) {
			int written = snprintf(l, SZ - (l - line), "%02x ", buffer[i]);
			
			l += written;
			i++;
			if(i % COLUMNS_P == 0) {
				
				for(size_t j = line_start; j < i; j++) {
					int written = snprintf(l, SZ - (l - line), "%c", 
						(j < length) ? ((char)(buffer[j] >= 0x20 && buffer[j] <= 0x7e) ? buffer[j] : '.') : '_');
					l += written ;
				}
				debug_->debug("%s", line);
				l = line;
				line_start = i;
			}
		}
		if(l != line) {
			for( ; i % COLUMNS_P; i++) {
				*l = '.'; l++;
				*l = '.'; l++;
				*l = ' '; l++;
			}
			
			for(size_t j = line_start; j < i; j++) {
				int written = snprintf(l, SZ - (l - line), "%c", 
					(j < length) ? ((char)(buffer[j] >= 0x20 && buffer[j] <= 0x7e) ? buffer[j] : '.') : '_');
				l += written ;
			}
			debug_->debug("%s", line);
		}
		debug_->debug("");
	} // debug_buffer()
	
}

#endif // DEBUGGING_H

