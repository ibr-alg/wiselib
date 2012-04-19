
#ifndef BUFFER_DYNAMIC_H
#define BUFFER_DYNAMIC_H

#include "util/pstl/vector_dynamic.h"

namespace wiselib {
	namespace protobuf {
	
	template<
		typename OsModel_P,
		typename Allocator_P
	>
	class buffer_dynamic {
		public:
			typedef OsModel_P OsModel;
			typedef typename OsModel::block_data_t block_data_t;
			typedef typename OsModel::size_t size_t;
			typedef Allocator_P Allocator;
			typedef buffer_dynamic<OsModel, Allocator> self_type;
			typedef vector_dynamic<OsModel, block_data_t, Allocator> vector_t;
			
			buffer_dynamic() : /*virgin_(true),*/ pos_(0) {
			}
			
			buffer_dynamic(typename Allocator::self_pointer_t alloc) : /*virgin_(true),*/ pos_(0) {
				data_.set_allocator(alloc);
			}
			
			void set_allocator(typename Allocator::self_pointer_t alloc) {
				data_.set_allocator(alloc);
			}
			
			self_type& operator++() {
				pos_++;
				return *this;
			}
			
			block_data_t& operator*() {
				while(pos_ >= data_.size()) {
					//virgin_ = false;
					data_.push_back(0);
				}
				return data_[pos_];
			}
			
			//bool virgin() { return virgin_; }
			
			/**
			 * return false iff the next call to operator*() would change the
			 * buffers size
			 */
			bool readonly() {
				return (position() < (size()-1));
			}
			
			size_t position() { return pos_; }
			
			size_t size() { return data_.size(); }
			
			vector_t& vector() { return data_; }
			
			block_data_t* data() {
				return data_.data();
			}
			
		private:
			vector_t data_;
			//bool virgin_;
			size_t pos_;
	};
	
	}
}

#endif // BUFFER_DYNAMIC_H

