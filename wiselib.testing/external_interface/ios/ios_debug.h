#ifndef IOS_DEBUG_H
#define IOS_DEBUG_H


namespace wiselib {
	template<typename OsModel_P>
	class iOsDebug {
		public:
			typedef OsModel_P OsModel;
			
			typedef iOsDebug<OsModel> self_type;
			typedef self_type* self_pointer_t;
			
			iOsDebug(iOsSystem& system)
            : system_(system)
         {
			}
         
			void debug(const char* msg, ...) {
         
				va_list fmtargs;
				char buffer[1024];
				va_start(fmtargs, msg);
				vsnprintf(buffer, sizeof(buffer) - 1, msg, fmtargs);
				va_end(fmtargs);
				        
            NSString *str = [[NSString alloc] initWithCString:buffer];
            NSLog(@"DEBUG: %@",str);
            [str release];
            
			}
      private:
         iOsSystem& system_;
	};
}

#endif // IOS_DEBUG_H

