
#ifndef STRING_DYNAMIC_H
#define STRING_DYNAMIC_H

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
			
			string_dynamic() : buffer_(0), size_(0), allocator_(0) {
			}
			
			string_dynamic(Allocator& alloc) : buffer_(0), size_(0), allocator_(&alloc) {
			}
			
			string_dynamic(const string_dynamic& other) : buffer_(0), size_(0), allocator_(other.allocator_) {
				// TODO: Implement buffer sharing with copy-on-write
				resize(other.size_);
				to_buffer_(other.buffer_, size_);
			}
			
			string_dynamic(const Char* c, typename Allocator::self_pointer_t alloc)
				: buffer_(0), size_(0), allocator_(alloc) {
				resize(strlen(c));
				to_buffer_(c, strlen(c));
			}
			
			string_dynamic(const Char* c, size_t size, typename Allocator::self_pointer_t alloc)
				: buffer_(0), size_(0), allocator_(alloc) {
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
				resize(strlen(other));
				to_buffer_(other, size_);
				return *this;
			}
			
			~string_dynamic() {
				if(buffer_) {
					allocator_->template free_array<Char>(buffer_);
					buffer_ = 0;
					size_ = 0;
				}
			}
			
			
			size_t size() const {
				return size_;
			}
			
			void resize(size_t n) {
				if(n == size_) { return; }
				size_ = n;
				
				if(buffer_) { allocator_->template free_array<Char>(buffer_); }
				buffer_ = allocator_->template allocate_array<Char>(size_ + 1);
				buffer_[size_] = '\0';
			}
			
			const Char* c_str() const {
				return buffer_.raw();
			}
			
			int cmp(const string_dynamic& other) const {
				if(size_ != other.size_) { return size_ < other.size_ ? -1 : size_ > other.size_; }
				for(size_t i=0; i<size_; i++) {
					if(buffer_[i] < other.buffer_[i]) { return -1; }
					if(buffer_[i] > other.buffer_[i]) { return  1; }
				}
				return 0;
			}
			bool operator<(const string_dynamic& other) const { return cmp(other) < 0; }
			bool operator<=(const string_dynamic& other) const { return cmp(other) <= 0; }
			bool operator>(const string_dynamic& other) const { return cmp(other) > 0; }
			bool operator>=(const string_dynamic& other) const { return cmp(other) >= 0; }
			bool operator==(const string_dynamic& other) const { return cmp(other) == 0; }
			bool operator!=(const string_dynamic& other) const { return cmp(other) != 0; }
			
			string_dynamic& append(const Char* other) {
				return append(string_dynamic(other, allocator_));
			}
			
			string_dynamic& append(const string_dynamic& other) {
				char_pointer_t old_buffer = buffer_;
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
			
		private:
			template<typename T>
			void to_buffer_(T src, size_t n, size_t offset = 0) {
				Char* ptr = buffer_.raw() + offset;
				for(size_t i=0; i<n; i++) {
					*ptr = *src;
					++ptr; ++src;
				}
			}
			
			char_pointer_t buffer_;
			size_t size_;
			typename Allocator::self_pointer_t allocator_;
	};
	
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

