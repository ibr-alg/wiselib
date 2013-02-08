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

#ifndef TRAITS_H
#define TRAITS_H

namespace wiselib {
	
	template<typename T>
	struct Compare {
		static int cmp(const T& a, const T& b) {
			return (a < b) ? -1 : (b < a);
		}
	};
	
	template<>
	struct Compare<char*> {
		static int cmp(const char* a, const char* b) {
			return strcmp(a, b);
		}
	};
	
	
	
	template<typename T>
	struct Payload {
		static const bool fixed_size = true;
		static int size(const T& a) { return sizeof(T); }
		static ::uint8_t* data(T& a) { return reinterpret_cast< ::uint8_t*>(&a); }
		static const ::uint8_t* data(const T& a) { return reinterpret_cast<const ::uint8_t*>(&a); }
		template<typename U> static T& from_data(U* p) { return *reinterpret_cast<T*>(p); }
	};
	
	template<>
	struct Payload<char*> {
		static const bool fixed_size = false;
		static int size(const char* a) { return strlen((const char*)a) + 1; }
		static ::uint8_t* data(char* a) { return reinterpret_cast< ::uint8_t*>(a); }
		static const ::uint8_t* data(const char* a) { return reinterpret_cast<const ::uint8_t*>(a); }
		template<typename U> static char* from_data(U* p) { return reinterpret_cast<char*>(p); }
		template<typename U> static const char* from_data(const U* p) { return reinterpret_cast<const char*>(p); }
	};
	
}

#endif // TRAITS_H



