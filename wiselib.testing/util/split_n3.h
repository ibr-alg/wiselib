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

#ifndef SPLIT_N3_H
#define SPLIT_N3_H

#include <util/string_util.h>

namespace wiselib {
	
	/**
	 * @brief
	 * 
	 * @ingroup
	 * 
	 * @tparam 
	 */
	template<
		typename OsModel_P,
		int MAX_LINE_LENGTH_P = 20480,
		int MAX_ELEMENTS_P = 16
	>
	class SplitN3 {
		
		public:
			typedef OsModel_P OsModel;
			typedef typename OsModel::block_data_t block_data_t;
			typedef typename OsModel::size_t size_type;
			
			enum Restrictions {
				MAX_LINE_LENGTH = MAX_LINE_LENGTH_P,
				MAX_ELEMENTS = MAX_ELEMENTS_P
			};
			
			SplitN3() : size_(0) {
			}
			
			void parse_line(char *line) {
				strncpy(line_, line, MAX_LINE_LENGTH_P);
				
				char *p = line_;
				
				for(size_ = 0; *p; size_++) {
					elements_[size_] = parse_element(p);
					p++;
					if(*p == '.') { size_++; break; }
				}
			}
			
			char*& operator[](size_type i) { return elements_[i]; }
			size_type size() { return size_; }
		
		private:
			
			char* parse_element(char*& p) {
				char *r;
				p = skip_whitespace(p);
				if(*p == '"') { r = parse_literal(p); }
				else if(*p == '<') { r = parse_uri(p); }
				else { r = parse_name(p); }
				p = skip_whitespace(p);
				return r;
			}
			
			char* parse_literal(char*& p) {
				bool escaped = false;
				size_type loss = 0; // how many chars did we cut out due to escaping?
				char *r = p;
				p++;
				for( ; *p; p++) {
					if(escaped) {
						escaped = false;
					}
					else {
						if(*p == '\\') {
							switch(*(p + 1)) {
								case 'n':
									memmove(p, p + 1, strlen(p));
									*p = '\n';
									break;
								case '"':
									escaped = true;
									break;
							}
						}
						else {
							if(*p == '"') {
								p++;
								if(*p == '^') {
									p++;
									assert(*p == '^');
									parse_element(p);
								}
								else if(*p == '@') {
									parse_lang(p);
								}
								*p = '\0';
								//assert(*p == '\0');
								//p++;
								break;
							} // if p == quote
						} // else p == backslash
					} // else escaped
				} // for p
				return r;
			}
			
			char* parse_lang(char*& p) {
				assert(*p == '@');
				p++;
				return parse_name(p);
			}
			
			char* parse_name(char*& p) {
				char* r = p;
				for( ; *p && !is_whitespace(*p); p++) {
				}
				*p = '\0';
				return r;
			}
			
			char* parse_uri(char*& p) {
				char* r = p;
				for( ; *p; p++) {
					if(*p == '>') {
						p++;
						*p = '\0';
						break;
					}
				}
				assert(*p == '\0');
				return r;
			}
			
			char line_[MAX_LINE_LENGTH];
			
			size_type size_;
			char *elements_[MAX_ELEMENTS];
		
	}; // SplitN3
}

#endif // SPLIT_N3_H

