
#ifndef SHDT_SERIALIZER_H
#define SHDT_SERIALIZER_H

#include <external_interface/external_interface.h>
#include <util/serialization/serialization.h>
#include <util/meta.h>
#include <util/pstl/string_utils.h>

namespace wiselib {
	
	template<
		typename OsModel_P,
		size_t TABLE_SIZE_P,
		size_t TUPLE_SIZE_P = 3
	>
	class ShdtSerializer {
		public:
			typedef OsModel_P OsModel;
			enum { TABLE_SIZE = TABLE_SIZE_P };
			enum { TUPLE_SIZE = TUPLE_SIZE_P };
			
			typedef typename OsModel::size_t size_type;
			typedef typename OsModel::block_data_t block_data_t;
			typedef ::uint8_t instruction_t;
			typedef ::uint8_t field_id_t;
			typedef ::uint8_t sz_t;
			typedef typename SmallUint<TABLE_SIZE + 1>::t table_id_t;
			
			enum { npos = (size_type)(-1) };
			enum { nidx = (table_id_t)(-1) };
			enum Commands {
				CMD_VALUE = 0xfb,
				CMD_CAT = 0xfc,
				CMD_TABLE_VALUE = 0xfd,
				
				CMD_INSERT = 0xfe,
				CMD_END = 0xff
			};
			enum { SUCCESS = OsModel::SUCCESS, ERR_UNSPEC = OsModel::ERR_UNSPEC };
			enum SpecialIds {
				NO_FIELD_ID = (field_id_t)(-1)
			};
			
		private:
			
			class Instruction {
				public:
					instruction_t type() { return instruction_; }
					void set_type(instruction_t c) { instruction_ = c; }
					
					bool fits(size_type buffer_size) {
						// TODO
					}
					
					/**
					 * @return true iff all data was written, false if another
					 * write is necessary.
					 */
					bool write(block_data_t*& buffer, size_type& buffer_size) {
						// TODO
					}
					
					void read(block_data_t*& buffer, size_type& buffer_size) {
						// TODO
					}
					
				private:
					instruction_t instruction_;
			};
			
		public:
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
				for(size_type i=0; i<TABLE_SIZE; i++) {
					if(lookup_table_[i]) {
						get_allocator().free(lookup_table_[i]);
					}
				}
				memset((void*)lookup_table_, 0, sizeof(lookup_table_));
			}
			
			/**
			 * 
			 * Usage:
			 * Call this in a loop, providing the buffer position to write
			 * into and how many bytes to write (max_size).
			 * This will increase 'current' until it hits 'end' and return the
			 * number of bytes written.
			 * 
			 * There are 3 possible outcomes of a call to fill_buffer():
			 * 1. return > 0, current != end.
			 *    Some data has been written, call again with updated buffer
			 *    and max_size values.
			 * 2. current == end.
			 *    All data has been written to the buffer.
			 * 3. return == 0, current != end (also in this case current will
			 *    not be altered)
			 *    No data has been written although not all data has been read
			 *    yet. This means the supplied buffer was too small.
			 *    Call again with a bigger buffer (i.e. larger max_size).
			 * 
			 * \param buffer pointer to buffer to write into
			 * \param max_size maximum amount of bytes to write into buffer
			 * \param begin reference to iterator over tuples, will be ++'ed
			 * 	during operation of this method
			 * \param end end iterator (when begin==end, this method will
			 * exit)
			 * 
			 * \return Number of bytes written.
			 */
			template<typename iterator>
			size_type fill_buffer(block_data_t* buffer, size_type max_size, iterator& current, const iterator& end) {
				block_data_t *old_buffer = buffer;
				//size_type old_max_size = max_size;
				table_id_t ids[TUPLE_SIZE];
				memset(&ids, 0xff, sizeof(table_id_t)*TUPLE_SIZE);
				

				for( ; current != end; ++current) {
					typename iterator::Tuple t = *current;
					bool ins = insert_tuple(buffer, max_size, ids, t);
					if(!ins) { break; }
					
					const size_type cmdlen = TUPLE_SIZE*sizeof(table_id_t);
					if(max_size < cmdlen) { break; }
					
					for(size_type p=0; p<TUPLE_SIZE; p++) {
						wiselib::write<OsModel, table_id_t>(buffer, ids[p]);
						buffer += sizeof(table_id_t);
					}
					max_size -= cmdlen;
				}
				return buffer - old_buffer;
			}
			
			template<typename T>
			size_type fill_buffer(block_data_t* buffer, size_type max_size, field_id_t field_id, const T& data, bool& call_again) {
				block_data_t *old_buffer = buffer;
				
				size_type cmdlen = sizeof(command_t) + sizeof(sz_t) + sizeof(field_id_t) + sizeof(T);
				if(max_size < cmdlen) {
					call_again = true;
					return 0;
				}
				
				wiselib::write<OsModel, command_t>(buffer, CMD_VALUE); buffer += sizeof(command_t);
				wiselib::write<OsModel, field_id_t>(buffer, field_id); buffer += sizeof(field_id_t);
				wiselib::write<OsModel, sz_t>(buffer, sizeof(T)); buffer += sizeof(sz_t);
				wiselib::write<OsModel, T>(buffer, data); buffer += sizeof(T);
				max_size -= cmdlen;
				
				return buffer - old_buffer;
			}
			
			size_type fill_buffer(block_data_t* buffer, size_type max_size, field_id_t field_id, const char* data, bool& call_again) {
				block_data_t *old_buffer = buffer;
				
				table_id_t id = hash((block_data_t*)data);
				// is there already a prefix of the string there?
				char *current = (char*)lookup_table_[id];
				size_type p = 0;
				
				if(current) {
					p = prefix_length(current, data);
				}
				
				size_type cmdlen = sizeof(command_t) + sizeof(table_id_t) + sizeof(sz_t) + sizeof(table_id_t) + sizeof(sz_t);
				
				// how much of the string will have been communicated
				// after this method returns? (including previous calls!)
				size_type len = strlen(data) + 1;
				size_type len_orig = len;
				
				// How much of the string actually fits into the buffer?
				size_type affordable = max_size - cmdlen;
				
				// Copy this many bytes is this call
				size_type copy = len - p;
				
				if(copy > affordable) {
					copy = affordable;
					len = p + copy;
				}
				
				if(copy) {
					wiselib::write<OsModel, command_t>(buffer, CMD_CAT); buffer += sizeof(command_t);
					wiselib::write<OsModel, table_id_t>(buffer, id); buffer += sizeof(table_id_t);
					wiselib::write<OsModel, sz_t>(buffer, p); buffer += sizeof(sz_t);
					wiselib::write<OsModel, table_id_t>(buffer, id); buffer += sizeof(table_id_t);
					wiselib::write<OsModel, sz_t>(buffer, copy); buffer += sizeof(sz_t);
					max_size -= cmdlen;
					
					memcpy(buffer, data + p, copy); buffer += copy;
					max_size -= copy;
					
					get_allocator().free_array(lookup_table_[id]); lookup_table_[id] = 0;
					block_data_t* d = get_allocator().allocate_array<block_data_t>(len).raw();
					memcpy(d, data, len);
					lookup_table_[id] = d;
				}
				
				// Do we have the complete string by now?
				if(len < len_orig) {
					call_again = true;
					return buffer - old_buffer;
				}
					
				
				// Do we want to write a value command?
				if(field_id == NO_FIELD_ID) {
					call_again = false;
					return buffer - old_buffer;
				}
				
				cmdlen = sizeof(command_t) + sizeof(field_id_t) + sizeof(table_id_t);
				if(cmdlen > max_size) {
					call_again = true;
					return buffer - old_buffer;
				}
				
				wiselib::write<OsModel, command_t>(buffer, CMD_TABLE_VALUE); buffer += sizeof(command_t);
				wiselib::write<OsModel, field_id_t>(buffer, field_id); buffer += sizeof(field_id_t);
				wiselib::write<OsModel, table_id_t>(buffer, id); buffer += sizeof(table_id_t);
			}
			
			size_type read_field(block_data_t* buffer, size_type buffer_size, block_data_t*& data, size_type& data_size, field_id_t& field_id) {
				block_data_t *old_buffer = buffer;
				block_data_t *old_buffer_size = buffer_size;
				
				if(!buffer_size) { return 0; }
				
				Instruction in;
				
				do {
					in.read(buffer, buffer_size);
				
					if(in.is_field
				data = in.data();
				data_size = in.data_size();
				field_id =
				
				return buffer - old_buffer;
			}
			
			/**
			 * read buffer contents into tuple, returning the number of bytes
			 * read.
			 * This might change internal state (lookup table) if the buffer
			 * contains insert commands.
			 * This will only read the first tuple from the given buffer, so
			 * if buffer_size - $returnvalue is positive, this should be
			 * called again.
			 * This will use tuple.set(...) in order to return the data.
			 * (I.e. after a call to this, tuple might hold references pointing into
			 * the internal lookup table.)
			 * 
			 * It is expected that if buffer_size is not 0, it contains at
			 * least one complete and executable tuple command.
			 */
			template<typename Tuple>
			size_type read_buffer(Tuple& tuple, block_data_t* buffer, size_type buffer_size) {
				block_data_t *old_buffer = buffer;
				
				while(buffer_size) {
					if(buffer_size < sizeof(table_id_t)) {
						buffer += buffer_size;
						break;
					}
					
					table_id_t tid = wiselib::read<OsModel, block_data_t, table_id_t>(buffer);
					buffer += sizeof(table_id_t); buffer_size -= sizeof(table_id_t);
					
					if(tid == nidx) { // command mode
						if(buffer_size < sizeof(command_t)) {
							buffer += buffer_size;
							break;
						}
						
						command_t cmd = wiselib::read<OsModel, block_data_t, command_t>(buffer);
						buffer += sizeof(command_t); buffer_size -= sizeof(command_t);
						
						if(cmd == CMD_INSERT) { // insert command
							table_id_t pos = wiselib::read<OsModel, block_data_t, table_id_t>(buffer);
							buffer += sizeof(table_id_t); buffer_size -= sizeof(table_id_t);
							size_type l = strlen((char*)buffer) + 1; //strnlen((char*)buffer, buffer_size);
							if(lookup_table_[pos]) {
								get_allocator().free_array(lookup_table_[pos]);
							}
							lookup_table_[pos] = get_allocator().allocate_array<block_data_t>(l) .raw();
							memcpy((void*)lookup_table_[pos], (void*)buffer, l);
							buffer += l; buffer_size -= l;
						}
						else if(cmd == CMD_END) {
							buffer += buffer_size;
							break;
						}
					} // command mode
					
					else { // tuple moda
						tuple.set(0, lookup_table_[tid]);
						for(size_type i=1; i<TUPLE_SIZE; i++) {
							tid = wiselib::read<OsModel, block_data_t, table_id_t>(buffer);
							buffer += sizeof(table_id_t); buffer_size -= sizeof(table_id_t);
							tuple.set(i, lookup_table_[tid]);
						}
						break; // we got one tuple --> exit
					}
				} // while buffer_size
				
				return buffer - old_buffer;
			}
			
			//bool has_data() { return current_ != end_; }
				
				
		private:
			
			/**
			 * \return
			 * true -> success (inserted everything)
			 * false -> partial or no insert
			 */
			template<typename Tuple>
			bool insert_tuple(block_data_t*& buffer, size_type& max_size, table_id_t* ids, Tuple tuple) {
				//printf("tuple=(%s %s %s)\n", tuple.get(0), tuple.get(1), tuple.get(2));
				
				ids[0] = insert_hash_avoid(tuple.get(0), nidx, nidx, buffer, max_size);
				if(ids[0] == nidx) { return false; }
				ids[1] = insert_hash_avoid(tuple.get(1), ids[0], nidx, buffer, max_size);
				if(ids[1] == nidx) { return false; }
				ids[2] = insert_hash_avoid(tuple.get(2), ids[0], ids[1], buffer, max_size);
				if(ids[2] == nidx) { return false; }
				
				return true;
			}
			
			table_id_t insert_hash_avoid(block_data_t* data, table_id_t avoid1, table_id_t avoid2, block_data_t*& buffer, size_type& max_size) {
				//printf("data=(%s)\n", data);
				table_id_t id = hash(data);
				table_id_t id2 = nidx;
				for(size_type offs = 0; ; offs++) {
					id2 = (id + offs) % TABLE_SIZE;
					
					if(id2 == avoid1 || id2 == avoid2) { continue; }
					
					block_data_t *t = lookup_table_[id2];
					//printf("  table[%d]=%s\n", id2, (char*)t);
					
					// empty slot in table --> insert here!
					if(t == 0) {
						//printf("  --> empty\n");
						if(insert(id2, data, buffer, max_size) == ERR_UNSPEC) { return nidx; }
						break;
					}
					
					// data already there, we're done!
					else if(strcmp((char*)t, (char*)data) == 0) {
						//printf("  --> already there :)\n");
						break;
					}
					
					else {
						//printf("  --> overwrite\n");
//						GET_OS.debug("free(2) %x", t);
						get_allocator().free(t);
						lookup_table_[id2] = 0;
						
						if(insert(id2, data, buffer, max_size) == ERR_UNSPEC) { return nidx; }
						break;
					}
				} // for offs
				
				return id2;
			} // insert_hash_avoid(...)
			
			int insert(table_id_t id, block_data_t* data, block_data_t*& buffer, size_type& max_size) {
				const size_type l = strlen((char*)data) + 1;
				const size_type cmdlen = sizeof(table_id_t) + sizeof(command_t) + sizeof(table_id_t) + l;
				if(max_size < cmdlen) { return ERR_UNSPEC; }
				
				// ----- actual command write ------
				table_id_t tid;
				command_t cmd;
				
				// id = nidx --> enter command mode
				tid = nidx;
				wiselib::write<OsModel, table_id_t>(buffer, tid); buffer += sizeof(table_id_t);
				
				// command: INSERT
				cmd = CMD_INSERT;
				wiselib::write<OsModel, command_t>(buffer, cmd); buffer += sizeof(command_t);
				
				// id
				tid = id;
				wiselib::write<OsModel, table_id_t>(buffer, tid); buffer += sizeof(table_id_t);
				
				// data
				memcpy((void*)buffer, (void*)data, l); buffer += l;
//
				max_size -= cmdlen;

				block_data_t* d = get_allocator().allocate_array<block_data_t>(l).raw();
//				GET_OS.debug("aod: %x", d);
//				block_data_t* d = (block_data_t*)isense::malloc(l);
				memcpy((void*)d, (void*)data, l);
//				memset(d, '@', l);
//				GET_OS.debug("id: %u %c%c", id, (char)((int)d>>8), (char)((int)d&0xff));
				lookup_table_[id] = d;
				
				return SUCCESS;
			}
			
			static table_id_t hash(const block_data_t *s) {
				//return (s[0] ^ s[1]) % TABLE_SIZE;
				//return reinterpret_cast<Uint<sizeof(block_data_t*)>::t>(s) % TABLE_SIZE;
				
				// stolen from STL, slightly modified
				// source: http://www.fantasy-coders.de/projects/gh/html/x435.html
				table_id_t r = 0;
				for(size_type i=0; s[i] /*&& i<12*/; i++) {
					r = (5*r + s[i]) % TABLE_SIZE;
				}
				return r % TABLE_SIZE;
			}
			
			
			//iterator current_, end_;
			block_data_t *lookup_table_[TABLE_SIZE];
	};
	
} // namespace wiselib

#endif // SHDT_SERIALIZER_H

