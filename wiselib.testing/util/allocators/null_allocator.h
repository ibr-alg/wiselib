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

#ifndef __WISELIB_UTIL_ALLOCATORS_NULL_ALLOCATOR_H
#define __WISELIB_UTIL_ALLOCATORS_NULL_ALLOCATOR_H

namespace wiselib {

/**
 * Dummy allocator that does nothing (useful for codesize tests in which
 * the allocator is not to be considered)
 */	
template<
	typename OsModel_P
>
class NullAllocator {
	public:
		typedef OsModel_P OsModel;
		typedef NullAllocator<OsModel> self_type;
		typedef self_type* self_pointer_t;
		typedef typename OsModel::size_t size_type;
		typedef typename OsModel::size_t size_t;
		typedef typename OsModel::block_data_t block_data_t;
		
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
				
			friend class NullAllocator<OsModel_P>;
		}; // __attribute__((__packed__));
		template<typename T>
		struct array_pointer_t {
			public:
				array_pointer_t() : p_(0) { }
				array_pointer_t(T* p) : p_(p) { }
				array_pointer_t(const array_pointer_t& other) : p_(other.p_) { }
				array_pointer_t& operator=(const array_pointer_t& other) { p_ = other.p_; return *this; }
				T& operator*() const { return *p_; }
				T* operator->() const { return p_; }
				T& operator[](size_t idx) { return p_[idx]; }
				const T& operator[](size_t idx) const { return p_[idx]; }
				bool operator==(const array_pointer_t& other) const { return p_ == other.p_; }
				bool operator!=(const array_pointer_t& other) const { return p_ != other.p_; }
				operator bool() const { return p_ != 0; }
				array_pointer_t& operator++() { ++p_; return *this; }
				array_pointer_t& operator--() { --p_; return *this; }
                array_pointer_t operator + (size_t i){return array_pointer_t(p_+i);}
				
				// Only for allocator-internal use! (we need to make this
				// public for operator new to work)
				T* raw() { return p_; }
				const T* raw() const { return p_; }
			protected:
				T* p_;
				
			friend class NullAllocator<OsModel_P>;
		}; // __attribute__((__packed__));
	
		template<typename T>
		pointer_t<T> allocate() { return pointer_t<T>(); }
		
		template<typename T>
		array_pointer_t<T> allocate_array(size_type n) { return array_pointer_t<T>(); }
		
		template<typename T>
		int free(T* p) { return OsModel::SUCCESS; }
	
		template<typename T>
		int free_array(T* p) { return OsModel::SUCCESS; }
		
		template<typename T>
		int free(pointer_t<T> p) { return OsModel::SUCCESS; }
	
		template<typename T>
		int free_array(array_pointer_t<T> p) { return OsModel::SUCCESS; }
};

} // namespace wiselib

#endif // __WISELIB_UTIL_ALLOCATORS_NULL_ALLOCATOR_H

