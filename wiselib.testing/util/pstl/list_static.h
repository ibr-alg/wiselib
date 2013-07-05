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
#ifndef __WISELIB_INTERNAL_INTERFACE_STL_LIST_STATIC_H
#define __WISELIB_INTERNAL_INTERFACE_STL_LIST_STATIC_H

#include <string.h>

#include "util/pstl/iterator.h"
#include "util/pstl/reverse_iterator.h"

namespace wiselib {

   /// type for list size
   typedef int list_size_t;

   template<typename Value_P>
   struct list_node
   {
      // --------------------------------------------------------------------
      typedef list_node<Value_P> node_type;
      typedef node_type* pointer;
      typedef node_type& reference;
      // --------------------------------------------------------------------
      /// default constructor
      list_node() :
         element_(), next_( this ), prev_( this )
      {

      }
      // --------------------------------------------------------------------
      /// insert node in chain "before" position
      void hook( pointer const position )
      {
         if ( position == 0 )
         {
            // TODO: unhook?
            return;
         }

         next_ = position;
         prev_ = position->prev_;

         next_->prev_ = this;
         prev_->next_ = this;
      }
      // --------------------------------------------------------------------
      /// remove node from chain
      void unhook()
      {
         prev_->next_ = next_;
         next_->prev_ = prev_;

         next_ = this;
         prev_ = this;
      }
      // --------------------------------------------------------------------
      /// swap neighbor nodes
      void reverse()
      {
         pointer tmp = next_;
         next_ = prev_;
         prev_ = tmp;
      }
      // --------------------------------------------------------------------
      void swap( reference node )
      {
         node_type tmp = node;
         node = *this;
         *this = tmp;
      }
      // --------------------------------------------------------------------
      Value_P element_;
      pointer next_;
      pointer prev_;
      // --------------------------------------------------------------------
   };
   // -----------------------------------------------------------------------
   // -----------------------------------------------------------------------
   // -----------------------------------------------------------------------
   template<typename Value_P>
   class list_iterator
   {
   public:
      typedef list_size_t difference_type;
      typedef list_iterator<Value_P> iterator_type;
      typedef list_node<Value_P> node_type;
      typedef Value_P* pointer;
      typedef Value_P& reference;
      // --------------------------------------------------------------------
      /// default constructor
      list_iterator( node_type* node ) :
         node_( node )
      {

      }
      // --------------------------------------------------------------------
      node_type* node()
      {
         return node_;
      }
      // --------------------------------------------------------------------
      reference operator*() const
      {
         return node_->element_;
      }
      // --------------------------------------------------------------------
      pointer operator->() const
      {
         return &node_->element_;
      }
      // --------------------------------------------------------------------
      iterator_type& operator++()
      {
         node_ = node_->next_;
         return *this;
      }
      // --------------------------------------------------------------------
      iterator_type operator++( int )
      {
         iterator_type tmp = *this;
         node_ = node_->next_;
         return tmp;
      }
      // --------------------------------------------------------------------
      iterator_type& operator--()
      {
         node_ = node_->prev_;
         return *this;
      }
      // --------------------------------------------------------------------
      iterator_type operator--( int )
      {
         iterator_type tmp = *this;
         node_ = node_->prev_;
         return tmp;
      }
      // --------------------------------------------------------------------
      bool operator==( const iterator_type& x ) const
      {
         return node_ == x.node_;
      }
      // --------------------------------------------------------------------
      bool operator!=( const iterator_type& x ) const
      {
         return node_ != x.node_;
      }
      // --------------------------------------------------------------------
   private:
      node_type* node_;
      // --------------------------------------------------------------------
      // we don't want anybody to create iterators
      list_iterator() :
         node_( 0 )
      {

      }
   };
   // -----------------------------------------------------------------------
   // -----------------------------------------------------------------------
   // -----------------------------------------------------------------------
   template<typename OsModel_P, typename Value_P, list_size_t ListSize_P>
   struct list_static
   {
      // --------------------------------------------------------------------
      // we do not want empty lists (or less)
      static const list_size_t LIST_SIZE = (ListSize_P < 1) ? 1 : ListSize_P;
      // --------------------------------------------------------------------
      typedef list_iterator<Value_P> iterator;
      typedef list_static<OsModel_P, Value_P, ListSize_P> list_type;
      typedef list_node<Value_P> node_type;
      typedef reverse_iterator<iterator> riterator;
      typedef Value_P value_type;
      typedef value_type* pointer;
      typedef value_type& reference;
      // --------------------------------------------------------------------
      /// default constructor
      list_static() :
         data_(), empty_nodes_()
      {
         init();
      }
      // --------------------------------------------------------------------
      /// copy constructor
      list_static(const list_static& list) :
         data_(), empty_nodes_()
      {
         init();
         *this = list;
      }
      // --------------------------------------------------------------------
      list_static& operator=( const list_static& list )
      {
         clear();
         for(iterator it = list.begin();it != list.end();++it)
         {
            push_back(*it);
         }
         return *this;
      }
      // --------------------------------------------------------------------
      ///@name Iterators
      ///@{
      /// @return iterator to first list node
      iterator begin() const
      {
         return iterator( data_.next_ );
      }
      // --------------------------------------------------------------------
      /// @return iterator to node after last list element
      iterator end() const
      {
         node_type* tmp = const_cast<node_type*> ( &nodes_[LIST_SIZE] );
         return iterator( tmp );
      }
      // --------------------------------------------------------------------
      /// @return reverse iterator to first list node
      riterator rbegin()
      {
         // skip dummy node
         iterator tmp = end();
         --tmp;
         return riterator( tmp );
      }
      // --------------------------------------------------------------------
      /// @return reverse iterator to node after last list element
      riterator rend()
      {
         // skip dummy node
         iterator tmp = begin();
         --tmp;
         return rterator( tmp );
      }
      ///@}
      // --------------------------------------------------------------------
      ///@name Capacity
      ///@{
      /// @return true if list contains no elements
      bool empty() const
      {
         return (begin() == end());
      }
      // --------------------------------------------------------------------
      /// @return true if no elements can be inserted into list
      bool full() const
      {
         return (size() == max_size());
      }
      // --------------------------------------------------------------------
      /// @return size() of largest possible list
      list_size_t max_size() const
      {
         return LIST_SIZE;
      }
      // --------------------------------------------------------------------
      /// @return number of list elements
      list_size_t size() const
      {
         return end() - begin();
      }
      // --------------------------------------------------------------------
      /// @return maximal number of list elements
      list_size_t capacity() const
      {
         return LIST_SIZE;
      }
      /// @}
      // --------------------------------------------------------------------
      ///@name Element Access
      ///@{
      /// @return reference to first element
      value_type& front()
      {
         return *begin();
      }
      // --------------------------------------------------------------------
      /// @return constant reference to first element
      const value_type& front() const
      {
         return *begin();
      }
      // --------------------------------------------------------------------
      /// @return reference to last element
      value_type& back()
      {
         iterator tmp = end();
         --tmp;
         return *tmp;
      }
      // --------------------------------------------------------------------
      /// @return constant reference to last element
      const value_type& back() const
      {
         iterator tmp = end();
         --tmp;
         return *tmp;
      }
      ///@}
      // --------------------------------------------------------------------
      ///@name Modifiers
      ///@{
      /// remove element at given position
      const node_type* erase( iterator position )
      {
         if(empty())
         {
            return 0;
         }
         position.node()->unhook();
         put_node( position.node() );
         return position.node();
      }
      // --------------------------------------------------------------------
      /// remove all elements in given range
      void erase( iterator first, iterator last )
      {
         iterator it = first;
         while ( it != last )
         {
            erase( it );
            it++;
            if ( it == first )
            {
               return;
            }
         }
      }
      // --------------------------------------------------------------------
      /// insert new element at given position with given value
      bool insert( iterator position, const value_type& x )
      {
         node_type* tmp = create_node( x );

         // no space left
         if ( tmp == 0 )
         {
            return false;
         }

         tmp->hook( position.node() );
         return true;
      }
      // --------------------------------------------------------------------
      /// insert n elements with given value at given position
      void insert( iterator position, list_size_t n, const value_type& x )
      {
         for ( list_size_t i = 0; i < n; i++ )
         {
            insert( position, x );
         }
      }
      // --------------------------------------------------------------------
      /**
       * copy elements from first to last at position
       * @return true if successful
       */
      template<typename InputIterator_P>
      bool insert( iterator position, InputIterator_P first,
         InputIterator_P last )
      {
         InputIterator_P it = first;

         while ( it != last )
         {
            if ( !insert( position, *it ) )
            {
               return false;
            }
            position++;
            it++;
            if ( it == first )
            {
               return false;
            }
         }
         return true;
      }
      // --------------------------------------------------------------------
      /// insert element at list end
      bool push_back( const value_type& x )
      {
         return insert( end(), x );
      }
      // --------------------------------------------------------------------
      /// insert element at list beginning
      bool push_front( const value_type& x )
      {
         return insert( begin(), x );
      }
      // --------------------------------------------------------------------
      /// removes last element
      const node_type* pop_back()
      {
         return erase( iterator(end().node()->prev_) );
      }
      // --------------------------------------------------------------------
      /// removes first element
      const node_type* pop_front()
      {
         return erase( begin() );
      }
      // --------------------------------------------------------------------
      /// reverse order of list elements
      void reverse()
      {
         iterator it = begin();

         do
         {
            it.node()->reverse();

            // going backwards after reverse()
            it--;
         }
         while ( it != begin() );
      }
      // --------------------------------------------------------------------
      /// remove all list elements in constant time
      void clear()
      {
         if ( empty() )
         {
            return;
         }

         node_type* first_element = data_.next_;

         first_element->next_->prev_ = empty_nodes_.prev_;
         empty_nodes_.prev_->next_ = first_element->next_;

         first_element->next_ = &empty_nodes_;
         empty_nodes_.prev_ = first_element;

         data_.unhook();
      }
      // --------------------------------------------------------------------
      /// remove every element with given value
      void remove( const value_type& value )
      {
         for ( iterator it = begin(); it != end(); ++it )
         {
            while ( *it == value )
            {
               erase( it++ );
               if ( it == end() )
               {
                  break;
               }
            }
         }
      }
      // --------------------------------------------------------------------
      /// transfer elements of list l to position
      void splice( iterator position, list_type& l )
      {
         while ( !full() && !l.empty() )
         {
            insert( position, l.pop_front()->elment_ );
            position++;
         }
      }
      // --------------------------------------------------------------------
      /**
       *  removes every but the first occurrence in a consecutive set of
       *  elements with the same value
       */
      void unique()
      {
         iterator it = begin();
         iterator next = it;
         while ( it != end() )
         {
            ++next;
            while ( *it == *next )
            {
               erase( next++ );
               if ( next == end() )
               {
                  break;
               }
            }
            it = next;
         }
      }
      // --------------------------------------------------------------------
      ///@}
      // --------------------------------------------------------------------
   private:
      node_type nodes_[LIST_SIZE];
      node_type data_;
      node_type empty_nodes_;
      // --------------------------------------------------------------------
      /// @return node with value x or 0 if list full
      node_type* create_node( const value_type& x )
      {
         node_type* tmp = get_node();

         if ( tmp != 0 )
         {
            tmp->element_ = x;
         }

         return tmp;
      }
      // --------------------------------------------------------------------
      /// @return empty node or 0 if list full
      node_type* get_node()
      {
         // no node left
         if ( &empty_nodes_ == empty_nodes_.next_ )
         {
            return 0;
         }

         node_type* tmp = empty_nodes_.next_;
         tmp->unhook();

         return tmp;
      }
      // --------------------------------------------------------------------
      /// initialize chain
      void init()
      {
         for ( int i = 0; i < LIST_SIZE - 1; i++ )
         {
            nodes_[i].next_ = &nodes_[i + 1];
            nodes_[i + 1].prev_ = &nodes_[i];
         }

         // close chain
         nodes_[LIST_SIZE - 1].next_ = &nodes_[0];
         nodes_[0].prev_ = &nodes_[LIST_SIZE - 1];

         // insert dummy node
         empty_nodes_.hook( &nodes_[0] );
      }
      // --------------------------------------------------------------------
      /// add node to empty nodes
      void put_node( node_type* node )
      {
         // TODO: what if node is 0?
         node->hook( empty_nodes_.next_ );
      }
      // --------------------------------------------------------------------
   };
   // -----------------------------------------------------------------------
   // -----------------------------------------------------------------------
   // -----------------------------------------------------------------------
   /// calculate distance between two list iterators
   template<typename Value_P>
inline   typename list_iterator<Value_P>::difference_type
   operator-(const list_iterator<Value_P>& lhs,
      const list_iterator<Value_P>& rhs)
   {
      list_iterator<Value_P> tmp = rhs;
      typename list_iterator<Value_P>::difference_type i = 0;

      while(tmp != lhs)
      {
         ++tmp;
         if(tmp == rhs)
         {
            // lhs not in rhs's list
            return -1;
         }
         ++i;
      }
      return i;
   }
}

#endif /* __WISELIB_INTERNAL_INTERFACE_STL_LIST_STATIC_H */
