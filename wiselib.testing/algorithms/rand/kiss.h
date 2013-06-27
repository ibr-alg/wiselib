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

#ifndef KISS_H
#define KISS_H

namespace wiselib {
	
	/**
	 * @brief
	 * 
	 * @ingroup
	 * 
	 * @tparam 
	 */
	template<
		typename OsModel_P
	>
	class Kiss {
		
		public:
			typedef OsModel_P OsModel;
			typedef typename OsModel::block_data_t block_data_t;
			typedef typename OsModel::size_t size_type;
			typedef ::uint32_t rand_t;
			typedef ::uint32_t value_t;
			typedef Kiss<OsModel> self_type;
			typedef self_type* self_pointer_t;
			
			enum { RANDOM_MAX = (value_t)(-1) };
			
			Kiss()
				: x_(123456789), y_(362436000), z_(521288629), c_(7654321) {
			}
		
			void init() {
			}
			
			void srand(value_t seed) {
				x_ = seed;
			}
			
			value_t operator()() {
				uint64_t t;
				// linear congruency
				x_ = 69069 * x_ + 12345;

				// x_orshift
				y_ ^= y_ << 13;
				y_ ^= y_ >> 17;
				y_ ^= y_ << 5;

				// Multiply_-with-carry_
				t = 698769069ULL * z_ + c_;
				c_ = t >> 32;
				z_ = (uint32_t) t;

				return x_ + y_ + z_;
			}
			
		private:
			::uint32_t x_, y_, z_, c_;
	}; // Kiss
}

#endif // KISS_H

/* vim: set ts=4 sw=4 tw=78 noexpandtab :*/
