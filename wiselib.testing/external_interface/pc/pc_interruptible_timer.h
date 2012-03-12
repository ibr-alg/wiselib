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

// vim: set noexpandtab ts=4 sw=4:

#ifndef PC_INTERRUPTIBLE_TIMER_H
#define PC_INTERRUPTIBLE_TIMER_H

#include <time.h>
#include <errno.h>
#include <stdio.h>
#include <signal.h>
#include <pthread.h>
#include <sys/time.h>
#include <unistd.h>

#include <vector>
#include <iostream>

#include "util/delegates/delegate.hpp"
#include "pc_os.h"
#include "util/pstl/list_static.h"

#define PC_INTERRUPTIBLE_TIMER_DEBUG

namespace wiselib {
	template<typename OsModel_P, size_t MaxTimers_P>
	class PCInterruptibleTimerModel {
		public:
			typedef OsModel_P OsModel;
			typedef suseconds_t millis_t;
			typedef suseconds_t micros_t;
			typedef delegate1<void, void*> timer_delegate_t;
			typedef PCInterruptibleTimerModel<OsModel_P, MaxTimers_P> self_t;
			typedef self_t* self_pointer_t;
			
			enum Restrictions {
				MAX_TIMERS = MaxTimers_P
			};
			
			PCInterruptibleTimerModel();
			PCInterruptibleTimerModel(PCOs& os);
			
			template<typename T, void (T::*TMethod)(void*)>
			int set_timer(millis_t millis, T* obj, void* userdata);
			
			int sleep(millis_t duration) {
				return ssleep( duration );
			}

		private:
			struct Timer {
				timer_delegate_t callback_;
				micros_t offset_;
				void *userdata_;
			};

			typedef list_static<OsModel, Timer, MAX_TIMERS> timers_list_t;

			static int ssleep(millis_t millis) {
				timespec interval, remainder;

				interval.tv_sec = millis / 1000;
				interval.tv_nsec = (millis % 1000) * 1000000;

				// Unblock SIGALRM.
				sigset_t signal_set, old_signal_set;
				if ( ( sigemptyset( &signal_set ) == -1 ) ||
						( sigaddset( &signal_set, SIGALRM ) == -1 ) ||
						pthread_sigmask( SIG_UNBLOCK, &signal_set, &old_signal_set ) )
				{
					perror( "Failed to unblock SIGALRM" );
				}

				// nanosleep does not use SIGALRM and therefore does not interfer with the timer.
				while( ( nanosleep( &interval, &remainder ) == -1 ) && ( errno == EINTR ) ) {
					interval.tv_sec = remainder.tv_sec;
					interval.tv_nsec = remainder.tv_nsec;
				}

				// Block SIGALRM if it was blocked before.
				if( sigismember( &old_signal_set, SIGALRM ) == 1 ) {
					if ( ( sigemptyset( &signal_set ) == -1 ) ||
							( sigaddset( &signal_set, SIGALRM ) == -1 ) ||
							pthread_sigmask( SIG_BLOCK, &signal_set, 0 ) )
					{
						perror( "Failed to block SIGALRM" );
					}
				}

				return OsModel::SUCCESS;
			}
			
			// Sorted list of timers.
			// Time is stored relative, i.e. how many miliseconds after the
			// predecessor in the list the timer should fire
			static timers_list_t timers;
			static timers_list_t to_call;

			static void timer_handler_(int signum);
	}; // class PCInterruptibleTimerModel
	
	template<typename OsModel_P, size_t MaxTimers_P>
	typename PCInterruptibleTimerModel<OsModel_P, MaxTimers_P>::timers_list_t
	PCInterruptibleTimerModel<OsModel_P, MaxTimers_P>::timers;

	template<typename OsModel_P, size_t MaxTimers_P>
	typename PCInterruptibleTimerModel<OsModel_P, MaxTimers_P>::timers_list_t
	PCInterruptibleTimerModel<OsModel_P, MaxTimers_P>::to_call;


	template<typename OsModel_P, size_t MaxTimers_P>
	PCInterruptibleTimerModel<OsModel_P, MaxTimers_P>::PCInterruptibleTimerModel() {
		struct sigaction alarm_action;
		alarm_action.sa_handler = &PCInterruptibleTimerModel::timer_handler_;
		alarm_action.sa_flags = 0;

		if( ( sigemptyset( &alarm_action.sa_mask ) == -1 ) ||
			( sigaddset( &alarm_action.sa_mask, SIGALRM ) == -1 ) ||
			( sigaction( SIGALRM, &alarm_action, 0 ) == -1 ) )
		{
			perror( "Failed to install SIGALRM-handler" );
		}
	}
	
	template<typename OsModel_P, size_t MaxTimers_P>
	PCInterruptibleTimerModel<OsModel_P, MaxTimers_P>::PCInterruptibleTimerModel(PCOs& os) {
		struct sigaction alarm_action;
		alarm_action.sa_handler = &PCInterruptibleTimerModel::timer_handler_;
		alarm_action.sa_flags = 0;

		if( ( sigemptyset( &alarm_action.sa_mask ) == -1 ) ||
				( sigaddset( &alarm_action.sa_mask, SIGALRM ) == -1 ) ||
				( sigaction( SIGALRM, &alarm_action, 0 ) == -1 ) )
		{
			perror( "Failed to install SIGALRM-handler" );
		}
	}
	
	template<typename OsModel_P, size_t MaxTimers_P>
	template<typename T, void (T::*TMethod)(void*)>
	int PCInterruptibleTimerModel<OsModel_P, MaxTimers_P>::
	set_timer(millis_t millis, T* obj, void* userdata) {
		struct itimerval timer;

		// Call with millis==0 would deactivate the timer.
		if( millis <= 0 )
		{
			std::cerr << "WARNING: set_timer(): millis has to be at leat 1!" << std::endl;
			return OsModel::ERR_UNSPEC;
		}

		if( timers.full() )
		{
			std::cerr << "WARNING: Could not add timer. Timer-list is full!" << std::endl;
			return OsModel::ERR_UNSPEC;
		}

		// Block SIGALRM to avoid interrupting call of timer_handler.
		sigset_t signal_set, old_signal_set;
		if ( ( sigemptyset( &signal_set ) == -1 ) ||
				( sigaddset( &signal_set, SIGALRM ) == -1 ) ||
				pthread_sigmask( SIG_BLOCK, &signal_set, &old_signal_set ) )
		{
			perror( "Failed to block SIGALRM" );
			return OsModel::ERR_UNSPEC;
		}

		getitimer(ITIMER_REAL, &timer);
		
		// Update front timer value with the timer value, so sorting the new
		// timer in is more natural
		if(!timers.empty()) {
			timers.front().offset_ = timer.it_value.tv_sec * 1000000 + timer.it_value.tv_usec;
		}
		
		// insertion-sort the new timer
		micros_t t = 0, t_prev = 0;
		typename timers_list_t::iterator iter = timers.begin();
		for(; iter != timers.end(); ++iter) {
			t += iter->offset_;
			
			if(millis*1000 < t) {
				break;
			}
			
			t_prev = t;
		} // for
				
		Timer new_timer;
		new_timer.callback_ = timer_delegate_t::from_method<T, TMethod>(obj);
		new_timer.offset_ = millis*1000 - t_prev;
		new_timer.userdata_ = userdata;
		
		// fix following offset
		if(iter != timers.end()) {
			assert(iter->offset_ >= new_timer.offset_);
			iter->offset_ -= new_timer.offset_;
		}
		timers.insert(iter, new_timer);
		
		// Set the timer to the offset of the first entry
		timer.it_interval.tv_sec = 0;
		timer.it_interval.tv_usec = 0;
		timer.it_value.tv_sec = timers.front().offset_ / 1000000;
		timer.it_value.tv_usec = timers.front().offset_ % 1000000;

		if( ( timer.it_value.tv_sec == 0 ) && ( timer.it_value.tv_usec < 100 ) )
			timer.it_value.tv_usec = 100;

#ifdef PC_INTERRUPTIBLE_TIMER_DEBUG
		std::cout << " set_timer --> tvsec: "<< timer.it_value.tv_sec <<" --> usec: "<< timer.it_value.tv_usec << std::endl;
#endif

		setitimer(ITIMER_REAL, &timer, 0);

		//Unblock alrm-signal if necessary
		if(  sigismember( &old_signal_set, SIGALRM ) == 1 )
		{
			if ( ( sigemptyset( &signal_set ) == -1 ) ||
					( sigaddset( &signal_set, SIGALRM ) == -1 ) ||
					pthread_sigmask( SIG_UNBLOCK, &signal_set, 0 ) )
			{
				perror( "Failed to unblock SIGALRM" );
				return OsModel::ERR_UNSPEC;
			}
		}

		return OsModel::SUCCESS;
	}
	
	template<typename OsModel_P, size_t MaxTimers_P>
	void PCInterruptibleTimerModel<OsModel_P, MaxTimers_P>::
	timer_handler_(int signum) {
#ifdef PC_INTERRUPTIBLE_TIMER_DEBUG
		std::cout << "[ ";
#endif

		int save_errno = errno;

		sigset_t signal_set;
		struct itimerval timer;
		Timer current;

		bool finished = false;
		while( !finished )
		{
			if( timers.empty() ) {
				finished = true;
			} else {
				getitimer(ITIMER_REAL, &timer);
				micros_t offset =  timer.it_value.tv_sec * 1000000 + timer.it_value.tv_usec;

#ifdef PC_INTERRUPTIBLE_TIMER_DEBUG
				std::cout << "offset: " << offset << std::endl;
#endif

				if( offset > 100 ) {
					finished = true;
				} else {
					current = timers.front();
					timers.pop_front();

					if( to_call.full() ) {
						std::cout << "Warning: Could not execute timer-callback because container to_call is full." << std::endl;
					} else {
						to_call.push_back( current );
					}

					if( !timers.empty() ) {
						timer.it_interval.tv_sec = 0;
						timer.it_interval.tv_usec = 0;
						timer.it_value.tv_sec = timers.front().offset_ / 1000000;
						timer.it_value.tv_usec = timers.front().offset_ % 1000000;

						if( ( timer.it_value.tv_sec == 0 ) && ( timer.it_value.tv_usec < 100 ) )
							timer.it_value.tv_usec = 100;

#ifdef PC_INTERRUPTIBLE_TIMER_DEBUG
						std::cout << " --> tvsec: "<< timer.it_value.tv_sec <<" --> usec: "<< timer.it_value.tv_usec << std::endl;
#endif

						setitimer(ITIMER_REAL, &timer, 0);
					}
				}
			}
		}

		finished = false;
		while( !finished )
		{
			// Block SIGALRM to avoid interrupting call of timer_handler.
			if ( ( sigemptyset( &signal_set ) == -1 ) ||
					( sigaddset( &signal_set, SIGALRM ) == -1 ) ||
					pthread_sigmask( SIG_BLOCK, &signal_set, 0 ) )
			{
				perror( "Failed to block SIGALRM" );
			}

			if( to_call.empty() )
				finished = true;
			else
			{
				current = to_call.front();
				to_call.pop_front();
			}

			// Unblock SIGALRM.
			if ( ( sigemptyset( &signal_set ) == -1 ) ||
					( sigaddset( &signal_set, SIGALRM ) == -1 ) ||
					pthread_sigmask( SIG_UNBLOCK, &signal_set, 0 ) )
			{
				perror( "Failed to unblock SIGALRM" );
			}

			if( !finished )
			{
#ifdef PC_INTERRUPTIBLE_TIMER_DEBUG
				std::cout << "(" << std::endl;
#endif
				current.callback_(current.userdata_);
#ifdef PC_INTERRUPTIBLE_TIMER_DEBUG
				std::cout << ")" << std::endl;
#endif
			}
		}

		errno = save_errno;

#ifdef PC_INTERRUPTIBLE_TIMER_DEBUG
		std::cout << "]" << std::endl;
#endif
	}
	
} // namespace wiselib

#endif // PC_INTERRUPTIBLE_TIMER_H

