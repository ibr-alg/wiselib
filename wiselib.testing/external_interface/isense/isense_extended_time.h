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
#ifndef CONNECTOR_ISENSE_EXTENDED_TIME_H
#define CONNECTOR_ISENSE_EXTENDED_TIME_H

#include <isense/time.h>

namespace wiselib
{
   /** \brief iSense Implementation of \ref exttime_concept "Extended Time Concept"
    *  \ingroup exttime_concept
    *
    * iSense implementation of the \ref exttime_concept "Extended Time Concept" ...
    */
   template<typename OsModel_P>
   class iSenseExtendedTime
      : public isense::Time
   {
   public:
      typedef OsModel_P OsModel;
      // --------------------------------------------------------------------
      iSenseExtendedTime()
         : isense::Time()
      {}
      // --------------------------------------------------------------------
      iSenseExtendedTime( isense::Time time )
         : isense::Time( time )
      {}
      // --------------------------------------------------------------------
      iSenseExtendedTime operator+(const iSenseExtendedTime& exttime)
      {
         return isense::Time(*this) + isense::Time(exttime);
      }
      // --------------------------------------------------------------------
      iSenseExtendedTime operator+=(const iSenseExtendedTime& exttime)
      {
         *this = *this + exttime;
         return *this;
      }
      // --------------------------------------------------------------------
      iSenseExtendedTime operator-(const iSenseExtendedTime& exttime)
      {
         return isense::Time(*this) - isense::Time(exttime);
      }
      // --------------------------------------------------------------------
      iSenseExtendedTime operator-=(const iSenseExtendedTime& exttime)
      {
         *this = *this - exttime;
         return *this;
      }
      // --------------------------------------------------------------------
      iSenseExtendedTime operator+(int value)
      {
         return (*this + isense::Time(value));
      }
      // --------------------------------------------------------------------
      iSenseExtendedTime operator+=(int value)
      {
         *this = *this + value;
         return *this;
      }
      // --------------------------------------------------------------------
      iSenseExtendedTime operator-(int value)
      {
         return (*this - isense::Time(value));
      }
      // --------------------------------------------------------------------
      iSenseExtendedTime operator-=(int value)
      {
         *this = *this - value;
         return *this;
      }
      // --------------------------------------------------------------------
      iSenseExtendedTime operator*(int value)
      {
         isense::Time temp( this->sec() * value,
                            this->ms() * value );
         return temp;
      }
      // --------------------------------------------------------------------
      iSenseExtendedTime operator*=(int value)
      {
         *this = *this * value;
         return *this;
      }
      // --------------------------------------------------------------------
      iSenseExtendedTime operator/(int value)
      {
         isense::Time temp( this->sec() / value,
                            this->ms() / value );
         return temp;
      }
      // --------------------------------------------------------------------
      iSenseExtendedTime operator/=(int value)
      {
         *this = *this / value;
         return *this;
      }

   };
}

#endif
