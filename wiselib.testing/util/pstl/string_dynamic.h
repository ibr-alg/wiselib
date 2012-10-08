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


#ifndef STRING_DYNAMIC_H
#define STRING_DYNAMIC_H

#ifndef assert
	#define assert(X)
#endif

namespace wiselib {
	
	/**
	 * Dynamic string implementation.
	 * 
	 * @ingroup String_concept
	 * @ingroup PSTL
	 */
	template<
		typename OsModel_P,
		typename Allocator_P,
		typename Char_P = char
	>
	class string_dynamic {
		public:
			typedef OsModel_P OsModel;
			typedef typename OsModel_P::size_t size_t;
			typedef Char_P Char;
			typedef Allocator_P Allocator;
			
			typedef string_dynamic<OsModel, Allocator, Char> self_type;
			typedef self_type* self_pointer_t;
			typedef typename Allocator::template pointer_t<Char> char_pointer_t;
			typedef typename Allocator::template array_pointer_t<Char> char_arr_pointer_t;
			
			string_dynamic() : buffer_(0), size_(0) {
			}
			
			string_dynamic(typename Allocator::self_pointer_t alloc) : buffer_(0), size_(0) {
			}
			
			string_dynamic(const string_dynamic& other) : buffer_(0), size_(0){
				// TODO: Implement buffer sharing with copy-on-write
				resize(other.size_);
				to_buffer_(other.buffer_, size_);
			}
			
			string_dynamic(const Char* c) : buffer_(0), size_(0) {
				resize(strlen((const char*)c));
				to_buffer_(c, strlen((const char*)c));
			}
			
			string_dynamic(const Char* c, size_t size) : buffer_(0), size_(0) {
				resize(size);
				to_buffer_(c, size);
			}
			
			string_dynamic& operator=(const string_dynamic& other) {
				resize(other.size_);
				to_buffer_(other.buffer_, size_);
				return *this;
			}
			
			string_dynamic& operator=(const Char* other) {
				resize(strlen((const char*)other));
				to_buffer_(other, size_);
				return *this;
			}
			
			~string_dynamic() {
				if(buffer_) { // && !weak_) {
					get_allocator().template free_array<Char>(buffer_);
					buffer_ = 0;
					size_ = 0;
				}
			}
			
			size_t size() const {
				return size_;
			}
			
			size_t length() const {
				return size_;
			}
			
			void clear() {
				resize(0);
			}
			
			void resize(size_t n) {
				if(n == size_) { return; }
				size_ = n;
				
				if(buffer_) {
					get_allocator().template free_array<Char>(buffer_);
				}
				buffer_ = get_allocator().template allocate_array<Char>(size_ + 1);
				assert(buffer_);
				buffer_[size_] = '\0';
			}
			
			const Char* c_str() const { return buffer_.raw(); }
			Char* c_str() { return buffer_.raw(); }
			
			const Char* data() const { return buffer_.raw(); }
			Char* data() { return buffer_.raw(); }
			
			int cmp(const string_dynamic& other) const {
				int r = 0;
				if(size_ != other.size_) { r = (size_ < other.size_) ? -1 : (size_ > other.size_); }
				else {
					for(size_t i=0; i<size_; i++) {
						if(buffer_[i] < other.buffer_[i]) { r = -1; break; }
						if(buffer_[i] > other.buffer_[i]) { r =  1; break; }
					}
				}
				return r;
			}
			bool operator<(const string_dynamic& other) const { return cmp(other) < 0; }
			bool operator<=(const string_dynamic& other) const { return cmp(other) <= 0; }
			bool operator>(const string_dynamic& other) const { return cmp(other) > 0; }
			bool operator>=(const string_dynamic& other) const { return cmp(other) >= 0; }
			bool operator==(const string_dynamic& other) const { return cmp(other) == 0; }
			bool operator!=(const string_dynamic& other) const { return cmp(other) != 0; }
		//	char operator[] (const size_t pos) const {return (pos >= size_) ? 0 : buffer_[pos]; }
			Char& operator[] (const size_t pos)  { return buffer_[pos]; }
			
			string_dynamic& append(const Char* other) {
				return append(string_dynamic(other));
			}
			
			string_dynamic& append(const string_dynamic& other) {
				char_arr_pointer_t old_buffer = buffer_;
				buffer_ = get_allocator().template allocate_array<Char>(size_ + other.size_ + 1);
				
				if(old_buffer) {
					to_buffer_(old_buffer, size_);
				}
				to_buffer_(other.buffer_, other.size_, size_);
				
				size_ = size_ + other.size_;
				if(old_buffer) {
					get_allocator().template free_array<Char>(old_buffer);
				}
				buffer_[size_] = '\0';
				
				return *this;
			}
			
			string_dynamic& push_back(Char c) {
				char_arr_pointer_t old_buffer = buffer_;
				buffer_ = get_allocator().template allocate_array<Char>(size_ + 2);
				
				if(old_buffer) {
					to_buffer_(old_buffer, size_);
				}
				to_buffer_(&c, 1, size_);
				
				size_ = size_ + 1;
				if(old_buffer) {
					get_allocator().template free_array<Char>(old_buffer);
				}
				buffer_[size_] = '\0';
				
				return *this;
			}
			
			int first_index_of(Char c) const {
				for(int i=0; i<size_; i++) {
					if(buffer_[i] == c) { return i; }
				}
				return -1;
			}
			
			string_dynamic substr(int from, int length=-1) const {
				if(length == 0) length = size_ - from;
				return string_dynamic(buffer_.raw() + from, length);
			}
			
			template<typename Int>
			void append_int(Int i, Int base=10) {
				if(i < 0) {
					push_back('-');
					i = -i;
				}
				int_to_string_r(i, base);
			}
			
			/**
			 * Currently only handles positive integer numbers
			 */
			template<typename Int>
			Int parse_int(Int base=10) {
				Int r = 0;
				for(size_t i=0; i<size(); i++) {
					if(isnum((*this)[i])) {
						r = (r * base) + to_int((*this)[i]); 
					}
				}
				return r;
			}
			
		private:
		size_t strlen(const char* s) {
			size_t r = 0;
			while(s[r] != '\0') r++;
			return r;
		}
			int isnum(Char c) {
				if(c >= '0' && c <= '9') {
					return true;
				}
				if(c >= 'a' && c <= 'z') {
					return true;
				}
			}
			int to_int(Char c) {
				if(c >= '0' && c <= '9') {
					return c - '0';
				}
				return c - 'a';
			}
			
			template<typename Int>
			void int_to_string_r(Int i, Int base, bool first=true) {
				if(i > base) {
					int_to_string_r((Int)(i / base), (Int)base, false);
				}
				if((i == 0) && !first) {
					return;
				}
				if((i%base) < 10) {
					push_back('0' + (i % base));
				}
				else {
					push_back('a' + ((i % base) - 10));
				}
			}
			
			
		
			template<typename T>
			void to_buffer_(T src, size_t n, size_t offset = 0) {
				if(n == 0) { return; }
				
				Char* ptr = buffer_.raw() + offset;
				for(size_t i=0; i<n; i++) {
					*ptr = *src;
					++ptr; ++src;
				}
			}
			
			char_arr_pointer_t buffer_;
			size_t size_;
	};
} // ns

#endif // STRING_DYNAMIC_H

