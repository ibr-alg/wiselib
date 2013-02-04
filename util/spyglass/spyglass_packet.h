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

/*
 * File:   spyglass_packet.h
 * Author: Krueger
 *
 */
#ifndef SPYGLASS_PACKET_H
#define	SPYGLASS_PACKET_H

#include "util/delegates/delegate.hpp"


namespace wiselib
{
#define SPYGLASS_MAXIMUM_SIZE 98
#define SPYGLASS_PACKET_STANDARD_PACKET_SIZE 19

template<typename OsModel_P>

    class SpyglassPacket {

public:
	//typedef OsModel_P OsModel;


    	//typedef typename OsModel_P::Clock::time_t time_t;

    	typedef SpyglassPacket<OsModel_P> self_t;

/**
	    * Number of bytes in this packet (excluding this field).
	    */
	   uint16_t length_;

	   uint8_t version_;

	   /**
	    * Syntax type of this spyglass packet.
	    */
	   uint8_t syntax_type_;

	   /**
	    * Semantic type of this spyglass packet.
	    */
	   uint8_t semantic_type_;

	   /**
	    * Sender ID of this spyglass packet.
	    */
	   uint16_t sender_id_;

	   /**
	    * Timestamp of this spyglass packet.
	    */
	   uint32_t sec_;
	   uint16_t millis_;

	   /**
	    * X-Coordinate of this spyglass packet.
	    */
	   int16_t x_;

	   /**
	    * Y-Coordinate of this spyglass packet.
	    */
	   int16_t y_;

	   /**
	    * Z-Coordinate of this spyglass packet.
	    */
	   int16_t z_;

        SpyglassPacket():
        	length_(SPYGLASS_PACKET_STANDARD_PACKET_SIZE),
        	version_(2),
        	syntax_type_(0),
        	semantic_type_(0),
        	sender_id_(0),
        	sec_(0),
        	millis_(0),
        	//time_(Time(0,0)),
        	x_(0),
        	y_(0),
        	z_(0)
	    {
	    };
	
	    SpyglassPacket(uint16_t len, uint8_t syntax_type, uint8_t semantic_type, uint16_t sender_id, uint32_t sec, uint16 millis, int16_t x, int16_t y, int16_t z):
            	length_(len),
            	version_(2),
            	syntax_type_(syntax_type),
            	semantic_type_(semantic_type),
            	sender_id_(sender_id),
            	sec_(sec),
            	millis_(millis),
            	x_(x),
            	y_(y),
            	z_(z)
        	{
	        };
	    // ----------------------------------------------------------------------
	    
	    ~SpyglassPacket()
	    {};
	    /*
		 initialize the module
		 */
		void init () {
		};
	    // ----------------------------------------------------------------------
	    uint16_t
	    serialize(uint8_t * buf, uint8_t buf_size)
	    {
	    	if (buf_size >= SPYGLASS_PACKET_STANDARD_PACKET_SIZE)
	    	{
		    	buf[0] = ((length_-2)>>8)& 0xFF;
		    	buf[1]=(length_-2)& 0xFF;
		    	buf[2] = version_& 0xFF;
		    	buf[3] = syntax_type_& 0xFF;
		    	buf[4] = semantic_type_& 0xFF;
		    	buf[5] = (sender_id_>>8)& 0xFF;
		    	buf[6] = sender_id_ & 0xFF;

		    	buf[7] = (sec_>>24)& 0xFF;
		    	buf[8] = (sec_>>16)& 0xFF;
		    	buf[9] = (sec_>>8)& 0xFF;
		    	buf[10] = sec_ & 0xFF;
		    	buf[11] = (millis_>>8)& 0xFF;
		    	buf[12]= millis_ & 0xFF;
		    	buf[13] = (x_>>8)& 0xFF;
		    	buf[14] = x_ & 0xFF;
		    	buf[15] = (y_>>8)& 0xFF;
		    	buf[16] = y_ & 0xFF;
		    	buf[17] = (z_>>8)& 0xFF;
		    	buf[18] = z_ & 0xFF;
		    	return SPYGLASS_PACKET_STANDARD_PACKET_SIZE;
	    	}
	    	return 0;
	    }
	     // ----------------------------------------------------------------------
		uint16_t
		get_size()
		{
			return SPYGLASS_PACKET_STANDARD_PACKET_SIZE;
		}
	protected:
/*
		uint8_t
		serializeInt16(int16_t value,uint8_t * buf, uint8_t buf_size,uint8_t startIndex)
		{
			if(startIndex+2>=buf_size) return 0;
			buf[startIndex] =(value>>8)& 0xFF;
			buf[startIndex+1] = value& 0xFF;
			return 2;
		}
	
		uint8
		serializeInt64(int64 value,uint8 * buf, uint8 buf_size,uint8 startIndex)
		{
			 if(startIndex+8>=buf_size) return 0;
			 buf[startIndex] =(value>>56)& 0xFF;
			 buf[startIndex+1] =(value>>48)& 0xFF;
			 buf[startIndex+2] =(value>>40)& 0xFF;
			 buf[startIndex+3] =(value>>32)& 0xFF;
			 buf[startIndex+4] =(value>>24)& 0xFF;
			 buf[startIndex+5] =(value>>16)& 0xFF;
			 buf[startIndex+6] =(value>>8)& 0xFF;
			 buf[startIndex+7] = value& 0xFF;
			 return 8;
		}
	
		uint8
		serializeUInt16(uint16 value,uint8 * buf, uint8 buf_size,uint8 startIndex)
		{
			if(startIndex+2>=buf_size) return 0;
			buf[startIndex] =(value>>8)& 0xFF;
			buf[startIndex+1] = value & 0xFF;
			return 2;
		}
	
		uint8
		serializeUInt32(uint32 value,uint8 * buf, uint8 buf_size,uint8 startIndex)
		{
			if(startIndex+4>=buf_size) return 0;
			buf[startIndex] =(value>>24)& 0xFF;
			buf[startIndex+1] =(value>>16)& 0xFF;
		 	buf[startIndex+2] =(value>>8)& 0xFF;
		 	buf[startIndex+3] = value& 0xFF;
			return 4;
		}
	
		uint8
		serializeUInt8(uint8 value,uint8 * buf, uint8 buf_size,uint8 startIndex)
		{
			if(startIndex+1>=buf_size) return 0;
			buf[startIndex] = value;
			return 1;
		}
	
		uint8
		serializeFloat(float value,uint8 * buf, uint8 buf_size,uint8 startIndex)
		{
			if(startIndex+4>=buf_size) return 0;
			int intvalue ;
			memcpy(&intvalue, &value, 4);
			buf[startIndex] = (uint8) ((intvalue >> 24 )& 0xFF);
			buf[startIndex+1] = (uint8) ((intvalue >> 16 )& 0xFF);
			buf[startIndex+2] = (uint8) ((intvalue >> 8 )& 0xFF);
			buf[startIndex+3] = (uint8) (intvalue & 0xFF);
			return 4;
		}

*/
	};
}

#endif

/*-----------------------------------------------------------------------
* Source  $Source: /cvs/shawn/shawn/apps/spyglass/spyglass_packet.cpp,v $
* Version $Revision: 38 $
* Date    $Date: 2007-06-08 14:30:12 +0200 (Fr, 08 Jun 2007) $
*-----------------------------------------------------------------------
* $Log: spyglass_packet.cpp,v $
 *-----------------------------------------------------------------------*/
