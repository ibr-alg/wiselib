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
#ifndef CONNECTOR_NS3_SERIAL_COM_UART0_H
#define CONNECTOR_NS3_SERIAL_COM_UART0_H

#include "external_interface/ns3/ns3_types.h"
#include "external_interface/ns3/ns3_debug.h"

namespace wiselib
{

   /** \brief NS-3 Implementation of \ref serial_communication_concept "Serial Communication Facet"
   *  \ingroup serial_communication_concept
   *
   * NS-3 implementation of the \ref serial_communication_concept "Serial Communication Facet" ...
   */
   template<typename OsModel_P,
            typename Debug_P = typename OsModel_P::Debug>
   class Ns3DebugComUartModel
   {
   public:
      typedef OsModel_P OsModel;
      typedef Debug_P Debug;

      typedef Ns3DebugComUartModel<OsModel,Debug> self_type;
      typedef self_type* self_pointer_t;

      typedef WiselibExtIface::block_data_t block_data_t;
      typedef WiselibExtIface::size_t size_t;

      // --------------------------------------------------------------------
      enum ErrorCodes
      {
         SUCCESS = OsModel::SUCCESS,
         ERR_UNSPEC = OsModel::ERR_UNSPEC
      };
      // --------------------------------------------------------------------
      Ns3DebugComUartModel( Ns3Os& os )
         :  os_(os),
            enabled_(false),
            debug_(0)
      {}
      // --------------------------------------------------------------------
      int enable_serial_comm()
      {
          enabled_ = true;
          return SUCCESS;
      }
      // --------------------------------------------------------------------
      int disable_serial_comm()
      {
          enabled_ = false;
          return SUCCESS;
      }
      // --------------------------------------------------------------------
      int write( size_t len, block_data_t *buf )
      {
          if(enabled_ && debug_)
          {
              debug_->debug("%s",buf);
              return SUCCESS;
          }
          return ERR_UNSPEC;
      }
      // --------------------------------------------------------------------
      template<class T, void (T::*TMethod)(size_t, block_data_t*)>
      int reg_read_callback( T *obj_pnt )
      {
         index_ = os().proc.template RegReadCallback<T, TMethod>( obj_pnt );
         return index_;
      }
      // --------------------------------------------------------------------
      int unreg_read_callback( int idx )
      {
         os().proc.UnregReadCallback (idx);
         return idx;
      }
      // --------------------------------------------------------------------
      int init(Debug& debug)
      {
          debug_ = &debug;
          return SUCCESS;
      }
      // --------------------------------------------------------------------
      void receive(size_t len, block_data_t *buf)
      {
        os().proc.Receive (len, buf, index_);
      }

   private:

      Ns3Os& os()
      { return os_; }
      // --------------------------------------------------------------------
      Ns3Os& os_;

      // --------------------------------------------------------------------
      bool enabled_;
      Debug* debug_;

      int index_; // unique identifier of the read callback of this node in NS-3
                  // NOTE: this is not the node id.
   };
}

#endif
