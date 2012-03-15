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

// vim: set noexpandtab ts=3 sw=3:

#ifndef POSITION_H
#define POSITION_H

namespace wiselib {
	template<
		typename OsModel_P,
		typename Real_P
	>
	class Position2D {
		public:
			typedef OsModel_P OsModel;
			typedef Real_P Real;
			
			Position2D() {
				x_ = 0;
				y_ = 0;
			}
			
			Position2D(Real x, Real y) {
				x_ = x;
				y_ = y;
			}
			
			Position2D(const Position2D& other) {
				x_ = other.x_;
				y_ = other.y_;
			}
			
			Position2D& operator=(const Position2D& other) {
				x_ = other.x_;
				y_ = other.y_;
				return *this;
			}
			
			Position2D operator+(const Position2D& other) const {
				return Position2D(x + other.x, y + other.y);
			}
			
			Position2D& operator+=(const Position2D& other) {
				x_ += other.x_;
				y_ += other.y_;
				return *this;
			}
			
			Position2D operator-(const Position2D& other) const {
				return Position2D(x_ - other.x_, y_ - other.y_);
			}
			
			Position2D& operator-=(const Position2D& other) {
				x_ -= other.x_;
				y_ -= other.y_;
				return *this;
			}
			
			Real x() const { return x_; }
			Real y() const { return y_; }
			
		private:
			Real x_, y_;
	};
	
} // namespace wiselib

#endif // POSITION_H

