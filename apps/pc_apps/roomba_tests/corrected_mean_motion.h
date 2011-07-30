#ifndef MEDIAN_MOTION_H_
#define MEDIAN_MOTION_H_

#include "util/standalone_math.h"

namespace wiselib {

/**
 * Moves a Roomba by a specific error profile based on mean values of several
 * test runs.
 *
 * @tparam Robot_P Should implement @ref TurnWalkMotion_concept
 */
template<
  typename OsModel_P,
  typename Robot_P,
  typename Odometer_P = Robot_P,
  typename Math_P = StandaloneMath<OsModel_P>,
  typename OsModel_P::size_t MAX_RECEIVERS = 16
  >
  class CorrectedMeanMotion {
    public:
      typedef OsModel_P OsModel;
      typedef Robot_P Robot;
      typedef Odometer_P Odometer;
      typedef Math_P Math;

      typedef CorrectedMeanMotion<OsModel, Robot, Odometer, Math, MAX_RECEIVERS> self_type;
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

      /**
       * Floor profile
       */
      enum Profile {
        UNSPECIFIED = 0, CARPET, LAMINATE
      };

      inline int init();
      inline int init(Robot&);
      inline int init(Robot&, Odometer&);

      inline int destruct();

      inline void set_profile(Profile p) { profile_ = p; }
      inline int profile() { return profile_; }

      inline int move_distance(distance_t d, velocity_t v = Robot::PRECISE_VELOCITY);
      inline int turn_about(angle_t a, angular_velocity_t v = Robot::PRECISE_ANGULAR_VELOCITY);
      inline int turn_to(angle_t a, angular_velocity_t v = Robot::PRECISE_ANGULAR_VELOCITY);

      void on_state_change(int);

      template<typename T, void (T::*TMethod)(void)>
      inline int reg_stopped_callback(T*);
      inline int unreg_stopped_callback(typename OsModel::size_t);

      Robot& robot() { return *robot_; }

    private:
      enum Mode { NONE, ANGLE, DISTANCE };

      typename Robot::self_pointer_t robot_;
      typename Odometer::self_pointer_t odometer_;

      Mode mode_;
      Profile profile_;
      distance_t distance_, target_distance_;
      angle_t angle_, target_angle_;
      bool angle_increasing_, distance_increasing_;

      stopped_delegates_t stopped_callbacks_;

      int on_state_change_idx_;
  }; // CorrectedMeanMotion

template<typename OsModel_P, typename Robot_P, typename Odometer_P, typename Math_P, typename OsModel_P::size_t MAX_RECEIVERS>
int
CorrectedMeanMotion<OsModel_P, Robot_P, Odometer_P, Math_P, MAX_RECEIVERS>::
init() {
  destruct();
  init(*robot_, *odometer_);
  return SUCCESS;
}

template<typename OsModel_P, typename Robot_P, typename Odometer_P, typename Math_P, typename OsModel_P::size_t MAX_RECEIVERS>
int
CorrectedMeanMotion<OsModel_P, Robot_P, Odometer_P, Math_P, MAX_RECEIVERS>::
init(Robot_P& robot) {
  mode_ = NONE;
  robot_ = &robot;
  odometer_ = &robot;
  on_state_change_idx_ = odometer_->template register_state_callback<self_type, &self_type::on_state_change>(this);
  return SUCCESS;
}

template<typename OsModel_P, typename Robot_P, typename Odometer_P, typename Math_P, typename OsModel_P::size_t MAX_RECEIVERS>
int
CorrectedMeanMotion<OsModel_P, Robot_P, Odometer_P, Math_P, MAX_RECEIVERS>::
init(Robot_P& robot, Odometer_P& odometer) {
  mode_ = NONE;
  robot_ = &robot;
  odometer_ = &odometer;
  on_state_change_idx_ = odometer_->template register_state_callback<self_type, &self_type::on_state_change>(this);
  return SUCCESS;
}

template<typename OsModel_P, typename Robot_P, typename Odometer_P, typename Math_P, typename OsModel_P::size_t MAX_RECEIVERS>
int
CorrectedMeanMotion<OsModel_P, Robot_P, Odometer_P, Math_P, MAX_RECEIVERS>::
destruct() {
  odometer_.unregister_state_callback(on_state_change_idx_);
  return SUCCESS;
}

template<typename OsModel_P, typename Robot_P, typename Odometer_P, typename Math_P, typename OsModel_P::size_t MAX_RECEIVERS>
int
CorrectedMeanMotion<OsModel_P, Robot_P, Odometer_P, Math_P, MAX_RECEIVERS>::
move_distance(
  CorrectedMeanMotion<OsModel_P, Robot_P, Odometer_P, Math_P, MAX_RECEIVERS>::distance_t distance,
  CorrectedMeanMotion<OsModel_P, Robot_P, Odometer_P, Math_P, MAX_RECEIVERS>::velocity_t velocity
) {
  mode_ = DISTANCE;
  distance_ = robot_->distance();

  /**
   * [i][0]: intercept, [i][1]: input value, [i][2]: input velocity
   * with i: Profile enum value
   */
  static const distance_t distanceFit_[][3] = {
    { 0, 1, 1 }, // UNSPECIFIED
    { 2.578843, -0.005908, 0.079851 }, // CARPET
    { -3.840179, 0.004569, 0.143651 } // LAMINATE
  };
  distance_t correction = distanceFit_[profile_][0] + distanceFit_[profile_][1] * distance + distanceFit_[profile_][2] * velocity;

  target_distance_ = distance_ + distance - correction;
  /***FIXME**/printf("dist %f, velo %d, correcting by %f\n", distance, velocity, correction);/******/

  distance_increasing_ = (target_distance_ >= distance_);

  robot_->move(distance_increasing_ ? velocity : -velocity);
  return SUCCESS;
}

template<typename OsModel_P, typename Robot_P, typename Odometer_P, typename Math_P, typename OsModel_P::size_t MAX_RECEIVERS>
int
CorrectedMeanMotion<OsModel_P, Robot_P, Odometer_P, Math_P, MAX_RECEIVERS>::
turn_about(
  CorrectedMeanMotion<OsModel_P, Robot_P, Odometer_P, Math_P, MAX_RECEIVERS>::angle_t angle,
  CorrectedMeanMotion<OsModel_P, Robot_P, Odometer_P, Math_P, MAX_RECEIVERS>::angular_velocity_t velocity
) {
  return turn_to(angle_ + angle, velocity);
}

template<typename OsModel_P, typename Robot_P, typename Odometer_P, typename Math_P, typename OsModel_P::size_t MAX_RECEIVERS>
int
CorrectedMeanMotion<OsModel_P, Robot_P, Odometer_P, Math_P, MAX_RECEIVERS>::
turn_to(
  CorrectedMeanMotion<OsModel_P, Robot_P, Odometer_P, Math_P, MAX_RECEIVERS>::angle_t angle,
  CorrectedMeanMotion<OsModel_P, Robot_P, Odometer_P, Math_P, MAX_RECEIVERS>::angular_velocity_t velocity
) {
  mode_ = ANGLE;

  /**
   * [i][0]: intercept, [i][1]: input value, [i][2]: input velocity
   * with i: Profile enum value
   */
  static const angle_t angleFit_[][3] = {
    { 0, 1, 1 }, // UNSPECIFIED
    { 0.85640, -0.03630, 0.05016 }, // CARPET
    { 0.88015, -0.01415, 0.05357 } // LAMINATE
  };

  angle_t correction = angleFit_[profile_][0] + angleFit_[profile_][1] * angle + angleFit_[profile_][2] * velocity;
  target_angle_ = angle - Math::degrees_to_radians(correction);
  /**FIXME***/printf("angle %f, velo %d, correcting by %f\n", angle, velocity, correction);/******/

  if(target_angle_ < angle_) {
    velocity = -velocity;
    angle_increasing_ = false;
  }
  else {
    angle_increasing_ = true;
  }

  robot_->turn(velocity);
  return SUCCESS;
}

template<typename OsModel_P, typename Robot_P, typename Odometer_P, typename Math_P, typename OsModel_P::size_t MAX_RECEIVERS>
template<typename T, void (T::*TMethod)(void)>
int
CorrectedMeanMotion<OsModel_P, Robot_P, Odometer_P, Math_P, MAX_RECEIVERS>::
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

template<typename OsModel_P, typename Robot_P, typename Odometer_P, typename Math_P, typename OsModel_P::size_t MAX_RECEIVERS>
int
CorrectedMeanMotion<OsModel_P, Robot_P, Odometer_P, Math_P, MAX_RECEIVERS>::
unreg_stopped_callback(typename OsModel_P::size_t idx) {
  stopped_callbacks_[idx] = stopped_delegate_t();
  return OsModel::SUCCESS;
}

template<typename OsModel_P, typename Robot_P, typename Odometer_P, typename Math_P, typename OsModel_P::size_t MAX_RECEIVERS>
void
CorrectedMeanMotion<OsModel_P, Robot_P, Odometer_P, Math_P, MAX_RECEIVERS>::
on_state_change(int state) {
  if(state == Odometer::DATA_AVAILABLE) {
    angle_ = odometer_->angle();
    switch(mode_) {
      case NONE:
      break;

      case ANGLE:
      if((angle_increasing_ && (angle_ >= target_angle_)) ||
        ((!angle_increasing_ && (angle_ <= target_angle_)))
      ) {
        robot_->stop();
      }
      break;

      case DISTANCE:
      distance_ = odometer_->distance();
      if((distance_increasing_ && (distance_ >= target_distance_)) ||
        ((!distance_increasing_ && (distance_ <= target_distance_)))
      ) {
        robot_->stop();
      }
      break;
    } // switch
  } // DATA_AVAILABLE
} // on_state_change()

}; // namespace

#endif /* MEDIAN_MOTION_H_ */
