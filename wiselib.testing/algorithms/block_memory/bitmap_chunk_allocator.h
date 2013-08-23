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

#ifndef BITMAP_CHUNK_ALLOCATOR_H
#define BITMAP_CHUNK_ALLOCATOR_H

#if (DEBUG_OSTREAM || DEBUG_GRAPHVIZ)
	#include <fstream>
	#include <iostream>
	#include <iomanip>
#endif

#include <util/meta.h>
#include <util/standalone_math.h>
#include <util/debugging.h>
#include "chunk_map_block.h"
#include "summary_block.h"

namespace wiselib {
	
	/**
	 * @brief
	 * 
	 * @ingroup
	 * 
	 * @tparam 
	 */
	template<
		typename OsModel_P,
		typename BlockMemory_P,
		size_t CHUNK_SIZE_P,
		typename Address_P = typename BlockMemory_P::address_t,
		typename Debug = typename OsModel_P::Debug
	>
	class BitmapChunkAllocator {
		public:
			class ChunkAddress;
			
			typedef OsModel_P OsModel;
			typedef typename OsModel::block_data_t block_data_t;
			typedef typename OsModel::size_t size_type;
			typedef BlockMemory_P BlockMemory;
			typedef Address_P address_t;
			typedef BitmapChunkAllocator<OsModel, BlockMemory, CHUNK_SIZE_P, Address_P, Debug> self_type;
			typedef self_type* self_pointer_t;
			typedef StandaloneMath<OsModel> Math;
			
		//private:
			
			typedef typename SmallUint< BlockMemory::BLOCK_SIZE / CHUNK_SIZE_P >::t summary_t;
			enum { NO_SUMMARY = (summary_t)(-1) };
			
			enum {
				SUMMARY_SIZE = sizeof(summary_t),
				CHUNK_SIZE = CHUNK_SIZE_P,
				CHUNKS_PER_BLOCK = BlockMemory::BLOCK_SIZE / CHUNK_SIZE,
				ENTRIES_PER_SUMMARY_BLOCK = BlockMemory::BLOCK_SIZE / SUMMARY_SIZE,
				BLOCKS_PER_BLOCK = 8 * BlockMemory::BLOCK_SIZE / CHUNKS_PER_BLOCK,
				CHUNK_BITS = Log<CHUNKS_PER_BLOCK, 2>::value
				
			};
			
		public:

			enum {
				SUCCESS = OsModel::SUCCESS, ERR_UNSPEC = OsModel::ERR_UNSPEC
			};

			class ChunkAddress {
				
				public:
					ChunkAddress() : addr_(0) {
					}
					
					ChunkAddress(const ChunkAddress& other) : addr_(other.addr_) {
					}
					
					ChunkAddress(typename Uint<sizeof(address_t)>::t addr) : addr_(addr) {
					}
					
					/*ChunkAddress(address_t a) : addr_(a) {
					}*/
					
					static ChunkAddress invalid() {
						ChunkAddress r;
						r.addr_ = -1;
						return r;
					}
					
					address_t address() const { return addr_ >> CHUNK_BITS; }
					void set_address(address_t address) { addr_ = (address << CHUNK_BITS) | offset(); }
					size_type offset() const { return addr_ & ((1 << CHUNK_BITS) - 1); }
					
					ChunkAddress& operator+=(size_type i) {
						address_t chunk = offset();
						address_t a = address();
						chunk += i;
						if(chunk >= CHUNKS_PER_BLOCK) {
							a += chunk / CHUNKS_PER_BLOCK;
							chunk %= CHUNKS_PER_BLOCK;
						}
						addr_ = (a << CHUNK_BITS) | chunk;
						return *this;
					}
					
					ChunkAddress operator+(size_type i) {
						ChunkAddress r(*this);
						r += i;
						return r;
					}
					
					size_type absolute_chunk() const {
						return address() * CHUNKS_PER_BLOCK + offset();
					}
					
					operator typename Uint<sizeof(address_t)>::t() { return addr_; }
					
					bool operator==(const ChunkAddress& other) const { return other.addr_ == addr_; }
					bool operator!=(const ChunkAddress& other) const { return !(*this == other); }
					
#if (DEBUG_OSTREAM || DEBUG_GRAPHVIZ)
					friend std::ostream& operator<<(std::ostream& os, ChunkAddress a) {
						os << "@" << a.address() << "." << a.offset();
						return os;
					}
#endif // DEBUG_OSTREAM
					
				private:
					
					/**
					 * We need to fit all of the chunk address into one
					 * address_t so it can be a dictionary key
					 * (whose size in turn is limited by sizeof(block_data_t)
					 * and thus by the system word size.
					 */
					
					address_t addr_;
			};
			
			typedef ChunkMapBlock<OsModel, BlockMemory, summary_t, CHUNK_SIZE> CMBlock;
			typedef SummaryBlock<OsModel, BlockMemory, summary_t> SBlock;
			
			enum {
				BLOCK_SIZE = BlockMemory::BLOCK_SIZE,
				BUFFER_SIZE = BlockMemory::BUFFER_SIZE,
				NO_ADDRESS = (address_t)BlockMemory::NO_ADDRESS
			};
			
			void init(BlockMemory* block_memory, typename Debug::self_pointer_t debug) {
				block_memory_ = block_memory;
				debug_ = debug;
				
				// reserve the "special" part of the cache below for caching
				// the chunk index
				block_memory_->set_special_range(0, layer_start(summary_height_) + layer_size(summary_height_));
				parent_size_ = block_memory_->size();
				
				if((unsigned)Math::log2(block_memory_->size()) + CHUNK_BITS > sizeof(address_t)*8) {
					parent_size_ = ((size_type)1 << (sizeof(address_t)*8 - CHUNK_BITS)) - 1;
					debug_->debug("WARNING: %d bit chunk addr, medium size %lu (need %d bits)! Using first %lu blocks of the medium.",
							(int)sizeof(address_t)*8 - CHUNK_BITS, (unsigned long)block_memory_->size(), (int)Math::log2(block_memory_->size()), (unsigned long)parent_size_);
				}
				
				calculate_limits();


				
				debug_->debug("bitmap chunk allocator init");
				debug_->debug("chunk size          : %lu", (unsigned long)CHUNK_SIZE);
				debug_->debug("chunks per block    : %lu", (unsigned long)CHUNKS_PER_BLOCK);
				debug_->debug("mem size (#blocks)  : %lu", (unsigned long)parent_size());
				debug_->debug("chunk map blocks    : %lu", (unsigned long)chunk_map_blocks_);
				debug_->debug("summary height      : log%lu(cbm=%lu) = %lu",
						(unsigned long)ENTRIES_PER_SUMMARY_BLOCK,
						(unsigned long)chunk_map_blocks_,
						(unsigned long)summary_height_);
				debug_->debug("root entries        : %lu", (unsigned long)entries_in_root_block_);
				debug_->debug("summary entries     : %lu", (unsigned long)ENTRIES_PER_SUMMARY_BLOCK);
				debug_->debug("chunk map block bits: %lu", (unsigned long)(BLOCK_SIZE * 8));
				debug_->debug("");
				for(size_type i = 0; i <= summary_height_; i++) {
					debug_->debug("[layer %lu  %8lx - %8lx]",
							(unsigned long)i, (unsigned long)layer_start(i), (unsigned long)layer_start(i) + (unsigned long)layer_size(i)
					);
				}
				debug_->debug("end                 : %lu", (unsigned long)((layer_start(summary_height_) + layer_size(summary_height_))));
				
				assert(total_map_blocks_ == layer_start(summary_height_) + layer_size(summary_height_));
			}

			size_type size() { return parent_size() - total_map_blocks_; }
			
			void wipe() {
				block_memory_->wipe();
				format();
			}
			
			void format() {
				debug_->debug("formatting block memory device...%s", "");
				
				// This allocator needs the first TOTAL_MAP_BLOCKS for
				// himself, thus those need to be marked allocated initially.
				
				block_data_t buf[BUFFER_SIZE];
				
				// 
				// ---- mark chunk map blocks ----
				// 
				address_t a = layer_start(summary_height_);
				
				// ASSUMPTION: CHUNKS_PER_BLOCK is divisible by 8, else the
				// marking will not be correct and thus userdata might
				// overwrite allocation data
				size_type zeros = total_map_blocks_ * (CHUNKS_PER_BLOCK / 8);
				size_type full_blocks = zeros / BLOCK_SIZE;
				size_type ones = layer_size(summary_height_) * BLOCK_SIZE - zeros;
				
				debug_->debug("CMB zero blocks start at %lu", (unsigned long)a);
				memset(buf, 0, BUFFER_SIZE);
				for( ; zeros >= BLOCK_SIZE; zeros -= BLOCK_SIZE, a++) {
					block_memory_->write(buf, a);
				}
				if(zeros) {
					debug_->debug("CMB mixed at %lu", (unsigned long)a);
					memset(buf, 0, zeros);
					memset(buf + zeros, 0xff, BUFFER_SIZE - zeros);
					block_memory_->write(buf, a);
					a++;
					ones -= (BUFFER_SIZE - zeros);
				}
				debug_->debug("CMB ones start at at %lu", (unsigned long)a);
				memset(buf, 0xff, BUFFER_SIZE);
				for( ; ones >= BLOCK_SIZE; ones -= BLOCK_SIZE, a++) {
					block_memory_->write(buf, a);
				}
				assert(ones == 0);
				
				debug_->debug("user blocks start at %lu", (unsigned long)a);
				
				for(size_type layer = summary_height_ - 1; layer != (size_type)(-1); layer--) {
					zeros = full_blocks * SUMMARY_SIZE;
					size_type maxs = layer_size(layer) * BLOCK_SIZE - zeros;
					a = layer_start(layer);
					memset(buf, 0, BUFFER_SIZE);
					for( ; zeros >= BLOCK_SIZE; zeros -= BLOCK_SIZE, a++) {
						block_memory_->write(buf, a);
					}
					if(zeros) {
						memset(buf, 0x0, zeros);
						assert(SUMMARY_SIZE == 1);
						assert(CHUNKS_PER_BLOCK <= 255);
						memset(buf + zeros, CHUNKS_PER_BLOCK, BUFFER_SIZE - zeros);
						block_memory_->write(buf, a);
						a++;
						maxs -= (BUFFER_SIZE - zeros);
					}
					
					assert(SUMMARY_SIZE == 1);
					assert(CHUNKS_PER_BLOCK <= 255);
					memset(buf, CHUNKS_PER_BLOCK, BUFFER_SIZE);
					for( ; maxs >= BLOCK_SIZE; maxs -= BLOCK_SIZE, a++) {
						block_memory_->write(buf, a);
					}
					assert(maxs == 0);
					
					full_blocks = (full_blocks * SUMMARY_SIZE) / BLOCK_SIZE;
				}
				
				debug_->debug("verifying CMB summaries...");
				for(size_type cmb = 0; cmb < chunk_map_blocks_; cmb++) {
					check_chunk_map_block(layer_start(summary_height_) + cmb);
				}

				debug_->debug("formatting done.");
			}
			
			address_t create(block_data_t* buffer) {
				debug_graphviz();
				return allocate_block();
			}
			
			void free(address_t a) {
				free_block(a);
			}
			
			void write(block_data_t* buffer, address_t a) {
				block_memory_->write(buffer, a);
			}
			
			void read(block_data_t* buffer, address_t a) {
				block_memory_->read(buffer, a);
			}
			
			const block_data_t* get(address_t a) {
				return block_memory_->get(a);
			}

			void invalidate(address_t a) {
				return block_memory_->invalidate(a);
			}
			
			ChunkAddress create_chunks(block_data_t* buffer, size_type bytes) {
				size_type chunks = (bytes + CHUNK_SIZE - 1) / CHUNK_SIZE;
				assert(chunks * CHUNK_SIZE >= bytes);
				
				ChunkAddress r = allocate_chunks(chunks);
				block_data_t buf[BlockMemory::BUFFER_SIZE];

				block_memory_->read(buf, r.address());
				memcpy(buf + r.offset() * CHUNK_SIZE, buffer, bytes);
				block_memory_->write(buf, r.address());
				return r;
			}
			
			ChunkAddress allocate_chunks(size_type n) {
				SBlock summary_block;
				Finder finder(summary_block, n, this);
				//finder.debug_ = debug_;
				finder.allocate(0, 0);
				
			#if BITMAP_CHUNK_ALLOCATOR_CHECK
				check_chunk_map_block(finder.chunk_map_block_addr_);
			#endif
				//debug_->debug("-- allocate [%d.%d .. %d.%d]",
						//(int)finder.chunk_address().address(), (int)finder.chunk_address().offset(),
						//(int)(finder.chunk_address() + (n - 1)).address(),
						//(int)(finder.chunk_address() + (n - 1)).offset());
				
				return finder.chunk_address();
			}
			
			void free_chunks(ChunkAddress a, size_type bytes) {
				size_type chunks = (bytes + CHUNK_SIZE - 1) / CHUNK_SIZE;
				
				assert(((a.offset() + chunks) * CHUNK_SIZE) <= BLOCK_SIZE);
				
				//debug_->debug("-- free [%d.%d .. %d.%d]",
						//(int)a.address(), (int)a.offset(),
						//(int)(a + (chunks - 1)).address(), (int)(a + (chunks - 1)).offset());

				CMBlock cmblock;
				address_t a_cmblock = layer_start(summary_height_) + a.absolute_chunk() / (BlockMemory::BLOCK_SIZE * 8);

				read_block(cmblock, a_cmblock);
				cmblock.unmark(a.absolute_chunk() % (BlockMemory::BLOCK_SIZE * 8), chunks);
				write_block(cmblock, a_cmblock);
			}
			
			void read_chunks(block_data_t* buffer, ChunkAddress addr, size_type bytes) {
				assert(((addr.offset() + (bytes + CHUNK_SIZE - 1) / CHUNK_SIZE) * CHUNK_SIZE) <= BLOCK_SIZE);
				block_data_t buf[BlockMemory::BUFFER_SIZE];

				//debug_->debug("-- read %d bytes at %d.%d", (int)bytes, (int)addr.address(), (int)addr.offset());
				block_memory_->read(buf, addr.address());
				memcpy(buffer, buf + addr.offset() * CHUNK_SIZE, bytes);
			}
			
			void write_chunks(block_data_t* buffer, ChunkAddress addr, size_type bytes) {
				assert(((addr.offset() + (bytes + CHUNK_SIZE - 1) / CHUNK_SIZE) * CHUNK_SIZE) <= BLOCK_SIZE);
				block_data_t buf[BlockMemory::BUFFER_SIZE];

				//debug_->debug("-- write %d bytes at %d.%d", (int)bytes, (int)addr.address(), (int)addr.offset());
				
				block_memory_->read(buf, addr.address());
				memcpy(buf + addr.offset() * CHUNK_SIZE, buffer, bytes);
				block_memory_->write(buf, addr.address());
			}
			
			// Debugging -- Graphviz
			// {{{
			
#if DEBUG_GRAPHVIZ
			void debug_graphviz(const char *fn = "bitmap_chunk_allocator.dot") {
				enum { MAX_PER_LAYER = 100U };
				std::ofstream out(fn);
				SBlock block;
				
				out << "digraph G {" << std::endl;
				
				for(size_type layer = 0; layer < summary_height_; layer++) {
					for(size_type i = 0; i < Math::template min<size_type>(MAX_PER_LAYER, layer_size(layer)); i++) {
						address_t address = layer_start(layer) + i;
						read_block(block, address);
						out << "b" << address << " [shape=plaintext, label=";
						block.debug_graphviz_label(out, address);
						out << "] ;" << std::endl;
						
						size_type n = (layer == 0) ? (size_type)entries_in_root_block_ : (size_type)ENTRIES_PER_SUMMARY_BLOCK;
						for(size_type j = 0; j < Math::template min<size_type>(MAX_PER_LAYER, n); j++) {
							out << "b" << address << " -> b" <<
								(layer_start(layer + 1) + i * n + j) << ";" << std::endl;
						}
					}
				}
				
				
				CMBlock &cmblock = *reinterpret_cast<CMBlock*>(&block);
				
				for(size_type i = 0; i < Math::template min<size_type>(MAX_PER_LAYER, layer_size(summary_height_)); i++) {
					address_t address = layer_start(summary_height_) + i;
					read_block(cmblock, address);
					out << "  b" << address << " [shape=plaintext, label=";
					cmblock.debug_graphviz_label(out, address);
					out << "  ] ;" << std::endl;
				}
				
				out << "}" << std::endl;
				out.close();
			} // debug_graphviz()
#else
			void debug_graphviz(const char *_ = 0) { }
#endif
			// }}}
			
			// Debugging -- Checks
			// {{{
#if BITMAP_CHUNK_ALLOCATOR_CHECK
			void check() {
			}
			
			void check_chunk_map_block(address_t a) {
				CMBlock cmblock;
				read_block(cmblock, a);
				typename CMBlock::summary_t s = cmblock.summary();
				
				address_t offset = a - layer_start(summary_height_);
				for(size_type layer = summary_height_ - 1; layer != (size_type)(-1); layer--) {
					address_t new_offset = offset / ENTRIES_PER_SUMMARY_BLOCK;
					address_t idx = offset % ENTRIES_PER_SUMMARY_BLOCK;
					
					SBlock sblock;
					read_block(sblock, new_offset + layer_start(layer));
					assert(sblock[idx] == s);
					
					s = sblock.summary(ENTRIES_PER_SUMMARY_BLOCK);
					offset = new_offset;
				}
			}
#else
			void check() { }
			void check_chunk_map_block(address_t) { }
			
#endif // BITMAP_CHUNK_ALLOCATOR_CHECK
			// }}}
			
		private:
			
			size_type parent_size() {
				return parent_size_;
			}
			
			void calculate_limits() {
				const size_type chunk_map_bits = parent_size() * CHUNKS_PER_BLOCK;
				const size_type chunk_map_bytes = (chunk_map_bits + 7) / 8;
				chunk_map_blocks_ = (chunk_map_bytes + BlockMemory::BLOCK_SIZE - 1) / BlockMemory::BLOCK_SIZE;
				
				entries_in_root_block_ = root_block_entries(chunk_map_blocks_);
				total_map_blocks_ = tree_nodes(chunk_map_blocks_);
				summary_height_ = log(chunk_map_blocks_) + 1;
			}
			
			static size_type root_block_entries(size_type entries) {
				if(entries <= ENTRIES_PER_SUMMARY_BLOCK) { return entries; }
				return root_block_entries((entries + ENTRIES_PER_SUMMARY_BLOCK - 1) / ENTRIES_PER_SUMMARY_BLOCK);
			}
			
			static size_type tree_nodes(size_type x) {
				if(x <= 1) { return 1; }
				return x + tree_nodes((x + ENTRIES_PER_SUMMARY_BLOCK - 1) / ENTRIES_PER_SUMMARY_BLOCK);
			}
			
			static size_type log(size_type x) {
				if(x < ENTRIES_PER_SUMMARY_BLOCK) { return 0; }
				return 1 + log((x + ENTRIES_PER_SUMMARY_BLOCK - 1) / ENTRIES_PER_SUMMARY_BLOCK);
			}
			
			
			template<typename Block>
			void read_block(Block& block, address_t a) {
				block_memory_->read(reinterpret_cast<block_data_t*>(&block), a);
			}
			
			template<typename Block>
			void write_block(Block& block, address_t a) {
				block_memory_->write(reinterpret_cast<block_data_t*>(&block), a);
			}
			
			address_t allocate_block() {
				return allocate_chunks(CHUNKS_PER_BLOCK).address();
			}
			
			void free_block(address_t a) {
				CMBlock cmblock;
				address_t a_cmblock = layer_start(summary_height_) + a / BLOCKS_PER_BLOCK;
				read_block(cmblock, a_cmblock);
				cmblock.unmark(CHUNKS_PER_BLOCK * (a % BLOCKS_PER_BLOCK), CHUNKS_PER_BLOCK);
				write_block(cmblock, a_cmblock);
			}
			
			class Finder {
				public:
					Finder(SBlock& summary_block, size_type n, self_type* allocator)
						: summary_block_(summary_block), n_(n), allocator_(allocator) {
					}
					
					/*
					 * @return NO_SUMMARY if summary did not change, else the new
					 * summary
					 */
					summary_t allocate(size_type layer, address_t address) {
						summary_t r = NO_SUMMARY;
						allocator_->read_block(summary_block_, allocator_->layer_start(layer) + address);
						
						if(layer == allocator_->summary_height_) {
						#if BITMAP_CHUNK_ALLOCATOR_CHECK
							chunk_map_block_addr_ = allocator_->layer_start(layer) + address;
						#endif
							
							CMBlock &chunk_map_block = *reinterpret_cast<CMBlock*>(&summary_block_);
							chunk_address_.set_address(address * BlockMemory::BLOCK_SIZE * 8 / CHUNKS_PER_BLOCK);
							size_type offset = chunk_map_block.find(n_);
							if(offset == NO_ADDRESS) {
								DBG("no %ld free block in chunkmap at %ld", (long)n_, (long)address);
							}
							assert(offset != NO_ADDRESS);
							
							chunk_address_ += offset;
							summary_t before = chunk_map_block.summary();
							
							//if(n_ == 2) {
							//debug_->debug("before mark:");
							//debug_buffer<OsModel, 64>(debug_, (block_data_t*)(&chunk_map_block), 512);
							//}
							
							chunk_map_block.mark(offset, n_);
							
							//if(n_ == 2) {
							//debug_->debug("after mark:");
							//debug_buffer<OsModel, 64>(debug_, (block_data_t*)(&chunk_map_block), 512);
							//}
							
							
							allocator_->write_block(chunk_map_block, allocator_->layer_start(layer) + address);
							summary_t after = chunk_map_block.summary();
							if(after != before) { r = after; }
						}
						else {
							address_t offs = summary_block_.find(n_, ENTRIES_PER_SUMMARY_BLOCK);
							assert(offs != NO_ADDRESS);
							address_t next_address = address * ENTRIES_PER_SUMMARY_BLOCK + offs;
							summary_t s = allocate(layer + 1, next_address);
							if(s != NO_SUMMARY) {
								allocator_->read_block(summary_block_, allocator_->layer_start(layer) + address);
								summary_t before = summary_block_.summary(ENTRIES_PER_SUMMARY_BLOCK);
								
								summary_block_[offs] = s;
								allocator_->write_block(summary_block_, allocator_->layer_start(layer) + address);
								summary_t after = summary_block_.summary(ENTRIES_PER_SUMMARY_BLOCK);
								if(after != before) { r = after; }
							} // if s
						}
						
						return r;
					}
					
					ChunkAddress& chunk_address() { return chunk_address_; }
					
				#if BITMAP_CHUNK_ALLOCATOR_CHECK
					address_t chunk_map_block_addr_;
				#endif
					
					//Debug *debug_;
					
				private:
					SBlock &summary_block_;
					size_type n_;
					ChunkAddress chunk_address_;
					self_type *allocator_;
					
			};
			
			address_t layer_start(size_type layer) {
				switch(layer) {
					case 0:
						return 0;
						break;
					case 1:
						return 1;
						break;
					default: {
							address_t r = 1;
							size_type l = entries_in_root_block_;
							for(size_type i = 1; i < layer; i++) {
								r += l;
								l *= ENTRIES_PER_SUMMARY_BLOCK;
							}
							return r;
						}
						break;
				};
			}
			
			address_t layer_size(size_type layer) {
				if(layer == 0) { return 1; }
				else if(layer == summary_height_) { return chunk_map_blocks_; }
				
				size_type l = entries_in_root_block_;
				for(size_type i = 1; i < layer; i++) {
					l *= ENTRIES_PER_SUMMARY_BLOCK;
				}
				return l;
			}
			
			address_t root_;
			BlockMemory *block_memory_;
			typename Debug::self_pointer_t debug_;
			size_type entries_in_root_block_, chunk_map_blocks_, summary_height_, total_map_blocks_;
			size_type parent_size_;
		
	}; // BitmapChunkAllocator
	
}

#endif // BITMAP_CHUNK_ALLOCATOR_H

