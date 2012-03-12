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

#ifndef GLOBAL_POINTER_H
#define GLOBAL_POINTER_H

namespace wiselib {

	template<
		typename T
	>
	class GlobalPointer {
		public:
			GlobalPointer() { }
			GlobalPointer(int) { }
			GlobalPointer(const GlobalPointer& other) { }
			GlobalPointer& operator=(const GlobalPointer& other) { return *this; }
			T& operator*() const { return instance; }
			T* operator->() const { return &instance; }
			operator bool() { return true; }
			/*
			T& operator[](size_t idx) { return p_[idx]; }
			const T& operator[](size_t idx) const { return p_[idx]; }
			bool operator==(const GlobalPointer& other) const { return p_ == other.p_; }
			bool operator!=(const GlobalPointer& other) const { return p_ != other.p_; }
			operator bool() const { return p_ != 0; }
			GlobalPointer& operator++() { ++p_; return *this; }
			GlobalPointer& operator--() { --p_; return *this; }
			GlobalPointer operator + (size_t i){return GlobalPointer(p_+i);}
			*/
			static T instance;
		private:
			char _[0];
	} __attribute__((__packed__));

	//template<typename T>
	//T GlobalPointer<T>::instance;
}

#endif // GLOBAL_POINTER_H
// vim: set ts=3 sw=3:
