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

#ifndef CHUNK_MAP_BLOCK_H
#define CHUNK_MAP_BLOCK_H

#if (DEBUG_GRAPHVIZ || DEBUG_OSTREAM)
	#include <iostream>
#endif

#if DEBUG_ISENSE
	#include <isense/util/get_os.h>
#endif

#include <util/standalone_math.h>

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
		typename Summary_P,
		size_t CHUNK_SIZE_P
	>
	class ChunkMapBlock {
		public:
			typedef OsModel_P OsModel;
			typedef typename OsModel::block_data_t block_data_t;
			typedef typename OsModel::size_t size_type;
			typedef size_type word_t;
			typedef BlockMemory_P BlockMemory;
			typedef Summary_P summary_t;
			typedef StandaloneMath<OsModel> Math;
			
		private:
			enum {
				CHUNK_SIZE = CHUNK_SIZE_P,
				// chunks per block, equals bits in CM block per allocated block
				BLOCKBITS = BlockMemory::BLOCK_SIZE / CHUNK_SIZE,
				// blocks that are mapped with one chunk map block
				BLOCKBLOCKS = (BlockMemory::BLOCK_SIZE * 8) / BLOCKBITS,
				WORDBITS = sizeof(word_t) * 8,
				BLOCKWORDS = BLOCKBITS / WORDBITS
			};
			
		public:
			size_type find(size_type n) {
				// 1 -> free, 0 -> used
				for(size_type block = 0; block < BLOCKBLOCKS; block++) {
					size_type bits_left = BLOCKBITS;
					size_type bits_found = 0;
					size_type start = -1;
						
					for(size_type word = 0; word < BLOCKWORDS; word++) {
						word_t w = *reinterpret_cast<word_t*>(data_ + (word + block * BLOCKWORDS) * sizeof(word_t));
						if(w == (word_t)(-1)) {
							bits_found += WORDBITS;
							if(start == (size_type)(-1)) {
								start = block * BLOCKBITS + word * WORDBITS;
							}
							if(bits_found >= n) { return start; }
						}
						else {
							size_type pre = ones_prefix(w);
							size_type post = ones_postfix(w);
							bits_found += pre;
							if(pre && start == (size_type)(-1)) {
								start = block * BLOCKBITS + word * WORDBITS;
							}
							if(bits_found >= n) { return start; }
							bits_found = post;
							if(post) {
								start = block * BLOCKBITS + (word + 1) * WORDBITS - post;
								if(bits_found >= n) { return start; }
							}
							else {
								start = -1;
							}
						}
						
						bits_left -= WORDBITS;
						if(bits_found + bits_left < n) { break; }
					} // for word
				} // for block
				
				return -1;
			} // find(n)
			
			void mark(size_type offset, size_type n) {
				size_type pos = offset / WORDBITS;
				if(offset % WORDBITS) {
					//GET_OS.debug("offset %% WORDBITS != 0");
					word_t &w = *reinterpret_cast<word_t*>(data_ + pos * sizeof(word_t));
					// word 'starts' at lsb, ends at msb
					// set to 0 all bits from offset % WORDBITS to msb.
					w &= ~( (word_t)((((word_t)1) << (word_t)Math::min(n, WORDBITS - (offset % WORDBITS))) - 1UL) << (word_t)(offset % WORDBITS) );
					n -= Math::min(WORDBITS - (offset % WORDBITS), n);
					pos++;
				}
				
				size_type i = 0;
				for( ; i < n / WORDBITS; i++) {
					word_t &w = *reinterpret_cast<word_t*>(data_ + (pos + i) * sizeof(word_t));
					w = 0;
				}
				pos += i;
				if(n % WORDBITS) {
					word_t &w = *reinterpret_cast<word_t*>(data_ + pos * sizeof(word_t));
					// set to 0 all bits from lsb to n % WORDBITS
					w &= ~((1 << (n % WORDBITS)) - 1);
				}
				
				if(n > 0) {
					//assert(used(offset));
				}
			}
			
			void unmark(size_type offset, size_type n) {
				size_type pos = offset / WORDBITS;
				if(offset % WORDBITS) {
					word_t &w = *reinterpret_cast<word_t*>(data_ + pos * sizeof(word_t));
					// word 'starts' at lsb, ends at msb
					// set to 1 all bits from offset % WORDBITS to msb.
					//w |= ((1UL << Math::min(n, WORDBITS - (offset % WORDBITS))) - 1UL) << offset;
					w |= (word_t)((((word_t)1) << (word_t)Math::min(n, WORDBITS - (offset % WORDBITS))) - 1UL) << (word_t)(offset % WORDBITS);
					n -= Math::min(WORDBITS - (offset % WORDBITS), n);
					pos++;
				}
				
				size_type i = 0;
				for( ; i < n / WORDBITS; i++) {
					word_t &w = *reinterpret_cast<word_t*>(data_ + (pos + i) * sizeof(word_t));
					w = (word_t)(-1);
				}
				pos += i;
				if(n % WORDBITS) {
					word_t &w = *reinterpret_cast<word_t*>(data_ + pos * sizeof(word_t));
					// set to 1 all bits from lsb to n % WORDBITS
					w |= (1 << (n % WORDBITS)) - 1;
				}
			}
			
			bool used(size_type pos) {
				return !((data_[pos / 8] >> (pos % 8)) & 1);
			}
			
			
			summary_t summary() {
				summary_t s = 0;
				
				for(size_type block = 0; block < BLOCKBLOCKS; block++) {
					size_type bits_left = BLOCKBITS;
					size_type bits_found = 0;
						
					for(size_type word = 0; word < BLOCKWORDS; word++) {
						word_t w = *reinterpret_cast<word_t*>(data_ + (word + block * BLOCKWORDS) * sizeof(word_t));
						if(w == (word_t)(-1)) {
							bits_found += WORDBITS;
							if(bits_found > s) { s = bits_found; }
						}
						else {
							size_type pre = ones_prefix(w);
							size_type post = ones_postfix(w);
							
							bits_found += pre;
							if(bits_found > s) { s = bits_found; }
							bits_found = post;
							if(bits_found > s) { s = bits_found; }
						}
						
						bits_left -= WORDBITS;
						// we can't beat the current s value within this
						// blocks chunks anymore
						if(bits_found + bits_left <= s) { break; }
					} // for word
				} // for block
				
				return s;
			}
			
#if DEBUG_GRAPHVIZ
			void debug_graphviz_label(std::ostream& os, typename BlockMemory::address_t addr) {
				enum { W = 32 };
				
				os << "< <TABLE BORDER=\"0\" CELLSPACING=\"0\" CELLBORDER=\"1\">\n";
				os << "<TR><TD COLSPAN=\"" << W << "\"> #" << addr << "</TD></TR>\n";
				for(size_type row = 0; row < (BlockMemory::BLOCK_SIZE * 8 / W); row++) {
					os << "<TR>\n";
					for(size_type col = 0; col < W; col++) {
						if(used(row * W + col)) {
							os << "<TD BGCOLOR=\"red\" WIDTH=\"3\"></TD>\n";
						}
						else {
							os << "<TD BGCOLOR=\"white\" WIDTH=\"3\"></TD>\n";
						}
					} // for col
					os << "</TR>\n";
				}
				os << "</TABLE> >";
			}
#endif // DEBUG_GRAPHVIZ
			
			
#if (DEBUG_GRAPHVIZ || DEBUG_OSTREAM)
			void debug(std::ostream& os) {
				enum { G = 32, N = 4 };
				
				size_type bit = 0;
				
				while(bit < BlockMemory::BLOCK_SIZE * 8) {
					for(size_type n = 0; n < N; n++) {
						for(size_type g = 0; g < G; g++) {
							os << (used(bit++) ? "X" : ".");
						}
						os << " ";
					}
					os << std::endl;
				}
			}
#endif
		
		private:
			template<typename T>
			static T msb() { return (T)(1UL << (sizeof(T) * 8 - 1)); }

			template<typename T>
			static size_type ones_postfix(T w) {
				size_type r = 0;
				while(w & msb<T>()) {
					w <<= 1;
					r++;
				}
				return r;
			}
			
			template<typename T>
			static size_type ones_prefix(T w) {
				size_type r = 0;
				while(w & 1) {
					w >>= 1;
					r++;
				}
				return r;
			}
			
			block_data_t data_[BlockMemory::BUFFER_SIZE];
		
	}; // ChunkMapBlock
}

#endif // CHUNK_MAP_BLOCK_H

/* vim: set ts=3 sw=3 tw=78 noexpandtab :*/
