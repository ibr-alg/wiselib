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

#ifndef NAP_CONTROL_H
#define NAP_CONTROL_H

namespace wiselib {
	
	/**
	 * @brief Control sleep state of the node.
	 * 
	 * 
	 * 
	 *            COFFEE !!!
	 *
	 * 
	 *                (
	 *             )  )  )
	 *            (  (  (
	 *            )  )  )
	 *            ,-----.
	 *           ' ~ ~ ~ `,
	 *          |\~ ~ ~ ~,|--.
	 *          | `-._.-' | )|
	 *          |         |_/
	 *          |         |
	 *          \         |
	 *           `-.___.-'
	 * 
	 * @ingroup
	 * 
	 * @tparam 
	 */
	template<
		typename OsModel_P,
		typename Radio_P
	>
	class NapControl {
		
		public:
			typedef OsModel_P OsModel;
			typedef typename OsModel::block_data_t block_data_t;
			typedef typename OsModel::size_t size_type;
			
			NapControl() : caffeine_(0) {
			}
			
			void init(typename Radio::self_pointer_t radio) {
				radio_ = radio;
				caffeine_ = 0;
			}
			
			/**
			 */
			void push_caffeine(void* = 0) {
				if(caffeine_level_ == 0) {
					radio_->enable_radio();
				}
				caffeine_level_++;
			}
			
			/**
			 */
			void pop_caffeine(void* = 0) {
				assert(caffeine_level_ > 0);
				caffeine_level_--;
				
				if(caffeine_level_ == 0) {
					radio_->disable_radio();
				}
			}
		
		private:
			size_type caffeine_;
			typename Radio::self_pointer_t radio_;
		
	}; // NapControl
}

#endif // NAP_CONTROL_H

