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

#ifndef COM_UART_PACKET_H
#define COM_UART_PACKET_H

#include <stdint.h>
#include <cassert>
#include "util/pstl/vector_static.h"

namespace wiselib {
	
	template<
		typename OsModel_P,
	   typename Size_P = typename OsModel_P::size_t,
	   typename Blockdata_P = typename OsModel_P::block_data_t,
		int MaxPacketSize = 255
	>
	class ComISensePacket {
		public:
			typedef OsModel_P OsModel;
			typedef Size_P size_t;
		   typedef Blockdata_P block_data_t;
			
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
			ComISensePacket(size_t, block_data_t*);
			
			block_data_t* header();
			size_t header_size();
			block_data_t* data();
			size_t data_size();
			
			void push_header(block_data_t);
			void push_header16(uint16_t);
			
			void set_data(size_t, block_data_t*);
			
			MessageType type();
			SubType subtype();
			
		private:
			MessageType type_;
			SubType subtype_;
			
			vector_static<OsModel, block_data_t, MaxPacketSize> header_;
			block_data_t* data_;
			size_t data_size_;
			
			static size_t header_size(SubType);
	}; // ComISensePacket
// <OsModel_P, MaxPacketSize>
// <OsModel_P, Size_P, Blockdata_P, MaxPacketSize>
	template<typename OsModel_P, typename Size_P, typename Blockdata_P, int MaxPacketSize>
	ComISensePacket<OsModel_P, Size_P, Blockdata_P, MaxPacketSize>::
	ComISensePacket(SubType st, MessageType t) {
		push_header(t);
		if(st != SUB_NONE) {
			push_header(st);
			push_header(0);
		}
		
		data_size_ = 0;
		data_ = 0;
	}
	
	template<typename OsModel_P, typename Size_P, typename Blockdata_P, int MaxPacketSize>
	ComISensePacket<OsModel_P, Size_P, Blockdata_P, MaxPacketSize>::
	ComISensePacket(size_t size, block_data_t* data) {
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
	
	template<typename OsModel_P, typename Size_P, typename Blockdata_P, int MaxPacketSize>
	typename ComISensePacket<OsModel_P, Size_P, Blockdata_P, MaxPacketSize>::size_t ComISensePacket<OsModel_P, Size_P, Blockdata_P, MaxPacketSize>::
	data_size() { return data_size_; }
	
	template<typename OsModel_P, typename Size_P, typename Blockdata_P, int MaxPacketSize>
	typename ComISensePacket<OsModel_P, Size_P, Blockdata_P, MaxPacketSize>::block_data_t* ComISensePacket<OsModel_P, Size_P, Blockdata_P, MaxPacketSize>::
	data() { return data_; }
	
	template<typename OsModel_P, typename Size_P, typename Blockdata_P, int MaxPacketSize>
	typename ComISensePacket<OsModel_P, Size_P, Blockdata_P, MaxPacketSize>::size_t ComISensePacket<OsModel_P, Size_P, Blockdata_P, MaxPacketSize>::
	header_size() { return header_.size(); }
	
	template<typename OsModel_P, typename Size_P, typename Blockdata_P, int MaxPacketSize>
	typename ComISensePacket<OsModel_P, Size_P, Blockdata_P, MaxPacketSize>::block_data_t* ComISensePacket<OsModel_P, Size_P, Blockdata_P, MaxPacketSize>::
	header() { return header_.data(); }
	
	template<typename OsModel_P, typename Size_P, typename Blockdata_P, int MaxPacketSize>
	void ComISensePacket<OsModel_P, Size_P, Blockdata_P, MaxPacketSize>::
	push_header(block_data_t byte) {
		header_.push_back(byte);
	}
	
	template<typename OsModel_P, typename Size_P, typename Blockdata_P, int MaxPacketSize>
	void ComISensePacket<OsModel_P, Size_P, Blockdata_P, MaxPacketSize>::
	push_header16(uint16_t word) {
		header_.push_back(word >> 8);
		header_.push_back(word & 0xff);
	}
	
	template<typename OsModel_P, typename Size_P, typename Blockdata_P, int MaxPacketSize>
	void ComISensePacket<OsModel_P, Size_P, Blockdata_P, MaxPacketSize>::
	set_data(size_t size, block_data_t* data) {
		data_size_ = size;
		data_ = data;
	}
	
	template<typename OsModel_P, typename Size_P, typename Blockdata_P, int MaxPacketSize>
	typename ComISensePacket<OsModel_P, Size_P, Blockdata_P, MaxPacketSize>::MessageType ComISensePacket<OsModel_P, Size_P, Blockdata_P, MaxPacketSize>::
	type() {
		return static_cast<MessageType>(header_[0]);
	}
	
	template<typename OsModel_P, typename Size_P, typename Blockdata_P, int MaxPacketSize>
	typename ComISensePacket<OsModel_P, Size_P, Blockdata_P, MaxPacketSize>::SubType ComISensePacket<OsModel_P, Size_P, Blockdata_P, MaxPacketSize>::
	subtype() {
		return static_cast<SubType>(header_[1]);
	}
	
	/**
	 * Return expected header size by subtype in bytes.
	 * Header includes subtype but not isense packet type.
	 */
	template<typename OsModel_P, typename Size_P, typename Blockdata_P, int MaxPacketSize>
	typename ComISensePacket<OsModel_P, Size_P, Blockdata_P, MaxPacketSize>::size_t ComISensePacket<OsModel_P, Size_P, Blockdata_P, MaxPacketSize>::
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

