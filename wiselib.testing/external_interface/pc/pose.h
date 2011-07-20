// vim: set noexpandtab ts=4 sw=4:

#ifndef POSE_H
#define POSE_H

namespace wiselib {

template<typename Angle_P, typename Distance_P>
class Pose {
	public:
		typedef Angle_P angle_t;
		typedef Distance_P distance_t;
		
		angle_t angle;
		
		struct {
			distance_t x, y;
		} position;
		
	private:
};

} // namespace wiselib

#endif // POSE_H

