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
}

#endif // STRING_UTIL_H

