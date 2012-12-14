#ifndef NEIGHBORHOODITERATOR_H
#define NEIGHBORHOODITERATOR_H

template<typename Container_P, typename Neighbor_P>
class NeighborhoodIterator
{
public:
   typedef typename Container_P::iterator internal_iterator;

   explicit NeighborhoodIterator( internal_iterator it, Neighbor<Radio>::State state,
         internal_iterator current_end ) :
      it_( it ), current_end_( current_end ), state_( state )
   {
   }

   NeighborhoodIterator& operator++()
   {
      ++it_;
      bool found = false;

      while ( ( it_ != current_end_ ) && ( !found ) )
      {
         Neighbor_P n = *it_;
         if ( ( n->state() & state_ ) != state_ )
         {
            ++it_;
         }
         else
         {
            found = true;
         }
      }
      return *this;
   }

   bool operator==( const NeighborhoodIterator& other )
   {
      return it_ == other.it_;
   }

   bool operator!=( const NeighborhoodIterator& other )
   {
      return it_ != other.it_;
   }

   Neighbor_P& operator*() const
   {
      return *it_;
   }

   internal_iterator get_internal_iterator()
   {
      return it_;
   }

private:
   internal_iterator it_;
   internal_iterator current_end_;
   Neighbor<Radio>::State state_;
};

#endif // NEIGHBORHOODITERATOR_H
/* vim: set ts=3 sw=3 tw=78 expandtab :*/
