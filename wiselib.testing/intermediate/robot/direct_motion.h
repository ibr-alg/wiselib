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

// vim: set noexpandtab ts=3 sw=3:

#ifndef DIRECT_MOTION_H
#define DIRECT_MOTION_H

#include "util/standalone_math.h"

namespace wiselib {
	
	/**
	 * Implements the "Motion" concept
	 * 
	 * Given a target position, this will rotate the robot and let it move the
	 * appropriate distance in order to reach that position.
	 * 
	 * @tparam Position_P should implement the Position concept
	 * @tparam ControlledMotion_P should implement the ControlledMotion concept
	 * @tparam Math_P should implement the Math concept
	 * 
	 * @ingroup Motion_concept
	 * @ingroup BasicAlgorithm_concept
	 * @ingroup BasicReturnValues_concept
	 */
	template<
		typename OsModel_P,
		typename Position_P,
		typename ControlledMotion_P,
		typename Math_P = StandaloneMath<OsModel_P>
	>
	class DirectMotion {
		public:
			typedef OsModel_P OsModel;
			typedef Position_P Position;
			typedef ControlledMotion_P ControlledMotion;
			typedef Math_P Math;
			typedef typename Position::position_t position_t;
			
			typedef DirectMotion<OsModel, Position, ControlledMotion, Math> self_type;
			typedef self_type* self_pointer_t;
			
			enum ErrorCodes {
				SUCCESS = OsModel::SUCCESS, ERR_UNSPEC = OsModel::ERR_UNSPEC
			};
			
			inline int init();
			inline int init(Position&, ControlledMotion&);
			inline int destruct();
			
			/**
			 */
			inline int move_to(const position_t&);
			
		private:
			typename Position::self_pointer_t position_;
			typename ControlledMotion::self_pointer_t controlled_motion_;
	};
	
	template<
		typename OsModel_P,
		typename Position_P,
		typename ControlledMotion_P,
		typename Math_P
	>
	int
	DirectMotion<OsModel_P, Position_P, ControlledMotion_P, Math_P>::
	init() {
		return SUCCESS;
	}
	
	template<
		typename OsModel_P,
		typename Position_P,
		typename ControlledMotion_P,
		typename Math_P
	>
	int
	DirectMotion<OsModel_P, Position_P, ControlledMotion_P, Math_P>::
	init(Position_P& position, ControlledMotion_P& cm) {
		position_ = &position;
		controlled_motion_ = &cm;
		return SUCCESS;
	}
	
	template<
		typename OsModel_P,
		typename Position_P,
		typename ControlledMotion_P,
		typename Math_P
	>
	int
	DirectMotion<OsModel_P, Position_P, ControlledMotion_P, Math_P>::
	destruct() {
		return SUCCESS;
	}
	
	template<
		typename OsModel_P,
		typename Position_P,
		typename ControlledMotion_P,
		typename Math_P
	>
	int
	DirectMotion<OsModel_P, Position_P, ControlledMotion_P, Math_P>::
	move_to(const position_t& target) {
		position_t current_pos, delta;
		typename ControlledMotion::distance_t distance;
		typename ControlledMotion::angle_t angle;
		
		current_pos = (*position_)();
		delta = target - current_pos;
		
		distance = Math::sqrt(delta.x() * delta.x() + delta.y() * delta.y());
		angle = Math::asin(-delta.x() / distance);
		
		// Correct quadrant if necessary since asin() can not possibly
		// reconstruct it from only the x-position
		if(delta.y() < 0) {
			angle = Math::sgn(angle) * Math::PI_2 - angle;
		}
		
		controlled_motion_->turn_to(angle);
		controlled_motion_->robot().wait_for_stop();
		controlled_motion_->move_distance(distance);
		controlled_motion_->robot().wait_for_stop();
		
		return SUCCESS;
	}
	
} // namespace

#endif // DIRECT_MOTION_H

