#ifndef CONVERGECAST_H
#define CONVERGECAST_H

template<typename OsModel_P, typename Debug_P, typename Radio_P, typename Neighbor_P,
      typename Neighborhood_P, typename Container_P, typename MessageType_P>

/** \brief Example implementation of a convergecast algorithm.
 *
 *  \ingroup Ccast_concept
 *
 *  Example implementation of the \ref CCast_concept "Convergecast" concept.
 */
class Convergecast
{
public:
   typedef OsModel_P OsModel;
   typedef Debug_P Debug;
   typedef Radio_P Radio;
   typedef Neighbor_P Neighbor_t;
   typedef Neighborhood_P Neighborhood;
   typedef Container_P Container;
   typedef MessageType_P Message;

   typedef Convergecast<OsModel, Debug, Radio, Neighbor_t, Neighborhood, Container, Message> self_type;
   typedef self_type* self_pointer_t;

   typedef typename Debug::self_pointer_t Debug_t;
   typedef typename Radio::self_pointer_t Radio_t;
   typedef typename Neighborhood::self_pointer_t Neighborhood_t;
   typedef typename Neighborhood::iterator neighbors_iterator_t;
   typedef typename Radio::block_data_t block_data_t;
   typedef typename Radio::size_t size_t;
   typedef typename Radio::node_id_t node_id_t;
   typedef typename Container::iterator container_iterator_t;
   typedef typename Neighbor_t::State state_t;

   typedef delegate1<void, Message&> finished_delegate_t;
   typedef delegate2<Message, Container&, Message&> function_delegate_t;

   int init( Debug_t debug, Radio_t radio, Neighborhood_t neighbors, uint8_t cc_id )
   {
      debug_ = debug;
      radio_ = radio;
      neighbors_ = neighbors;
      cc_id_ = cc_id;

      return init();
   }

   int init()
   {
      radio_->template reg_recv_callback<self_type, &self_type::on_receive> ( this );

      child_t = Neighbor_t::OUT_EDGE;
      parent_t = Neighbor_t::IN_EDGE;

      return OsModel::SUCCESS;
   }

   template<class T, Message(T::*TMethod)( Container&, Message& )>
   void reg_function_callback( T *obj_pnt )
   {
      debug_->debug( "function callback registered" );
      function_delegate = function_delegate_t::template from_method<T, TMethod>( obj_pnt );
   }

   template<class T, void(T::*TMethod)( Message& )>
   void reg_finished_callback( T *obj_pnt )
   {
      finished_delegate = finished_delegate_t::template from_method<T, TMethod>( obj_pnt );
   }

   void startConvergecast()
   {
      Message message;
      message.set_command_type( Message::CMD_CCINIT );
      message.set_cc_id( cc_id_ );

      for ( neighbors_iterator_t iter = neighbors_->neighbors_begin( child_t ); iter
            != neighbors_->neighbors_end(); ++iter )
      {
         Neighbor_t* n = *iter;
         radio_->send( n->id(), message.size(), message.data() );

         debug_->debug( "cc %d: %d sends now to %d\n", cc_id_, radio_->id(), n->id() );
      }
   }

private:
   void onReceiveCall()
   {
      buffer_.clear();

      Message message;
      message.set_cc_id( cc_id_ );

      if ( neighbors_->neighbors_begin( child_t ) == neighbors_->neighbors_end() )
      {
         // no children => call function
         // send response to parent
         debug_->debug( "start calculation" );

         message.set_command_type( Message::CMD_CCRESPONSE );
         message = function_delegate( buffer_, message );
         debug_->debug( "calculation done" );

         neighbors_iterator_t iter = neighbors_->neighbors_begin( parent_t );
         Neighbor_t* n = *iter;
         radio_->send( n->id(), message.size(), message.data() );
      }
      else
      {
         // else forward init message to children
         message.set_command_type( Message::CMD_CCINIT );
         for ( neighbors_iterator_t iter = neighbors_->neighbors_begin( child_t ); iter
               != neighbors_->neighbors_end(); ++iter )
         {
            Neighbor_t* n = *iter;
            radio_->send( n->id(), message.size(), message.data() );

            debug_->debug( "cc %d: %d forwards to %d\n", cc_id_, radio_->id(), n->id() );
         }
      }
   }

   void onReceiveResponse( Message msg )
   {
      // save message to buffer
      buffer_.push_back( msg );

      if ( buffer_.size() == neighbors_->neighbors_count( child_t ) )
      {
         // responses from all children received, send own response
         // call function that uses both the own value and the values from the children
         Message message;
         message.set_command_type( Message::CMD_CCRESPONSE );
         message.set_cc_id( cc_id_ );

         message = function_delegate( buffer_, message );

         neighbors_iterator_t n_iter = neighbors_->neighbors_begin( parent_t );
         if ( n_iter == neighbors_->neighbors_end() )
         {
            // this is the root
            finished_delegate( message );
         }
         else
         {
            Neighbor_t* n = *n_iter;
            radio_->send( n->id(), message.size(), message.data() );

            debug_->debug( "cc %d: %d sends answer to parents\n", cc_id_, radio_->id() );
         }
      }
   }

   void on_receive( node_id_t from, size_t len, block_data_t* data )
   {
      debug_->debug( "cc %d: %d received message from %d\n", cc_id_, radio_->id(), from );

      Message *msg = reinterpret_cast<Message*> ( data );

      if ( msg->cc_id() == cc_id_ )
      {
         if ( msg->command_type() == Message::CMD_CCINIT )
         {
            onReceiveCall();
         }

         if ( msg->command_type() == Message::CMD_CCRESPONSE )
         {
            onReceiveResponse( *msg );
         }
      }
   }

   uint8_t cc_id_;
   Debug_t debug_;
   Radio_t radio_;
   Neighborhood_t neighbors_;
   Container buffer_;

   state_t child_t;
   state_t parent_t;

   function_delegate_t function_delegate;
   finished_delegate_t finished_delegate;
};

#endif // CONVERGECAST.H
/* vim: set ts=3 sw=3 tw=78 expandtab :*/
