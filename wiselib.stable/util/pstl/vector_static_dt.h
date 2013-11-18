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
#ifndef __WISELIB_INTERNAL_INTERFACE_STL_VECTOR_STATIC_DT_H
#define __WISELIB_INTERNAL_INTERFACE_STL_VECTOR_STATIC_DT_H

//#include <external_interface/external_interface.h>
//#include <external_interface/external_interface_testing.h>
#include <string.h>
#include <util/pstl/iterator_base_types.h>

namespace wiselib
{
	
	namespace vector_static__detail {
	
		template<typename OsModel_P>
		class vector_static_base
		{
		public:
			typedef typename OsModel_P::size_t size_type;
			typedef typename OsModel_P::block_data_t block_data_t;
			
			class iterator {
				public:
					iterator(block_data_t* pos, ::uint8_t element_size)
						: pos_(pos), element_size_(element_size) {
					}
					
					iterator& operator++() {
						pos_ += element_size_;
						return *this;
					}
					
					iterator& operator--() {
						pos_ -= element_size_;
						return *this;
					}
					
					iterator operator--(int) {
						iterator old_me = *this;
						pos_ -= element_size_;
						return old_me;
					}
					
					size_type operator-(const iterator& other) {
						return (pos_ - other.pos_) / element_size_;
					}
					
					bool operator==(const iterator& other) const { return pos_ == other.pos_; }
					bool operator!=(const iterator& other) const { return pos_ != other.pos_; }
					
				protected:
					block_data_t* pos_;
					::uint8_t element_size_;
				
				template<typename OsModel_>
				friend class vector_static_base;
			};
			
			vector_static_base(size_type vector_size, ::uint8_t element_size) {
				vector_size_ = vector_size;
				element_size_ = element_size;
				start_ = &vec_[0];
				finish_ = start_;
				end_of_storage_ = start_ + vector_size_ * element_size_;
			}
			
			vector_static_base( const vector_static_base& vec ) {
				*this = vec;
			}
			
			///@name Iterators
			///@{
			iterator begin() { return iterator(start_, element_size_); }
			iterator end() { return iterator(finish_, element_size_); }
			const iterator begin() const { return iterator(start_, element_size_); }
			const iterator end() const { return iterator(finish_, element_size_); }
			
			///@}
			
			///@name Capacity
			///@{
			size_type size() const {
				return (finish_ - start_) / element_size_;
			}
			
			size_type max_size() const {
				return vector_size_;
			}
			
			size_type capacity() const {
				return vector_size_;
			}
			
			bool empty() const {
				return finish_ == start_;
			}
			
			bool full() const {
				return finish_ == end_of_storage_;
			}
			
			///@}
			
			block_data_t* at(size_type n) {
				return start_ + n * element_size_;
			}
			
			void push_back(block_data_t* d) {
				if(!full()) {
					memcpy(finish_, d, element_size_);
					finish_ += element_size_;
				}
			}
			
			void pop_back() {
				if(!empty()) {
					finish_ -= element_size_;
				}
			}
			
			iterator insert(block_data_t *d) {
				return insert(end(), d);
			}
			
			iterator insert(iterator position, block_data_t* d) {
				if(full()) {
					return end();
				}
				
				for(block_data_t* p = finish_ - element_size_; p >= position.pos_; p -= element_size_) {
					memcpy(p + element_size_, p, element_size_);
				}
				
				memcpy(position.pos_, d, element_size_);
				finish_ += element_size_;
				
				return position;
			}
			
			iterator erase(iterator position) {
				if(position == end()) {
					return end();
				}
				
				for(block_data_t* p = position.pos_; p < finish_; p += element_size_) {
					memcpy(p, p + element_size_, element_size_);
				}
				
				pop_back();

				return position;
			}
			
			void clear() {
				finish_ = start_;
			}
			
			iterator find(block_data_t *d) {
				for(iterator it = begin(); it != end(); ++it) {
					if(memcmp(d, it.pos_, element_size_) == 0) {
						return it;
					}
				}
				return end();
			}

			///@}

		protected:
			block_data_t *start_, *finish_, *end_of_storage_;
			size_type element_size_;
			size_type vector_size_;
			
			block_data_t vec_[0];
		};
		
	} // namespace vector_static__detail

	/**
	 * Codesize-saving implementation of vector_static
	 * (savings kick in when you use multiple instances).
	 * 
	 * The basic idea is to only provide a templated frontend and have all the
	 * actual code in a base class that has no more than the OsModel as
	 * template parameter. This way we avoid multiple instantiations of the
	 * same code.
	 */
	template<typename OsModel_P,
				typename Value_P,
				int VECTOR_SIZE>
	class vector_static_dt : public vector_static__detail::vector_static_base<OsModel_P> {
		typedef vector_static__detail::vector_static_base<OsModel_P> base; 
	public:
		typedef Value_P value_type;
		typedef value_type* pointer;
		typedef const value_type* const_pointer;
		typedef value_type& reference;
		typedef const value_type& const_reference;
		typedef vector_static_dt<OsModel_P, value_type, VECTOR_SIZE> vector_type;
		typedef typename OsModel_P::size_t size_type;
		typedef typename OsModel_P::block_data_t block_data_t;
		
		class iterator : public vector_static__detail::vector_static_base<OsModel_P>::iterator {
				typedef typename vector_static__detail::vector_static_base<OsModel_P>::iterator iterator_base;
			public:
				typedef typename vector_type::value_type value_type;
				typedef random_access_iterator_tag iterator_category;
				typedef size_type difference_type;
				typedef value_type* pointer;
				typedef value_type& reference;
				
				iterator() : iterator_base(0, VECTOR_SIZE) {
				}
				
				iterator(block_data_t* pos) : iterator_base(pos, VECTOR_SIZE) {
				}
				
				iterator(const iterator_base& other) : iterator_base(other) {
				}
				
				iterator& operator++() {
					iterator_base::operator++();
					return *this;
				}
				
				bool operator==(const iterator& other) const {
					return iterator_base::operator==(other);
				}
				
				bool operator!=(const iterator& other) const {
					return !(*this == other);
				}
				
				value_type& operator*() {
					return *reinterpret_cast<value_type*>(iterator_base::pos_);
				}
				
				value_type* operator->() {
					return reinterpret_cast<value_type*>(iterator_base::pos_);
				}
				
				value_type& operator[](size_type idx) const {
					return *reinterpret_cast<value_type*>(iterator_base::pos_ + idx * sizeof(value_type));
				}
		};
		
		vector_static_dt()
			: base(VECTOR_SIZE, sizeof(value_type)) {
		}
		
		iterator begin() { return iterator(base::begin()); }
		iterator end() { return iterator(base::end()); }
		
		const iterator begin() const { return iterator(base::begin()); }
		const iterator end() const { return iterator(base::end()); }
		
		value_type& operator[](size_type idx) {
			return *reinterpret_cast<value_type*>(base::at(idx));
		}
		
		void push_back(value_type& v) {
			base::push_back(reinterpret_cast<block_data_t*>(&v));
		}
		
		iterator insert(value_type& v) {
			return iterator(base::insert(reinterpret_cast<block_data_t*>(&v)));
		}
		
		iterator insert(iterator it, value_type& v) {
			return iterator(base::insert(it, reinterpret_cast<block_data_t*>(&v)));
		}
		
		iterator erase(iterator position) {
			return iterator(base::erase(position));
		}
		
		iterator find(value_type& v) {
			return iterator(base::find(reinterpret_cast<block_data_t*>(&v)));
		}
		
	protected:
		block_data_t v_[VECTOR_SIZE * sizeof(value_type)];
	};

}

#endif

/* vim: set ts=4 sw=4 tw=78 noexpandtab :*/

