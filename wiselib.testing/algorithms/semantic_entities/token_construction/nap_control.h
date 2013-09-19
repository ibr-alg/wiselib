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

#ifndef NAP_CONTROL_H
#define NAP_CONTROL_H

#if defined(CONTIKI)
	#include <contiki.h>
	#include <netstack.h>
#endif

namespace wiselib {
	
	/**
	 * @brief Control sleep state of the node.
	 * 
	 * 
	 * 
	 *            COFFEE !!!
	 *
	 * 
	 *                (
	 *             )  )  )
	 *            (  (  (
	 *            )  )  )
	 *            ,-----.
	 *           ' ~ ~ ~ `,
	 *          |\~ ~ ~ ~,|--.
	 *          | `-._.-' | )|
	 *          |         |_/
	 *          |         |
	 *          \         |
	 *           `-.___.-'
	 * 
	 * @ingroup
	 * 
	 * @tparam 
	 */
	template<
		typename OsModel_P,
		typename Radio_P,
		typename Debug_P = typename OsModel_P::Debug,
		typename Clock_P = typename OsModel_P::Clock
	>
	class NapControl {
		public:
			typedef NapControl self_type;
			typedef self_type* self_pointer_t;
			
			typedef OsModel_P OsModel;
			typedef typename OsModel::block_data_t block_data_t;
			typedef typename OsModel::size_t size_type;
			typedef Radio_P Radio;
			typedef Debug_P Debug;
			typedef Clock_P Clock;
			typedef typename Clock::time_t time_t;
			typedef ::uint32_t abs_millis_t;
			
			NapControl() : caffeine_(0), radio_(0), debug_(0) {
			}
			
			void init(typename Radio::self_pointer_t radio, typename Debug::self_pointer_t debug, typename Clock::self_pointer_t clock) {
				radio_ = radio;
				debug_ = debug;
				clock_ = clock;
				caffeine_ = 0;
			}
			
			bool on() {
				return caffeine_ > 0;
			}
			
			/**
			 */
			void push_caffeine(void* = 0) {
				if(caffeine_ == 0) {
					
					#if defined(CONTIKI)
						NETSTACK_RDC.on();
					#endif
					
					radio_->enable_radio();
					#if NAP_CONTROL_DEBUG_STATE
						debug_->debug("@%d on t%d", (int)radio_->id(), (int)(now() & 0xffff)); // (int)radio_->id());
					#endif
				}
				caffeine_++;
				
				#if NAP_CONTROL_DEBUG_STATE
					debug_->debug("caf%d", (int)caffeine_);
				#endif
				#if !WISELIB_DISABLE_DEBUG
				debug_->debug("node %d caffeine %d", (int)radio_->id(), (int)caffeine_);
				#endif
			}
			
			/**
			 */
			void pop_caffeine(void* = 0) {
				assert(caffeine_ > 0);
				caffeine_--;
				#if !WISELIB_DISABLE_DEBUG
				debug_->debug("node %d caffeine %d", (int)radio_->id(), (int)caffeine_);
				#endif
				
				#if NAP_CONTROL_DEBUG_STATE
					debug_->debug("caf%d", (int)caffeine_);
				#endif
				if(caffeine_ == 0) {
					#if defined(CONTIKI)
						NETSTACK_RDC.off(false);
					#endif
					radio_->disable_radio();
					#if NAP_CONTROL_DEBUG_STATE
						debug_->debug("@%d off t%d", (int)radio_->id(), (int)(now() & 0xffff)); //, (int)radio_->id());
					#endif
				}
			}
		
		private:
			abs_millis_t absolute_millis(const time_t& t) {
				return clock_->seconds(t) * 1000 + clock_->milliseconds(t);
			}
			
			abs_millis_t now() {
				return absolute_millis(clock_->time());
			}
			
			size_type caffeine_;
			typename Radio::self_pointer_t radio_;
			typename Debug::self_pointer_t debug_;
			typename Clock::self_pointer_t clock_;
		
	}; // NapControl
}

#endif // NAP_CONTROL_H

