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

#ifndef ROOMBA_H
#define ROOMBA_H

#warning ">>> You are using the new ROOMBA API which is currently work in progress. You have been warned."

#include "util/delegates/delegate.hpp"
#include "util/pstl/vector_static.h"
#include "util/standalone_math.h"

namespace wiselib {
	
	namespace {
		struct RoombaData {
			uint8_t bumps, wall, cliff_left, cliff_front_left,
				cliff_front_right, cliff_right, virtual_wall,
				wheel_overcurrents, dirt, unused_1,
				infrared_omni, buttons;
			int16_t distance;
			int16_t angle;
			uint8_t charging;
			uint16_t voltage;
			int16_t current;
			int8_t temperature;
			uint16_t charge, capacity, wall_signal, cliff_left_signal,
				 cliff_front_left_signal, cliff_front_right_signal,
				 cliff_right_signal;
			uint8_t unused_2, unused_3, unused_4;
			uint8_t charging_sources, oi_mode, song_nr, song_playing,
				stream_packet_count;
			int16_t requested_velocity, requested_radius, requested_right_velocity,
				requested_left_velocity;
			int16_t left_encoder_counts, right_encoder_counts;
			uint8_t light_bumper;
			uint16_t light_bump_left_signal, light_bump_front_left_signal,
				 light_bump_center_left_signal,
				 light_bump_center_right_signal,
				 light_bump_front_right_signal,
				 light_bump_right_signal;
			uint8_t ir_opcode_left, ir_opcode_right;
			int16_t left_motor_current, right_motor_current,
				main_brush_motor_current,
				side_brush_motor_current;
			uint8_t stasis;
		} __attribute__ ((packed)); // struct RoombaData
	} // anonymous namespace
	
	/**
	 * @brief Represents a roomba robot.
	 * 
	 * Implements the @ref TurnWalkMotion_concept and @ref Odometer_concept concepts.
	 * 
	 * @ingroup Odometer_concept
	 * @ingroup TurnWalkMotion_concept
	 * @ingroup BasicAlgorithm_concept
	 * 
	 * @tparam ComUartModel_P Has to implement @ref SerialCommunicationFacet_concept
	 * @tparam Timer_P Has to implement the @ref TimerFacet_concept and also provide a sleep(ms) method
	 */
	template<typename OsModel_P,
		typename ComUartModel_P,
		typename Timer_P = typename OsModel_P::Timer,
		typename Debug_P = typename OsModel_P::Debug,
		typename Math_P = StandaloneMath<OsModel_P>,
		typename OsModel_P::size_t MAX_RECEIVERS = 16>
	class RoombaModel {
		public:
			typedef RoombaModel<OsModel_P, ComUartModel_P, Timer_P, Debug_P, Math_P, MAX_RECEIVERS> self_type;
			typedef self_type* self_pointer_t;
			
			typedef OsModel_P OsModel;
			typedef Math_P Math;
			typedef ComUartModel_P ComUartModel;
			typedef Timer_P Timer;
			typedef Debug_P Debug;
			
			typedef delegate1<void, int> state_delegate_t;
			typedef vector_static<OsModel, state_delegate_t, MAX_RECEIVERS> state_delegates_t;
			
			typedef double angle_t;
			typedef RoombaData value_t;
			typedef int16_t angular_velocity_t;
			typedef double distance_t;
			typedef int16_t velocity_t;
			typedef typename OsModel::size_t size_t;
			typedef uint8_t block_data_t;
			
			// The slower, the more precive the motions will be
			enum MotionParameters {
				PRECISE_ANGULAR_VELOCITY = 20,
				DEFAULT_ANGULAR_VELOCITY = 60,
				FAST_ANGULAR_VELOCITY = 200,
				PRECISE_VELOCITY = 50,
				DEFAULT_VELOCITY = 200,
				FAST_VELOCITY = 300
			};
			
			enum DataTypes {
				WALL = 0x01, POSITION = 0x02,
				BATTERY_AND_TEMPERATURE = 0x04, LIGHT_BUMPS = 0x08,
				ALL = 0x0f,
			};
			
			enum State {
				READY = 0, NO_VALUE = 1, INACTIVE = 2, DATA_AVAILABLE = 3
			};
			
			enum ErrorCodes {
				SUCCESS = OsModel::SUCCESS, ERR_UNSPEC = OsModel::ERR_UNSPEC
			};
			
			RoombaModel() { }
			RoombaModel(typename OsModel::Os& os) { }
			
			inline int init(ComUartModel_P& uart, Timer_P& timer, int data_types);
			inline int init(ComUartModel_P& uart, Timer_P& timer, Debug_P& debug, int data_types);
			inline int init();
			inline int destruct();
			
			inline int data_types();
			
			// --- TurnWalkMotion
			
			inline int turn(angular_velocity_t v = DEFAULT_ANGULAR_VELOCITY);
			inline int move(velocity_t v = DEFAULT_VELOCITY);
			inline int stop();
			int wait_for_stop();
			
			int enable_stable_motion(bool enable) { stable_motion_ = enable; return SUCCESS; }
			
			// --- Odometer
			
			inline int reset_angle();
			inline angle_t angle();
			inline int reset_distance();
			inline distance_t distance();
			
			inline int state();
			template<typename T, void (T::*TMethod)(int)>
			int register_state_callback(T* obj);
			int unregister_state_callback(typename OsModel::size_t);
			
			inline value_t& operator()();
			
			inline int bump();
			
			// More or less "internal" methods
			
			int notify_state_receivers(int);
			
			typename OsModel::size_t packets();
			typename OsModel::size_t errors();
			
			int set_ticks_per_mm(double t) { ticks_per_mm_ = t; return SUCCESS; }
			int set_ticks_per_radian(double t) { ticks_per_radian_ = t; return SUCCESS; }
			
		private:
			typedef uint16_t rotation_t;
			
			enum Command {
				CMD_START = 0x80, CMD_BAUD, CMD_CONTROL, CMD_SAFE, CMD_FULL, CMD_POWER,
				CMD_SPOT, CMD_CLEAN, CMD_MAX,
				CMD_DRIVE, CMD_MOTORS, CMD_LEDS, CMD_SONG, CMD_PLAY, CMD_SENSORS,
				CMD_DOCK,
				CMD_PWM_MOTORS = 0x90, CMD_DRIVE_DIRECT, CMD_DRIVE_PWM,
				CMD_STREAM = 0x94, CMD_QUERY_LIST, CMD_ENABLE_STREAM,
				CMD_SCHEDULE_LEDS = 0xa2, CMD_BUTTONS = 0xa5,
				CMD_SCHEDULE = 0xa7, CMD_SET_TIME
			};
			
			enum Rotation {
				ROTATE_NONE = 0x8000,
				ROTATE_CW = 0xffff, ROTATE_CCW = 0x0001
			};
			
			inline int execute_motion(velocity_t speed, rotation_t rotation);
			
			int16_t get_packet_offset(uint8_t);
			int16_t get_packet_length(uint8_t);
			
			void send(uint8_t);
			void read(typename ComUartModel::size_t, typename ComUartModel::block_data_t*);
			
			void update_odometry();
			
			ComUartModel_P* uart_;
			int data_types_;
			RoombaData roomba_data_;
			state_delegates_t callbacks_;
			typename Timer::self_pointer_t timer_;
			typename Debug::self_pointer_t debug_;
			
			typename OsModel::size_t packets_;
			typename OsModel::size_t errors_;
			
			bool stop_, stable_motion_;
			double ticks_per_mm_, ticks_per_radian_;
			double distance_, angle_;
			int16_t latest_encoder_left_,
				latest_encoder_right_;
			
			velocity_t stable_motion_speed_;
			angle_t stable_motion_angle_,
				stable_motion_max_err_angle_;
	};
	
	template<
		typename OsModel_P,
		typename ComUartModel_P,
		typename Timer_P,
		typename Debug_P,
		typename Math_P,
		typename OsModel_P::size_t MAX_RECEIVERS
	>
	int
	RoombaModel<OsModel_P, ComUartModel_P, Timer_P, Debug_P, Math_P, MAX_RECEIVERS>::
	init(ComUartModel_P& uart, Timer_P& timer, int data_types) {
		ticks_per_mm_ = 2.27;
		ticks_per_radian_ = ticks_per_mm_ * 115.0; // half wheel dist. in mm
		
		packets_ = 0;
		errors_ = 0;
		assert(data_types != 0);
		
		uart_ = &uart;
		timer_ = &timer;
		debug_ = 0;
		data_types_ = data_types;
		stable_motion_ = false;
		stable_motion_speed_ = 0;
		stable_motion_max_err_angle_ = Math::degrees_to_radians(1.0);
		stable_motion_speed_ = 0;
		
		uart_->template reg_read_callback<self_type, &self_type::read>(this);
		
		send(CMD_START);
		send(CMD_SAFE);
		stop();
		
		uint8_t data_groups[10];
		uint8_t n = 0;
		
		// Enable data stream
		if(data_types_ & WALL) {
			data_groups[n++] = 7;
		}
		if(data_types_ & POSITION) {
			data_groups[n++] = 43;
			data_groups[n++] = 44;
		}
		if(data_types_ & BATTERY_AND_TEMPERATURE) {
			//data_groups[n++] = 3; // byte-swapping breaks for groups!
			data_groups[n++] = 22;
			data_groups[n++] = 23;
			data_groups[n++] = 25;
			data_groups[n++] = 26;
		}
		if(data_types_ & LIGHT_BUMPS) {
			data_groups[n++] = 106;
		}
		
		send(CMD_STREAM);
		send(n);
		
		for(uint8_t i = 0; i < n; i++) {
			send(data_groups[i]);
		}
		
		memset(&roomba_data_, 0, sizeof(roomba_data_));
		
		send(CMD_ENABLE_STREAM);
		send(1);
		
		timer_->sleep(200);
		
		return SUCCESS;
	}
	
	template<
		typename OsModel_P,
		typename ComUartModel_P,
		typename Timer_P,
		typename Debug_P,
		typename Math_P,
		typename OsModel_P::size_t MAX_RECEIVERS
	>
	int
	RoombaModel<OsModel_P, ComUartModel_P, Timer_P, Debug_P, Math_P, MAX_RECEIVERS>::
	init(ComUartModel_P& uart, Timer_P& timer, Debug_P& debug, int data_types) {
		int r = init(uart, timer, data_types);
		debug_ = &debug;
		return r;
	}
	
	template<
		typename OsModel_P,
		typename ComUartModel_P,
		typename Timer_P,
		typename Debug_P,
		typename Math_P,
		typename OsModel_P::size_t MAX_RECEIVERS
	>
	int
	RoombaModel<OsModel_P, ComUartModel_P, Timer_P, Debug_P, Math_P, MAX_RECEIVERS>::
	init() {
		destruct();
		init(*uart_, *timer_, data_types_);
		return SUCCESS;
	} // init()
	
	template<
		typename OsModel_P,
		typename ComUartModel_P,
		typename Timer_P,
		typename Debug_P,
		typename Math_P,
		typename OsModel_P::size_t MAX_RECEIVERS
	>
	int
	RoombaModel<OsModel_P, ComUartModel_P, Timer_P, Debug_P, Math_P, MAX_RECEIVERS>::
	destruct() {
		send(CMD_ENABLE_STREAM);
		send(0);
		uart_->unreg_read_callback();
		stop();
		send(CMD_POWER);
		return SUCCESS;
	}
	
	template<
		typename OsModel_P,
		typename ComUartModel_P,
		typename Timer_P,
		typename Debug_P,
		typename Math_P,
		typename OsModel_P::size_t MAX_RECEIVERS
	>
	int
	RoombaModel<OsModel_P, ComUartModel_P, Timer_P, Debug_P, Math_P, MAX_RECEIVERS>::
	data_types() {
		return data_types_;
	}
	
	template<
		typename OsModel_P,
		typename ComUartModel_P,
		typename Timer_P,
		typename Debug_P,
		typename Math_P,
		typename OsModel_P::size_t MAX_RECEIVERS
	>
	int
	RoombaModel<OsModel_P, ComUartModel_P, Timer_P, Debug_P, Math_P, MAX_RECEIVERS>::
	move(RoombaModel<OsModel_P, ComUartModel_P, Timer_P, Debug_P, Math_P, MAX_RECEIVERS>::velocity_t speed) {
		if(stable_motion_) {
			stable_motion_speed_ = speed;
			stable_motion_angle_ = 0;
			
			send(CMD_DRIVE_DIRECT);
			// speed right
			send((speed / 2) >> 8); // i know this is the same as >> 9, but its clearer this way
			send((speed / 2) & 0xff);
			// speed left
			send((speed / 2) >> 8); // i know this is the same as >> 9, but its clearer this way
			send((speed / 2) & 0xff);
			stop_ = false;
		}
		return execute_motion(speed, ROTATE_NONE);
	}
	
	template<
		typename OsModel_P,
		typename ComUartModel_P,
		typename Timer_P,
		typename Debug_P,
		typename Math_P,
		typename OsModel_P::size_t MAX_RECEIVERS
	>
	int
	RoombaModel<OsModel_P, ComUartModel_P, Timer_P, Debug_P, Math_P, MAX_RECEIVERS>::
	turn(RoombaModel<OsModel_P, ComUartModel_P, Timer_P, Debug_P, Math_P, MAX_RECEIVERS>::angular_velocity_t r_speed) {
		if(r_speed < 0) {
			return execute_motion(-r_speed, ROTATE_CW);
		}
		else {
			return execute_motion(r_speed, ROTATE_CCW);
		}
	}
	
	template<
		typename OsModel_P,
		typename ComUartModel_P,
		typename Timer_P,
		typename Debug_P,
		typename Math_P,
		typename OsModel_P::size_t MAX_RECEIVERS
	>
	int
	RoombaModel<OsModel_P, ComUartModel_P, Timer_P, Debug_P, Math_P, MAX_RECEIVERS>::
	stop() {
		return execute_motion(0, ROTATE_NONE);
	}

	template<
		typename OsModel_P,
		typename ComUartModel_P,
		typename Timer_P,
		typename Debug_P,
		typename Math_P,
		typename OsModel_P::size_t MAX_RECEIVERS
	>
	int
	RoombaModel<OsModel_P, ComUartModel_P, Timer_P, Debug_P, Math_P, MAX_RECEIVERS>::
	wait_for_stop() {
		typename OsModel::Timer t;
		while(!stop_) {
			timer_->sleep(100);
		}
		return SUCCESS;
	}

	template<
		typename OsModel_P,
		typename ComUartModel_P,
		typename Timer_P,
		typename Debug_P,
		typename Math_P,
		typename OsModel_P::size_t MAX_RECEIVERS
	>
	typename RoombaModel<OsModel_P, ComUartModel_P, Timer_P, Debug_P, Math_P, MAX_RECEIVERS>::value_t&
	RoombaModel<OsModel_P, ComUartModel_P, Timer_P, Debug_P, Math_P, MAX_RECEIVERS>::
	operator()() {
		return roomba_data_;
	}
	
	template<
		typename OsModel_P,
		typename ComUartModel_P,
		typename Timer_P,
		typename Debug_P,
		typename Math_P,
		typename OsModel_P::size_t MAX_RECEIVERS
	>
	int
	RoombaModel<OsModel_P, ComUartModel_P, Timer_P, Debug_P, Math_P, MAX_RECEIVERS>::
	state() {
		return READY;
	}
	
	template<
		typename OsModel_P,
		typename ComUartModel_P,
		typename Timer_P,
		typename Debug_P,
		typename Math_P,
		typename OsModel_P::size_t MAX_RECEIVERS
	>
	int
	RoombaModel<OsModel_P, ComUartModel_P, Timer_P, Debug_P, Math_P, MAX_RECEIVERS>::
	reset_angle() {
		angle_ = 0;
		return SUCCESS;
	}
	
	template<
		typename OsModel_P,
		typename ComUartModel_P,
		typename Timer_P,
		typename Debug_P,
		typename Math_P,
		typename OsModel_P::size_t MAX_RECEIVERS
	>
	typename RoombaModel<OsModel_P, ComUartModel_P, Timer_P, Debug_P, Math_P, MAX_RECEIVERS>::angle_t
	RoombaModel<OsModel_P, ComUartModel_P, Timer_P, Debug_P, Math_P, MAX_RECEIVERS>::
	angle() {
		return angle_;
	}
	
	template<
		typename OsModel_P,
		typename ComUartModel_P,
		typename Timer_P,
		typename Debug_P,
		typename Math_P,
		typename OsModel_P::size_t MAX_RECEIVERS
	>
	int
	RoombaModel<OsModel_P, ComUartModel_P, Timer_P, Debug_P, Math_P, MAX_RECEIVERS>::
	reset_distance() {
		distance_ = 0;
		return SUCCESS;
	}
	
	template<
		typename OsModel_P,
		typename ComUartModel_P,
		typename Timer_P,
		typename Debug_P,
		typename Math_P,
		typename OsModel_P::size_t MAX_RECEIVERS
	>
	typename RoombaModel<OsModel_P, ComUartModel_P, Timer_P, Debug_P, Math_P, MAX_RECEIVERS>::distance_t
	RoombaModel<OsModel_P, ComUartModel_P, Timer_P, Debug_P, Math_P, MAX_RECEIVERS>::
	distance() {
		return distance_;
	}
	
	template<
		typename OsModel_P,
		typename ComUartModel_P,
		typename Timer_P,
		typename Debug_P,
		typename Math_P,
		typename OsModel_P::size_t MAX_RECEIVERS
	>
	int
	RoombaModel<OsModel_P, ComUartModel_P, Timer_P, Debug_P, Math_P, MAX_RECEIVERS>::
	bump() {
		return roomba_data_.bumps & 0x11;
	}
	
	template<
		typename OsModel_P,
		typename ComUartModel_P,
		typename Timer_P,
		typename Debug_P,
		typename Math_P,
		typename OsModel_P::size_t MAX_RECEIVERS
	>
	template<typename T, void (T::*TMethod)(int)>
	int
	RoombaModel<OsModel_P, ComUartModel_P, Timer_P, Debug_P, Math_P, MAX_RECEIVERS>::
	register_state_callback(T* obj) {
		if(callbacks_.empty()) {
			callbacks_.assign(MAX_RECEIVERS, state_delegate_t());
		}
		
		for(typename OsModel::size_t i = 0; i < callbacks_.size(); ++i) {
			if(callbacks_[i] == state_delegate_t()) {
				callbacks_[i] = state_delegate_t::template from_method<T, TMethod>(obj);
				return i;
			}
		}
		return -1;
	}
	
	template<
		typename OsModel_P,
		typename ComUartModel_P,
		typename Timer_P,
		typename Debug_P,
		typename Math_P,
		typename OsModel_P::size_t MAX_RECEIVERS
	>
	int
	RoombaModel<OsModel_P, ComUartModel_P, Timer_P, Debug_P, Math_P, MAX_RECEIVERS>::
	unregister_state_callback(typename OsModel_P::size_t idx) {
		callbacks_[idx] = state_delegate_t();
		return OsModel::SUCCESS;
	}
	
	template<
		typename OsModel_P,
		typename ComUartModel_P,
		typename Timer_P,
		typename Debug_P,
		typename Math_P,
		typename OsModel_P::size_t MAX_RECEIVERS
	>
	int
	RoombaModel<OsModel_P, ComUartModel_P, Timer_P, Debug_P, Math_P, MAX_RECEIVERS>::
	notify_state_receivers(int state) {
		typedef typename state_delegates_t::iterator iter_t;
		
		for(iter_t iter = callbacks_.begin(); iter != callbacks_.end(); ++iter) {
			if(*iter != state_delegate_t()) {
				(*iter)(state);
			}
		}
		return SUCCESS;
	}
	
	template<
		typename OsModel_P,
		typename ComUartModel_P,
		typename Timer_P,
		typename Debug_P,
		typename Math_P,
		typename OsModel_P::size_t MAX_RECEIVERS
	>
	int16_t
	RoombaModel<OsModel_P, ComUartModel_P, Timer_P, Debug_P, Math_P, MAX_RECEIVERS>::
	get_packet_offset(uint8_t packet_type) {
		switch(packet_type) {
			case 0:
			case 1:
			case 6:
			case 7:
			case 100: return 0;
			case 2: return 10;
			case 3: return 16;
			case 4: return 26;
			case 5: return 40;
			case 8: return 1;
			case 19: return (uint8_t*)&roomba_data_.distance - (uint8_t*)&roomba_data_; // 12
			case 20: return (uint8_t*)&roomba_data_.angle - (uint8_t*)&roomba_data_; // 14;
			case 22: return (uint8_t*)&roomba_data_.voltage - (uint8_t*)&roomba_data_;
			case 23: return (uint8_t*)&roomba_data_.current - (uint8_t*)&roomba_data_;
			case 25: return (uint8_t*)&roomba_data_.charge - (uint8_t*)&roomba_data_;
			case 26: return (uint8_t*)&roomba_data_.capacity - (uint8_t*)&roomba_data_;
			case 43: return (uint8_t*)&roomba_data_.left_encoder_counts - (uint8_t*)&roomba_data_;
			case 44: return (uint8_t*)&roomba_data_.right_encoder_counts - (uint8_t*)&roomba_data_;
			case 101: return 52;
			case 106: return 57;
			default:
				return -1;
				//assert(false && "Unknown packet type");
		}
	} // get_packet_offset
	
	template<
		typename OsModel_P,
		typename ComUartModel_P,
		typename Timer_P,
		typename Debug_P,
		typename Math_P,
		typename OsModel_P::size_t MAX_RECEIVERS
	>
	int16_t
	RoombaModel<OsModel_P, ComUartModel_P, Timer_P, Debug_P, Math_P, MAX_RECEIVERS>::
	get_packet_length(uint8_t packet_type) {
		switch(packet_type) {
			case 1:
			case 3: return 10;
			case 2: return 6;
			case 4: return 14;
			case 5: return 12;
			case 7:
			case 8: return 1;
			case 19:
			case 20:
			case 22:
			case 23:
			case 25:
			case 26:
			case 43:
			case 44: return 2;
			case 100: return 80;
			case 106: return 12;
			default:
				return -1;
		}
	} // get_packet_length
	
	template<
		typename OsModel_P,
		typename ComUartModel_P,
		typename Timer_P,
		typename Debug_P,
		typename Math_P,
		typename OsModel_P::size_t MAX_RECEIVERS
	>
	void
	RoombaModel<OsModel_P, ComUartModel_P, Timer_P, Debug_P, Math_P, MAX_RECEIVERS>::
	send(uint8_t byte) {
		uart_->write(1, reinterpret_cast<char*>(&byte));
	}
	
	template<
		typename OsModel_P,
		typename ComUartModel_P,
		typename Timer_P,
		typename Debug_P,
		typename Math_P,
		typename OsModel_P::size_t MAX_RECEIVERS
	>
	typename OsModel_P::size_t RoombaModel<OsModel_P, ComUartModel_P, Timer_P, Debug_P, Math_P, MAX_RECEIVERS>::packets() { return packets_; }
	
	template<
		typename OsModel_P,
		typename ComUartModel_P,
		typename Timer_P,
		typename Debug_P,
		typename Math_P,
		typename OsModel_P::size_t MAX_RECEIVERS
	>
	typename OsModel_P::size_t RoombaModel<OsModel_P, ComUartModel_P, Timer_P, Debug_P, Math_P, MAX_RECEIVERS>::errors() { return errors_; }
	
	
	template<
		typename OsModel_P,
		typename ComUartModel_P,
		typename Timer_P,
		typename Debug_P,
		typename Math_P,
		typename OsModel_P::size_t MAX_RECEIVERS
	>
	void
	RoombaModel<OsModel_P, ComUartModel_P, Timer_P, Debug_P, Math_P, MAX_RECEIVERS>::
	read(
			typename RoombaModel<OsModel_P, ComUartModel_P, Timer_P, Debug_P, Math_P, MAX_RECEIVERS>::ComUartModel::size_t size,
			typename RoombaModel<OsModel_P, ComUartModel_P, Timer_P, Debug_P, Math_P, MAX_RECEIVERS>::ComUartModel::block_data_t* data
	) {
		enum { STATE_HEADER, STATE_LEN, STATE_TYPE, STATE_PAYLOAD, STATE_CHECKSUM };
		typedef typename ComUartModel::size_t size_t;
		
		
		static uint16_t state = STATE_HEADER,
			pos = 0, bytes_read = 0, len = 0;
		static int16_t packet_offset = 0, packet_length = 0;
		static uint8_t check = 0, packet_type = 0;
		
		for(size_t i=0; i<size; i++) {
			block_data_t d = data[i];
			
			switch(state) {
				case STATE_HEADER:
					if(d == 19) {
						state = STATE_LEN;
						check = d;
					}
					break;
					
				case STATE_LEN:
					check += d;
					len = d;
					bytes_read = 0;
					state = STATE_TYPE;
					break;
					
				case STATE_TYPE:
					// Do byte-swapping for previous packet if necessary
					if(bytes_read) {
						if(packet_type >= 7 && packet_type <= 58 && packet_length == 2) {
							uint8_t *buf = reinterpret_cast<uint8_t*>(&roomba_data_) + packet_offset;
							if(OsModel::endianness == wiselib::WISELIB_LITTLE_ENDIAN) {
								uint8_t tmp = buf[0];
								buf[0] = buf[1];
								buf[1] = tmp;
							}
						}
					}
					
					check += d;
					++bytes_read;
					packet_type = d;
					packet_offset = get_packet_offset(packet_type);
					packet_length = get_packet_length(packet_type);
					if(packet_offset < 0 || packet_length < 0 ||
							static_cast<size_t>(packet_offset + packet_length) >= sizeof(roomba_data_)) {
						state = STATE_HEADER;
					}
					else {
						state = STATE_PAYLOAD;
						pos = packet_offset;
					}
					break;
					
				case STATE_PAYLOAD:
					assert(pos < sizeof(roomba_data_));
					(reinterpret_cast<uint8_t*>(&roomba_data_))[pos] = d;
					check += d;
					pos++;
					bytes_read++;
					
					if(pos == (packet_offset + packet_length)) {
						if(bytes_read == len) { state = STATE_CHECKSUM; }
						else { state = STATE_TYPE; }
					}
					break;
					
				case STATE_CHECKSUM:
					// Do byte-swapping for previous packet if necessary
					if(bytes_read) {
						if(packet_type >= 7 && packet_type <= 58 && packet_length == 2) {
							uint8_t *buf = reinterpret_cast<uint8_t*>(&roomba_data_) + packet_offset;
							if(OsModel::endianness == wiselib::WISELIB_LITTLE_ENDIAN) {
								uint8_t tmp = buf[0];
								buf[0] = buf[1];
								buf[1] = tmp;
							}
						}
					}
					
					packets_++;
					check += d;
					if(check == 0) {
						update_odometry();
						notify_state_receivers(DATA_AVAILABLE);
					}
					else {
						errors_++;
						// checksum failure for uart packet
						debug_->debug("roomba.h: Checksum error on UART receive.\n");
					}
					state = STATE_HEADER;
					break;
			}
		} // for
		
	} // read()

	template<
		typename OsModel_P,
		typename ComUartModel_P,
		typename Timer_P,
		typename Debug_P,
		typename Math_P,
		typename OsModel_P::size_t MAX_RECEIVERS
	>
	int
	RoombaModel<OsModel_P, ComUartModel_P, Timer_P, Debug_P, Math_P, MAX_RECEIVERS>::
	execute_motion(
			RoombaModel<OsModel_P, ComUartModel_P, Timer_P, Debug_P, Math_P, MAX_RECEIVERS>::velocity_t speed,
			RoombaModel<OsModel_P, ComUartModel_P, Timer_P, Debug_P, Math_P, MAX_RECEIVERS>::rotation_t rotation
			) {
		send(CMD_DRIVE);
		send(speed >> 8); send(speed & 0xff);
		send(rotation >> 8); send(rotation & 0xff);
		stop_ = (speed == 0);
		return SUCCESS;
	}
	
	template<
		typename OsModel_P,
		typename ComUartModel_P,
		typename Timer_P,
		typename Debug_P,
		typename Math_P,
		typename OsModel_P::size_t MAX_RECEIVERS
	>
	void
	RoombaModel<OsModel_P, ComUartModel_P, Timer_P, Debug_P, Math_P, MAX_RECEIVERS>::
	update_odometry() {
		static int stable_motion_pause = 0;
		
		int32_t left = (roomba_data_.left_encoder_counts - latest_encoder_left_),
			right = (roomba_data_.right_encoder_counts - latest_encoder_right_);
		
		// Care for wrapping of encoder counts
		if(right > 0x8000 || right <= -0x8000) {
			right = right - Math::sgn(right) * 0x10000;
		}
		if(left > 0x8000 || left <= -0x8000) {
			left = left - Math::sgn(left) * 0x10000;
		}
		
		//debug_->debug("right: %7d left: %7d\n", right, left);
		
		angle_ += static_cast<double>(right - left) / (2.0 * ticks_per_radian_);
		stable_motion_angle_ += static_cast<double>(right - left) / (2.0 * ticks_per_radian_);
		distance_ += static_cast<double>(right + left) / (2.0 * ticks_per_mm_);
		
		latest_encoder_left_ = roomba_data_.left_encoder_counts;
		latest_encoder_right_ = roomba_data_.right_encoder_counts;
		
		// Stable motion stuff
		
		if(!stop_ && stable_motion_ && (stable_motion_speed_ != 0)) {
			debug_->debug("[stable motion] angle diff: %f max: %f\n", Math::fabs(stable_motion_angle_), stable_motion_max_err_angle_);
			
			// Update wheel speed estimation, decrease wait time
			if(stable_motion_pause > 0) {
				stable_motion_pause--;
			}
			else {
				stable_motion_pause = 0;
			}
			if(Math::fabs(stable_motion_angle_) >= stable_motion_max_err_angle_ && (stable_motion_pause == 0)) {
			double T = 0.50, R = (T * stable_motion_speed_) / stable_motion_angle_;
			double dw = 220.0;
			double f = (R + (dw / 2.0)) / (R - (dw / 2.0));
			int16_t r = (2.0*stable_motion_speed_ / (1.0+f)), l = 2.0*stable_motion_speed_ - r;
			
			stop();
			stop_ = false;
			send(CMD_DRIVE_DIRECT);
			send(r >> 8); send(r & 0xff);
			send(l >> 8); send(l & 0xff);
			stable_motion_pause =  34; //66.67 * stable_motion_angle_ / stable_motion_speed_;
			debug_->debug("[stable motion] R: %f r: %7d l: %7d a: %7f pause: %7d\n", R, r, l, Math::radians_to_degrees(stable_motion_angle_), stable_motion_pause);
			}
		}
	}
} // ns wiselib

#endif // ROOMBA_H

