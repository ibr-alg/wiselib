#ifndef NEIGHBOR_H
#define NEIGHBOR_H

template<typename Radio_P>

/** \brief Example implementation for a Neighbor.
 *
 *  \ingroup nhood_concept
 *
 *  Example implementation of the \ref Neighbor_concept "Neighbor" concept.
 */
class FloodingNdNeighbor
{
public:
   typedef Radio_P radio_t;
   typedef typename Radio_P::node_id_t node_id_t;

   enum State
   {
      IN_EDGE = 1, OUT_EDGE = 2, BIDI_EDGE = IN_EDGE | OUT_EDGE
   };

   FloodingNdNeighbor()
   {
   }

   FloodingNdNeighbor( node_id_t id, State state )
   {
      id_ = id;
      state_ = state;
   }
   
   inline void set_id( node_id_t id )
   {
      id_ = id;
   }

   inline void set_state( State state )
   {
      state_ = state;
   }

   inline node_id_t id()
   {
      return id_;
   }

   inline State state()
   {
      return state_;
   }

private:
   node_id_t id_; // ID of the node
   State state_; // 1 = in / parent, 2 = out / child, more = edge not directed
};

#endif // NEIGHBOR_H
/* vim: set ts=3 sw=3 tw=78 expandtab :*/
