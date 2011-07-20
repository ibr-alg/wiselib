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

