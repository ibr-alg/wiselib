// vim: set noexpandtab ts=4 sw=4:

#ifndef COM_UART_PACKET_H
#define COM_UART_PACKET_H

#include <stdint.h>
#include <cassert>
#include "util/pstl/vector_static.h"

//#include "external_interface/pc/pc_os_model.h"

namespace wiselib {
	
	template<
		typename OsModel_P,
		int MaxPacketSize = 255
	>
	class ComISensePacket {
		public:
			typedef OsModel_P OsModel;
			typedef typename OsModel::size_t size_t;
			
			enum MessageType {
				MESSAGE_TYPE_RESET = 1, MESSAGE_TYPE_SERAERIAL, MESSAGE_TYPE_TIME,
				MESSAGE_TYPE_CAMERA_APP, MESSAGE_TYPE_AMR_APP, MESSAGE_TYPE_ACC_APP,
				MESSAGE_TYPE_IN_RESERVED_1, MESSAGE_TYPE_IN_RESERVED_2, MESSAGE_TYPE_IN_RESERVED_3,
				MESSAGE_TYPE_CUSTOM_IN_1, MESSAGE_TYPE_CUSTOM_IN_2, MESSAGE_TYPE_CUSTOM_IN_3,
				
				MESSAGE_TYPE_DEBUG = 100, MESSAGE_TYPE_INFO, MESSAGE_TYPE_WARNING, MESSAGE_TYPE_ERROR, MESSAGE_TYPE_LOG,
				MESSAGE_TYPE_PLOT, MESSAGE_TYPE_CUSTOM_OUT,
				
				MESSAGE_TYPE_TIMEREQUEST = 109, MESSAGE_TYPE_AUDIO, MESSAGE_TYPE_SPYGLASS, MESSAGE_TYPE_FLOAT_BUFFER,
				MESSAGE_TYPE_SQL
			};
			
			enum SubType {
				SUB_NONE = 0,
				SUB_RADIO_GET_ADDRESS = 'a',
				SUB_RADIO_ADDRESS = 'N',
				SUB_RADIO_DISTANCE_TEST = 'D',
				SUB_RADIO_IN = 'I', SUB_RADIO_OUT = 'O',
				SUB_SET_TX_POWER = 'T',
				SUB_GET_TX_POWER = 't',
				SUB_TX_POWER = 'P'
			};
			
			ComISensePacket(SubType st, MessageType t = MESSAGE_TYPE_CUSTOM_IN_1);
			ComISensePacket(size_t, uint8_t*);
			
			uint8_t* header();
			size_t header_size();
			uint8_t* data();
			size_t data_size();
			
			void push_header(uint8_t);
			void push_header16(uint16_t);
			
			void set_data(size_t, uint8_t*);
			
			MessageType type();
			SubType subtype();
			
		private:
			MessageType type_;
			SubType subtype_;
			
			vector_static<OsModel, uint8_t, MaxPacketSize> header_;
			uint8_t* data_;
			size_t data_size_;
			
			static size_t header_size(SubType);
	}; // ComISensePacket
	
	template<typename OsModel_P, int MaxPacketSize>
	ComISensePacket<OsModel_P, MaxPacketSize>::
	ComISensePacket(SubType st, MessageType t) {
		push_header(t);
		if(st != SUB_NONE) {
			push_header(st);
			push_header(0);
		}
		
		data_size_ = 0;
		data_ = 0;
	}
	
	template<typename OsModel_P, int MaxPacketSize>
	ComISensePacket<OsModel_P, MaxPacketSize>::
	ComISensePacket(size_t size, uint8_t* data) {
		assert(size >= 1);
		header_.push_back(data[0]);
		if(type() == MESSAGE_TYPE_CUSTOM_IN_1) {
			header_.push_back(data[1]);
			size_t size_h = header_size(subtype()) - 1;
			for(size_t i=0; i<size_h; i++) {
				header_.push_back(data[i+2]);
			}
		}
	}
	
	template<typename OsModel_P, int MaxPacketSize>
	typename ComISensePacket<OsModel_P, MaxPacketSize>::size_t ComISensePacket<OsModel_P, MaxPacketSize>::
	data_size() { return data_size_; }
	
	template<typename OsModel_P, int MaxPacketSize>
	uint8_t* ComISensePacket<OsModel_P, MaxPacketSize>::
	data() { return data_; }
	
	template<typename OsModel_P, int MaxPacketSize>
	typename ComISensePacket<OsModel_P, MaxPacketSize>::size_t ComISensePacket<OsModel_P, MaxPacketSize>::
	header_size() { return header_.size(); }
	
	template<typename OsModel_P, int MaxPacketSize>
	uint8_t* ComISensePacket<OsModel_P, MaxPacketSize>::
	header() { return header_.data(); }
	
	template<typename OsModel_P, int MaxPacketSize>
	void ComISensePacket<OsModel_P, MaxPacketSize>::
	push_header(uint8_t byte) {
		header_.push_back(byte);
	}
	
	template<typename OsModel_P, int MaxPacketSize>
	void ComISensePacket<OsModel_P, MaxPacketSize>::
	push_header16(uint16_t word) {
		header_.push_back(word >> 8);
		header_.push_back(word & 0xff);
	}
	
	template<typename OsModel_P, int MaxPacketSize>
	void ComISensePacket<OsModel_P, MaxPacketSize>::
	set_data(size_t size, uint8_t* data) {
		data_size_ = size;
		data_ = data;
	}
	
	template<typename OsModel_P, int MaxPacketSize>
	typename ComISensePacket<OsModel_P, MaxPacketSize>::MessageType ComISensePacket<OsModel_P, MaxPacketSize>::
	type() {
		return static_cast<MessageType>(header_[0]);
	}
	
	template<typename OsModel_P, int MaxPacketSize>
	typename ComISensePacket<OsModel_P, MaxPacketSize>::SubType ComISensePacket<OsModel_P, MaxPacketSize>::
	subtype() {
		return static_cast<SubType>(header_[1]);
	}
	
	/**
	 * Return expected header size by subtype in bytes.
	 * Header includes subtype but not isense packet type.
	 */
	template<typename OsModel_P, int MaxPacketSize>
	typename ComISensePacket<OsModel_P, MaxPacketSize>::size_t ComISensePacket<OsModel_P, MaxPacketSize>::
	header_size(SubType s) {
		switch(s) {
			case SUB_NONE:
			case SUB_RADIO_GET_ADDRESS: return 1;
			case SUB_RADIO_ADDRESS:
			case SUB_RADIO_OUT: return 3;
			case SUB_RADIO_IN: return 0; // TODO!
			case SUB_RADIO_DISTANCE_TEST: return 0; // TODO!
		}
		return 0;
	}
	
} // ns wiselib

#endif // COM_UART_PACKET_H

