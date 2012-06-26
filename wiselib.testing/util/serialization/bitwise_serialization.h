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
#ifndef __WISELIB_UTIL_SERIALIZATION_BITWISE_SERIALIZATION_H
#define __WISELIB_UTIL_SERIALIZATION_BITWISE_SERIALIZATION_H

namespace wiselib
{
   template <typename OsModel_P,
             typename BlockData_P,
             typename Type_P>
   struct Bitwise_Serialization
   {
      typedef OsModel_P OsModel;
      typedef BlockData_P BlockData;
      typedef Type_P Type;

      typedef typename OsModel::size_t size_t;
      // --------------------------------------------------------------------
      static inline size_t write( BlockData *target, Type& value, uint8_t target_shift, uint8_t value_length)
      {
	 
	//Default size is the full size of the type
	if( value_length == 0 )
	  value_length = sizeof( Type ) * 8;
	
	//Byte shift from the start
	int position = 0;
	
	while( 1 )
	{
	  //If the remaining bits are less than the space in the actual byte
	  if( value_length <= ( 8 - target_shift ))
	  {
	    //Clear the bits
	    //Set 1 to the bits upper than the value
	    BlockData mask = (0xFF << ( 8 - target_shift ));
	    //Set 1 to the bits lower than the value
	    mask |= ~(0xFF << ( 8 - target_shift - value_length ));
	    target[position] &= mask;
	    
	    target[position] |= value << ( 8 - value_length - target_shift );
	    return sizeof(Type);
	  }
	  //If there will be an overflow
	  
	  else
	  {
	   //Clear the lower bits in the byte if target_shift exists or the whole byte if not.
	   target[position] &= ~(0xFF >> target_shift);
	   
	     //remaining length
	    value_length -= ( 8 - target_shift );
	    
	    
	    
	    //Shift out the overflow bits
	    //1. Shift the actual highest bit to the top of the byte
	    //2. Mask (FF), the higher bits will be 0 after this, (if there is a target_shift the length will be less than 8 bit, but all the upper bits will be 0
	    target[position] |= ((value >> value_length) & 0xFF );
	   
	    //target_shift is just for the first byte
	    target_shift = 0;
	    //Byte shift
	    position++;
	  }
	}
      }
      // --------------------------------------------------------------------
      static Type read( BlockData *target, uint8_t target_shift, uint8_t value_length )
      {
	
        Type value;
	//memset( value, 0, sizeof( Type ));
	//for( int i=0; i < sizeof(Type); i++)
	  value=0;
	
	//Default size is the full size of the type
	if( value_length == 0 )
	  value_length = sizeof( Type ) * 8;
	
	//Byte shift from the start
	int position = 0;
	
	while( 1 )
	{
	  //Mask for the upper bits
	    // mask: 11111111
	    BlockData mask = 0xFF;
	    for( uint8_t i = 0; i < target_shift; i++ )
	    {
	      //mask: 01111111 ...
		mask /= 2;
	    }
	    
	    
	  //If the remaining bits are in the actual byte
	  if( value_length <= ( 8 - target_shift ))
	  {
	    //Free up space
	    value <<= ( value_length );
	    //1. Shift out the bits before the target_shift
	    //2. Shift the bits to the lowest position
	    value |= (( target[position] & (mask) ) >> ( 8 - value_length - target_shift));
	    return value;
	  }
	  //If there will be an overflow
	  else
	  {
	    //Free up space
	    value <<= ( 8 - target_shift );

	    value |= ( target[position] & (mask) );

	    //remaining length
	    value_length -= ( 8 - target_shift );
	    //target_shift is just for the first byte
	    target_shift = 0;
	    //Byte shift
	    position++;
	  }
	}
      }

   };
   
   
   template<typename OsModel_P,
            typename BlockData_P,
            typename Type_P>
   inline Type_P bitwise_read( BlockData_P *target, uint8_t shift=0, uint8_t value_length=0 )
   {
      return Bitwise_Serialization<OsModel_P, BlockData_P, Type_P>::read( target, shift, value_length );
   }

   // -----------------------------------------------------------------------
   template<typename OsModel_P,
            typename BlockData_P,
            typename Type_P>
   inline typename OsModel_P::size_t bitwise_write( BlockData_P *target, Type_P& value,  uint8_t shift=0, uint8_t value_length=0 )
   {
      return Bitwise_Serialization<OsModel_P, BlockData_P, Type_P>::write( target, value, shift, value_length );
   }
}
#endif
