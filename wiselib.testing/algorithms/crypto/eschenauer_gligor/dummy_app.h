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
 **                                                                       **
 ** Author: Christoph Knecht, University of Bern 2010                     **
 ***************************************************************************/
#ifndef DUMMY_APP_H
#define DUMMY_APP_H

#include "algorithm/routing/dsdv_routing.h"
#include <string.h>

namespace wiselib
{
   template<typename OsModel_P,
            typename Encryption_P,
            typename Radio_P = typename OsModel_P::Radio,
            typename Debug_P = typename OsModel_P::Debug>
   class DummyApp
      : public RoutingBase<OsModel_P, Radio_P>
   {
   public:
      typedef OsModel_P OsModel;
      typedef Radio_P Radio;
      typedef Debug_P Debug;
      typedef Encryption_P EncryptionType;
      typedef DummyApp<OsModel, EncryptionType, Radio, Debug> self_type;
      typedef typename Radio::node_id_t node_id_t;
      typedef typename Radio::size_t size_t;
      typedef typename Radio::block_data_t block_data_t;
      typedef typename Radio::message_id_t message_id_t;

      inline DummyApp();
      inline void enable();
      inline void disable();
      inline void send(  node_id_t, size_t, block_data_t* );
      inline void receive( node_id_t, size_t, block_data_t* );

      inline void set_encryption( EncryptionType* encryption )
      { encryption_ = encryption; };

      inline EncryptionType* encryption()
      { return encryption_; };

   private:
      EncryptionType *encryption_;

      int callback_id_;
      uint16_t seq_nr_;
   };
   // -----------------------------------------------------------------------
   // -----------------------------------------------------------------------
   // -----------------------------------------------------------------------
   template<typename OsModel_P,
            typename Encryption_P,
            typename Radio_P,
            typename Debug_P>
   DummyApp<OsModel_P, Encryption_P, Radio_P, Debug_P>::
   DummyApp()
      :  callback_id_ ( 0 ),
         seq_nr_      ( 0 )
   {}
   // -----------------------------------------------------------------------
   template<typename OsModel_P,
            typename Encryption_P,
            typename Radio_P,
            typename Debug_P>
   void
   DummyApp<OsModel_P, Encryption_P, Radio_P, Debug_P>::
   enable()
   {
      encryption_->template reg_recv_callback<self_type, &self_type::receive>( this );
   }
   // -----------------------------------------------------------------------
   template<typename OsModel_P,
            typename Encryption_P,
            typename Radio_P,
            typename Debug_P>
   void
   DummyApp<OsModel_P, Encryption_P, Radio_P, Debug_P>::
   disable()
   {
      encryption_->unreg_recv_callback();
   }
   // -----------------------------------------------------------------------
   template<typename OsModel_P,
            typename Encryption_P,
            typename Radio_P,
            typename Debug_P>
   void
   DummyApp<OsModel_P, Encryption_P, Radio_P, Debug_P>::
   receive( node_id_t from, size_t len, block_data_t *data )
   {
      notify_receivers( from, len, data );
   }
   // -----------------------------------------------------------------------
   template<typename OsModel_P,
            typename Encryption_P,
            typename Radio_P,
            typename Debug_P>
   void
   DummyApp<OsModel_P, Encryption_P, Radio_P, Debug_P>::
   send( node_id_t destination, size_t len, block_data_t *data )
   {
      encryption_->send( destination, len, data );
   }
}
#endif
