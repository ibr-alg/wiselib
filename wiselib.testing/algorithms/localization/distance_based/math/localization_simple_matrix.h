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
#ifndef __ALGORITHMS_LOCALIZATION_DISTANCE_BASED_MATH_SIMPLE_MATRIX_H
#define __ALGORITHMS_LOCALIZATION_DISTANCE_BASED_MATH_SIMPLE_MATRIX_H

#include "util/pstl/vector_static.h"

namespace wiselib
{

   // TODO: pass container as template parameter to matrix
   const int LOCALIZATION_SIMPLE_MATRIX_MAX_VECSIZE = 100;

   template<typename OsModel_P,
            typename T>
   class SimpleMatrix
   {

   public:
      typedef OsModel_P OsModel;
      typedef wiselib::vector_static<OsModel, T, LOCALIZATION_SIMPLE_MATRIX_MAX_VECSIZE> Vector;

      ///@name construction / destruction
      ///@{
      ///
      SimpleMatrix( size_t = 0, size_t = 0 );
      SimpleMatrix( const SimpleMatrix<OsModel, T>& );
      ~SimpleMatrix();
      ///@}

      ///
      inline T& at( size_t, size_t );
      ///
      inline const T& at( size_t, size_t ) const;
      ///
      inline T& operator() ( size_t, size_t );
      ///
      inline const T& operator() ( size_t, size_t ) const;

      ///
      inline SimpleMatrix<OsModel, T>& operator= (
         const SimpleMatrix<OsModel, T>& );
      ///
      inline SimpleMatrix<OsModel, T>& operator*= (
         const SimpleMatrix<OsModel, T>& );
      ///
      inline friend SimpleMatrix<OsModel, T> operator*(
         const SimpleMatrix<OsModel, T>& lsm1, const SimpleMatrix<OsModel, T>& lsm2 )
      {
         SimpleMatrix<OsModel, T> tmp( lsm1 );
         tmp *= lsm2;

         return tmp;
      }
      ///
      SimpleMatrix<OsModel, T>& operator*= ( T );
      ///
      friend SimpleMatrix<OsModel, T> operator*(
         const SimpleMatrix<OsModel, T>& lsm, T value )
      {
         SimpleMatrix<OsModel, T> tmp( lsm );
         tmp *= value;

         return tmp;
      }
      ///
      friend SimpleMatrix<OsModel, T> operator*(
         T value, const SimpleMatrix<OsModel, T>& lsm )
      { return lsm * value; }

      inline SimpleMatrix<OsModel, T>& operator-= (
         const SimpleMatrix<OsModel, T>& );
      ///
      inline friend SimpleMatrix<OsModel, T> operator-(
         const SimpleMatrix<OsModel, T>& lsm1, const SimpleMatrix<OsModel, T>& lsm2 )
      {
         SimpleMatrix<OsModel, T> tmp( lsm1 );
         tmp -= lsm2;

         return tmp;
      }
      inline SimpleMatrix<OsModel, T>& operator+= (
         const SimpleMatrix<OsModel, T>& );
      ///
      inline friend SimpleMatrix<OsModel, T> operator+(
         const SimpleMatrix<OsModel, T>& lsm1, const SimpleMatrix<OsModel, T>& lsm2 )
      {
         SimpleMatrix<OsModel, T> tmp( lsm1 );
         tmp += lsm2;

         return tmp;
      }
      ///
      ///
      SimpleMatrix<OsModel, T> transposed( void );
      ///
      double det( void );
      ///
      SimpleMatrix<OsModel, T> inverse( void );

      SimpleMatrix<OsModel, T> covariance ( void );

      ///
      inline size_t row_cnt( void ) const;
      ///
      inline size_t col_cnt( void ) const;
      ///
      inline const Vector& as_vector( void ) const;

      ///
      template<typename Debug_P>
      void to_debug( Debug_P& debug )
      {

         for ( size_t i=0; i < row_cnt(); i++ )
         {
            for ( size_t j=0; j < col_cnt(); j++ )
               debug.debug( "%f\t",  at( i, j ) );

            debug.debug( "\n" );
         }
      }

   private:
      size_t rows_, cols_;
      Vector matrix_;

   };
   // ----------------------------------------------------------------------
   // ----------------------------------------------------------------------
   // ----------------------------------------------------------------------
   template<typename OsModel_P,
            typename T>
   SimpleMatrix<OsModel_P, T>::
   SimpleMatrix( size_t rows, size_t cols )
      : rows_( rows ),
         cols_ ( cols )
   {
// not needed with static vector
//       matrix_ = std::vector<T>( rows * cols );
   }
   // ----------------------------------------------------------------------
   template<typename OsModel_P,
            typename T>
   SimpleMatrix<OsModel_P, T>::
   SimpleMatrix( const SimpleMatrix& lsm )
      : rows_( lsm.row_cnt() ),
         cols_ ( lsm.col_cnt() ),
         matrix_( lsm.as_vector() )
   {}
   // ----------------------------------------------------------------------
   template<typename OsModel_P,
            typename T>
   SimpleMatrix<OsModel_P, T>::
   ~SimpleMatrix()
   {}
   // ----------------------------------------------------------------------
   template<typename OsModel_P,
            typename T>
   T&
   SimpleMatrix<OsModel_P, T>::
   at( size_t row, size_t col )
   {
      // TODO: return defined "invalid value"
      if ( row < 0 || row > rows_ || col < 0 || col > cols_ )
         return matrix_[0];

      return matrix_[ row*cols_ + col ];
   }
   // ----------------------------------------------------------------------
   template<typename OsModel_P,
            typename T>
   const T&
   SimpleMatrix<OsModel_P, T>::
   at( size_t row, size_t col )
      const
   {
      // TODO: return defined "invalid value"
      if ( row < 0 || row > rows_ || col < 0 || col > cols_ )
         return (*const_cast<Vector*>(&matrix_))[0];

      return (*const_cast<Vector*>(&matrix_))[ row*cols_ + col ];



   }
   // ----------------------------------------------------------------------
   template<typename OsModel_P,
            typename T>
   T&
   SimpleMatrix<OsModel_P, T>::
   operator()( size_t row, size_t col )
   {
      return at( row, col );
   }
   // ----------------------------------------------------------------------
   template<typename OsModel_P,
            typename T>
   const T&
   SimpleMatrix<OsModel_P, T>::
   operator()( size_t row, size_t col )
      const
   {
      return at( row, col );
   }
   // ----------------------------------------------------------------------
   template<typename OsModel_P,
            typename T>
   SimpleMatrix<OsModel_P, T>&
   SimpleMatrix<OsModel_P, T>::
   operator=( const SimpleMatrix<OsModel, T>& lsm )
   {
      rows_ = lsm.row_cnt();
      cols_ = lsm.col_cnt();
      matrix_ = lsm.as_vector();

      return *this;
   }
   // ----------------------------------------------------------------------
   template<typename OsModel_P,
            typename T>
   SimpleMatrix<OsModel_P, T>&
   SimpleMatrix<OsModel_P, T>::
   operator*=( const SimpleMatrix& lsm )
   {
      if ( cols_ != lsm.row_cnt() )
         return *this;

      SimpleMatrix<OsModel, T> tmp( rows_, lsm.col_cnt() );

      for ( size_t i = 0; i < rows_; i++ )
         for ( size_t j = 0; j < lsm.col_cnt(); j++ )
         {
            tmp(i,j) = 0;
            for ( size_t k = 0; k < cols_; k++ )
               tmp(i,j) += at(i,k) * lsm(k,j);
         }

      *this = tmp;
      return *this;
   }
   // ----------------------------------------------------------------------
   template<typename OsModel_P,
            typename T>
   SimpleMatrix<OsModel_P, T>&
   SimpleMatrix<OsModel_P, T>::
   operator*=( T value )
   {
      for ( size_t i = 0; i < rows_; i++ )
         for ( size_t j = 0; j < cols_; j++ )
            at(i,j) *= value;

      return *this;
   }
   // ----------------------------------------------------------------------
   template<typename OsModel_P,
            typename T>
   SimpleMatrix<OsModel_P, T>&
   SimpleMatrix<OsModel_P, T>::
   operator+=( const SimpleMatrix& lsm )
   {
      if ( cols_ != lsm.col_cnt() || rows_ != lsm.row_cnt() )
         return *this;

      SimpleMatrix<OsModel, T> tmp( rows_,cols_);

      for ( size_t i = 0; i < rows_; i++ )
         for ( size_t j = 0; j < cols_; j++ )
         {
            tmp(i,j) = at(i,j) + lsm(i,j);
         }

      *this = tmp;
      return *this;
   }
   // ----------------------------------------------------------------------
   template<typename OsModel_P,
            typename T>
   SimpleMatrix<OsModel_P, T>&
   SimpleMatrix<OsModel_P, T>::
   operator-=( const SimpleMatrix& lsm )
   {
       if ( cols_ != lsm.col_cnt() || rows_ != lsm.row_cnt() )
         return *this;

      SimpleMatrix<OsModel, T> tmp( rows_,cols_);

      for ( size_t i = 0; i < rows_; i++ )
         for ( size_t j = 0; j < cols_; j++ )
         {
            tmp(i,j) = at(i,j) - lsm(i,j);
         }

      *this = tmp;
      return *this;
   }
   // ----------------------------------------------------------------------
   template<typename OsModel_P,
            typename T>
   SimpleMatrix<OsModel_P, T>
   SimpleMatrix<OsModel_P, T>::
   transposed( void )
   {
      SimpleMatrix<OsModel, T> tmp( cols_, rows_ );

      for ( size_t i = 0; i < rows_; i++ )
         for ( size_t j = 0; j < cols_; j++ )
            tmp(j,i) = at(i,j);

      return tmp;
   }
   // ----------------------------------------------------------------------
   template<typename OsModel_P,
            typename T>
   double
   SimpleMatrix<OsModel_P, T>::
   det( void )
   {
      //assert( rows_ == 2 && cols_ == 2);
	if( rows_ == 2 && cols_ == 2)
      return at(0,0) * at(1,1) - at(0,1) * at(1,0);
	 if( rows_ == 3 && cols_ == 3)
		 return ((at(0,0)*at(1,1)*at(2,2)) + (at(0,1)*at(1,2)*at(2,0)) +
		 (at(0,2)*at(1,0)*at(2,1)) - (at(0,2)*at(1,1)*at(2,0)) -
		 (at(0,0)*at(1,2)*at(2,1)) - (at(0,1)*at(1,0)*at(2,2)));
	 else 
		 return 0;
   }
   // ----------------------------------------------------------------------
   template<typename OsModel_P,
            typename T>
   SimpleMatrix<OsModel_P, T>
	   SimpleMatrix<OsModel_P, T>::
	   inverse( void )
   {
	 //  assert( rows_ == 2 && cols_ == 2 && det() != 0 );
		
	   SimpleMatrix<OsModel, T> tmp( *this );
	   if(rows_ == 2 && cols_ == 2 && det() != 0){ 
	   
		T save = tmp(0,0);
	   tmp(0,0) = tmp(1,1);
	   tmp(1,1) = save;

	   tmp(0,1) *= -1;
	   tmp(1,0) *= -1;

	   tmp *= ( 1/det() );
	   }
	   else if( (rows_ == 3 && cols_ == 3 && det() != 0))
	   {
		tmp(0,0) = at(1,1)*at(2,2) - at(1,2)* at(2,1);
	    tmp(0,1) = at(0,2)*at(2,1) - at(0,1)* at(2,2);
		tmp(0,2) = at(0,1)*at(1,2) - at(0,2)* at(1,1);
		tmp(1,0) = at(1,2)*at(2,0) - at(1,0)* at(2,2);
		tmp(1,1) = at(0,0)*at(2,2) - at(0,2)* at(2,0);
		tmp(1,2) = at(0,2)*at(1,0) - at(0,0)* at(1,2);
		tmp(2,0) = at(1,0)*at(2,1) - at(1,1)* at(2,0);
		tmp(2,1) = at(0,1)*at(2,0) - at(0,0)* at(2,1);
		tmp(2,2) = at(0,0)*at(1,1) - at(0,1)* at(1,0);
		tmp *= 1/det();
	   }
	

	   return tmp;
   }

      template<typename OsModel_P,
            typename T>
   SimpleMatrix<OsModel_P, T>
   SimpleMatrix<OsModel_P, T>::
   covariance( void )
   {
      SimpleMatrix<OsModel, T> tmp( *this );

	  tmp=tmp.transposed();
	  tmp*= *this; 
	  tmp=tmp.inverse();
      return tmp;
   }
   // ----------------------------------------------------------------------
   template<typename OsModel_P,
            typename T>
   size_t
   SimpleMatrix<OsModel_P, T>::
   row_cnt( void )
      const
   {
      return rows_;
   }
   // ----------------------------------------------------------------------
   template<typename OsModel_P,
            typename T>
   size_t
   SimpleMatrix<OsModel_P, T>::
   col_cnt( void )
      const
   {
      return cols_;
   }
   // ----------------------------------------------------------------------
   template<typename OsModel_P,
            typename T>
   const typename SimpleMatrix<OsModel_P, T>::Vector&
   SimpleMatrix<OsModel_P, T>::
   as_vector( void )
      const
   {
      return matrix_;
   }

}// namespace wiselib
#endif
