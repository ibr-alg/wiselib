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
#ifndef __GREEDYLOC_H__
#define __GREEDYLOC_H__

#include "algorithms/localization/localization_base.h"

#include "algorithms/localization/greedyloc/greedyloc_message.h"

#include <string.h>

#include "geometry.h"

namespace wiselib
{

    template<typename OsModel_P,
          typename Node_P,
          typename Timer_P = typename OsModel_P::Timer,
          typename Radio_P = typename OsModel_P::Radio,
          typename Debug_P = typename OsModel_P::Debug>
    class Greedyloc
      : public LocalizationBase<OsModel_P, Radio_P>

   {
   public:
      typedef OsModel_P OsModel;
      typedef Radio_P Radio;
      typedef Debug_P Debug;
      typedef Node_P Node;
      typedef typename Node::NodePosition NodePosition;
      typedef NodePosition position_t;
      typedef typename Node::NodePosition::Float Float;
      typedef Timer_P Timer;
      typedef Greedyloc<OsModel, Node, Timer, Radio, Debug> self_type;
      typedef typename Radio::node_id_t node_id_t;
      typedef typename Radio::size_t size_t;
      typedef typename Radio::block_data_t block_data_t;
      typedef typename Radio::message_id_t message_id_t;
      typedef typename Radio::ExtendedData ExtendedData;
      typedef Greedyloc_Message<OsModel, Node, Radio> Message;

      Greedyloc();
      ~Greedyloc();

      void enable( void );
      void disable( void );

      position_t position( ){
          if (anchor_flag == 1)
          {
              return self.position;
          }
          return NULL;
      }

      void send( node_id_t receiver, size_t len, block_data_t *data, message_id_t msg_id, Node n );

      void send_greedyloc_msg( void* userdata );

      void receive( node_id_t from, size_t len, block_data_t *data, ExtendedData const &ext );

      Node self;
      Node anchor1;
      Node anchor2;
      Node anchor3;
      Float R1;
      Float R2;
      Float R3;
      uint8_t anchor_flag;

      void init( Radio& radio, Timer& timer, Debug& debug ) {
         radio_ = &radio;
         timer_ = &timer;
         debug_ = &debug;
      }
      
      void destruct() {
      }
     
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
	      GREEDYLOC_MSG_ID
      };

      int callback_id_;
   };
   template<typename OsModel_P,
			typename Node_P,
			typename Timer_P,
            typename Radio_P,
            typename Debug_P>
   Greedyloc<OsModel_P, Node_P, Timer_P, Radio_P, Debug_P>::
   Greedyloc( )
         :
   		 R1            (-1 ),
		 R2            ( -1 ),
		 R3            ( -1 ),
         callback_id_  ( 0 )

   {}
   // -----------------------------------------------------------------------
   template<typename OsModel_P,
			typename Node_P,
			typename Timer_P,
            typename Radio_P,
            typename Debug_P>
   Greedyloc<OsModel_P, Node_P, Timer_P, Radio_P, Debug_P>::
   ~Greedyloc()
   {}
  // -----------------------------------------------------------------------
   template<typename OsModel_P,
			typename Node_P,
			typename Timer_P,
            typename Radio_P,
            typename Debug_P>
   void
   Greedyloc<OsModel_P, Node_P, Timer_P, Radio_P, Debug_P>::
   enable( void )
   {
      radio().enable_radio(  );
	  self.id = radio().id(  );
	  debug().debug(  "Greedyloc %i: Boot \n", self.id );
	  callback_id_ = radio().template reg_recv_callback<self_type, &self_type::receive>(  this );
	  timer().template set_timer<self_type, &self_type::send_greedyloc_msg>(  10000, this, 0 );
   }
  // -----------------------------------------------------------------------
   template<typename OsModel_P,
			typename Node_P,
			typename Timer_P,
            typename Radio_P,
            typename Debug_P>
   void
   Greedyloc<OsModel_P, Node_P, Timer_P, Radio_P, Debug_P>::
   disable( void )
   {
      debug().debug(  "Greedyloc %i: Disable \n", self.id );
      radio().unreg_recv_callback(  callback_id_ );
      radio().disable(  );
   }
  // -----------------------------------------------------------------------
   template<typename OsModel_P,
			typename Node_P,
			typename Timer_P,
            typename Radio_P,
            typename Debug_P>
   void
   Greedyloc<OsModel_P, Node_P, Timer_P, Radio_P, Debug_P>::
   send( node_id_t destination, size_t len, block_data_t *data, message_id_t msg_id, Node n )
   {
      debug().debug(  "Greedyloc %i : Send\n", self.id );
      Message message;
      message.set_msg_id( msg_id );
      message.set_node( n );
      message.set_payload( len, data );
      debug().debug(  "--- msg_id = %i, id = %i, position = ( %i, %i, %i ) \n", message.msg_id(), message.node().id, message.node().position.x, message.node().position.y, message.node().position.z );
      radio().send(  destination, message.buffer_size(), (block_data_t*)&message );
   }
  // -----------------------------------------------------------------------
   template<typename OsModel_P,
			typename Node_P,
			typename Timer_P,
            typename Radio_P,
            typename Debug_P>
   void
   Greedyloc<OsModel_P, Node_P, Timer_P, Radio_P, Debug_P>::
   receive( node_id_t from, size_t len, block_data_t *data,ExtendedData const &ext )
   {
	    if (anchor_flag == 0)
		{
		   Message* message = (Message*)data;
		   if ( message->msg_id() == GREEDYLOC_MSG_ID )
		   {
				if ( (R1 < 0) && ( R2 < 0 ) && ( R3 < 0 ) )
				{
					debug().debug(  "Greedyloc %i : Received first anchor message from %i\n", self.id, from );
					anchor1 = message->node();
					R1 = ext.link_metric();
				}
				else if ( (R1 >= 0) && ( R2 < 0 ) && ( R3 < 0 ) && ( anchor1.id != from) )
				{
					debug().debug(  "Greedyloc %i : Received second anchor message from %i\n", self.id, from );
					anchor2 = message->node();
					R2 = ext.link_metric();
				}
				else if ( (R1 >= 0) && ( R2 >= 0 ) && ( R3 < 0 ) && (anchor1.id != from) && (anchor2.id != from) )
				{
					debug().debug(  "Greedyloc %i : Received third anchor message from %i\n", self.id, from );
					anchor3 = message->node();
					R3 = ext.link_metric();
					Geometry <NodePosition>GM;
					self.position = GM.trilateration(anchor1.position.x, anchor1.position.y, R1, anchor2.position.x, anchor3.position.y, R2, anchor3.position.x, anchor3.position.y, R3);
					anchor_flag = 1;
				}
		   }
		}
   }
   // -----------------------------------------------------------------------
   template<typename OsModel_P,
			typename Node_P,
			typename Timer_P,
            typename Radio_P,
            typename Debug_P>
   void
   Greedyloc<OsModel_P, Node_P, Timer_P, Radio_P, Debug_P>::
   send_greedyloc_msg( void* userdata )
   {
	   if (anchor_flag == 1)
	   {
	       send( radio().BROADCAST_ADDRESS,0, NULL, GREEDYLOC_MSG_ID, self );
	   }
	   timer().template set_timer<self_type, &self_type::send_greedyloc_msg>(  10000, this, 0 );
   }
}
#endif
