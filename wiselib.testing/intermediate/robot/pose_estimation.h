// vim: set noexpandtab ts=3 sw=3:

#ifndef POSE_ESTIMATION_H
#define POSE_ESTIMATION_H

#include "util/standalone_math.h"
#include "position.h"

namespace wiselib {
	
	/**
	 * @tparam Odometer_P should implement Odometer_concept
	 * 
	 * @ingroup BasicAlgorithm_concept
	 * @ingroup Position_concept
	 * @ingroup Orientation_concept
	 * @ingroup BasicReturnValues_concept
	 */
	template<
		typename OsModel_P,
		typename Odometer_P,
		typename Position_P = Position2D<OsModel_P, double>,
		typename Math_P = StandaloneMath<OsModel_P>
	>
	class PoseEstimation {
		public:
			typedef OsModel_P OsModel;
			typedef Odometer_P Odometer;
			typedef Position_P position_t;
			typedef Math_P Math;
			typedef typename Odometer::angle_t angle_t;
			typedef typename Odometer::distance_t distance_t;
			typedef PoseEstimation<OsModel_P, Odometer_P, Position_P, Math_P> self_type;
			typedef self_type* self_pointer_t;
			
			enum ErrorCodes {
				SUCCESS = OsModel::SUCCESS, ERR_UNSPEC = OsModel::ERR_UNSPEC
			};
			
			PoseEstimation();
			
			int init();
			int init(Odometer&);
			int destruct();
			
			int reset_orientation(angle_t a = angle_t());
			int reset_position(position_t p = position_t());
			
			position_t operator()() { return position(); }
			position_t position() { return position_; }
			angle_t orientation() { return orientation_; }
			
			void on_state_change(int);
			
		private:
			position_t position_;
			angle_t orientation_;
			
			distance_t latest_distance_;
			angle_t latest_angle_;
			
			typename Odometer::self_pointer_t odometer_;
	};
	
	template<
		typename OsModel_P,
		typename Odometer_P,
		typename Position_P,
		typename Math_P
	>
	PoseEstimation<OsModel_P, Odometer_P, Position_P, Math_P>::
	PoseEstimation() : odometer_(0) {
	}
	
	template<
		typename OsModel_P,
		typename Odometer_P,
		typename Position_P,
		typename Math_P
	>
	int
	PoseEstimation<OsModel_P, Odometer_P, Position_P, Math_P>::
	init() {
		destruct();
		init(*odometer_);
		return SUCCESS;
	}
	
	template<
		typename OsModel_P,
		typename Odometer_P,
		typename Position_P,
		typename Math_P
	>
	int
	PoseEstimation<OsModel_P, Odometer_P, Position_P, Math_P>::
	init(Odometer_P& odometer) {
		latest_distance_ = 0;
		latest_angle_ = 0;
		odometer_ = &odometer;
		odometer_->template register_state_callback<self_type, &self_type::on_state_change>(this);
		return SUCCESS;
	}
	
	template<
		typename OsModel_P,
		typename Odometer_P,
		typename Position_P,
		typename Math_P
	>
	int
	PoseEstimation<OsModel_P, Odometer_P, Position_P, Math_P>::
	destruct() {
		// TODO: odometer->unregister_state_callback(idx);
		return SUCCESS;
	}
	
	template<
		typename OsModel_P,
		typename Odometer_P,
		typename Position_P,
		typename Math_P
	>
	int
	PoseEstimation<OsModel_P, Odometer_P, Position_P, Math_P>::
	reset_orientation(PoseEstimation<OsModel_P, Odometer_P, Position_P, Math_P>::angle_t a) {
		orientation_ = a;
		return SUCCESS;
	}
	
	template<
		typename OsModel_P,
		typename Odometer_P,
		typename Position_P,
		typename Math_P
	>
	int
	PoseEstimation<OsModel_P, Odometer_P, Position_P, Math_P>::
	reset_position(PoseEstimation<OsModel_P, Odometer_P, Position_P, Math_P>::position_t p) {
		position_ = p;
		return SUCCESS;
	}
	
	template<
		typename OsModel_P,
		typename Odometer_P,
		typename Position_P,
		typename Math_P
	>
	void
	PoseEstimation<OsModel_P, Odometer_P, Position_P, Math_P>::
	on_state_change(int state) {
		if(state == Odometer::DATA_AVAILABLE) {
			distance_t l = odometer_->distance() - latest_distance_;
			angle_t a = odometer_->angle() - latest_angle_;
			
			orientation_ += a;
			position_ += position_t(
				-l * Math::sin(orientation_),
				l * Math::cos(orientation_)
			);
			
			latest_distance_ = odometer_->distance();
			latest_angle_ = odometer_->angle();
		} // DATA_AVAILABLE
	} // on_state_change()
	
} // ns wiselib

#endif // POSE_ESTIMATION_H

