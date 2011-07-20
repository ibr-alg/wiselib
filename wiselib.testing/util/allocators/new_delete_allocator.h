
#ifndef __WISELIB_UTIL_ALLOCATORS_NEW_DELETE_ALLOCATOR_H
#define __WISELIB_UTIL_ALLOCATORS_NEW_DELETE_ALLOCATOR_H

#define KEEP_STATS 1
#define ALLOCATOR_LOG 1

template<typename pointer_t>
void* operator new(size_t size, pointer_t ptr) {
	return ptr.raw();
}

namespace wiselib {

/**
 * Simple new/delete allocator.
 * Uses the new/delete operators from the underlying OS.
 * 
 * @ingroup Allocator_concept
 */
template<
	typename OsModel_P
>
class NewDeleteAllocator {
	public:
		typedef OsModel_P OsModel;
		typedef NewDeleteAllocator<OsModel_P> self_type;
		typedef self_type* self_pointer_t;
		typedef typename OsModel::size_t size_t;
		typedef typename OsModel::block_data_t block_data_t;
		
		enum { SUCCESS = OsModel::SUCCESS, ERR_UNSPEC = OsModel::ERR_UNSPEC };
		
		template<typename T>
		struct pointer_t {
			public:
				pointer_t() : p_(0) { }
				pointer_t(T* p) : p_(p) { }
				pointer_t(const pointer_t& other) : p_(other.p_) { }
				pointer_t& operator=(const pointer_t& other) { p_ = other.p_; return *this; }
				T& operator*() const { return *p_; }
				T* operator->() const { return p_; }
				T& operator[](size_t idx) { return p_[idx]; }
				const T& operator[](size_t idx) const { return p_[idx]; }
				bool operator==(const pointer_t& other) const { return p_ == other.p_; }
				bool operator!=(const pointer_t& other) const { return p_ != other.p_; }
				operator bool() const { return p_ != 0; }
				pointer_t& operator++() { ++p_; return *this; }
				pointer_t& operator--() { --p_; return *this; }
				
				// Only for allocator-internal use! (we need to make this
				// public for operator new to work)
				T* raw() { return p_; }
				const T* raw() const { return p_; }
			private:
				T* p_;
				
			friend class NewDeleteAllocator<OsModel_P>;
		};
		
		NewDeleteAllocator()
			#if KEEP_STATS
				: news_(0), deletes_(0)
			#endif
		{
		}
		
		template<typename T>
		pointer_t<T> allocate() {
			#if KEEP_STATS
				news_++;
			#endif
			pointer_t<T> r(new T);
			return r;
		}
		
		template<typename T>
		pointer_t<T> allocate_array(typename OsModel::size_t n) {
			#if KEEP_STATS
				news_++;
			#endif
			pointer_t<T> r = new T[n];
			return r;
		}
		
		template<typename T>
		int free(pointer_t<T> p) {
			#if KEEP_STATS
				deletes_++;
			#endif
			delete p.p_;
			return SUCCESS;
		}
		
		template<typename T>
		int free_array(pointer_t<T> p) {
			#if KEEP_STATS
				deletes_++;
			#endif
			delete[] p.p_;
			return SUCCESS;
		}
		
		size_t size() { return 0; }
		size_t capacity() { return (size_t)-1; }
			
			
		template<typename Debug_P>
		void print_stats(Debug_P* d) {
			#if KEEP_STATS
			d->debug("\nnew/delete allocator statistics\n");
			d->debug("-------------------------------\n");
			d->debug("allocations    : %14lu\n", news_);
			d->debug("frees          : %14lu\n", deletes_);
			d->debug("\n");
			#endif
		}
	
	private:
		#if KEEP_STATS
		unsigned long news_, deletes_;
		#endif
};


} // namespace wiselib

#endif // NEW_DELETE_ALLOCATOR_H


