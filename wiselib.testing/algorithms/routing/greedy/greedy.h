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
#ifndef __GREEDY_H__
#define __GREEDY_H__

#include "util/base_classes/routing_base.h"
#include "greedy_message.h"

#include <string.h>


namespace wiselib
{
   /**\brief Greedy routing algorithm
    * 
    *  \ingroup routing_concept
    *  \ingroup radio_concept
    *  \ingroup basic_algorithm_concept
    *  \ingroup routing_algorithm
    * 
    * Greedy routing algorithm
    */
   template<typename OsModel_P,
			typename Node_P,
			typename NodeList_P,
			typename Timer_P = typename OsModel_P::Timer,
            typename Radio_P = typename OsModel_P::Radio,
            typename Debug_P = typename OsModel_P::Debug>
   class Greedy
      : public RoutingBase<OsModel_P, Radio_P>
   {
   public:
      typedef OsModel_P OsModel;
      typedef Radio_P Radio;
      typedef Debug_P Debug;
      typedef Node_P Node;
      typedef NodeList_P NodeList;
      typedef typename Node::NodePosition::Float Float;
      typedef Timer_P Timer;
      typedef Greedy<OsModel, Node, NodeList, Timer, Radio, Debug> self_type;
      typedef typename Radio::node_id_t node_id_t;
      typedef typename Radio::size_t size_t;
      typedef typename Radio::block_data_t block_data_t;
      typedef typename Radio::message_id_t message_id_t;
      typedef typename NodeList::iterator NodeList_Iterator;
      typedef typename Node::NodePosition Position;
      typedef Greedy_Message<OsModel, Node, Radio> Message;

      // --------------------------------------------------------------------
      enum SpecialNodeIds {
         BROADCAST_ADDRESS = Radio_P::BROADCAST_ADDRESS, ///< All nodes in communication range
         NULL_NODE_ID      = Radio_P::NULL_NODE_ID      ///< Unknown/No node id
      };
      // --------------------------------------------------------------------
      enum Restrictions {
         MAX_MESSAGE_LENGTH = Radio_P::MAX_MESSAGE_LENGTH - Message::PAYLOAD_POS  ///< Maximal number of bytes in payload
      };
      // --------------------------------------------------------------------

      Greedy( node_id_t sid);
      ~Greedy();

      void enable_radio( void );
      void disable_radio( void );

      void send( node_id_t receiver, size_t len, block_data_t *data, message_id_t msg_id, Node n );

      void receive( node_id_t from, size_t len, block_data_t *data );
        
      void send_neighbor_discovery( void* userdata );

      void send_greedily( void* userdata);

      node_id_t greedy_recipient()
      { 
          Node current_greedy_recipient = self;
          Node prospective_greedy_recipient;
          
          double current_closest_distance = distsq( destination, current_greedy_recipient.position );
          double prospective_closest_distance;
            
          for ( NodeList_Iterator neighbors_iterator = neighbors.begin(); neighbors_iterator != neighbors.end(); ++neighbors_iterator )
          {
              prospective_greedy_recipient = *neighbors_iterator;
              prospective_closest_distance = distsq( destination, prospective_greedy_recipient.position );
              
              if ( prospective_closest_distance < current_closest_distance )
              {
                current_closest_distance = prospective_closest_distance;
                current_greedy_recipient = prospective_greedy_recipient;
              }
          }
          return current_greedy_recipient.id;
      };

      void print_neighbors( void* userdata);

      NodeList neighbors;
      Node self;
      node_id_t sender_id;
      Position destination;

      void init( Radio& radio, Timer& timer, Debug& debug ) {
         radio_ = &radio;
         timer_ = &timer;
         debug_ = &debug;
      }
      
      void destruct() {
      }
      
      typename Radio::node_id_t id()
      {
         return radio_->id();
      };
      
   private:
      Radio& radio()
      { return *radio_; }
      
      Timer& timer()
      { return *timer_; }
      
      Debug& debug()
      { return *debug_; }
     
      Radio * radio_;
      Timer * timer_;
      Debug * debug_;

      
	  enum MessageIds
	  {
		  NEIGHBOR_DISCOVERY_MSG_ID,
	      GREEDY_MSG_ID
      };

      int callback_id_;

   };
   template<typename OsModel_P,
			typename Node_P,
			typename NodeList_P,
			typename Timer_P,
            typename Radio_P,
            typename Debug_P>
   Greedy<OsModel_P, Node_P, NodeList_P, Timer_P, Radio_P, Debug_P>::
   Greedy( node_id_t sid)
         : callback_id_  ( 0 )
   {
	   sender_id = sid;
   }
   // -----------------------------------------------------------------------
   template<typename OsModel_P,
			typename Node_P,
			typename NodeList_P,
			typename Timer_P,
            typename Radio_P,
            typename Debug_P>
   Greedy<OsModel_P, Node_P, NodeList_P, Timer_P, Radio_P, Debug_P>::
   ~Greedy()
   {}
   // -----------------------------------------------------------------------
   template<typename OsModel_P,
			typename Node_P,
			typename NodeList_P,
			typename Timer_P,
            typename Radio_P,
            typename Debug_P>
   void
   Greedy<OsModel_P, Node_P, NodeList_P, Timer_P, Radio_P, Debug_P>::
   enable_radio( void )
   {      
      radio().enable;
	  self.id = radio().id;
	  //debug().debug( "Greedy %i: Boot \n", self.id );
	  callback_id_ = radio().template reg_recv_callback<self_type, &self_type::receive>( this );
	  timer().template set_timer<self_type, &self_type::send_neighbor_discovery>( 10000, this, 0 );
	  if ( sender_id == self.id )
	  {
		  timer().template set_timer<self_type, &self_type::send_greedily>( 15000, this, 0 );
	  }
   }
  // -----------------------------------------------------------------------
   template<typename OsModel_P,
			typename Node_P,
			typename NodeList_P,
			typename Timer_P,
            typename Radio_P,
            typename Debug_P>
   void
   Greedy<OsModel_P, Node_P, NodeList_P, Timer_P, Radio_P, Debug_P>::
   disable_radio( void )
   {
      //debug().debug( "Greedy %i: Disable \n", self.id );
      radio().unreg_recv_callback( callback_id_ );
      radio().disable;
   }
  // -----------------------------------------------------------------------
   template<typename OsModel_P,
			typename Node_P,
			typename NodeList_P,
			typename Timer_P,
            typename Radio_P,
            typename Debug_P>
   void
   Greedy<OsModel_P, Node_P, NodeList_P, Timer_P, Radio_P, Debug_P>::
   send( node_id_t destination, size_t len, block_data_t *data, message_id_t msg_id, Node n )
   {
      //debug().debug( "Greedy %i : Send \n", self.id );
      Message message;
      message.set_msg_id( msg_id );
      message.set_node( n );
      message.set_payload( len, data );
      radio().send( destination, message.buffer_size(), (block_data_t*)&message );
   }
  // -----------------------------------------------------------------------
   template<typename OsModel_P,
			typename Node_P,
			typename NodeList_P,
			typename Timer_P,
            typename Radio_P,
            typename Debug_P>
   void
   Greedy<OsModel_P, Node_P, NodeList_P, Timer_P, Radio_P, Debug_P>::
   receive( node_id_t from, size_t len, block_data_t *data )
   {
		//debug().debug( "Greedy %i : Receive \n", self.id );
		Message* message = (Message*)data;
		if ( message->msg_id() == NEIGHBOR_DISCOVERY_MSG_ID )
		{
			NodeList_Iterator neighbors_iterator = neighbors.begin();
			uint8_t found = 0;
			while ( neighbors_iterator!=neighbors.end() && found == 0 )
			{
				if ( neighbors_iterator->id == message->node().id )
				{
					found = 1;
				}
				if ( found == 0 ){ ++neighbors_iterator; }
			}  
			if ( found == 1 )
			{
				neighbors.erase( neighbors_iterator );	
			}
			neighbors.push_back( message->node() );
		}
		else if ( message->msg_id() == GREEDY_MSG_ID )
		{
			debug().debug( "Greedy %i : Receive Greedy from %i \n", self.id, from, message->node().position.x , message->node().position.y );
			destination = message->node().position;
			send_greedily(NULL);
		}
		notify_receivers( from, message->payload_size(), message->payload() );	  
   }
   // -----------------------------------------------------------------------
   template<typename OsModel_P,
			typename Node_P,
			typename NodeList_P,
			typename Timer_P,
            typename Radio_P,
            typename Debug_P>
   void
   Greedy<OsModel_P, Node_P, NodeList_P, Timer_P, Radio_P, Debug_P>::
   send_neighbor_discovery( void* userdata)
   {
	  //debug().debug( "Greedy %i : Send neighbor discovery message \n", self.id );
	  block_data_t bd[1];
	  bd[0] = 'n';
	  send( radio().BROADCAST_ADDRESS,1, bd, NEIGHBOR_DISCOVERY_MSG_ID, self );
   }
   // -----------------------------------------------------------------------
   template<typename OsModel_P,
			typename Node_P,
			typename NodeList_P,
			typename Timer_P,
            typename Radio_P,
            typename Debug_P>
   void
   Greedy<OsModel_P, Node_P, NodeList_P, Timer_P, Radio_P, Debug_P>::
   send_greedily( void* userdata)
   {
	  node_id_t rec_id = greedy_recipient();
	  if (rec_id != self.id)
	  {
		  debug().debug( "Greedy %i : Send greedily to %i \n", self.id, rec_id );
		  Node dest;
		  dest.position = destination;
		  block_data_t bd[1];
		  bd[0] = 'g';
		  send( greedy_recipient(), 1, bd, GREEDY_MSG_ID, dest );
	  }
   }

   // -----------------------------------------------------------------------
   template<typename OsModel_P,
			typename Node_P,
			typename NodeList_P,
			typename Timer_P,
            typename Radio_P,
            typename Debug_P>
   void
   Greedy<OsModel_P, Node_P, NodeList_P, Timer_P, Radio_P, Debug_P>::
   print_neighbors( void* userdata)
   {
	  debug().debug( "Greedy %i : Begin Neighbor printout \n", self.id );
	  for ( NodeList_Iterator neighbors_iterator = neighbors.begin(); neighbors_iterator != neighbors.end(); ++neighbors_iterator )
	  {
		  debug().debug( "--- neighbor: id = %i, position = ( %i, %i, %i ) \n", neighbors_iterator->id, neighbors_iterator->position.x, neighbors_iterator->position.y, neighbors_iterator->position.z );
	  }
	  debug().debug( "Greedy %i : End Neighbor printout \n", self.id );
   }

}
#endif
