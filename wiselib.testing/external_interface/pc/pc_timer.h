// vim: set noexpandtab ts=4 sw=4:

#ifndef PC_TIMER_H
#define PC_TIMER_H

#include <time.h>
#include <err.h>
#include <stdarg.h>
#include <errno.h>
#include <signal.h>
#include <pthread.h>
#include <sys/time.h>
#include <unistd.h>

#include <vector>
#include <iostream>

#include "util/delegates/delegate.hpp"
#include "pc_os.h"
#include "util/pstl/list_static.h"

namespace wiselib {
	
	template<typename OsModel_P, size_t MaxTimers_P>
	class TimerQueue {
		public:
			typedef TimerQueue<OsModel_P, MaxTimers_P> self_t;
			typedef suseconds_t micros_t;
			typedef suseconds_t millis_t;
			typedef delegate1<void, void*> timer_delegate_t;
			typedef OsModel_P OsModel;
			
			enum Restrictions {
				MAX_TIMERS = MaxTimers_P
			};
			
			
			enum { SUCCESS = OsModel::SUCCESS, ERR_UNSPEC = OsModel::ERR_UNSPEC };
			
			TimerQueue();
			
			int insert(micros_t interval, timer_delegate_t callback, void* userdata);
			int lock();
			int unlock();
			int from_itimer(struct itimerval& timer);
			int to_itimer(struct itimerval& timer);
			bool has_event();
			timer_delegate_t current_callback();
			void* current_userdata();
			int pop();
			
			size_t size() { return data_.size(); }
			void debug();
			
		private:
			struct Timer {
				timer_delegate_t callback_;
				micros_t offset_;
				void *userdata_;
			};
			typedef list_static<OsModel, typename self_t::Timer, MAX_TIMERS> timers_list_t;
			
			timers_list_t data_;
			bool locked_;
	};
	
	template<typename OsModel_P, size_t MaxTimers_P>
	class PCTimerModel {
		public:
			typedef OsModel_P OsModel;
			typedef suseconds_t millis_t;
			typedef suseconds_t micros_t;
			typedef delegate1<void, void*> timer_delegate_t;
			typedef PCTimerModel<OsModel_P, MaxTimers_P> self_t;
			typedef self_t* self_pointer_t;
			
			enum Restrictions {
				MAX_TIMERS = MaxTimers_P
			};
			enum { SUCCESS = OsModel::SUCCESS, ERR_UNSPEC = OsModel::ERR_UNSPEC };
			
			PCTimerModel();
			PCTimerModel(PCOs& os);
			
			template<typename T, void (T::*TMethod)(void*)>
			int set_timer(millis_t millis, T* obj, void* userdata);
			
			int sleep(millis_t duration) {
				return ssleep( duration );
			}
			
		private:
			static int ssleep(millis_t millis) {
				timespec interval, remainder;
				
				interval.tv_sec = millis / 1000;
				interval.tv_nsec = (millis % 1000) * 1000000;
				
				// nanosleep does not use SIGALRM and therefore does not interfer with the timer.
				while((nanosleep(&interval, &remainder) == -1) && (errno == EINTR)) {
					interval.tv_sec = remainder.tv_sec;
					interval.tv_nsec = remainder.tv_nsec;
				}
				
				return OsModel::SUCCESS;
			}
			
			static TimerQueue<OsModel_P, MaxTimers_P> queue_;
			static void timer_handler_(int signum);
			
			/**
			 * true iff the currently getitimer() returning a timer with zero
			 * interval does not mean, a new timer event should be fired.
			 * (I.e. that event has already been accounted for).
			 */
			static bool itimer_active_;
	}; // class PCTimerModel
	
	//
	// Implementation TimerQueue
	//

	template<typename OsModel_P, size_t MaxTimers_P>
	TimerQueue<OsModel_P, MaxTimers_P>::TimerQueue() : locked_(false) {
	}
		
	template<typename OsModel_P, size_t MaxTimers_P>
	int TimerQueue<OsModel_P, MaxTimers_P>::insert(
		TimerQueue<OsModel_P, MaxTimers_P>::micros_t interval,
		TimerQueue<OsModel_P, MaxTimers_P>::timer_delegate_t callback,
		void* userdata
	) {
		if(data_.full()) {
			return ERR_UNSPEC;
		}
		
		micros_t t = 0, t_prev = 0;
		
		// Find place in list to insert timer (keep list sorted all times)
		typename timers_list_t::iterator iter = data_.begin();
		for(; iter != data_.end(); ++iter) {
			t += iter->offset_;
			if(interval < t) { break; }
			t_prev = t;
		}
		
		// Create list element
		Timer new_timer;
		new_timer.callback_ = callback;
		new_timer.offset_ = interval - t_prev;
		new_timer.userdata_ = userdata;
		
		// Fix following offset
		if(iter != data_.end()) {
			assert(iter->offset_ >= new_timer.offset_);
			iter->offset_ -= new_timer.offset_;
		}
		data_.insert(iter, new_timer);
		
		return SUCCESS;
	}

	template<typename OsModel_P, size_t MaxTimers_P>
	int TimerQueue<OsModel_P, MaxTimers_P>::lock() {
		if(locked_) {
			return ERR_UNSPEC;
		}
		locked_ = true;
		return SUCCESS;
	}

	template<typename OsModel_P, size_t MaxTimers_P>
	int TimerQueue<OsModel_P, MaxTimers_P>::unlock() {
		locked_ = false;
		return SUCCESS;
	}

	template<typename OsModel_P, size_t MaxTimers_P>
	int TimerQueue<OsModel_P, MaxTimers_P>::from_itimer(struct itimerval& timer) {
		if(!data_.empty()) {
			data_.front().offset_ = timer.it_value.tv_sec * 1000000 + timer.it_value.tv_usec;
		}
		return SUCCESS;
	}

	template<typename OsModel_P, size_t MaxTimers_P>
	int TimerQueue<OsModel_P, MaxTimers_P>::to_itimer(struct itimerval& timer) {
		timer.it_interval.tv_sec = 0;
		timer.it_interval.tv_usec = 0;
		timer.it_value.tv_sec = data_.front().offset_ / 1000000;
		timer.it_value.tv_usec = data_.front().offset_ % 1000000;
		return SUCCESS;
	}
	
	template<typename OsModel_P, size_t MaxTimers_P>
	bool TimerQueue<OsModel_P, MaxTimers_P>::has_event() {
		return !data_.empty() && (data_.front().offset_ == 0);
	}
	
	template<typename OsModel_P, size_t MaxTimers_P>
	typename TimerQueue<OsModel_P, MaxTimers_P>::timer_delegate_t TimerQueue<OsModel_P, MaxTimers_P>::current_callback() {
		return data_.front().callback_;
	}
	
	template<typename OsModel_P, size_t MaxTimers_P>
	void* TimerQueue<OsModel_P, MaxTimers_P>::current_userdata() {
		return data_.front().userdata_;
	}
	
	template<typename OsModel_P, size_t MaxTimers_P>
	int TimerQueue<OsModel_P, MaxTimers_P>::pop() {
		if(!data_.empty()) {
			data_.pop_front();
		}
		return SUCCESS;
	}
	
	template<typename OsModel_P, size_t MaxTimers_P>
	void TimerQueue<OsModel_P, MaxTimers_P>::debug() {
		typename timers_list_t::iterator iter(data_.begin());
		for(; iter != data_.end(); iter++) {
			std::cout << iter->offset_ << " ";
		}
		if(has_event()) {
			std::cout << "[has_event] ";
		}
		if(data_.full()) {
			std::cout << "[FULL!] ";
		}
		std::cout << std::endl;
	}
	
	//
	// Implementation PCTimerModel
	//
	
	template<typename OsModel_P, size_t MaxTimers_P>
	TimerQueue<OsModel_P, MaxTimers_P>
	PCTimerModel<OsModel_P, MaxTimers_P>::queue_;
	
	template<typename OsModel_P, size_t MaxTimers_P>
	bool
	PCTimerModel<OsModel_P, MaxTimers_P>::itimer_active_;

	template<typename OsModel_P, size_t MaxTimers_P>
	PCTimerModel<OsModel_P, MaxTimers_P>::PCTimerModel() {
		struct sigaction alarm_action;
		alarm_action.sa_handler = &PCTimerModel::timer_handler_;
		alarm_action.sa_flags = 0;

		if((sigemptyset(&alarm_action.sa_mask) == -1) ||
			(sigaddset(&alarm_action.sa_mask, SIGALRM) == -1) ||
			(sigaction(SIGALRM, &alarm_action, 0) == -1)
		) {
			perror("Failed to install SIGALRM-handler");
		}
		itimer_active_ = false;
	}
	
	template<typename OsModel_P, size_t MaxTimers_P>
	PCTimerModel<OsModel_P, MaxTimers_P>::PCTimerModel(PCOs& os) {
		struct sigaction alarm_action;
		alarm_action.sa_handler = &PCTimerModel::timer_handler_;
		alarm_action.sa_flags = 0;

		if((sigemptyset(&alarm_action.sa_mask) == -1) ||
				(sigaddset(&alarm_action.sa_mask, SIGALRM) == -1) ||
				(sigaction(SIGALRM, &alarm_action, 0) == -1)
		) {
			perror("Failed to install SIGALRM-handler");
		}
		itimer_active_ = false;
	}
	
	template<typename OsModel_P, size_t MaxTimers_P>
	template<typename T, void (T::*TMethod)(void*)>
	int PCTimerModel<OsModel_P, MaxTimers_P>::
	set_timer(millis_t millis, T* obj, void* userdata) {
		struct itimerval timer;
		
		if(millis < 1) {
			return ERR_UNSPEC;
		}
		
		queue_.lock();
		
		if(getitimer(ITIMER_REAL, &timer) == -1) {
			perror("Error on getitimer()");
		}
		
		if(itimer_active_) {
			queue_.from_itimer(timer);
		}
		
		if(queue_.insert(millis * 1000, timer_delegate_t::from_method<T, TMethod>(obj), userdata) == ERR_UNSPEC) {
			queue_.debug();
			queue_.unlock();
			return ERR_UNSPEC;
		}
		
		// handle missed events
		while(queue_.has_event()) {
			std::cout << "handling missing event" << std::endl;
			
			timer_delegate_t callback = queue_.current_callback();
			void *userdata = queue_.current_userdata();
			queue_.pop();
			
			queue_.unlock();
			callback(userdata);
			if(queue_.lock() == ERR_UNSPEC) {
				errx(1, "timer handler didn't release timer queue lock!");
			}
		}
		
		queue_.to_itimer(timer);
		if(setitimer(ITIMER_REAL, &timer, 0) == -1) {
			perror("setitimer() failed");
		}
		itimer_active_ = true;
		
		queue_.unlock();
		
		return OsModel::SUCCESS;
	}
	
	template<typename OsModel_P, size_t MaxTimers_P>
	void PCTimerModel<OsModel_P, MaxTimers_P>::
	timer_handler_(int signum) {
		int save_errno = errno;
		
		// In case set_timer is currently using the queue, just do nothing,
		// it will handle our callbacks when it is done.
		if(queue_.lock() == ERR_UNSPEC) {
			std::cout << "missing event" << std::endl;
			errno = save_errno;
			return;
		}
		
		struct itimerval timer;
		if(getitimer(ITIMER_REAL, &timer) == -1) {
			perror("getitimer() failed");
		}
		itimer_active_ = false;
		queue_.from_itimer(timer);
		
		while(queue_.has_event()) {
			timer_delegate_t callback = queue_.current_callback();
			void *userdata = queue_.current_userdata();
			queue_.pop();
			
			queue_.unlock();
			callback(userdata);
			if(queue_.lock() == ERR_UNSPEC) {
				errx(1, "timer handler didn't release timer queue lock!");
			}
		}
		
		queue_.to_itimer(timer);
		queue_.unlock();
		
		if(setitimer(ITIMER_REAL, &timer, 0) == -1) {
			perror("setitimer() failed");
		}
		itimer_active_ = true;
		
		errno = save_errno;
	}

} // namespace wiselib

#endif // PC_TIMER_H

