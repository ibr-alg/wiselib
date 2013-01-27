#ifndef TREENEIGHBORHOOD_H
#define TREENEIGHBORHOOD_H

#include "NeighborhoodIterator.h"
#include "util/serialization/simple_types.h"

template<typename Container_P, typename Neighbor_P>

/** \brief Example implementation for a Neighborhood.
 *
 *  \ingroup nhood_concept
 *
 *  Example implementation of the \ref Nhood_concept "Neighborhood" concept and the
 *  \ref Better_nhood_concept "BetterNeighborhood" concept. This shows a part of a
 *  tree and offers the possibility to not only iterate through all neighbors but
 *  also through all children or through all parents.
 */
class TreeNeighborhood
{
public:
   typedef Container_P Container;
   typedef Neighbor_P Neighbor_t;

   typedef TreeNeighborhood<Container, Neighbor_t> self_type;
   typedef self_type* self_pointer_t;

   typedef NeighborhoodIterator<Container, Neighbor_t> iterator;

   inline iterator neighbors_begin()
   {
      return neighbors_begin( Neighbor<Radio>::BIDI_EDGE );
   }

   inline iterator neighbors_begin( Neighbor<Radio>::State state )
   {
      internal_iterator it = nh_.begin();
      internal_iterator end = nh_.end();
      iterator returned_it = iterator( it, state, end );

      if ( returned_it != neighbors_end() )
      {
         Neighbor_t value = *returned_it;
         if ( ( value->state() & state ) != state )
         {
            ++returned_it; // goes on till end or first neighbor that fits to state
         }
      }
      return returned_it;
   }

   inline iterator neighbors_end()
   {
      internal_iterator it = nh_.end();
      return iterator( it, Neighbor<Radio>::BIDI_EDGE, it );
   }

   inline uint16_t neighbors_count( Neighbor<Radio>::State state )
   {
      uint16_t result = 0;
      for ( iterator iter = neighbors_begin( state ); iter != neighbors_end(); ++iter )
      {
         ++result;
      }
      return result;
   }

   inline void add( Neighbor_t n )
   {
      nh_.push_back( n );
   }

private:
   typedef typename Container::iterator internal_iterator;

   Container nh_;
};

#endif // TREENEIGHBORHOOD_H
/* vim: set ts=3 sw=3 tw=78 expandtab :*/
