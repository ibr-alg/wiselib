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

#ifndef SUMMARY_BLOCK_H
#define SUMMARY_BLOCK_H

#include <util/meta.h>

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
		typename Summary_P
	>
	class SummaryBlock {
		
		public:
			typedef OsModel_P OsModel;
			typedef typename OsModel::block_data_t block_data_t;
			typedef typename OsModel::size_t size_type;
			typedef Summary_P summary_t;
			typedef BlockMemory_P BlockMemory;
			enum { BUFFER_SIZE = BlockMemory::BUFFER_SIZE };
			
			size_type find(size_type n, size_type elements) {
				size_type best = -1;
				size_type best_index = -1;
				for(size_type i = 0; i < elements; i++) {
					if((*this)[i] >= n && (best == (size_type)(-1) || (*this)[i] < best)) {
						best = (*this)[i];
						best_index = i;
					}
				}
				assert(best != (size_type)(-1));
				return best_index;
			}
			
			summary_t& operator[](size_type i) {
				return *reinterpret_cast<summary_t*>(data_ + i * sizeof(summary_t));
			}
			
			summary_t summary(size_type elements) {
				summary_t s = 0;
				for(size_type i = 0; i < elements; i++) {
					if((*this)[i] > s) {
						s = (*this)[i];
					}
				}
				return s;
			}
			
#if DEBUG_GRAPHVIZ
			void debug_graphviz_label(std::ostream& os, typename BlockMemory::address_t addr) {
				os << "< <TABLE BORDER=\"0\" CELLSPACING=\"0\" CELLBORDER=\"1\">\n";
				os << "<TR><TD COLSPAN=\"8\"> #" << addr << "</TD></TR>\n";
				for(size_type row = 0; row < (BlockMemory::BLOCK_SIZE / (sizeof(summary_t) * 8)); row++) {
					os << "<TR>\n";
					for(size_type col = 0; col < 8; col++) {
						if((*this)[row * 8 + col] == 0) {
							os << "<TD BGCOLOR=\"red\">" << (unsigned int)(*this)[row * 8 + col] << "</TD>\n";
						}
						else {
							os << "<TD>" << (unsigned int)(*this)[row * 8 + col] << "</TD>\n";
						}
					} // for col
					os << "</TR>\n";
				}
				os << "</TABLE> >";
			}
#endif
	
#if (DEBUG_GRAPHVIZ || DEBUG_OSTREAM)		
			void debug(std::ostream& os) {
				enum { W = 8 };
				for(size_type row = 0; row < (BlockMemory::BLOCK_SIZE / (sizeof(summary_t) * W)); row++) {
					for(size_type col = 0; col < W; col++) {
						os << (int)(*this)[row * W + col] << " ";
					}
					os << std::endl;
				}
			}
#endif
			
		private:
			block_data_t data_[BUFFER_SIZE];
		
	}; // SummaryBlock
}

#endif // SUMMARY_BLOCK_H

