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
#ifndef __CONNECTOR_DEFAULT_RETURN_VALUES_H__
#define __CONNECTOR_DEFAULT_RETURN_VALUES_H__

namespace wiselib
{

   /** \brief Implementation of \ref basic_return_values_concept "Basic Return Values".
    *
    *  Default return values that can be used/provided by an \ref os_concept "Os Model".
    *
    *  \ingroup basic_return_values_concept
    *  \ingroup basic_concepts
    */
   template<typename OsModel_P>
   class DefaultReturnValues
   {
   public:
      typedef OsModel_P OsModel;
      // --------------------------------------------------------------------
      enum ErrorCodes
      {
         SUCCESS         =   0, /// Default return value of success
         ERR_UNSPEC      =  -1, /// Unspecified error value - if no other fits
         ERR_NOMEM       =  12, /// Out of memory
         ERR_BUSY        =  16, /// Device or resource busy - try again later
         ERR_NOTIMPL     =  38, /// Function not implemented
         ERR_NETDOWN     = 100, /// Network is down
         ERR_HOSTUNREACH = 113, /// No route to host
         ERR_IO          = 121, /// Input/Output error
//EINVAL       =  22, /// Invalid argument
//EFBIG        =  27, /// File too large - not only files, but also passed buffers
//EDOM         =  33, /// Math argument out of domain of func
//ERANGE       =  34, /// Math result not representable
//ENODATA      =  61, /// No data available
//EPROTO       =  71, /// Protocol error
//EOVERFLOW    =  75, /// Value too large for defined data type
//EMSGSIZE     =  90, /// Message too long
//ENOBUFS      = 105, /// No buffer space available
//ETIMEDOUT    = 110, /// Connection timed out
//ECONNREFUSED = 111, /// Connection refused
      };
      // --------------------------------------------------------------------
      enum StateValues
      {
         READY    = 0, /// Ready for asking for data
         NO_VALUE = 1, /// Currently no data available
         INACTIVE = 2  /// Currently inactive - so no values available.
      };
   };
}

#endif
