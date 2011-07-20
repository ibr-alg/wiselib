
#ifndef ROOMBA_EVENT_SENSOR_H
#define ROOMBA_EVENT_SENSOR_H

#include "util/delegates/delegate.hpp"

#include <iostream>

namespace wiselib {
	
	template<typename OsModel_P,
		typename Roomba_P
	>
	class RoombaEventSensor {
		public:
			typedef OsModel_P OsModel;
			typedef Roomba_P Roomba;
			typedef delegate1<void, uint8_t> event_callback_t;
			typedef RoombaEventSensor<OsModel, Roomba> self_type;
			typedef self_type* self_pointer_t;
			
			enum Event {
				EVENT_NONE = 0x0, EVENT_WALL = 0x1
			};
			
			enum ReturnValues {
				SUCCESS = OsModel::SUCCESS, ERR_UNSPEC = OsModel::ERR_UNSPEC
			};
			
			int init(Roomba& roomba);
			int init();
			int destruct();
			
			template<typename T, void (T::*TMethod)(uint8_t)>
			int reg_event_callback(T* obj);
			
			void on_new_data();
			
		private:
			typename Roomba::self_pointer_t roomba_;
			event_callback_t event_callback_;
			uint8_t bumps_;
	};
	
	template<typename OsModel_P, typename Roomba_P>
	int RoombaEventSensor<OsModel_P, Roomba_P>::
	init(Roomba_P& roomba) {
		roomba_ = &roomba;
		roomba_->template reg_new_data_callback<
			self_type, &self_type::on_new_data
		>(this);
		return SUCCESS;
	}
	
	template<typename OsModel_P, typename Roomba_P>
	void RoombaEventSensor<OsModel_P, Roomba_P>::
	on_new_data() {
		typename Roomba::value_t& v = (*roomba_)();
		int events = 0;
		if(v.bumps != bumps_) {
			//std::cout << "bumps before: " << bumps_ << " bumps now: " << v.bumps << "\n";
		}
		if(v.bumps & ~bumps_) { // if bumps has any 1s it didnt have before
			events |= EVENT_WALL;
		}
		bumps_ = v.bumps;
		
		if(events && event_callback_) {
			event_callback_(events);
		}
	}
	
	template<typename OsModel_P, typename Roomba_P>
	int RoombaEventSensor<OsModel_P, Roomba_P>::
	init() {
		return SUCCESS;
	}
	
	template<typename OsModel_P, typename Roomba_P>
	int RoombaEventSensor<OsModel_P, Roomba_P>::
	destruct() {
		return SUCCESS;
	}
	
	template<typename OsModel_P, typename Roomba_P>
	template<typename T, void (T::*TMethod)(uint8_t)>
	int RoombaEventSensor<OsModel_P, Roomba_P>::
	reg_event_callback(T* obj) {
		event_callback_ = event_callback_t::from_method<T, TMethod>(obj);
		return 1;
	}
	
} // namespace wiselib

#endif // ROOMBA_EVENT_SENSOR_H

