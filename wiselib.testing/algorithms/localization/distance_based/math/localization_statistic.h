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
#ifndef __ALGORITHMS_LOCALIZATION_DISTANCE_BASED_MATH_STATISTIC_H
#define __ALGORITHMS_LOCALIZATION_DISTANCE_BASED_MATH_STATISTIC_H

#include "util/pstl/list_static.h"
#include "util/pstl/algorithm.h"
#include <math.h>
#include <float.h>
#include <limits.h>


namespace wiselib
{

   /// Class that provides statistic information.
   /** This class provides statistic information such like mean, variance and
    *  standard deviation.
    */
   template<typename OsModel_P,
            int MAX_ENTRIES = 20>
   class LocalizationStatistic
   {

   public:
      typedef OsModel_P OsModel;

      typedef list_static<OsModel, double, MAX_ENTRIES> List;
      typedef typename List::iterator ListIterator;

      typedef LocalizationStatistic<OsModel, MAX_ENTRIES> self_type;

      ///@name construction / destruction
      ///@{
      ///
      LocalizationStatistic();
      LocalizationStatistic( const self_type& );
      ~LocalizationStatistic();
      ///@}


      ///@name adding values
      ///@{
      /** Add value to internal representation.
       *  \sa operator+=()
       */
      void add( double );
      /** Add value to internal representation.
       *  \sa add()
       */
      LocalizationStatistic& operator+=( double );
      ///@}


      ///@name statistic methods
      ///@{
      /** This method gives the mean of all values with:
       *  \f[ \mu = \frac{1}{N}\sum^N_{i=1}x_i \f]
       *
       *  \return average of all values
       *  \sa variance(), std_dev()
       */
      double mean( void );
      /** This method gives the variance of all values with:
       *  \f[ \sigma^2 = \frac{1}{N}\sum^N_{i=1}{(x_i - \mu)^2} \f]
       *
       *  \return variance of all values
       *  \sa mean(), std_dev()
       */
      double variance( void );
      /** This method gives the standard deviation of all values with:
       *  \f[ \sigma = \sqrt{\sigma^2} \f]
       *
       *  \return standard deviation of all values
       *  \sa mean(), variance()
       */
      double std_dev( void );
      /** \return Minimum of all values
       */
      double min( void );
      /** \return Maximum of all values
       */
      double max( void );
      ///@}


      ///@name some internal info
      ///@{
      /** \return Number of values
       */
      size_t size( void );
      /** \return Values as list representation
       */
      List as_list( void );
      ///@}
      void clear( void );

   private:

      List values_;

   };
   // -----------------------------------------------------------------------
   // -----------------------------------------------------------------------
   // -----------------------------------------------------------------------
   template<typename OsModel_P, int MAX_ENTRIES>
   LocalizationStatistic<OsModel_P, MAX_ENTRIES>::
   LocalizationStatistic()
   {}
   // ----------------------------------------------------------------------
   template<typename OsModel_P, int MAX_ENTRIES>
   LocalizationStatistic<OsModel_P, MAX_ENTRIES>::
   LocalizationStatistic( const self_type& ls )
      : values_( ls.as_list() )
   {}
   // ----------------------------------------------------------------------
   template<typename OsModel_P, int MAX_ENTRIES>
   LocalizationStatistic<OsModel_P, MAX_ENTRIES>::
   ~LocalizationStatistic()
   {}
   // ----------------------------------------------------------------------
   template<typename OsModel_P, int MAX_ENTRIES>
   void
   LocalizationStatistic<OsModel_P, MAX_ENTRIES>::
   add( double value )
   {
      values_.push_back( value );
   }
   // ----------------------------------------------------------------------
   template<typename OsModel_P, int MAX_ENTRIES>
   typename LocalizationStatistic<OsModel_P, MAX_ENTRIES>::self_type&
   LocalizationStatistic<OsModel_P, MAX_ENTRIES>::
   operator+=( double value )
   {
      this->add( value );

      return *this;
   }
   // ----------------------------------------------------------------------
   template<typename OsModel_P, int MAX_ENTRIES>
   double
   LocalizationStatistic<OsModel_P, MAX_ENTRIES>::
   mean( void )
   {
      if ( values_.size() == 0 ) return 0;

      double sum = 0;

      for ( ListIterator
               it = values_.begin();
               it != values_.end();
               ++it )
         sum += *it;

      return ( sum / values_.size() );
   }
   // ----------------------------------------------------------------------
   template<typename OsModel_P, int MAX_ENTRIES>
   double
   LocalizationStatistic<OsModel_P, MAX_ENTRIES>::
   variance( void )
   {
      if ( values_.size() == 0 ) return 0;

      double var = 0;
      double avg = mean();

      for ( ListIterator
               it = values_.begin();
               it != values_.end();
               ++it )
      {
         double tmp = *it - avg;
         var += tmp * tmp;
      }

      return ( var / values_.size() );
   }
   // ----------------------------------------------------------------------
   template<typename OsModel_P, int MAX_ENTRIES>
   double
   LocalizationStatistic<OsModel_P, MAX_ENTRIES>::
   std_dev( void )
   {
      return sqrt( variance() );
   }
   // ----------------------------------------------------------------------
   template<typename OsModel_P, int MAX_ENTRIES>
   double
   LocalizationStatistic<OsModel_P, MAX_ENTRIES>::
   min( void )
   {
      if ( size() == 0 )
         return DBL_MAX;

      return *min_element( values_.begin(), values_.end() );
   }
   // ----------------------------------------------------------------------
   template<typename OsModel_P, int MAX_ENTRIES>
   double
   LocalizationStatistic<OsModel_P, MAX_ENTRIES>::
   max( void )
   {
      if ( size() == 0 )
         return DBL_MIN;

      return *max_element( values_.begin(), values_.end() );
   }
   // ----------------------------------------------------------------------
   template<typename OsModel_P, int MAX_ENTRIES>
   size_t
   LocalizationStatistic<OsModel_P, MAX_ENTRIES>::
   size( void )
   {
      return values_.size();
   }
   // ----------------------------------------------------------------------
   template<typename OsModel_P, int MAX_ENTRIES>
   typename LocalizationStatistic<OsModel_P, MAX_ENTRIES>::List
   LocalizationStatistic<OsModel_P, MAX_ENTRIES>::
   as_list( void )
   {
      return values_;
   }
   // ----------------------------------------------------------------------
   template<typename OsModel_P, int MAX_ENTRIES>
   void 
   LocalizationStatistic<OsModel_P, MAX_ENTRIES>::
   clear( void )
   {
      values_.clear();
   }

}// namespace wiselib
#endif
