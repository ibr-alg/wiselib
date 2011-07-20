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

#ifndef __UTIL_WISEBED_NODE_API_SENSOR_ENCODING_H
#define __UTIL_WISEBED_NODE_API_SENSOR_ENCODING_H

#include "util/serialization/simple_types.h"

namespace wiselib
{

   enum SensorEncodings
   {
      UNKNOWN_ENCODING = 0xff,
      BOOL             = 0x01,
      UINT8            = 0x02,
      INT8             = 0x03,
      UINT16           = 0x04,
      INT16            = 0x05,
      UINT32           = 0x06,
      INT32            = 0x07,
      UINT64           = 0x08,
      INT64            = 0x09,
      STRING           = 0x0a,
      DOUBLE           = 0x0b,
   };
   // -----------------------------------------------------------------------
   template<typename Type_P>
   class SensorEncoding
   {
   public:
      static uint8_t encoding()
      { return UNKNOWN_ENCODING; }
   };
   // -----------------------------------------------------------------------
   template<> class SensorEncoding<bool>
   {
   public:
      static uint8_t encoding()
      { return BOOL; }
   };
   // -----------------------------------------------------------------------
   template<> class SensorEncoding<uint8_t>
   {
   public:
      static uint8_t encoding()
      { return UINT8; }
   };
   // -----------------------------------------------------------------------
   template<> class SensorEncoding<int8_t>
   {
   public:
      static uint8_t encoding()
      { return INT8; }
   };
   // -----------------------------------------------------------------------
   template<> class SensorEncoding<uint16_t>
   {
   public:
      static uint8_t encoding()
      { return UINT16; }
   };
   // -----------------------------------------------------------------------
   template<> class SensorEncoding<int16_t>
   {
   public:
      static uint8_t encoding()
      { return INT16; }
   };
   // -----------------------------------------------------------------------
   template<> class SensorEncoding<uint32_t>
   {
   public:
      static uint8_t encoding()
      { return UINT32; }
   };
   // -----------------------------------------------------------------------
   template<> class SensorEncoding<int32_t>
   {
   public:
      static uint8_t encoding()
      { return INT32; }
   };
   // -----------------------------------------------------------------------
   template<> class SensorEncoding<uint64_t>
   {
   public:
      static uint8_t encoding()
      { return UINT64; }
   };
   // -----------------------------------------------------------------------
   template<> class SensorEncoding<int64_t>
   {
   public:
      static uint8_t encoding()
      { return INT64; }
   };
   // -----------------------------------------------------------------------
   template<> class SensorEncoding<double>
   {
   public:
      static uint8_t encoding()
      { return DOUBLE; }
   };
   // -----------------------------------------------------------------------
   template<> class SensorEncoding<char*>
   {
   public:
      static uint8_t encoding()
      { return STRING; }
   };
}
#endif	/* _SENSOR_ENCODING_H */

