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

#ifndef __ALGORITHMS_ROUTING_SECROUTING_H__
#define __ALGORITHMS_ROUTING_SECROUTING_H__

#include "util/base_classes/routing_base.h"

namespace wiselib
{
   /** \brief Sec routing
    * 
    *  \ingroup routing_concept
    *  \ingroup radio_concept
    *  \ingroup basic_algorithm_concept
    *  \ingroup routing_algorithm
    * 
    * 
    */
   template<typename OsModel_P,
            typename Radio_P,
            typename crypto_P,
            typename Routing_P>
   class SecRouting
      : public RoutingBase<OsModel_P, Radio_P>
   {
   public:
      typedef OsModel_P OsModel;
      typedef Radio_P Radio;
      typedef crypto_P crypto_t;
      typedef Routing_P routing_t;
      typedef SecRouting<OsModel, Radio_P,crypto_P, Routing_P> self_type;

      typedef typename Radio::node_id_t node_id_t;
      typedef typename Radio::size_t size_t;
      typedef typename Radio::block_data_t block_data_t;
      typedef uint8_t message_id_t;
      // --------------------------------------------------------------------
      enum SpecialNodeIds {
         BROADCAST_ADDRESS = Radio_P::BROADCAST_ADDRESS, ///< All nodes in communication range
         NULL_NODE_ID      = Radio_P::NULL_NODE_ID      ///< Unknown/No node id
      };
      // --------------------------------------------------------------------
      enum Restrictions {
         MAX_MESSAGE_LENGTH = Radio_P::MAX_MESSAGE_LENGTH   ///< Maximal number of bytes in payload
      };
      // --------------------------------------------------------------------
      ///@name Construction / Destruction
      ///@{
      SecRouting();
      ~SecRouting();
      ///@}
      // --------------------------------------------------------------------
      int init( routing_t& routing, crypto_t& crypto )
      { routing_ = &routing; crypto_ = &crypto; }
      // --------------------------------------------------------------------
      ///@name Radio Concept
      ///@{
      /**
       */
      void enable_radio( void );
      /**
       */
      void disable_radio( void );
      /**
       */
      void send( node_id_t receiver, size_t len, block_data_t *data );
      /**
       */
      void receive( node_id_t from, size_t len, block_data_t *data );
      /**
       */
      typename Radio::node_id_t id()
      {
         return radio_->id();
      };
      ///@}

   private:
      routing_t* routing_;
      crypto_t* crypto_;
   };
   // -----------------------------------------------------------------------
   // -----------------------------------------------------------------------
   // -----------------------------------------------------------------------
   template<typename OsModel_P,
            typename Radio_P,
            typename crypto_P,
            typename Routing_P>
   SecRouting<OsModel_P, Radio_P,crypto_P, Routing_P>::
   SecRouting()
   {}
   // -----------------------------------------------------------------------
   template<typename OsModel_P,
            typename Radio_P,
            typename crypto_P,
            typename Routing_P>
   SecRouting<OsModel_P, Radio_P,crypto_P, Routing_P>::
   ~SecRouting()
   {}
   // -----------------------------------------------------------------------
   template<typename OsModel_P,
            typename Radio_P,
            typename crypto_P,
            typename Routing_P>
   inline void
   SecRouting<OsModel_P,Radio_P ,crypto_P, Routing_P>::
   enable_radio( void )
   {
      routing_->enable_radio();
      routing_->reg_recv_callback<self_type,&self_type::receive>(this);
      crypto_->enable();
   }
   // -----------------------------------------------------------------------
   template<typename OsModel_P,
            typename Radio_P,
            typename crypto_P,
            typename Routing_P>
   void
   SecRouting<OsModel_P, Radio_P,crypto_P, Routing_P>::
   disable_radio( void )
   {

   }
   // -----------------------------------------------------------------------
   template<typename OsModel_P,
            typename Radio_P,
            typename crypto_P,
            typename Routing_P>
   void
   SecRouting<OsModel_P, Radio_P,crypto_P, Routing_P>::
   send( node_id_t receiver, size_t len, block_data_t *data )
   {
      uint8 b[len];
      crypto_->encrypt( data, b, len );
      routing_->send( receiver, len, b );
   }
   // -----------------------------------------------------------------------
   template<typename OsModel_P,
            typename Radio_P,
            typename crypto_P,
            typename Routing_P>
   void
   SecRouting<OsModel_P,Radio_P ,crypto_P, Routing_P>::
   receive( node_id_t from, size_t len, block_data_t *data )
   {
      uint8 b[len];
      crypto_->decrypt( data, b, len);
      notify_receivers( from, len, b );
   }
}
#endif
