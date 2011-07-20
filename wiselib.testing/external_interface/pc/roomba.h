// vim: set noexpandtab ts=4 sw=4:

#ifndef ROOMBA_H
#define ROOMBA_H

#warning "There is a new ROOMBA API under intermediate/robot which is more complete & precise than the one you are using right now!"

// TODO: Remove STL dependencies (substitute with pSTL / template parameter)
// TODO: Subsitute cout with debug facet

#include <list>

#include "external_interface/pc/roomba_angle.h"
#include "external_interface/pc/pc_os.h"
#include "util/delegates/delegate.hpp"

#include <iostream>

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
			uint16_t left_encoder_counts, right_encoder_counts;
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
	 * Represents a roomba robot.
	 * Not intended for direct use, rather use as parameter to  RoombaMotion,
	 * RoombaIRDistanceSensors etc... in order to access sensor data and
	 * motion routines of the roomba in a standarized way.
	 */
	template<typename OsModel_P,
		typename ComUartModel_P,
		typename Math_P,
		typename OsModel_P::size_t MAX_RECEIVERS = 16>
	class RoombaModel {
		public:
			typedef RoombaModel<OsModel_P, ComUartModel_P, Math_P, MAX_RECEIVERS> self_type;
			typedef self_type* self_pointer_t;
			
			typedef Math_P Math;
			typedef ComUartModel_P ComUartModel;
			typedef OsModel_P OsModel;
			
			typedef delegate0<void> new_data_delegate_t;
			typedef vector_static<OsModel, new_data_delegate_t, MAX_RECEIVERS> new_data_delegates_t;
			
			typedef RoombaAngle<Math> angle_t;
			typedef RoombaData value_t;
			typedef int16_t angular_velocity_t;
			typedef int16_t distance_t;
			typedef int16_t velocity_t;
			typedef typename OsModel::size_t size_t;
			typedef uint8_t block_data_t;
			
			enum DataTypes {
				WALL = 0x01,
				POSITION = 0x02,
				BATTERY_AND_TEMPERATURE = 0x04,
				LIGHT_BUMPS = 0x08,
				
				ALL = 0x0f,
			};
			
			enum {
				READY = 0, NO_VALUE = 1, INACTIVE = 2
			};
			
			RoombaModel(PCOs& os) { }
			
			void init(ComUartModel_P& uart, int data_types);
			void init();
			void destruct();
			
			// You should not manually call any method below
			// this comment, use RoombaSomething interfaces instead!
			
			inline void data_types();
			
			inline void move(velocity_t speed);
			inline void turn(angular_velocity_t speed);
			inline void stop();
			
			/// Raw roomba data block
			inline value_t& operator()();
			inline int state();
			inline angle_t angle();
			inline distance_t distance();
			inline bool wall();
			
			template<typename T, void (T::*TMethod)(void)>
			int reg_new_data_callback(T* obj);
			int unreg_new_data_callback(typename OsModel::size_t);
			void notify_new_data_receivers();
			
			typename OsModel::size_t packets();
			typename OsModel::size_t errors();
			
		private:
			typedef int16_t rotation_t;
			
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
			
			int16_t get_packet_offset(uint8_t);
			int16_t get_packet_length(uint8_t);
			
			void send(uint8_t);
			void read(typename ComUartModel::size_t, typename ComUartModel::block_data_t*);
			void execute_motion(velocity_t speed, rotation_t rotation);
			
			ComUartModel_P* uart_;
			int data_types_;
			RoombaData roomba_data_;
			new_data_delegates_t callbacks_;
			
			typename OsModel::size_t packets_;
			typename OsModel::size_t errors_;
	};
	
	template<typename OsModel_P, typename ComUartModel_P, typename Math_P, typename OsModel_P::size_t MAX_RECEIVERS>
	void
	RoombaModel<OsModel_P, ComUartModel_P, Math_P, MAX_RECEIVERS>::
	init(ComUartModel_P& uart, int data_types) {
		packets_ = 0;
		errors_ = 0;
		assert(data_types != 0);
		uart_ = &uart;
		data_types_ = data_types;
		// uart_->set_config(19200, 8, 'N', 1)
		init();
	}
	
	template<typename OsModel_P, typename ComUartModel_P, typename Math_P, typename OsModel_P::size_t MAX_RECEIVERS>
	void
	RoombaModel<OsModel_P, ComUartModel_P, Math_P, MAX_RECEIVERS>::
	init() {
		memset(&roomba_data_, 0, sizeof(roomba_data_));

		uart_->template reg_read_callback<self_type, &self_type::read>(this);
		
		send(CMD_START);
		send(CMD_SAFE);
		
		// Enable data stream
		std::list<int> data_groups;
		if(data_types_ & WALL) { data_groups.push_back(7); }
		if(data_types_ & POSITION) {
			data_groups.push_back(19);
			data_groups.push_back(20);
		}
		if(data_types_ & BATTERY_AND_TEMPERATURE) { data_groups.push_back(3); }
		if(data_types_ & LIGHT_BUMPS) { data_groups.push_back(106); }
		
		send(CMD_STREAM);
		send(data_groups.size());
		for(std::list<int>::iterator iter = data_groups.begin(); iter != data_groups.end(); ++iter) {
			send(*iter);
		}
		
		send(CMD_ENABLE_STREAM);
		send(1);
	} // init()
	
	template<typename OsModel_P, typename ComUartModel_P, typename Math_P, typename OsModel_P::size_t MAX_RECEIVERS>
	void
	RoombaModel<OsModel_P, ComUartModel_P, Math_P, MAX_RECEIVERS>::
	destruct() {
		//std::cout << "RoombaModel::destruct()\n";
		
		send(CMD_ENABLE_STREAM);
		send(0);
		uart_->unreg_read_callback();
		stop();
		send(CMD_POWER);
	}
	
	template<typename OsModel_P, typename ComUartModel_P, typename Math_P, typename OsModel_P::size_t MAX_RECEIVERS>
	void
	RoombaModel<OsModel_P, ComUartModel_P, Math_P, MAX_RECEIVERS>::
	data_types() {
		return data_types_;
	}
	
	
	template<typename OsModel_P, typename ComUartModel_P, typename Math_P, typename OsModel_P::size_t MAX_RECEIVERS>
	void
	RoombaModel<OsModel_P, ComUartModel_P, Math_P, MAX_RECEIVERS>::
	move(RoombaModel<OsModel_P, ComUartModel_P, Math_P, MAX_RECEIVERS>::velocity_t speed) {
		execute_motion(speed, ROTATE_NONE);
	}
	
	template<typename OsModel_P, typename ComUartModel_P, typename Math_P, typename OsModel_P::size_t MAX_RECEIVERS>
	void
	RoombaModel<OsModel_P, ComUartModel_P, Math_P, MAX_RECEIVERS>::
	turn(RoombaModel<OsModel_P, ComUartModel_P, Math_P, MAX_RECEIVERS>::angular_velocity_t r_speed) {
		//std::cout << "turning with " << r_speed << "\n";
		if(r_speed < 0) {
			execute_motion(-r_speed, ROTATE_CW);
		}
		else {
			execute_motion(r_speed, ROTATE_CCW);
		}
	}
	
	template<typename OsModel_P, typename ComUartModel_P, typename Math_P, typename OsModel_P::size_t MAX_RECEIVERS>
	void
	RoombaModel<OsModel_P, ComUartModel_P, Math_P, MAX_RECEIVERS>::
	stop() {
		execute_motion(0, ROTATE_NONE);
	}
	
	
	template<typename OsModel_P, typename ComUartModel_P, typename Math_P, typename OsModel_P::size_t MAX_RECEIVERS>
	typename RoombaModel<OsModel_P, ComUartModel_P, Math_P, MAX_RECEIVERS>::value_t&
	RoombaModel<OsModel_P, ComUartModel_P, Math_P, MAX_RECEIVERS>::
	operator()() {
		return roomba_data_;
	}
	
	template<typename OsModel_P, typename ComUartModel_P, typename Math_P, typename OsModel_P::size_t MAX_RECEIVERS>
	int
	RoombaModel<OsModel_P, ComUartModel_P, Math_P, MAX_RECEIVERS>::
	state() {
		return READY;
	}
	
	template<typename OsModel_P, typename ComUartModel_P, typename Math_P, typename OsModel_P::size_t MAX_RECEIVERS>
	typename RoombaModel<OsModel_P, ComUartModel_P, Math_P, MAX_RECEIVERS>::angle_t
	RoombaModel<OsModel_P, ComUartModel_P, Math_P, MAX_RECEIVERS>::
	angle() {
		angle_t angle;
		angle.set_roomba_units_(roomba_data_.angle);
		//if(roomba_data_.angle != 0)
		//	std::cout << "roomba angle=" << roomba_data_.angle << " -> " << angle << "\n";
		return angle;
	}
	
	template<typename OsModel_P, typename ComUartModel_P, typename Math_P, typename OsModel_P::size_t MAX_RECEIVERS>
	typename RoombaModel<OsModel_P, ComUartModel_P, Math_P, MAX_RECEIVERS>::distance_t
	RoombaModel<OsModel_P, ComUartModel_P, Math_P, MAX_RECEIVERS>::
	distance() {
		return roomba_data_.distance;
	}
	
	template<typename OsModel_P, typename ComUartModel_P, typename Math_P, typename OsModel_P::size_t MAX_RECEIVERS>
	bool
	RoombaModel<OsModel_P, ComUartModel_P, Math_P, MAX_RECEIVERS>::
	wall() {
		return roomba_data_.wall;
	}
	
	template<typename OsModel_P, typename ComUartModel_P, typename Math_P, typename OsModel_P::size_t MAX_RECEIVERS>
	template<typename T, void (T::*TMethod)(void)>
	int
	RoombaModel<OsModel_P, ComUartModel_P, Math_P, MAX_RECEIVERS>::
	reg_new_data_callback(T* obj) {
		if(callbacks_.empty()) {
			callbacks_.assign(MAX_RECEIVERS, new_data_delegate_t());
		}
		
		for(typename OsModel::size_t i = 0; i < callbacks_.size(); ++i) {
			if(callbacks_[i] == new_data_delegate_t()) {
				callbacks_[i] = new_data_delegate_t::template from_method<T, TMethod>(obj);
				return i;
			}
		}
		return -1;
	}
	
	template<typename OsModel_P, typename ComUartModel_P, typename Math_P, typename OsModel_P::size_t MAX_RECEIVERS>
	int
	RoombaModel<OsModel_P, ComUartModel_P, Math_P, MAX_RECEIVERS>::
	unreg_new_data_callback(typename OsModel_P::size_t idx) {
		callbacks_[idx] = new_data_delegate_t();
		return OsModel::SUCCESS;
	}
	
	template<typename OsModel_P, typename ComUartModel_P, typename Math_P, typename OsModel_P::size_t MAX_RECEIVERS>
	void
	RoombaModel<OsModel_P, ComUartModel_P, Math_P, MAX_RECEIVERS>::
	notify_new_data_receivers() {
		typedef typename new_data_delegates_t::iterator iter_t;
		
		for(iter_t iter = callbacks_.begin(); iter != callbacks_.end(); ++iter) {
			if(*iter != new_data_delegate_t()) {
				(*iter)();
			}
		}
	}
	
	template<typename OsModel_P, typename ComUartModel_P, typename Math_P, typename OsModel_P::size_t MAX_RECEIVERS>
	int16_t
	RoombaModel<OsModel_P, ComUartModel_P, Math_P, MAX_RECEIVERS>::
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
			case 19: return 12;
			case 20: return 14;
			case 101: return 52;
			case 106: return 57;
			default:
				return -1;
				//assert(false && "Unknown packet type");
		}
	} // get_packet_offset
	
	template<typename OsModel_P, typename ComUartModel_P, typename Math_P, typename OsModel_P::size_t MAX_RECEIVERS>
	int16_t
	RoombaModel<OsModel_P, ComUartModel_P, Math_P, MAX_RECEIVERS>::
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
			case 20: return 2;
			case 100: return 80;
			case 106: return 12;
			default:
				//std::cout << "?";
				//std::cout.flush();
				return -1;
				//assert(false && "Unknown packet type");
		}
	} // get_packet_length
	
	template<typename OsModel_P, typename ComUartModel_P, typename Math_P, typename OsModel_P::size_t MAX_RECEIVERS>
	void
	RoombaModel<OsModel_P, ComUartModel_P, Math_P, MAX_RECEIVERS>::
	send(uint8_t byte) {
		//std::cout << "sending: " << (unsigned)byte << "\n";
		//std::cout.flush();
		
		uart_->write(1, reinterpret_cast<char*>(&byte));
	}
	
	template<typename OsModel_P, typename ComUartModel_P, typename Math_P, typename OsModel_P::size_t MAX_RECEIVERS>
	typename OsModel_P::size_t RoombaModel<OsModel_P, ComUartModel_P, Math_P, MAX_RECEIVERS>::packets() { return packets_; }
	
	template<typename OsModel_P, typename ComUartModel_P, typename Math_P, typename OsModel_P::size_t MAX_RECEIVERS>
	typename OsModel_P::size_t RoombaModel<OsModel_P, ComUartModel_P, Math_P, MAX_RECEIVERS>::errors() { return errors_; }
	
	
	template<typename OsModel_P, typename ComUartModel_P, typename Math_P, typename OsModel_P::size_t MAX_RECEIVERS>
	void
	RoombaModel<OsModel_P, ComUartModel_P, Math_P, MAX_RECEIVERS>::
	read(
			typename RoombaModel<OsModel_P, ComUartModel_P, Math_P, MAX_RECEIVERS>::ComUartModel::size_t size,
			typename RoombaModel<OsModel_P, ComUartModel_P, Math_P, MAX_RECEIVERS>::ComUartModel::block_data_t* data
	) {
		enum { STATE_HEADER, STATE_LEN, STATE_TYPE, STATE_PAYLOAD, STATE_CHECKSUM };
		typedef typename ComUartModel::size_t size_t;
		
		static uint16_t state = STATE_HEADER,
			pos = 0, bytes_read = 0, len = 0;
		static int16_t packet_offset = 0, packet_length = 0;
		static uint8_t check = 0, packet_type = 0;
		
		/*std::cout << ".";
		std::cout.flush();*/
		
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
						//std::cout << ".";
						//std::cout.flush();
						notify_new_data_receivers();
					}
					else {
						errors_++;
						// checksum failure for uart packet
						std::cout << "!";
						std::cout.flush();
					}
					state = STATE_HEADER;
					break;
			}
		} // for
		
	} // read()

	template<typename OsModel_P, typename ComUartModel_P, typename Math_P, typename OsModel_P::size_t MAX_RECEIVERS>
	void
	RoombaModel<OsModel_P, ComUartModel_P, Math_P, MAX_RECEIVERS>::
	execute_motion(
			RoombaModel<OsModel_P, ComUartModel_P, Math_P, MAX_RECEIVERS>::velocity_t speed,
			RoombaModel<OsModel_P, ComUartModel_P, Math_P, MAX_RECEIVERS>::rotation_t rotation
			) {
		send(CMD_DRIVE);
		send(speed >> 8); send(speed & 0xff);
		send(rotation >> 8); send(rotation & 0xff);
	}
	
	
} // ns wiselib

#endif // ROOMBA_H

