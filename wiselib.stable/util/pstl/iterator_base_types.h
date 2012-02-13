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
#ifndef __WISELIB_INTERNAL_INTERFACE_STL_ITERATOR_BASE_TYPES_H
#define __WISELIB_INTERNAL_INTERFACE_STL_ITERATOR_BASE_TYPES_H

#include <stdint.h>

namespace wiselib
{
#ifdef __unix__
    typedef std::input_iterator_tag          input_iterator_tag;
    typedef std::output_iterator_tag         output_iterator_tag;
    typedef std::forward_iterator_tag        forward_iterator_tag;
    typedef std::bidirectional_iterator_tag  bidirectional_iterator_tag;
    typedef std::random_access_iterator_tag  random_access_iterator_tag;
#else
    struct input_iterator_tag {};
    struct output_iterator_tag {};
    struct forward_iterator_tag : public input_iterator_tag {};
    struct bidirectional_iterator_tag : public forward_iterator_tag {};
    struct random_access_iterator_tag : public bidirectional_iterator_tag {};
#endif
    // -----------------------------------------------------------------------
    // -----------------------------------------------------------------------
    // -----------------------------------------------------------------------
    template<typename Category_P,
             typename Iterator_P,
             typename Distance_P  = int16_t,
             typename Pointer_P   = Iterator_P*,
             typename Reference_P = Iterator_P&>
    struct iterator
    {
      typedef Category_P  iterator_category;
      typedef Iterator_P  value_type;
      typedef Distance_P  difference_type;
      typedef Pointer_P   pointer;
      typedef Reference_P reference;
    };
   // -----------------------------------------------------------------------
   // -----------------------------------------------------------------------
   // -----------------------------------------------------------------------
   template<typename Iterator_P>
   struct iterator_traits
   {
      typedef typename Iterator_P::iterator_category iterator_category;
      typedef typename Iterator_P::value_type        value_type;
      typedef typename Iterator_P::difference_type   difference_type;
      typedef typename Iterator_P::pointer           pointer;
      typedef typename Iterator_P::reference         reference;
   };
   // -----------------------------------------------------------------------
   // -----------------------------------------------------------------------
   // -----------------------------------------------------------------------
   template<typename Iterator_P>
   struct iterator_traits<Iterator_P*>
   {
      typedef random_access_iterator_tag iterator_category;
      typedef Iterator_P  value_type;
      typedef int16_t     difference_type;
      typedef Iterator_P* pointer;
      typedef Iterator_P& reference;
   };

}

#endif
