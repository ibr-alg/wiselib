// vim: set noexpandtab ts=3 sw=3:

#ifndef SOFT_START_MOTION_H
#define SOFT_START_MOTION_H

#include "util/standalone_math.h"

namespace wiselib {

/**
 * Soft Start Motion - Moves with increasing velocity at start until the
 * target velocity is reached. Ripped from wiselib::ControlledMotion.
 * @tparam Robot_P Should implement @ref TurnWalkMotion_concept
 */
template<
  typename OsModel_P,
  typename Robot_P,
  typename Odometer_P = Robot_P,
  typename Timer_P = typename OsModel_P::Timer,
  typename Math_P = StandaloneMath<OsModel_P>,
  typename OsModel_P::size_t MAX_RECEIVERS = 16
  >
  class SoftStartMotion {
    public:
      typedef OsModel_P OsModel;
      typedef Robot_P Robot;
      typedef Odometer_P Odometer;
      typedef Timer_P Timer;
      typedef Math_P Math;

      typedef SoftStartMotion<OsModel, Robot, Odometer, Timer, Math, MAX_RECEIVERS> self_type;
      typedef self_type* self_pointer_t;

      typedef typename Odometer::distance_t distance_t;
      typedef typename Odometer::angle_t angle_t;

      typedef typename Robot::velocity_t velocity_t;
      typedef typename Robot::angular_velocity_t angular_velocity_t;

      typedef delegate0<void> stopped_delegate_t;
      typedef vector_static<OsModel, stopped_delegate_t, MAX_RECEIVERS> stopped_delegates_t;

      enum ErrorCodes {
        SUCCESS = OsModel::SUCCESS, ERR_UNSPEC = OsModel::ERR_UNSPEC
      };

      inline int init();
      inline int init(Robot&, Timer&);
      inline int init(Robot&, Odometer&, Timer&);

      inline int destruct();

      inline int move_distance(distance_t d, velocity_t v = Robot::PRECISE_VELOCITY);
      inline int turn_about(angle_t a, angular_velocity_t v = Robot::PRECISE_ANGULAR_VELOCITY);
      inline int turn_to(angle_t a, angular_velocity_t v = Robot::PRECISE_ANGULAR_VELOCITY);

      void on_state_change(int);
      void on_next_step(void*);

      template<typename T, void (T::*TMethod)(void)>
      inline int reg_stopped_callback(T*);
      inline int unreg_stopped_callback(typename OsModel::size_t);

      Robot& robot() { return *robot_; }

    private:
      enum Mode { NONE, ANGLE, DISTANCE };
      static const int SOFT_START_INTERVAL = 10000; // in milliseconds
	   static const unsigned int STEPS = 20; // TODO: increase this

      typename Robot::self_pointer_t robot_;
      typename Odometer::self_pointer_t odometer_;
      typename Timer::self_pointer_t timer_;

      Mode mode_;
      bool soft_stopping_;
      unsigned int step_, steps_total_;
      distance_t distance_, target_distance_, begin_stop_distance_;
      velocity_t velocity_, target_velocity_;
      angle_t angle_, target_angle_, begin_stop_angle_;
      angular_velocity_t angular_velocity_, target_angular_velocity_;
      bool angle_increasing_, distance_increasing_;

      stopped_delegates_t stopped_callbacks_;

      int on_state_change_idx_;
  }; // SoftStartMotion

template<typename OsModel_P, typename Robot_P, typename Odometer_P, typename Timer_P, typename Math_P, typename OsModel_P::size_t MAX_RECEIVERS>
int
SoftStartMotion<OsModel_P, Robot_P, Odometer_P, Timer_P, Math_P, MAX_RECEIVERS>::
init() {
  destruct();
  init(*robot_, *odometer_, *timer_);
  return SUCCESS;
}

template<typename OsModel_P, typename Robot_P, typename Odometer_P, typename Timer_P, typename Math_P, typename OsModel_P::size_t MAX_RECEIVERS>
int
SoftStartMotion<OsModel_P, Robot_P, Odometer_P, Timer_P, Math_P, MAX_RECEIVERS>::
init(Robot_P& robot, Timer_P& timer) {
  mode_ = NONE;
  robot_ = &robot;
  odometer_ = &robot;
  timer_ = &timer;
  step_ = steps_total_ = 0;
  soft_stopping_ = false;
  on_state_change_idx_ = odometer_->template register_state_callback<self_type, &self_type::on_state_change>(this);
  return SUCCESS;
}

template<typename OsModel_P, typename Robot_P, typename Odometer_P, typename Timer_P, typename Math_P, typename OsModel_P::size_t MAX_RECEIVERS>
int
SoftStartMotion<OsModel_P, Robot_P, Odometer_P, Timer_P, Math_P, MAX_RECEIVERS>::
init(Robot_P& robot, Odometer_P& odometer, Timer_P& timer) {
  mode_ = NONE;
  robot_ = &robot;
  odometer_ = &odometer;
  timer_ = &timer;
  step_ = steps_total_ = 0;
  soft_stopping_ = false;
  on_state_change_idx_ = odometer_->template register_state_callback<self_type, &self_type::on_state_change>(this);
  return SUCCESS;
}

template<typename OsModel_P, typename Robot_P, typename Odometer_P, typename Timer_P, typename Math_P, typename OsModel_P::size_t MAX_RECEIVERS>
int
SoftStartMotion<OsModel_P, Robot_P, Odometer_P, Timer_P, Math_P, MAX_RECEIVERS>::
destruct() {
  odometer_.unregister_state_callback(on_state_change_idx_);
  return SUCCESS;
}

template<typename OsModel_P, typename Robot_P, typename Odometer_P, typename Timer_P, typename Math_P, typename OsModel_P::size_t MAX_RECEIVERS>
int
SoftStartMotion<OsModel_P, Robot_P, Odometer_P, Timer_P, Math_P, MAX_RECEIVERS>::
move_distance(
  SoftStartMotion<OsModel_P, Robot_P, Odometer_P, Timer_P, Math_P, MAX_RECEIVERS>::distance_t distance,
  SoftStartMotion<OsModel_P, Robot_P, Odometer_P, Timer_P, Math_P, MAX_RECEIVERS>::velocity_t velocity
) {
  mode_ = DISTANCE;
  steps_total_ = STEPS;
  soft_stopping_ = false;
  target_velocity_ = velocity;
  distance_ = robot_->distance();
  target_distance_ = distance_ + distance;
  distance_increasing_ = (target_distance_ >= distance_);

  // first iteration
  step_ = 0;
  on_next_step(0);
  return SUCCESS;
}

template<typename OsModel_P, typename Robot_P, typename Odometer_P, typename Timer_P, typename Math_P, typename OsModel_P::size_t MAX_RECEIVERS>
int
SoftStartMotion<OsModel_P, Robot_P, Odometer_P, Timer_P, Math_P, MAX_RECEIVERS>::
turn_about(
  SoftStartMotion<OsModel_P, Robot_P, Odometer_P, Timer_P, Math_P, MAX_RECEIVERS>::angle_t angle,
  SoftStartMotion<OsModel_P, Robot_P, Odometer_P, Timer_P, Math_P, MAX_RECEIVERS>::angular_velocity_t velocity
) {
  return turn_to(angle_ + angle, velocity);
}

template<typename OsModel_P, typename Robot_P, typename Odometer_P, typename Timer_P, typename Math_P, typename OsModel_P::size_t MAX_RECEIVERS>
int
SoftStartMotion<OsModel_P, Robot_P, Odometer_P, Timer_P, Math_P, MAX_RECEIVERS>::
turn_to(
  SoftStartMotion<OsModel_P, Robot_P, Odometer_P, Timer_P, Math_P, MAX_RECEIVERS>::angle_t angle,
  SoftStartMotion<OsModel_P, Robot_P, Odometer_P, Timer_P, Math_P, MAX_RECEIVERS>::angular_velocity_t velocity
) {
  mode_ = ANGLE;
  steps_total_ = STEPS;
  soft_stopping_ = false;
  target_angular_velocity_ = velocity;
  target_angle_ = angle;

  if(target_angle_ < angle_) {
    target_angular_velocity_ = -target_angular_velocity_;
    angle_increasing_ = false;
  }
  else {
    angle_increasing_ = true;
  }

  // first iteration
  step_ = 0;
  on_next_step(0);
  return SUCCESS;
}

template<typename OsModel_P, typename Robot_P, typename Odometer_P, typename Timer_P, typename Math_P, typename OsModel_P::size_t MAX_RECEIVERS>
template<typename T, void (T::*TMethod)(void)>
int
SoftStartMotion<OsModel_P, Robot_P, Odometer_P, Timer_P, Math_P, MAX_RECEIVERS>::
reg_stopped_callback(T* obj) {
  if(stopped_callbacks_.empty()) {
    stopped_callbacks_.assign(MAX_RECEIVERS, stopped_delegate_t());
  }

  for(typename OsModel::size_t i=0; i<stopped_callbacks_.size(); i++) {
    if(stopped_callbacks_[i] == stopped_delegate_t()) {
      stopped_callbacks_[i] = stopped_delegate_t::template from_method<T, TMethod>(obj);
      return i;
    }
  }
  return -1;
}

template<typename OsModel_P, typename Robot_P, typename Odometer_P, typename Timer_P, typename Math_P, typename OsModel_P::size_t MAX_RECEIVERS>
int
SoftStartMotion<OsModel_P, Robot_P, Odometer_P, Timer_P, Math_P, MAX_RECEIVERS>::
unreg_stopped_callback(typename OsModel_P::size_t idx) {
  stopped_callbacks_[idx] = stopped_delegate_t();
  return OsModel::SUCCESS;
}

template<typename OsModel_P, typename Robot_P, typename Odometer_P, typename Timer_P, typename Math_P, typename OsModel_P::size_t MAX_RECEIVERS>
void
SoftStartMotion<OsModel_P, Robot_P, Odometer_P, Timer_P, Math_P, MAX_RECEIVERS>::
on_state_change(int state) {
  if(state == Odometer::DATA_AVAILABLE) {
    switch(mode_) {
      case NONE:
        break;

      case ANGLE:
        angle_ = odometer_->angle();
        if((angle_increasing_ && (angle_ >= target_angle_)) ||
          ((!angle_increasing_ && (angle_ <= target_angle_)))) {
          //printf("stopping!\n");
          robot_->stop();
          step_ = 0;
          soft_stopping_ = false;
        } else if((angle_increasing_ && (angle_ >= begin_stop_angle_)) ||
          ((!angle_increasing_ && (angle_ <= begin_stop_angle_)))) {
          if(!soft_stopping_) {
            printf("soft-stopping!\n");
            soft_stopping_ = true;
            --step_;
            on_next_step(0);
          }
        }
        break;

      case DISTANCE:
        distance_ = odometer_->distance();
        if((distance_increasing_ && (distance_ >= target_distance_)) ||
          ((!distance_increasing_ && (distance_ <= target_distance_)))) {
          //printf("stopping!\n");
          robot_->stop();
          step_ = 0;
          soft_stopping_ = false;
        } else if((distance_increasing_ && (distance_ >= begin_stop_distance_)) ||
          ((!distance_increasing_ && (distance_ <= begin_stop_distance_)))) {
          if(!soft_stopping_) {
            printf("soft-stopping!\n");
            soft_stopping_ = true;
            //--step_;
            on_next_step(0);
          }
        }
        break;
    } // switch
  } // DATA_AVAILABLE
} // on_state_change()

template<typename OsModel_P, typename Robot_P, typename Odometer_P, typename Timer_P, typename Math_P, typename OsModel_P::size_t MAX_RECEIVERS>
void
SoftStartMotion<OsModel_P, Robot_P, Odometer_P, Timer_P, Math_P, MAX_RECEIVERS>::
on_next_step(void * userdata) {
  //soft_stopping_ = static_cast<bool>(userdata);
  printf("on_next_step: stopping: %d", + soft_stopping_);

  switch(mode_) {
    case NONE:
    break;

    case ANGLE:
      // calculate angle where to begin with soft stop
      if(!soft_stopping_) {
        begin_stop_angle_ = target_angle_;
        for(int i = step_; i > 0; i--) {
          begin_stop_angle_ -= Math::degrees_to_radians((angular_velocity_ * i
            / step_) * (SOFT_START_INTERVAL / 1000 / step_));
          std::cout << "- (" << angular_velocity_ << "*" << i << "/" << step_
            << ") * (" << (SOFT_START_INTERVAL / 1000) << "/" << step_ << ") = "
            << begin_stop_angle_ << std::endl;
        }
        printf("cur angle: %f, stop angle: %f, target: %f\n", angle_, 
          begin_stop_angle_, target_angle_);
        
        if((angle_increasing_ && begin_stop_angle_ <= angle_) ||
          (!angle_increasing_ && begin_stop_angle_ >= angle_)) {
          soft_stopping_ = true; // stopping before end of soft start
          --step_;
          printf("stopping before end of soft start\n");
        } else {
          ++step_;
        }
      }
      if(soft_stopping_) {
        --step_;
      }
      if(step_ < 1 || step_ > steps_total_) {
        return;
      }

      angular_velocity_ = target_angular_velocity_ * step_ / steps_total_;
      printf("step %d: new angular velocity: %d mm/s, stop:%d\n", step_, angular_velocity_, soft_stopping_);
      timer_->template set_timer<self_type, &self_type::on_next_step>
        (SOFT_START_INTERVAL / steps_total_, this, (void *)soft_stopping_);
      //printf("new timer event in %d seconds\n",
      //  int(SOFT_START_INTERVAL / steps_total_));
      robot_->turn(angular_velocity_);
    break;

    case DISTANCE:
      // calculate distance where to begin with soft stop
      if(!soft_stopping_) {
        begin_stop_distance_ = target_distance_;
        for(int i = step_; i > 0; i--) {
          begin_stop_distance_ -= (velocity_ * i / step_) * 
            (SOFT_START_INTERVAL / 1000 / step_);
          std::cout << "- (" << velocity_ << "*" << i << "/" << step_ << 
            ") * (" << (SOFT_START_INTERVAL /1000) << "/" << step_ << ") = " 
            << begin_stop_distance_ << std::endl;
        }
        printf("cur dist: %f, stop dist: %f, target: %f\n", distance_, 
          begin_stop_distance_, target_distance_);
        
        if((distance_increasing_ && begin_stop_distance_ <= distance_) ||
          (!distance_increasing_ && begin_stop_distance_ >= distance_)) {
          soft_stopping_ = true; // stopping before end of soft start
          --step_;
          printf("stopping before end of soft start\n");
        } else {
          ++step_;
        }
      }
      if(soft_stopping_) {
        --step_;
      }
      if(step_ < 1 || step_ > steps_total_) {
        return;
      }
      velocity_ = target_velocity_ * step_ / steps_total_;
      printf("step %d: new drive velocity: %d mm/s\n", step_, velocity_);
      timer_->template set_timer<self_type, &self_type::on_next_step>
        (SOFT_START_INTERVAL / steps_total_, this, (void *)soft_stopping_);
      //printf("new timer event in %d seconds\n",
      //  int(SOFT_START_INTERVAL / steps_total_));
      robot_->move(distance_increasing_ ? velocity_ : -velocity_);
    break;
  }
}

};
// namespace

#endif // SOFT_START_MOTION_H
