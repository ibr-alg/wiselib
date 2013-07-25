
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
			//typedef typename SmallUint<MAX_TABLE_SIZE + 1>::t table_id_t;
			typedef ::uint8_t table_id_t;
			
			enum { npos = (size_type)(-1) };
			enum { nidx = (table_id_t)(-1) };
			enum Commands {
				CMD_TUPLE = 't', CMD_HEADER = 'h', CMD_VALUE = 'v', CMD_CAT = 'c',
				CMD_TABLE_VALUE = 'V', CMD_INSERT = 'i', CMD_END = 0xff
			};
			
			enum {
				MAX_INSTRUCTION_HEADER_SIZE = 6,
				MIN_INSTRUCTION_HEADER_SIZE = 3
			};
			
			enum { SUCCESS = OsModel::SUCCESS, ERR_UNSPEC = OsModel::ERR_UNSPEC };
			enum SpecialIds {
				NO_FIELD_ID = (field_id_t)(-1)
			};
			enum { SHDT_VERSION = 2 };
			
			// }}}
			
		private:
			class Instruction {
				// {{{
				public:
					Instruction() {
						data_[0] = nidx;
						data_[1] = CMD_END;
					}
					
					block_data_t& operator[](size_type i) { return data_[i]; }
					block_data_t& command() { return data_[1]; }
					block_data_t*& payload() { return payload_; }
					block_data_t& payload_size() { return data_[2]; }
					
					bool is_tuple() {
						return data_[0] != nidx;
					}
					
					size_type size() {
						return header_size() + (has_payload() ? payload_size() : 0);
					}
					
					static size_type header_size(block_data_t cmd) {
						switch(cmd) {
							case CMD_HEADER: return HeaderInstruction::header_size();
							case CMD_INSERT: return InsertInstruction::header_size();
							case CMD_CAT: return CatInstruction::header_size();
							case CMD_VALUE: return ValueInstruction::header_size();
							case CMD_TABLE_VALUE: return TableValueInstruction::header_size();
							default: assert(false);
						}
						return 0;
					}
					
					/**
					 * @return -1 if this instruction is actually a tuple, the
					 * header size of the instruction else.
					 */
					int header_size() {
						return (data_[0] == nidx) ? header_size(command()) : -1;
					}
					
					static bool has_payload(block_data_t cmd) {
						return (cmd == CMD_INSERT || cmd == CMD_CAT || cmd == CMD_VALUE);
					}
					bool has_payload() { return !is_tuple() && has_payload(command()); }
					
					table_id_t* tuple() { return reinterpret_cast<table_id_t*>(data_); }
					
				protected:
					block_data_t data_[MAX_INSTRUCTION_HEADER_SIZE];
					block_data_t *payload_;
				// }}}
			};
			
			class HeaderInstruction : public Instruction {
				// {{{
				public:
					HeaderInstruction() {
						this->data_[0] = nidx;
						this->data_[1] = CMD_HEADER;
					}
					
					block_data_t& version() { return this->data_[2]; }
					block_data_t& table_size() { return this->data_[3]; }
					block_data_t& tuple_size() { return this->data_[4]; }
					
					static size_type header_size() { return 5; }
				// }}}
			};
			
			class ValueInstruction : public Instruction {
				// {{{
				public:
					ValueInstruction() {
						this->data_[0] = nidx;
						this->data_[1] = CMD_VALUE;
					}
					
					block_data_t& field_id() { return this->data_[3]; }
					
					static size_type header_size() { return 4; }
				// }}}
			};
			
			class TableValueInstruction : public Instruction {
				// {{{
				public:
					TableValueInstruction() {
						this->data_[0] = nidx;
						this->data_[1] = CMD_TABLE_VALUE;
					}
					
					block_data_t& field_id() { return this->data_[2]; }
					block_data_t& field_index() { return this->data_[3]; }
					
					static size_type header_size() { return 4; }
				// }}}
			};
			
			class CatInstruction : public Instruction {
				// {{{
				public:
					CatInstruction() {
						this->data_[0] = nidx;
						this->data_[1] = CMD_CAT;
					}
					
					block_data_t& source_index() { return this->data_[3]; }
					block_data_t& source_prefix() { return this->data_[4]; }
					block_data_t& target_index() { return this->data_[5]; }
					
					static size_type header_size() { return 6; }
				// }}}
			};
			
			class InsertInstruction : public Instruction {
				// {{{
				public:
					InsertInstruction() {
						this->data_[0] = nidx;
						this->data_[1] = CMD_INSERT;
					}
					
					block_data_t& index() { return this->data_[3]; }
					
					static size_type header_size() { return 4; }
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
						check();
						
						serializer_->set_table_size(table_size);
						serializer_->set_tuple_size(tuple_size);
						HeaderInstruction in;
						in.version() = SHDT_VERSION;
						in.table_size() = table_size;
						in.tuple_size() = tuple_size;
						write_instruction((Instruction&)in);
						
						check();
					}
					
					table_id_t write_data(block_data_t* data, size_type data_size, size_type n_avoid = 0, table_id_t* avoid = 0) {
						check();
						
						bool call_again = false;
						table_id_t pos;
						do {
							assert(buffer_end_ >= buffer_current_);
							call_again = serializer_->write_data(
									data, data_size,
									buffer_current_, buffer_end_ - buffer_current_,
									pos, n_avoid, avoid);
							if(call_again) { write_callback_(*this); }
						}
						while(call_again);
						
						check();
						return pos;
					}
					
					template<typename Tuple>
					void write_tuple(Tuple& t) {
						check();
						//table_id_t ids_[serializer_->tuple_size()];
						Instruction in;
						for(size_type i = 0; i < serializer_->tuple_size(); ++i) {
							in.tuple()[i] = write_data(t.get(i), t.length(i), i, in.tuple());
						}
						write_instruction(in);
						
						check();
					}
					
					
					void write_field(field_id_t field_id, block_data_t* data, size_type data_size) {
						check();
						if(sizeof(ValueInstruction) + data_size < buffer_space()) {
							ValueInstruction in;
							in.field_id() = field_id;
							in.payload_size() = data_size;
							in.payload() = data;
							write_instruction((Instruction&)in);
						}
						else {
							// do a table insert (as there we can split it up)
							table_id_t id = write_data(data, data_size);
							
							TableValueInstruction in;
							in.field_id() = field_id;
							in.field_index() = id;
							write_instruction((Instruction&)in);
						}
						check();
					}
					
					void flush() {
						check();
						if(buffer_current_ > buffer_start_) {
							write_callback_(*this);
						}
						check();
					}
					
					block_data_t* buffer() { return buffer_start_; }
					size_type buffer_used() { return buffer_current_ - buffer_start_; }
					size_type buffer_space() { return buffer_end_ - buffer_current_; }
					
					void reuse_buffer(size_type offset = 0) { buffer_current_ = buffer_start_; }
					
				private:
					
					void check() {
						assert(buffer_end_ >= buffer_current_);
					}
					
					void write_instruction(Instruction& in) {
						bool call_again = false;
						do {
							call_again = serializer_->write_instruction(in, buffer_current_, buffer_end_);
							check();
							
							if(call_again) { write_callback_(*this); }
							check();
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
					
					/**
					 * @return true iff field could be read, false if there
					 * was insufficient data available.
					 */
					bool read_field(field_id_t& field_id, block_data_t*& data, size_type& data_size) {
						Instruction in = serializer_->read_instruction(buffer_current_, buffer_end_);
						while(buffer_current_ < buffer_end_ && in.command() != CMD_VALUE && in.command() != CMD_TABLE_VALUE && in.command() != CMD_END) {
							serializer_->process_instruction(in);
							in = serializer_->read_instruction(buffer_current_, buffer_end_);
						}
						
						if(in.command() == CMD_VALUE) {
							ValueInstruction &v = (ValueInstruction&)in;
							field_id = v.field_id();
							data = v.payload();
							data_size = v.payload_size();
							return true;
						}
						else if(in.command() == CMD_TABLE_VALUE) {
							TableValueInstruction &v = (TableValueInstruction&)in;
							field_id = v.field_id();
							data = serializer_->get_table(v.field_index(), data_size);
							return true;
						}
						
						serializer_->process_instruction(in);
						data = 0;
						data_size = 0;
						return false;
					}
					
					template<typename Tuple>
					bool read_tuple(Tuple& tuple) {
						Instruction in = serializer_->read_instruction(buffer_current_, buffer_end_);
						while(buffer_current_ < buffer_end_ && !in.is_tuple() && in.command() != CMD_END) {
							serializer_->process_instruction(in);
							in = serializer_->read_instruction(buffer_current_, buffer_end_);
						}
						
						if(in.is_tuple()) {
							table_id_t *t = reinterpret_cast<table_id_t*>(&in);
							for(size_type i = 0; i < serializer_->tuple_size(); i++) {
								size_type sz;
								tuple.set(i, serializer_->get_table(t[i], sz));
								assert(tuple.get(i) != 0);
							}
							return true;
						}
						
						serializer_->process_instruction(in);
						return false;
					}
							
					size_type position() { return buffer_current_ - buffer_start_; }
					
					bool done() {
						return buffer_current_ >= buffer_end_;
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
			
			
			table_id_t ensure_avoids(table_id_t id, size_type n_avoid, table_id_t* avoid) { //, bool* wrap = 0) {
				bool ok;
				do {
					ok = true;
					for(size_type i = 0; i < n_avoid; i++) {
						if(id == avoid[i]) {
							ok = false;
							id++;
							if(id >= table_size_) {
								//if(wrap) { *wrap = true; }
								id = 0;
							}
						}
					}
				} while(!ok);
				return id;
			}
			
			/**
			 * @return true iff this method should be called again with a
			 * fresh buffer.
			 */
			bool write_data(block_data_t* data, size_type data_size, block_data_t*& buffer, size_type buffer_size,
					table_id_t& id, size_type n_avoid = 0, table_id_t* avoid = 0) {
				bool call_again = false;
				
				size_type p = 0; // length of prefix already there
				table_id_t source_id = 0;
				
				#if SHDT_REUSE_PREFIXES
					// find a string in table with the longest common prefix
					
					id = nidx;
					
					for(table_id_t cid = 0; cid < table_size_; cid++) {
						size_type current_size = 0;
						block_data_t *current = get_table(cid, current_size);
						if(current) {
							size_type pn = prefix_length_n(min(data_size, current_size), current, data);
							if(pn > p) {
								p = pn;
								source_id = cid;
								if(p == data_size) { break; }
							}
						}
						else {
							if(id == nidx) { id = cid; }
						}
					}
					
					if(id == nidx) {
						id = hash(data, data_size);
						id = ensure_avoids(id, n_avoid, avoid);
					}
				#else 
					// just consider a single target position defined by
					// hash(complete string), maybe there is a prefix of the
					// string already there because we needed to split.
					
					id = hash(data, data_size);
					id = ensure_avoids(id, n_avoid, avoid);
					
					size_type current_size = 0;
					block_data_t *current = get_table(id, current_size);
					if(current) {
						p = prefix_length_n(min(data_size, current_size), current, data);
						source_id = id;
					}
				#endif
				
				// header size of the cat command
				size_t l = data_size; //strlen((char*)in.data());
				if(p && buffer_size > Instruction::header_size(CMD_CAT) + 1) {
					// cat
					size_t copy = buffer_size - Instruction::header_size(CMD_CAT);
					if(copy + p > l) { copy = l - p; }
					
					if(copy) {
						CatInstruction in;
						in.payload_size() = copy;
						in.source_index() = source_id;
						in.source_prefix() = p;
						in.target_index() = id;
						in.payload() = data + p;
						
						bool ca = write_instruction(in, buffer, buffer + buffer_size);
						if(!ca) {
							set_table(id, data, p + copy);
						}
						else {
							assert(false && "write cat instruction failed!");
						}
						
						call_again = ca || ((p + copy) < l);
					}
					else {
						// string is conveniently already completely there
						id = source_id;
						call_again = false;
					}
				}
				else if(buffer_size > Instruction::header_size(CMD_INSERT) + 1) {
					// insert
					size_t copy = buffer_size - Instruction::header_size(CMD_INSERT);
					if(copy > l) { copy = l; }
					
					InsertInstruction in;
					in.index() = id;
					in.payload_size() = copy;
					in.payload() = data;
					
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
					ValueInstruction in;
					in.field_id() = field_id;
					in.payload_size() = data_size;
					in.payload() = data;
					return write_instruction((Instruction&)in, buffer, buffer_end);
				}
				else {
					// do a table insert (as there we can split it up)
					table_id_t id;
					bool call_again = write_data(data, data_size, buffer, bufsiz, id);
					if(call_again) { return true; }
					
					TableValueInstruction in;
					in.field_id() = field_id;
					in.field_index() = id;
					return write_instruction((Instruction&)in, buffer, buffer_end);
				}
			}
			
			/**
			 * @return true iff this method should be called again with a
			 * fresh buffer.
			 */
			bool write_instruction(Instruction& in, block_data_t*& buffer, block_data_t* buffer_end) {
				int hs = in.header_size();
				if(hs == -1) { hs = tuple_size_; }
				
				if(buffer + hs + (in.has_payload() ? in.payload_size() : 0) > buffer_end) { return true; }
				
				memcpy(buffer, &in, hs);
				buffer += hs;
				
				if(in.has_payload()) {
					memcpy(buffer, in.payload(), in.payload_size());
					buffer += in.payload_size();
				}
				return false;
			}
			
			/**
			 */
			Instruction read_instruction(block_data_t*& buffer, block_data_t* buffer_end) {
				Instruction in;
				
				if((buffer_end - buffer) < MIN_INSTRUCTION_HEADER_SIZE) {
					return in;
				}
				
				// read the first few byte of the instruction
				// (as many as we can sure there will be regardless of the
				// command type)
				memcpy((block_data_t*)&in, buffer, MIN_INSTRUCTION_HEADER_SIZE);
				buffer += MIN_INSTRUCTION_HEADER_SIZE;
				
				// read rest of the instruction (if any)
				int hs = in.header_size();
				if(hs == -1) { hs = tuple_size_; }
				int to_read = hs - MIN_INSTRUCTION_HEADER_SIZE;
				assert(to_read >= 0);
				
				if(to_read > 0) {
					memcpy(((block_data_t*)&in) + MIN_INSTRUCTION_HEADER_SIZE, buffer, to_read);
					buffer += to_read;
				}
				
				if(in.has_payload()) {
					in.payload() = buffer;
					buffer += in.payload_size();
				}
				return in;
			}
			
			/**
			 */
			void process_instruction(Instruction& in) {
				switch(in.command()) {
					case CMD_HEADER: {
						HeaderInstruction& head = (HeaderInstruction&)in;
						table_size_ = head.table_size();
						assert(table_size_ > 0);
						tuple_size_ = head.tuple_size();
						
						// TODO: Do something appropriate if we receive a
						// wrong version number!
						break;
					}
						
					case CMD_INSERT: {
						InsertInstruction& ins = (InsertInstruction&)in;
						assert(ins.index() < table_size());
						assert(ins.payload_size() > 0);
						set_table(ins.index(), ins.payload(), ins.payload_size());
						break;
					}
						
					case CMD_VALUE:
					case CMD_TABLE_VALUE:
					case CMD_TUPLE:
						assert(false && "dunno how to process this");
						break;
						
					case CMD_END:
						break;
						
					case CMD_CAT: {
						CatInstruction& cat = (CatInstruction&)in;
						
						assert(cat.source_index() < table_size_);
						assert(cat.target_index() < table_size_);
						
						size_type current_size;
						block_data_t *current = get_table(cat.source_index(), current_size);
						assert(current != 0);
						
						size_type l = cat.source_prefix() + cat.payload_size();
						block_data_t buf[l];
						memcpy(buf, current, cat.source_prefix()); //current_size);
						memcpy(buf + cat.source_prefix(), cat.payload(), cat.payload_size());
						set_table(cat.target_index(), buf, l);
						break;
					}
					
				}
			}
			
			void set_table(table_id_t id, block_data_t* data, sz_t data_size) {
				//assert(strlen((char*)data) + 1 >= data_size);
				
				if(lookup_table_[id]) {
					get_allocator().free_array(lookup_table_[id]);
					lookup_table_[id] = 0;
				}
				block_data_t* d = get_allocator().allocate_array<block_data_t>(data_size + sizeof(sz_t)).raw();
				lookup_table_[id] = d;
				
				wiselib::write<OsModel>(d, data_size);
				memcpy(d + sizeof(sz_t), data, data_size);
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
			
		private:
			
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

