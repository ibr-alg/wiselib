
#ifndef __WISELIB_ALGORITHMS_SE_CONSTRUCTION_MESSAGE_H
#define __WISELIB_ALGORITHMS_SE_CONSTRUCTION_MESSAGE_H

namespace wiselib {
	
	
	template<
		typename Radio_P,
		typename ClassInfo_P
	>
	class SEConstructionMessage {
		public:
			typedef Radio_P Radio;
			typedef typename Radio::size_t size_t;
			typedef typename Radio::block_data_t block_data_t;
			typedef typename Radio::node_id_t node_id_t;
			typedef ClassInfo_P ClassInfo;
			
			enum MessageType {
				MSG_STATE,
			};
			
			class iterator {
				public:
					iterator() { buffer_ = 0; }
					iterator(block_data_t* buffer) { buffer_ = buffer; }
					iterator(const iterator& other) { buffer_ = other.buffer_; }
					iterator& operator=(const iterator& other) {
						buffer_ = other.buffer_; return *this;
					}
					iterator& operator++() {
						buffer_ += *((size_t*)buffer_) + sizeof(size_t);
						return *this;
					}
					bool operator!=(const iterator& other) { return buffer_ != other.buffer_; }
					block_data_t* operator*() { return buffer_ + sizeof(size_t); }
					size_t size() { return *((size_t*)buffer_); }
					
					block_data_t *buffer_;
				private:
			}; // class iterator
			
			SEConstructionMessage() {
				size_ = sizeof(MessageType);
			}
			
			SEConstructionMessage(size_t size, block_data_t* data) {
				//size_ = sizeof(MessageType);
				for(size_=0; size_<size;) {
					buffer_[size_++] = *(data++);
				}
			}
			
			iterator begin() { return iterator(buffer_ + sizeof(MessageType)); }
			iterator end() { return iterator(buffer_ + size_); }
			
			MessageType type() {
				return ((MessageType*)buffer_)[0];
			}
			void setType(MessageType t) {
				((MessageType*)buffer_)[0] = t;
			}
			
			void pushClassInfo(ClassInfo& classinfo) {
				size_t *size_info = (size_t*)(buffer_ + size_);
				size_ += sizeof(size_t);
				
				// TODO: endianness!!
				*size_info = classinfo.write_to(buffer_ + size_);
				size_ += *size_info;
			}
			
			/*
			void addClass(size_t size, block_data_t *data, node_id_t leader) {
				// TODO: Use serialization here for platform independence!
				
				assert(type() == MSG_ADVERTISE || type() == MSG_CONSTRUCTED);
				
				*((size_t*)(buffer_ + size_)) = size + sizeof(size_t) + sizeof(node_id_t);
				
				*((node_id_t*)(buffer_ + size_)) = leader;
				for(size_t i=0; i<size; ++i) {
					buffer_[size_ + i] = *(data++);
				}
				
				size_ += size + sizeof(size_t) + sizeof(node_id_t);
			}*/
			
			size_t size() { return size_; }
			block_data_t* data() { return buffer_; }
			
		private:
			MessageType type_;
			size_t size_;
			block_data_t buffer_[Radio::MAX_MESSAGE_LENGTH];
	};
} // namespace

#endif // __WISELIB_ALGORITHMS_SE_CONSTRUCTION_MESSAGE_H

