// vim: set noexpandtab ts=3 sw=3:

#ifndef CONDITIONAL_MOTION_H
#define CONDITIONAL_MOTION_H

#include "util/delegates/delegate.hpp"
#include "util/pstl/vector_static.h"

namespace wiselib {
	/**
	 * Moves a robot forward until a condition occurs.
	 * 
	 * Parameters:
	 * @tparam Robot_P should implement @ref TurnWalkMotion_concept.
	 * 
	 * @ingroup BasicAlgorithm_concept
	 */
	template<
		typename OsModel_P,
		typename Robot_P,
		typename Value_P = int,
		typename OsModel_P::size_t MAX_RECEIVERS = 16
	>
	class ConditionalMotion {
		public:
			typedef OsModel_P OsModel;
			typedef Robot_P Robot;
			typedef Value_P value_t;
			typedef ConditionalMotion<OsModel_P, Robot_P, Value_P, MAX_RECEIVERS> self_type;
			typedef self_type* self_pointer_t;
			typedef delegate0<value_t> condition_delegate_t;
			typedef delegate0<void> stop_delegate_t;
			typedef vector_static<OsModel, stop_delegate_t, MAX_RECEIVERS> stop_delegates_t;
			
			enum ErrorCodes {
				SUCCESS = OsModel::SUCCESS, ERR_UNSPEC = OsModel::ERR_UNSPEC
			};
			
			enum Mode { VALUE, NONZERO };
			
			int init();
			int init(Robot&);
			int destruct();
			
			/**
			 * Move forward until the condition occurs.
			 * Will return ERR_UNSPEC and not start motion
			 * when called before a condition has been set.
			 */
			int move();
			
			/**
			 * Check if the stop condition is fullfilled and stop the robot if
			 * so. See reg_condition() for details.
			 */
			int check_condition();
			
			/**
			 * Supposed to be used as a callback for register_state_callback at a
			 * robot. Will call check_condition() if the state indicates there is
			 * new data available.
			 */
			void on_state_change(int);
			
			/**
			 * Register condition method.
			 * When check_condition() is invoked and TMethod() returns
			 * the value stop_value, motion will be stopped.
			 * TMethod may be 0 in which case the condition will be assumed to
			 * hold true as soon as check_condition() is being called.
			 */
			template<typename T, value_t (T::*TMethod)(void)>
			int reg_condition(T* obj, Mode mode, value_t stop_value = value_t());
			
			/**
			 * The registered method will be called right after check_condition()
			 * has stopped the robot.
			 */
			template<typename T, void (T::*TMethod)(void)>
			int reg_stop_callback(T* obj);
			int unreg_stop_callback(typename OsModel::size_t idx);
			
		private:
			condition_delegate_t condition_;
			value_t condition_value_;
			Mode condition_mode_;
			bool active_;
			
			stop_delegates_t stop_callbacks_;
			typename Robot::self_pointer_t robot_;
			
			int notify_stop_receivers();
			int state_callback_idx_;
	};
	
	template<
		typename OsModel_P,
		typename Robot_P,
		typename Value_P,
		typename OsModel_P::size_t MAX_RECEIVERS
	>
	int
	ConditionalMotion<OsModel_P, Robot_P, Value_P, MAX_RECEIVERS>::
	init() {
		destruct();
		init(*robot_);
		return SUCCESS;
	}
	
	template<
		typename OsModel_P,
		typename Robot_P,
		typename Value_P,
		typename OsModel_P::size_t MAX_RECEIVERS
	>
	int
	ConditionalMotion<OsModel_P, Robot_P, Value_P, MAX_RECEIVERS>::
	init(Robot_P& robot) {
		active_ = false;
		robot_ = &robot;
		state_callback_idx_ = robot_->template register_state_callback<self_type, &self_type::on_state_change>(this);
		return SUCCESS;
	}
	
	template<
		typename OsModel_P,
		typename Robot_P,
		typename Value_P,
		typename OsModel_P::size_t MAX_RECEIVERS
	>
	int
	ConditionalMotion<OsModel_P, Robot_P, Value_P, MAX_RECEIVERS>::
	destruct() {
		robot_.unregister_state_callback(state_callback_idx_);
		return SUCCESS;
	}
	
	template<
		typename OsModel_P,
		typename Robot_P,
		typename Value_P,
		typename OsModel_P::size_t MAX_RECEIVERS
	>
	int
	ConditionalMotion<OsModel_P, Robot_P, Value_P, MAX_RECEIVERS>::
	move() {
		active_ = true;
		if(!condition_) {
			return ERR_UNSPEC;
		}
		robot_->move();
		return SUCCESS;
	}
	
	template<
		typename OsModel_P,
		typename Robot_P,
		typename Value_P,
		typename OsModel_P::size_t MAX_RECEIVERS
	>
	void
	ConditionalMotion<OsModel_P, Robot_P, Value_P, MAX_RECEIVERS>::
	on_state_change(int state) {
		if(state == Robot::DATA_AVAILABLE) {
			check_condition();
		}
	}
	
	template<
		typename OsModel_P,
		typename Robot_P,
		typename Value_P,
		typename OsModel_P::size_t MAX_RECEIVERS
	>
	int
	ConditionalMotion<OsModel_P, Robot_P, Value_P, MAX_RECEIVERS>::
	check_condition() {
		if(!active_) {
			return ERR_UNSPEC;
		}
		
		bool stop = true;
		
		if(condition_)
		switch(condition_mode_) {
			case VALUE:
				stop = (condition_() == condition_value_);
				break;
			case NONZERO:
				stop = static_cast<bool>(condition_());
				break;
		}
		
		if(stop) {
			active_ = false;
			robot_->stop();
			notify_stop_receivers();
		}
		return SUCCESS;
	}
	
	template<
		typename OsModel_P,
		typename Robot_P,
		typename Value_P,
		typename OsModel_P::size_t MAX_RECEIVERS
	>
	template<typename T, Value_P (T::*TMethod)(void)>
	int
	ConditionalMotion<OsModel_P, Robot_P, Value_P, MAX_RECEIVERS>::
	reg_condition(T* obj, Mode mode, value_t value) {
		condition_ = condition_delegate_t::template from_method<T, TMethod>(obj);
		condition_mode_ = mode;
		condition_value_ = value;
		return 0;
	}
	
	template<
		typename OsModel_P,
		typename Robot_P,
		typename Value_P,
		typename OsModel_P::size_t MAX_RECEIVERS
	>
	template<typename T, void (T::*TMethod)(void)>
	int
	ConditionalMotion<OsModel_P, Robot_P, Value_P, MAX_RECEIVERS>::
	reg_stop_callback(T* obj) {
		if(stop_callbacks_.empty()) {
			stop_callbacks_.assign(MAX_RECEIVERS, stop_delegate_t());
		}
		
		for(typename OsModel::size_t i=0; i<stop_callbacks_.size(); i++) {
			if(stop_callbacks_[i] == stop_delegate_t()) {
				stop_callbacks_[i] = stop_delegate_t::template from_method<T, TMethod>(obj);
				return i;
			}
		}
		return -1;
	} // reg_stop_callback()
	
	template<
		typename OsModel_P,
		typename Robot_P,
		typename Value_P,
		typename OsModel_P::size_t MAX_RECEIVERS
	>
	int
	ConditionalMotion<OsModel_P, Robot_P, Value_P, MAX_RECEIVERS>::
	unreg_stop_callback(typename OsModel_P::size_t idx) {
		stop_callbacks_[idx] = stop_delegate_t();
		return OsModel::SUCCESS;
	}
	
	template<
		typename OsModel_P,
		typename Robot_P,
		typename Value_P,
		typename OsModel_P::size_t MAX_RECEIVERS
	>
	int
	ConditionalMotion<OsModel_P, Robot_P, Value_P, MAX_RECEIVERS>::
	notify_stop_receivers() {
		typedef typename stop_delegates_t::iterator iter_t;
		
		for(iter_t iter = stop_callbacks_.begin(); iter != stop_callbacks_.end(); ++iter) {
			if(*iter != stop_delegate_t()) {
				(*iter)();
			}
		}
		return SUCCESS;
	} // notify_stop_receivers()

} // namespace wiselib

#endif // CONDITIONAL_MOTION_H

