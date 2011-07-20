// vim: set noexpandtab ts=4 sw=4:

#ifndef ROOMBA_ANGLE_H
#define ROOMBA_ANGLE_H

namespace wiselib {
	
	template<typename Math_P>
	class RoombaAngle {
		public:
			typedef Math_P Math;
			
			static const double GAUGE = 1.0;
			
			RoombaAngle() : radians_(0) { };
			RoombaAngle(double radians) : radians_(radians) { };
			
			RoombaAngle& operator=(RoombaAngle& other) {
				radians_ = other.radians_;
				return *this;
			}
			
			RoombaAngle& operator=(double r) {
				radians_ = r;
				return *this;
			}
			
			RoombaAngle& operator+=(double r) {
				radians_ += r;
				return *this;
			}
			
			operator double() { return radians_; }
			
			int16_t roomba_units_() {
				// Roomba uses a system where 1 unit equals 3 degrees
				return Math::round(Math::radians_to_degrees(radians_ / GAUGE)) % 120;
			}
			
			void set_roomba_units_(int16_t r) {
				radians_ = Math::degrees_to_radians(r * GAUGE);
			}
		private:
			double radians_;
	};
	
}

#endif // ROOMBA_ANGLE_H

