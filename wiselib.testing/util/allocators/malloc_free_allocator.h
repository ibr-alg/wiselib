
#ifndef __WISELIB_UTIL_ALLOCATORS_MALLOC_FREE_ALLOCATOR_H
#define __WISELIB_UTIL_ALLOCATORS_MALLOC_FREE_ALLOCATOR_H

#define KEEP_STATS 0

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
class MallocFreeAllocator {
	public:
		typedef OsModel_P OsModel;
		typedef MallocFreeAllocator<OsModel_P> self_type;
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
                                pointer_t operator + (size_t i){return pointer_t(p_+i);}
				
				// Only for allocator-internal use! (we need to make this
				// public for operator new to work)
				T* raw() { return p_; }
				const T* raw() const { return p_; }
			protected:
				T* p_;
				
			friend class MallocFreeAllocator<OsModel_P>;
		};
		
		template<typename T>
		struct array_pointer_t : public pointer_t<T> {
			public:
				array_pointer_t() : pointer_t<T>(0), elements_(0) { }
				array_pointer_t(T* p) : pointer_t<T>(p), elements_(1) { }
				array_pointer_t(T* p, size_t e) : pointer_t<T>(p), elements_(e) {
				}
				array_pointer_t(const array_pointer_t& other) : pointer_t<T>(other.p_), elements_(other.elements_) {
				}
				array_pointer_t& operator=(const array_pointer_t& other) {
					this->p_ = other.p_;
					elements_ = other.elements_;
					return *this;
				}
				array_pointer_t& operator++() { ++this->p_; --elements_; return *this; }
				array_pointer_t& operator--() { --this->p_; ++elements_; return *this; }
				
			private:
				size_t elements_;
		};
		
		MallocFreeAllocator()
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
			#ifdef ISENSE
				void *p = isense::malloc(sizeof(T));
			#else
				void *p = malloc(sizeof(T));
			#endif
			pointer_t<T> r((T*)p);
			new(r) T;
			return r;
		}
		
		template<typename T>
		array_pointer_t<T> allocate_array(typename OsModel::size_t n) {
			#if KEEP_STATS
				news_++;
			#endif
			#ifdef ISENSE
				array_pointer_t<T> r((T*)isense::malloc(sizeof(T) * n), n);
			#else
				array_pointer_t<T> r((T*)malloc(sizeof(T) * n), n);
			#endif
			for(typename OsModel::size_t i = 0; i < n; i++) {
				new(pointer_t<T>((T*)r.raw() + sizeof(T) * i)) T;
			}
			return r;
		}
		
		template<typename T>
		int free(pointer_t<T> p) {
			#if KEEP_STATS
				deletes_++;
			#endif
			p->~T();
			#ifdef ISENSE
				isense::free((void*)p.p_);
			#else
				::free((void*)p.p_);
			#endif
			return SUCCESS;
		}
		
		template<typename T>
		int free_array(array_pointer_t<T> p) {
			#if KEEP_STATS
				deletes_++;
			#endif
			#pragma warning("array freeing does not call destructors yet!!");
			/*for(typename OsModel::size_t i = 0; i < n; i++) {
				pointer_t<T>((T*)r.raw() + sizeof(T) * i)->~T();
			}*/
			#ifdef ISENSE
				isense::free((void*)p.p_);
			#else
				::free((void*)p.p_);
			#endif
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

#endif // MALLOC_FREE_ALLOCATOR_H


