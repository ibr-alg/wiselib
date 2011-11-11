
#ifndef __WISELIB_UTIL_ALLOCATORS_FIRST_FIT_ALLOCATOR_H
#define __WISELIB_UTIL_ALLOCATORS_FIRST_FIT_ALLOCATOR_H

#ifndef ALLOCATOR_KEEP_STATS
	#define ALLOCATOR_KEEP_STATS 0
#endif // ALLOCATOR_KEEP_STATS

#ifndef assert
	#define assert(X)
#endif // assert

template<typename T>
void* operator new(size_t size, T* place) {
	//return ptr.raw();
	return place;
}

namespace wiselib {


template<uint64_t N_>
struct small_uint {
	typedef typename small_uint<
		(N_ >= 0x100000001) ? 0x100000001LL :
		(N_ >= 0x000010001) ? 0x000010001L :
		(N_ >= 0x000000101) ? 0x000000101 :
			0x000000000
		>::t t;
};

template<> struct small_uint<0x000000000> { typedef uint8_t t; };
template<> struct small_uint<0x000000101> { typedef uint16_t t; };
template<> struct small_uint<0x000010001L> { typedef uint32_t t; };

//#if __WORDSIZE == 64
template<> struct small_uint<0x100000001LL> { typedef uint64_t t; };
//#endif


/**
 * Simple first fit allocator.
 * 
 * @ingroup Allocator_concept
 */
template<
	typename OsModel_P,
	size_t BUFFER_SIZE,
	size_t MAX_CHUNKS
>
class FirstFitAllocator {
	
	public:
		class Chunk;
		typedef OsModel_P OsModel;
		typedef FirstFitAllocator<OsModel_P, BUFFER_SIZE, MAX_CHUNKS> self_type;
		typedef self_type* self_pointer_t;
		typedef typename OsModel::size_t size_t;
		typedef typename OsModel::block_data_t block_data_t;
		
		typedef typename small_uint<BUFFER_SIZE>::t memory_size_t;
		typedef typename small_uint<MAX_CHUNKS>::t chunk_index_t;
		
		
		enum { SUCCESS = OsModel::SUCCESS, ERR_UNSPEC = OsModel::ERR_UNSPEC };
		
		template<typename T>
		struct pointer_t {
			public:
				pointer_t() : chunk_(0) { }
				pointer_t(Chunk* c) : chunk_(c) { }
				pointer_t(const pointer_t& other) : chunk_(other.chunk_) { }
				pointer_t& operator=(const pointer_t& other) { chunk_ = other.chunk_; return *this; }
				const T& operator*() const { return *raw(); }
				T& operator*() { return *raw(); }
				const T* operator->() const { return raw(); }
				T* operator->() { return raw(); }
				//T& operator[](size_t idx) { return raw()[idx]; }
				const T& operator[](size_t idx) const { return chunk_[idx]; }
				bool operator==(const pointer_t& other) const { return chunk_ == other.chunk_; }
				bool operator!=(const pointer_t& other) const { return chunk_ != other.chunk_; }
				operator bool() const { return chunk_ != 0; }
				T* raw() { return reinterpret_cast<T*>(chunk_->start()); }
				const T* raw() const { return reinterpret_cast<const T*>(chunk_->start()); }
			protected:
				Chunk* chunk_;
				
			friend class FirstFitAllocator<OsModel_P, BUFFER_SIZE, MAX_CHUNKS>;
		};
		
		template<typename T>
		struct array_pointer_t {
			public:
				array_pointer_t() : chunk_(0), offset_(0) { }
				array_pointer_t(Chunk* c) : chunk_(c), offset_(0) { }
				array_pointer_t(const array_pointer_t& other) : chunk_(other.chunk_), offset_(other.offset_) { }
				array_pointer_t& operator=(const array_pointer_t& other) {
					chunk_ = other.chunk_;
					offset_ = other.offset_;
					return *this;
				}
				const T& operator*() const { return *raw(); }
				T& operator*() { return *raw(); }
				T* operator->() { return raw(); }
				const T* operator->() const { return raw(); }
				T& operator[](size_t idx) { return raw()[idx]; }
				const T& operator[](size_t idx) const { return raw()[idx]; }
				bool operator==(const array_pointer_t& other) const { return chunk_ == other.chunk_; }
				bool operator!=(const array_pointer_t& other) const { return chunk_ != other.chunk_; }
				operator bool() const { return chunk_ != 0; }
				array_pointer_t& operator++() { ++offset_; return *this; }
			//	array_pointer_t& operator--() { --offset_; return *this; }
				T* raw() { return reinterpret_cast<T*>(chunk_->start()) + offset_; }
				const T* raw() const { return reinterpret_cast<const T*>(chunk_->start()) + offset_; }
			protected:
				Chunk* chunk_;
				size_t offset_;
				
			friend class FirstFitAllocator<OsModel_P, BUFFER_SIZE, MAX_CHUNKS>;
		};
		
		FirstFitAllocator() :
			chunks_used_(0),
		#if ALLOCATOR_KEEP_STATS
			bytes_used_(0),
		#endif
			first_chunk_id_(Chunk::NONE)
		{
		}
		
		template<typename T>
		Chunk* allocate_chunk(size_t n = 1) {
			if(chunks_used_ >= MAX_CHUNKS) {
				assert(false && "Allocator reached MAX_CHUNKS!");
				return 0;
			}
			
			block_data_t *end = memory_;
			int to_allocate = sizeof(T) * n;
			
			size_t prev = Chunk::NONE;
			for(size_t c = first_chunk_id_; c != Chunk::NONE; c = reserved_[c].next()) {
				if((reserved_[c].start() - end) >= to_allocate) { break; }
				end = reserved_[c].end();
				prev = c;
			} // for c
			
			size_t r = insert_chunk(prev, end, to_allocate);
			if(r == Chunk::NONE) {
				assert(false && "Allocator reached MAX_CHUNKS!");
				return 0;
			}
			
			assert(&reserved_[r] != 0);
			
			for(size_t i = 0; i < n; i++) {
				new(reinterpret_cast<T*>(reserved_[r].start()) + i) T;
			}
			
			return &reserved_[r];
		}
		
		template<typename T>
		pointer_t<T> allocate() {
			return pointer_t<T>(allocate_chunk<T>());
		}
		
		template<typename T>
		array_pointer_t<T> allocate_array(typename OsModel::size_t n) {
			return array_pointer_t<T>(allocate_chunk<T>(n));
		}
		
		template<typename T>
		int free(pointer_t<T> p) {
			free_chunk(p.chunk_ - reserved_);
			return SUCCESS;
		}
	
		template<typename T>
		int free_array(array_pointer_t<T> p) {
			free_chunk(p.chunk_ - reserved_);
			return SUCCESS;
		}
		
	#if ALLOCATOR_KEEP_STATS
		size_t chunks_used() { return chunks_used_; }
		size_t size() { return bytes_used_; }
	#endif
		
		size_t capacity() { return BUFFER_SIZE; }
			
	//private:
		
		class Chunk {
			public:
				enum { NONE = static_cast<chunk_index_t>(-1) };
				
				Chunk() : size_(0) { }
				bool occupied() { return size_ != 0; }
				block_data_t* start() { return start_; }
				void set_start(block_data_t* s) { start_ = s; }
				block_data_t* end() { return start_ + size_; }
				//void set_end(block_data_t* s) { end_ = s; }
				chunk_index_t next() { return next_; }
				void set_next(chunk_index_t n) { next_ = n; }
				memory_size_t size() { return size_; }
				void set_size(memory_size_t s) { size_ = s; }
				
				
			private:
				memory_size_t size_;
				chunk_index_t next_;
				block_data_t *start_;
				
		} __attribute__((__packed__));
		
		/**
		 * Insert chunk after the chunk at position idx
		 * Does not check if there is actually enough space for the
		 * chunk, just inserts the data into the list of allocated
		 * chunks.
		 */
		size_t insert_chunk(size_t idx, block_data_t* start, size_t size) {
			// First, find a free slot in the reserved table
			size_t c = 0;
			for(; c < MAX_CHUNKS && reserved_[c].occupied(); c++) {
			}
			
			if(c == MAX_CHUNKS) {
				assert(false && "Allocator reached MAX_CHUNKS!");
				return Chunk::NONE;
			}
			
			if(idx == Chunk::NONE) { // insert as first chunk
				first_chunk_id_ = c;
			}
			
			size_t old_next = Chunk::NONE;
			
			if(idx != Chunk::NONE) {
				old_next = reserved_[idx].next();
				reserved_[idx].set_next(c);
			}
			
			reserved_[c].set_start(start);
			reserved_[c].set_size(size);
			reserved_[c].set_next(old_next);
			
			chunks_used_++;
		#if ALLOCATOR_KEEP_STATS
			bytes_used_ += size;
		#endif
			
			return c;
		} // allocate_chunk

		/**
		 */
		void free_chunk(size_t idx) {
			if(first_chunk_id_ == Chunk::NONE) { return; } // free enough for us
			
			if(idx == first_chunk_id_) {
				first_chunk_id_ = reserved_[idx].next();
			}
			else {
				size_t prev;
				for(
					prev = first_chunk_id_;
					prev != Chunk::NONE && reserved_[prev].next() != idx;
					prev = reserved_[prev].next()
				) {}
				
				if(prev != Chunk::NONE) {
					// found a predecessor
					reserved_[prev].set_next(reserved_[idx].next());
				}
			}
			
			chunks_used_--;
		#if ALLOCATOR_KEEP_STATS
			bytes_used_ -= reserved_[idx].size();
		#endif
			reserved_[idx].set_size(0);
		} // free_chunk
		
		unsigned char memory_[BUFFER_SIZE];
		Chunk reserved_[MAX_CHUNKS];
		
		chunk_index_t chunks_used_;
	#if ALLOCATOR_KEEP_STATS
		memory_size_t bytes_used_;
	#endif
		chunk_index_t first_chunk_id_;
};


} // namespace wiselib

#endif // FIRST_FIT_ALLOCATOR_H


