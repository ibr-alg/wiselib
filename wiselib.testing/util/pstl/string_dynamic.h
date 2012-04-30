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
			typedef Allocator_P Allocator;
			typedef Char_P Char;
			
			typedef string_dynamic<OsModel_P, Allocator_P> self_type;
			typedef self_type* self_pointer_t;
			typedef typename Allocator::template pointer_t<Char> char_pointer_t;
			typedef typename Allocator::template array_pointer_t<Char> char_arr_pointer_t;
			
			string_dynamic() : buffer_(0), size_(0), allocator_(0) { //, weak_(false) {
			}
			
			string_dynamic(typename Allocator::self_pointer_t alloc) : buffer_(0), size_(0), allocator_(alloc) { //, weak_(false) {
			}
			
			string_dynamic(const string_dynamic& other) : buffer_(0), size_(0), allocator_(other.allocator_) { //, weak_(false) {
				// TODO: Implement buffer sharing with copy-on-write
				resize(other.size_);
				to_buffer_(other.buffer_, size_);
			}
			
			string_dynamic(const Char* c, typename Allocator::self_pointer_t alloc)
				: buffer_(0), size_(0), allocator_(alloc) { //, weak_(false) {
				resize(strlen((const char*)c));
				to_buffer_(c, strlen((const char*)c));
			}
			
			string_dynamic(const Char* c, size_t size, typename Allocator::self_pointer_t alloc)
				: buffer_(0), size_(0), allocator_(alloc) { //, weak_(false) {
				resize(size);
				to_buffer_(c, size);
			}
			
			string_dynamic& operator=(const string_dynamic& other) {
				if(!allocator_) {
					allocator_ = other.allocator_;
				}
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
					//allocator_->template free_array<Char>(buffer_.raw());
					allocator_->template free_array<Char>(buffer_);
					buffer_ = 0;
					size_ = 0;
				}
			}
				
			
			
			Allocator& allocator() { return *allocator_; }
			//void set_allocator(Allocator& alloc) { allocator_ = &alloc; }
			void set_allocator(typename Allocator::self_pointer_t alloc) { allocator_ = alloc; }
			
			/**
			 * If true, don't delete the internal buffer upon destruction.
			 * Useful if you (shallow) "serialize" the string instance into some other
			 * format and call its destructor but actually plan to cast it
			 * back later. Normally that wouldnt be possible because the
			 * internally buffer would get lost, if you set the object to be
			 * "weak" in the meantime, the buffer will persist and the
			 * reconstructed object can be used.
			 * Only use if you know what you are doing! These methods are
			 * basically a recipe for memory leaks!
			 */
			//bool weak() const { return weak_; }
			
			/**
			 * Set/unset "weak" property.
			 * See weak() for explanation on weakness.
			 */
			//void set_weak(bool s) const { weak_ = s; }
			
			
			size_t size() const {
				return size_;
			}
			
			size_t length() const {
				return size_;
			}
			
			void resize(size_t n) {
				if(n == size_) { return; }
				size_ = n;
				
				if(buffer_) {
					allocator_->template free_array<Char>(buffer_);
				}
				buffer_ = allocator_->template allocate_array<Char>(size_ + 1);
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
				return append(string_dynamic(other, allocator_));
			}
			
			string_dynamic& append(const string_dynamic& other) {
				char_arr_pointer_t old_buffer = buffer_;
				buffer_ = allocator_->template allocate_array<Char>(size_ + other.size_ + 1);
				
				if(old_buffer) {
					to_buffer_(old_buffer, size_);
				}
				to_buffer_(other.buffer_, other.size_, size_);
				
				size_ = size_ + other.size_;
				if(old_buffer) {
					allocator_->template free_array<Char>(old_buffer);
				}
				buffer_[size_] = '\0';
				
				return *this;
			}
			
			string_dynamic& push_back(Char c) {
				char_arr_pointer_t old_buffer = buffer_;
				buffer_ = allocator_->template allocate_array<Char>(size_ + 2);
				
				if(old_buffer) {
					to_buffer_(old_buffer, size_);
				}
				to_buffer_(&c, 1, size_);
				
				size_ = size_ + 1;
				if(old_buffer) {
					allocator_->template free_array<Char>(old_buffer);
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
				return string_dynamic(buffer_.raw() + from, length, allocator_);
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
			typename Allocator::self_pointer_t allocator_;
			//mutable bool weak_;
	}; // __attribute__((__packed__));
	
	/**
	 */
	template<
		typename String_P,
		typename Allocator_P
	>
	class string_creator {
		public:
			typedef String_P String;
			typedef Allocator_P Allocator;
			
			string_creator(Allocator& alloc) : allocator_(alloc) {
			}
			
			String operator()(const char* s) {
				return String(s, &allocator_);
			}
			
		private:
			Allocator& allocator_;
	};
	
} // ns

#endif // STRING_DYNAMIC_H

