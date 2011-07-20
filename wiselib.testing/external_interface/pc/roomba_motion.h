// vim: set noexpandtab ts=4 sw=4:

#ifndef ROOMBA_MOTION_H
#define ROOMBA_MOTION_H

#include "external_interface/pc/pose.h"

#include <iostream>

namespace wiselib {

template<typename Roomba_P, typename Math_P>
class RoombaMotion {
	public:
		typedef Roomba_P Roomba;
		typedef Math_P Math;
		
		typedef typename Roomba::distance_t distance_t;
		typedef typename Roomba::angle_t angle_t;
		typedef Pose<angle_t, distance_t> pose_t;
		typedef typename Roomba::velocity_t velocity_t;
		typedef typename Roomba::angular_velocity_t angular_velocity_t;
		
		typedef RoombaMotion<Roomba_P, Math_P> self_t;
		
		enum { DEFAULT_VELOCITY = 50, MAX_VELOCITY = 100};
		enum { DEFAULT_ANGULAR_VELOCITY = 20, MAX_ANGULAR_VELOCITY = 100};
		enum State {
			STATE_STOPPED, STATE_MOVING
		};
		
		void init(Roomba& roomba);
		void init();
		void destruct();
		
		/// Drive straight with given velocity
		void move(velocity_t v = DEFAULT_VELOCITY) {
			roomba_->move(v);
		}
		
		/// Like move(), but stop after given distance
		void move_distance(distance_t d) {
			accum_distance_ = 0;
			move();
			while(accum_distance_ < d) ;
			stop();
		}
		
		/// Like move(), but stop when hitting a wall
		void move_to_wall() {
			touched_wall_ = false;
			move();
			while(!touched_wall_) ;
			stop();
		}
		
		/// Turn counter-clockwise with given angular velocity
		void turn(angular_velocity_t v = DEFAULT_ANGULAR_VELOCITY) {
			roomba_->turn(v);
		}
		
		/// Like turn() but turn about given angle
		void turn_about(angle_t a) {
			angle_t old = a;
			accum_angle_ = 0;
			
			if(a < 0) {
				turn(-DEFAULT_ANGULAR_VELOCITY);
				while(accum_angle_ > a) {
					if(accum_angle_ != old) {
						old = accum_angle_;
					}
				}
			}
			else {
				turn();
				while(accum_angle_ < a) {
					if(accum_angle_ != old) {
						old = accum_angle_;
					}
				}
			}
			
			std::cout << "stopping!\n";
			
			stop();
		}
		
		/// Like turn() but turn until facing a certain angle (as reported by
		/// internal pose estimation)
		void turn_to(angle_t a) {
			flush_sensor_data();
			turn_about(a - pose_.angle);
		}
		
		/// Get current state
		State state();
		
		/// Stop current motion
		void stop() { roomba_->stop(); }
	
		/// Return moved distance since last call to this function
		distance_t distance() { return roomba_->distance(); }

		//void drive_arc(); // ?
		
		/// Estimate current pose
		pose_t pose() { return pose_; }
		
	private:
		
		static angle_t make_positive(angle_t a) {
			while(a < 0) {
				a += 2.0 * Math::PI;
			}
			return a;
		}
		
		void flush_sensor_data() {
			flush_ = false;
			while(!flush_) ;
		}
		
		void on_new_sensor_data() {
			touched_wall_ |= roomba_->wall();
			accum_distance_ += roomba_->distance();
			accum_angle_ += roomba_->angle();
			pose_.angle += roomba_->angle();
			
			pose_.position.x = roomba_->distance() * Math::sin(Math::radians_to_degrees(roomba_->angle()));
			pose_.position.y = roomba_->distance() * Math::cos(Math::radians_to_degrees(roomba_->angle()));
			flush_ = true;
		}
		
		Roomba* roomba_;
		pose_t pose_;
		distance_t accum_distance_;
		angle_t accum_angle_;
		bool touched_wall_;
		bool flush_;
};

template<typename Roomba_P, typename Math_P>
void RoombaMotion<Roomba_P, Math_P>::init(
		RoombaMotion<Roomba_P, Math_P>::Roomba& roomba
		) {
	roomba_ = &roomba;
	init();
}

template<typename Roomba_P, typename Math_P>
void RoombaMotion<Roomba_P, Math_P>::init() {
	roomba_->template reg_new_data_callback<self_t, &self_t::on_new_sensor_data>(this);
}

template<typename Roomba_P, typename Math_P>
void RoombaMotion<Roomba_P, Math_P>::destruct() {
	roomba_->unreg_new_data_callback();
}

} // namespace wiselib

#endif // ROOMBA_MOTION_H

