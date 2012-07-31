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
#ifndef __WISELIB_UTIL_SERIALIZATION_ENDIAN_H
#define __WISELIB_UTIL_SERIALIZATION_ENDIAN_H

#if defined(__ANDROID__)
#include <sys/endian.h>
#elif defined(__APPLE__) || defined(__ba__) || defined(__arm__)
#include <machine/endian.h>
#elif defined(__MSP430__)
#include <sys/ieeefp.h>
#elif defined(__AVR__)
// no include available
#else
#include <endian.h>
#endif

namespace wiselib
{

   enum Endianness
   {
      WISELIB_LITTLE_ENDIAN,
      WISELIB_BIG_ENDIAN
   };

#if (defined(__BYTE_ORDER) && __BYTE_ORDER == __BIG_ENDIAN) || \
      (defined(BYTE_ORDER) && BYTE_ORDER == BIG_ENDIAN) || \
      (defined(_BYTE_ORDER) && _BYTE_ORDER == _BIG_ENDIAN) || \
      (defined(__BYTE_ORDER__) && __BYTE_ORDER__ == __BIG_ENDIAN__) || \
      defined(__IEEE_BIG_ENDIAN)
   const Endianness WISELIB_ENDIANNESS = WISELIB_BIG_ENDIAN;
#elif (defined(__BYTE_ORDER) && __BYTE_ORDER == __LITTLE_ENDIAN) || \
         (defined(BYTE_ORDER) && BYTE_ORDER == LITTLE_ENDIAN) || \
         (defined(_BYTE_ORDER) && _BYTE_ORDER == _LITTLE_ENDIAN) || \
         (defined(__BYTE_ORDER__) && __BYTE_ORDER__ == __LITTLE_ENDIAN__) || \
         defined(__IEEE_LITTLE_ENDIAN) || \
         defined(__AVR__)
   const Endianness WISELIB_ENDIANNESS = WISELIB_LITTLE_ENDIAN;
#else
  #error "Endian determination failed"
#endif

}

#endif
