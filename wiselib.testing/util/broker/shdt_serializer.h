
#ifndef SHDT_SERIALIZER_H
#define SHDT_SERIALIZER_H

#include <external_interface/external_interface.h>
#include <util/serialization/serialization.h>
#include <util/meta.h>
#include <util/pstl/string_utils.h>
#include <util/pstl/algorithm.h>

namespace wiselib {
	
	/**
	 * ----
	 *  ShdtSerializer<...> shdt;
	 *  bool call_again = false;
	 *  do {
	 *    shdt.write_header(buffer, buffer_size, call_again, ...);
	 *    if(call_again) {
	 *      send_buffer(...);
	 *      continue;
	 *    }
	 *  } while(false);
	 *  
	 *  
	 *  
	 *  ShdtSerializer<...> shdt;
	 *  Writer w = shdt.make_writer(buffer_size, buffer, &send_buffer);
	 *  w.write_header(...);
	 *  w.write_field(...);
	 *  
	 *  void send_buffer(Writer& w) {
	 *    radio_->send(w.buffer_used(), w.buffer());
	 *    w.set_buffer(...); // or leave out to reuse same buffer
	 *    w.set_buffer_used(0);
	 *    w.set_buffer_size(BUF_SIZE); // or leave out to keep buffer size
	 *  }
	 * ----
	 */
	template<
		typename OsModel_P,
		size_t MAX_TABLE_SIZE_P,
		size_t MAX_TUPLE_SIZE_P = 4
	>
	class ShdtSerializer {
		public:
			// {{{ Typedefs & Enums
			
			typedef OsModel_P OsModel;
			enum { MAX_TABLE_SIZE = MAX_TABLE_SIZE_P };
			enum { MAX_TUPLE_SIZE = MAX_TUPLE_SIZE_P };
			
			typedef typename OsModel::size_t size_type;
			typedef typename OsModel::block_data_t block_data_t;
			typedef ::uint8_t instruction_t;
			typedef ::uint8_t field_id_t;
			typedef ::uint8_t sz_t;
			typedef typename SmallUint<MAX_TABLE_SIZE + 1>::t table_id_t;
			
			enum { npos = (size_type)(-1) };
			enum { nidx = (table_id_t)(-1) };
			enum Commands {
				CMD_TUPLE = 't', CMD_HEADER = 'h', CMD_VALUE = 'v', CMD_CAT = 'c',
				CMD_TABLE_VALUE = 'V', CMD_INSERT = 'i', CMD_END = 0xff,
			};
			enum { SUCCESS = OsModel::SUCCESS, ERR_UNSPEC = OsModel::ERR_UNSPEC };
			enum SpecialIds {
				NO_FIELD_ID = (field_id_t)(-1)
			};
			enum { SHDT_VERSION = 1 };
			
			// }}}
			
		private:
			class Instruction {
				// {{{{
				public:
					Instruction() : data_(0) {
						memset(data_idx_, (table_id_t)(nidx), MAX_TUPLE_SIZE);
					}
					
					static size_type header_size(instruction_t in) {
						switch(in) {
							case CMD_HEADER: return 4 * sizeof(table_id_t) + sizeof(instruction_t);
							case CMD_INSERT: return 2 * sizeof(table_id_t) + sizeof(instruction_t) + sizeof(sz_t);
							case CMD_CAT: return 3 * sizeof(table_id_t) + sizeof(instruction_t) + 2 * sizeof(sz_t);
							case CMD_VALUE: return sizeof(table_id_t) + sizeof(instruction_t) + sizeof(field_id_t) + sizeof(sz_t);
							case CMD_TABLE_VALUE: return 2 * sizeof(table_id_t) + sizeof(instruction_t) + sizeof(field_id_t);
							default: assert(false);
						}
					}
					
					size_type header_size() { return header_size(instruction_); }
					
					instruction_t& instruction() { return instruction_; }
					table_id_t& table_size() { return data_idx_[0]; }
					table_id_t& tuple_size() { return data_idx_[1]; }
					table_id_t& index(size_type i = 0) { return data_idx_[i]; }
					table_id_t& source_index() { return data_idx_[0]; }
					table_id_t& field_index() { return data_idx_[0]; }
					table_id_t& target_index() { return data_idx_[1]; }
					block_data_t*& data() { return data_; }
					sz_t& data_size() { return data_size_; }
					field_id_t& field_id() { return field_id_; }
					sz_t& field_size() { return data_size_2_; }
					sz_t& source_prefix() { return data_size_2_; }
					
				private:
					block_data_t *data_;
					instruction_t instruction_;
					table_id_t data_idx_[MAX_TUPLE_SIZE];
					sz_t data_size_;
					sz_t data_size_2_;
					field_id_t field_id_;
					
				template<
					typename OsModel_,
					size_t TABLE_SIZE_,
					size_t TUPLE_SIZE_
				>
				friend class ShdtSerializer;
				// }}}
			};
			
		public:
			
			class Writer {
				// {{{
				public:
					typedef delegate1<void, Writer&> write_callback_t;
					
					Writer(ShdtSerializer *serializer, block_data_t* buffer, size_type buffer_size, write_callback_t write_callback) :
						serializer_(serializer), buffer_start_(buffer), buffer_current_(buffer), buffer_end_(buffer + buffer_size),
						write_callback_(write_callback) {
					}
					
					void write_header(table_id_t table_size, table_id_t tuple_size) {
						serializer_->set_table_size(table_size);
						serializer_->set_tuple_size(tuple_size);
						
						Instruction in;
						in.instruction() = CMD_HEADER;
						in.table_size() = table_size;
						in.tuple_size() = tuple_size;
						
						write_instruction(in);
					}
					
					table_id_t write_data(block_data_t* data, size_type data_size, size_type n_avoid = 0, table_id_t* avoid = 0) {
						bool call_again = false;
						table_id_t pos;
						do {
							call_again = serializer_->write_data(
									data, data_size,
									buffer_current_, buffer_end_ - buffer_current_,
									pos, n_avoid, avoid);
							if(call_again) { write_callback_(*this); }
						}
						while(call_again);
						return pos;
					}
					
					template<typename Tuple>
					void write_tuple(Tuple& t) {
						table_id_t ids_[serializer_->tuple_size()];
						for(size_type i = 0; i < serializer_.tuple_size(); ++i) {
							ids_[i] = write_data(t.get(i), t.length(i), i, ids_);
						}
					}
					
					void write_field(field_id_t field_id, block_data_t* data, size_type data_size) {
						if(Instruction::header_size(CMD_VALUE) + data_size < buffer_space()) {
							Instruction in;
							in.instruction() = CMD_VALUE;
							in.field_id() = field_id;
							in.field_size() = data_size;
							in.data() = data;
							write_instruction(in);
						}
						else {
							// do a table insert (as there we can split it up)
							table_id_t id = write_data(data, data_size);
							
							Instruction in;
							in.instruction() = CMD_TABLE_VALUE;
							in.field_id() = field_id;
							in.field_index() = id;
							write_instruction(in);
						}
					}
					
					void flush() {
						if(buffer_current_ > buffer_start_) {
							write_callback_(*this);
						}
					}
					
					block_data_t* buffer() { return buffer_start_; }
					size_type buffer_used() { return buffer_current_ - buffer_start_; }
					size_type buffer_space() { return buffer_end_ - buffer_current_; }
					
					void reuse_buffer(size_type offset = 0) { buffer_current_ = buffer_start_; }
					
				private:
					void write_instruction(Instruction& in) {
						bool call_again = false;
						do {
							call_again = serializer_->write_instruction(in, buffer_current_, buffer_end_);
							if(call_again) { write_callback_(*this); }
						}
						while(call_again);
					}
					
					ShdtSerializer *serializer_;
					block_data_t *buffer_start_;
					block_data_t *buffer_current_;
					block_data_t *buffer_end_;
					write_callback_t write_callback_;
				// }}}
			};
			
			class Reader {
				// {{{
				public:
					
					Reader(ShdtSerializer *serializer, block_data_t* buffer, size_type buffer_size) :
						serializer_(serializer), buffer_start_(buffer), buffer_current_(buffer), buffer_end_(buffer + buffer_size) {
					}
					
					bool read_field(field_id_t& field_id, block_data_t*& data, size_type& data_size) {
						Instruction in = serializer_->read_instruction(buffer_current_, buffer_end_);
						while(buffer_current_ < buffer_end_ && in.instruction() != CMD_VALUE && in.instruction() != CMD_TABLE_VALUE && in.instruction() != CMD_END) {
							serializer_->process_instruction(in);
							in = serializer_->read_instruction(buffer_current_, buffer_end_);
						}
						
						if(in.instruction() == CMD_VALUE) {
							field_id = in.field_id();
							data = in.data();
							data_size = in.field_size();
							return true;
						}
						else if(in.instruction() == CMD_TABLE_VALUE) {
							field_id = in.field_id();
							data = serializer_->get_table(in.field_index(), data_size);
							return true;
						}
						
						serializer_->process_instruction(in);
						data = 0;
						data_size = 0;
						return false;
					}
					
					bool done() {
						return buffer_current_ < buffer_end_;
					}
					
				private:
					ShdtSerializer *serializer_;
					block_data_t *buffer_start_;
					block_data_t *buffer_current_;
					block_data_t *buffer_end_;
				// }}}
			};
			
			typedef delegate1<void, Writer&> write_callback_t;
			
			ShdtSerializer() {
				memset((void*)lookup_table_, 0, sizeof(lookup_table_));
			}
			
			~ShdtSerializer() {
				reset();
			}
			
			/**
			 * Reset internal state.
			 * For SHDT this means clear the lookup table,
			 * such that subsequent fill_buffer() or read_buffer() calls
			 * will not use any table entries created in former calls.
			 * 
			 * Useful if you want to reuse the same serializer instance for an
			 * unrelated communication in which the other side starts from
			 * scratch.
			 */
			void reset() {
				for(size_type i=0; i<MAX_TABLE_SIZE; i++) {
					if(lookup_table_[i]) {
						get_allocator().free(lookup_table_[i]);
					}
				}
				memset((void*)lookup_table_, 0, sizeof(lookup_table_));
			}
			
			table_id_t table_size() { return table_size_; }
			void set_table_size(table_id_t ts) { table_size_ = ts; }
			
			table_id_t tuple_size() { return tuple_size_; }
			void set_tuple_size(table_id_t ts) { tuple_size_ = ts; }
			
			/**
			 * @return true iff this method should be called again with a
			 * fresh buffer.
			 */
			bool write_data(block_data_t* data, size_type data_size, block_data_t*& buffer, size_type buffer_size,
					table_id_t& id, size_type n_avoid = 0, table_id_t* avoid = 0) {
				bool call_again = false;
				
				// how much of our string is already there?
				id = hash(data, data_size);
				size_type p = 0;
				size_type current_size = 0;
				block_data_t *current = get_table(id, current_size);
				if(current) {
					p = prefix_length_n(min(data_size, current_size), current, data);
				}
				// does the rest fit?
				
				// header size of the cat command
				size_t l = data_size; //strlen((char*)in.data());
				if(p && buffer_size > Instruction::header_size(CMD_CAT)) {
					// cat
					size_t copy = buffer_size - Instruction::header_size(CMD_CAT);
					if(copy + p > l) { copy = l - p; }
					
					if(copy) {
						Instruction in;
						in.instruction() = CMD_CAT;
						in.source_index() = id;
						in.source_prefix() = p;
						in.target_index() = id;
						in.data_size() = copy;
						in.data() = data + p;
						
						bool ca = write_instruction(in, buffer, buffer + buffer_size);
						if(!ca) {
							set_table(id, data, p + copy);
						}
						else {
							assert(false && "write can instruction failed!");
						}
						
						call_again = ca || ((p + copy) < l);
					}
					else {
						// string is conveniently already completely there
						call_again = false;
					}
				}
				else if(buffer_size > Instruction::header_size(CMD_INSERT)) {
					// insert
					size_t copy = buffer_size - Instruction::header_size(CMD_INSERT);
					if(copy > l) { copy = l; }
					
					Instruction in;
					in.instruction() = CMD_INSERT;
					in.index() = id;
					in.data_size() = copy;
					in.data() = data;
					
					bool ca = write_instruction(in, buffer, buffer + buffer_size);
					if(!ca) {
						set_table(id, data, copy);
						call_again = (copy < l);
					}
					else {
						assert(false && "write insert instruction failed!");
					}
					
				}
				else {
					call_again = true;
				}
				
				return call_again;
			}
			
			bool write_field(field_id_t field_id, block_data_t* data, size_type data_size, block_data_t*& buffer, block_data_t* buffer_end) {
				
				size_type bufsiz = buffer_end - buffer;
				if(Instruction::header_size(CMD_VALUE) + data_size < bufsiz) {
					Instruction in;
					in.instruction() = CMD_VALUE;
					in.field_id() = field_id;
					in.field_size() = data_size;
					in.data() = data;
					return write_instruction(in, buffer, buffer_end);
				}
				else {
					// do a table insert (as there we can split it up)
					table_id_t id;
					bool call_again = write_data(data, data_size, buffer, bufsiz, id);
					if(call_again) { return true; }
					
					Instruction in;
					in.instruction() = CMD_TABLE_VALUE;
					in.field_id() = field_id;
					in.field_index() = id;
					return write_instruction(in, buffer, buffer_end);
				}
			}
			
			/**
			 * @return true iff this method should be called again with a
			 * fresh buffer.
			 */
			bool write_instruction(Instruction& in, block_data_t*& buffer, block_data_t* buffer_end) {
				bool call_again = false;
				
				// TODO: check fit
				if(buffer + in.header_size() >= buffer_end) {
					return true;
				}
				
				switch(in.instruction()) {
					case CMD_HEADER:
						write(buffer, buffer_end, (table_id_t)nidx);
						write(buffer, buffer_end, (instruction_t)CMD_HEADER);
						write(buffer, buffer_end, (table_id_t)SHDT_VERSION);
						write(buffer, buffer_end, in.table_size());
						write(buffer, buffer_end, in.tuple_size());
						break;
						
					case CMD_INSERT: {
						write(buffer, buffer_end, (table_id_t)nidx);
						write(buffer, buffer_end, (instruction_t)CMD_INSERT);
						write(buffer, buffer_end, in.index());
						write(buffer, buffer_end, in.data_size());
						memcpy(buffer, in.data(), in.data_size());
						buffer += in.data_size();
						break;
					}
					
					case CMD_CAT: {
						write(buffer, buffer_end, (table_id_t)nidx);
						write(buffer, buffer_end, (instruction_t)CMD_CAT);
						write(buffer, buffer_end, in.source_index()); // take prefix from here
						write(buffer, buffer_end, in.source_prefix());
						write(buffer, buffer_end, in.target_index()); // write result here
						write(buffer, buffer_end, in.data_size());
						memcpy(buffer, in.data(), in.data_size());
						buffer += in.data_size();
						break;
					}
						
					case CMD_VALUE:
						write(buffer, buffer_end, (table_id_t)nidx);
						write(buffer, buffer_end, (instruction_t)CMD_VALUE);
						write(buffer, buffer_end, in.field_id());
						write(buffer, buffer_end, in.field_size());
						memcpy(buffer, in.data(), in.field_size());
						buffer += in.field_size();
						break;
					
					case CMD_TABLE_VALUE:
						write(buffer, buffer_end, (table_id_t)nidx);
						write(buffer, buffer_end, (instruction_t)CMD_TABLE_VALUE);
						write(buffer, buffer_end, in.field_id());
						write(buffer, buffer_end, in.field_index());
						break;
						
					case CMD_TUPLE:
						break;
				}
				
				return call_again;
			}
			
			/**
			 */
			Instruction read_instruction(block_data_t*& buffer, block_data_t* buffer_end) {
				Instruction in;
				
				read(buffer, buffer_end, in.index(0));
				if(in.index(0) != nidx) {
					// tuple mode
					in.instruction() = CMD_TUPLE;
					for(size_type i = 1; i < tuple_size_; i++) {
						read(buffer, buffer_end, in.index(i));
					}
				}
				else {
					read(buffer, buffer_end, in.instruction());
					switch(in.instruction()) {
						case CMD_HEADER: {
							table_id_t version;
							read(buffer, buffer_end, version);
							read(buffer, buffer_end, in.table_size());
							read(buffer, buffer_end, in.tuple_size());
							break;
						}
							
						case CMD_INSERT:
							read(buffer, buffer_end, in.index());
							read(buffer, buffer_end, in.data_size());
							in.data() = buffer;
							buffer += in.data_size();
							break;
							
						case CMD_VALUE:
							read(buffer, buffer_end, in.field_id());
							read(buffer, buffer_end, in.field_size());
							in.data() = buffer;
							buffer += in.data_size();
							break;
							
						case CMD_CAT:
							read(buffer, buffer_end, in.source_index());
							read(buffer, buffer_end, in.source_prefix());
							read(buffer, buffer_end, in.target_index());
							read(buffer, buffer_end, in.data_size());
							in.data() = buffer;
							buffer += in.data_size();
							break;
							
						case CMD_TABLE_VALUE:
							read(buffer, buffer_end, in.field_id());
							read(buffer, buffer_end, in.field_index());
							break;
					}
				}
				return in;
			}
			
			/**
			 */
			void process_instruction(Instruction& in) {
				switch(in.instruction()) {
					case CMD_HEADER:
						table_size_ = in.table_size();
						tuple_size_ = in.tuple_size();
						break;
						
					case CMD_INSERT:
						set_table(in.index(), in.data(), in.data_size());
						break;
						
					case CMD_VALUE:
					case CMD_TABLE_VALUE:
					case CMD_END:
					case CMD_TUPLE:
						assert(false && "dunno how to process this");
						break;
						
					case CMD_CAT: {
						size_type current_size;
						block_data_t *current = get_table(in.source_index(), current_size);
						size_type l = current_size + in.data_size();
						block_data_t buf[l];
						memcpy(buf, current, current_size);
						memcpy(buf + current_size, in.data(), in.data_size());
						set_table(in.target_index(), buf, l);
						break;
					}
					
				}
			}
			
		private:
			
			void set_table(table_id_t id, block_data_t* data, sz_t data_size) {
				if(lookup_table_[id]) {
					get_allocator().free_array(lookup_table_[id]);
					lookup_table_[id] = 0;
				}
				block_data_t* d = get_allocator().allocate_array<block_data_t>(data_size + sizeof(sz_t)).raw();
				lookup_table_[id] = d;
				
				wiselib::write<OsModel>(d, data_size);
				memcpy(d + sizeof(sz_t), data, data_size);
				
				
				DBG("[%d] := %s (%d)", id, (char*)(d + sizeof(sz_t)), (int)d[0]);
			}
			
			block_data_t* get_table(table_id_t id, size_type& sz) {
				sz_t s;
				block_data_t *r = get_table(id, s);
				sz = s;
				return r;
			}
				
			block_data_t* get_table(table_id_t id, sz_t& sz) {
				if(!lookup_table_[id]) {
					sz = 0;
					return 0;
				}
				wiselib::read<OsModel>(lookup_table_[id], sz);
				return lookup_table_[id] + sizeof(sz_t);
			}
			
			template<typename T>
			bool read(block_data_t*& buffer, block_data_t* buffer_end, T& tgt) {
				if(buffer + sizeof(T) > buffer_end) {
					return false;
				}
				wiselib::read<OsModel, block_data_t, T>(buffer, tgt);
				buffer += sizeof(T);
				return true;
			}
			
			template<typename T>
			bool write(block_data_t*& buffer, block_data_t* buffer_end, const T& tgt) {
				if(buffer + sizeof(T) >= buffer_end) {
					return false;
				}
				wiselib::write<OsModel, block_data_t, T>(buffer, const_cast<T&>(tgt));
				buffer += sizeof(T);
				return true;
			}
			
			
			table_id_t hash(const block_data_t *s, size_type sz) {
				//return (s[0] ^ s[1]) % TABLE_SIZE;
				//return reinterpret_cast<Uint<sizeof(block_data_t*)>::t>(s) % TABLE_SIZE;
				
				// stolen from STL, slightly modified
				// source: http://www.fantasy-coders.de/projects/gh/html/x435.html
				table_id_t r = 0;
				//for(size_type i=0; s[i] [>&& i<12<]; i++) {
				for(size_type i=0; i < sz; i++) {
					r = (5*r + s[i]) % table_size_;
				}
				return r % table_size_;
			}
			
			block_data_t *lookup_table_[MAX_TABLE_SIZE];
			table_id_t table_size_;
			table_id_t tuple_size_;
	};
	
} // namespace wiselib

#endif // SHDT_SERIALIZER_H

