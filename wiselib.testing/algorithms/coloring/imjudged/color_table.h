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
#ifndef __INTERNAL_INTERFACE_COLORING_COLORS_TABLE__
#define __INTERNAL_INTERFACE_COLORING_COLORS_TABLE__

#include "util/pstl/pair.h"

#ifndef __DEBUG_SORT_COLOR__
#define __DEBUG_SORT_COLOR__
#endif


namespace wiselib {

    template<typename OsModel_P,
             typename Value_P,
             uint16_t VECTOR_SIZE>
    class ColorsTable
    {
    public:
        typedef OsModel_P OsModel;
        typedef typename OsModel_P::Debug Debug;
        typedef typename OsModel_P::Radio Radio;

        typedef typename Radio::node_id_t node_id_t;
        typedef typename Radio::size_t size_t;
        typedef typename Radio::block_data_t block_data_t;

        typedef Value_P color_value_type;
        typedef color_value_type* color_pointer;
        typedef color_value_type& color_reference;

        typedef pair<color_value_type, uint16_t> value_type;
        typedef value_type* pointer;
        typedef value_type& reference;
        
        typedef typename OsModel_P::size_t size_type;
        

        ColorsTable()
        {
            start_ = &vec_[0];
            finish_ = start_;
            end_of_storage_ = start_ + VECTOR_SIZE;
        }

        //TODO: change the value type to data_block_t
        ColorsTable( const uint8_t *data, size_type len)
        {
             memcpy( vec_, data, len );
             start_ = &vec_[0];
             finish_ = start_ + (len/ sizeof( value_type ));
             end_of_storage_ = start_ + VECTOR_SIZE;
        }

        void parse_array( const uint8_t *data, size_type len)
        {
             memcpy( vec_, data, len );
             start_ = &vec_[0];
             finish_ = start_ + (len/ sizeof( value_type ));
             end_of_storage_ = start_ + VECTOR_SIZE;
        }

        ~ColorsTable()
        {
            
        }

        void
        remove(const color_value_type& color)
        {
            pointer tmp;
            tmp = start_;

            for ( ; tmp != finish_; tmp++ )
            {
                if ( tmp->first == color )
                {
                    (tmp->second)--;
                    break;
                }
            }

            if ( tmp == finish_ )
            {
                return;
            }

            for ( ; tmp!=finish_ ; tmp++ )
            {
                if ( tmp->second < (tmp+1)->second )
                {
                    swap(tmp, tmp+1);
                }
                else if ( tmp->second == 0 )
                {
                    finish_--;
                    break;
                }
            }

        }

        void
        insert(const color_value_type& color)
        {

            pointer tmp;
            tmp = start_;

            for ( ; tmp != finish_; tmp++ )
            {
                if ( tmp->first == color )
                {
                    (tmp->second)++;
                    break;
                }
            }

            if ( tmp == finish_ )
            {
                tmp->first = color;
                tmp->second = 1;
                finish_++;
            }

            for ( ; tmp!=start_ ; tmp-- )
            {
                if ( tmp->second > (tmp-1)->second )
                {
                    swap(tmp, tmp-1);
                }
                else
                {
                    break;
                }
            }

       }

      size_type bytes()
      { return size_type(finish_ - start_)*sizeof(value_type); }

      ///@name Capacity
      ///@{
      size_type size()
      { return size_type(finish_ - start_); }
      // --------------------------------------------------------------------
      size_type max_size()
      { return VECTOR_SIZE; }
      // --------------------------------------------------------------------
      size_type capacity()
      { return VECTOR_SIZE; }
      // --------------------------------------------------------------------
      bool empty()
      { return size() == 0; }

      void clear()
      { finish_ = start_; }

      ///@}
      // --------------------------------------------------------------------
      ///@name Element Access
      ///@{
      color_value_type operator[](size_type n)
      {
         return (*(this->start_ + n)).first;
      }
      // --------------------------------------------------------------------
      
      ColorsTable& operator=( ColorsTable& ct )
      {
          memcpy( vec_, ct.vec_, sizeof(vec_) );
          start_ = &vec_[0];
          finish_ = start_ + (ct.finish_ - ct.start_);
          end_of_storage_ = start_ + VECTOR_SIZE;

          return *this;
      }
      // --------------------------------------------------------------------

      uint16_t cardinality(color_value_type& color)
      {
          pointer tmp;
          tmp = start_;

          for ( ; tmp != finish_; tmp++ )
          {
              if ( tmp->first == color )
              {
                  return tmp->second;
              }
          }

          return -1;
      }
      // --------------------------------------------------------------------
      uint16_t cardinalityAt(size_type n)
      {
          return (*(this->start_ + n)).second;
      }
      // --------------------------------------------------------------------
      color_reference at(size_type n)
      {
         return (*this)[n];
      }
      // --------------------------------------------------------------------
      uint8_t*
      data()
      { return (uint8_t*)start_; }
      
      inline void swap(pointer color, pointer color1)
      {
          value_type tmp_data;
          pointer tmp = &tmp_data;
          
          tmp->first = color->first;
          tmp->second = color->second;

          color->first = color1->first;
          color->second = color1->second;

          color1->first = tmp->first;
          color1->second = tmp->second;
      }

        value_type vec_[VECTOR_SIZE];

    private:
        pointer start_, finish_, end_of_storage_;
    };

   
}

#endif
