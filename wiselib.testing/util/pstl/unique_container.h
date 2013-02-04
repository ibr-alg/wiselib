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

#ifndef UNIQUE_CONTAINER_H
#define UNIQUE_CONTAINER_H

namespace wiselib {
	
	/**
	 * @brief Wrapper around a container that ensures values are not inserted
	 * if they already exist in the contanier (by using find()).
	 * 
	 * @ingroup
	 * 
	 * @tparam 
	 */
	template<
		typename Container_P
	>
	class UniqueContainer : public Container_P {
		
		public:
			typedef Container_P Container;
			typedef typename Container::value_type value_type;
			typedef typename Container::iterator iterator;
				
			typedef typename Container::OsModel OsModel;
			typedef typename OsModel::block_data_t block_data_t;
			typedef typename OsModel::size_t size_type;
			
			iterator insert(const value_type& v) {
				iterator it = Container::find(v);
				if(it != Container::end()) { return it; }
				return Container::insert(v);
			}
			
			iterator insert(iterator iter, const value_type& v) {
				iterator it = Container::find(v);
				if(it != Container::end()) { return it; }
				return Container::insert(iter, v);
			}
			
			iterator push_back(const value_type& v) {
				iterator it = Container::find(v);
				if(it != Container::end()) { return it; }
				return Container::push_back(v);
			}
			
			iterator push_front(const value_type& v) {
				iterator it = Container::find(v);
				if(it != Container::end()) { return it; }
				return Container::push_front(v);
			}
			
		private:
		
	}; // UniqueContainer
}

#endif // UNIQUE_CONTAINER_H

