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
#ifndef UTIL_WISEBED_NODE_API_PACKET_EXTRACTOR_H
#define UTIL_WISEBED_NODE_API_PACKET_EXTRACTOR_H

#include "util/base_classes/uart_base.h"
#include "util/wisebed_node_api/command_types.h"

namespace wiselib
{

   /** \brief Implementation of \ref com_facet "Serial Communication Facet"
   *   \ingroup com_concept
   *
   * Implementation of the \ref com_facet "Serial Communication Facet" that
   * can read single bytes from the UART, and sums them up to Wisebed node API
   * packets, which in turn can be passed to the virtual radio. Helpful, if
   * a UART implementation doesn't support sending complete packets.
   */
   template<typename OsModel_P,
            typename Uart_P,
            int BUFFER_SIZE = 128>
   class UartPacketExtractor
      : public UartBase<OsModel_P, typename Uart_P::block_data_t, typename Uart_P::size_t>
   {
   public:
      typedef OsModel_P OsModel;
      typedef Uart_P Uart;

      typedef UartPacketExtractor<OsModel, Uart, BUFFER_SIZE> self_type;
      typedef self_type* self_pointer_t;

      typedef typename Uart_P::block_data_t block_data_t;
      typedef typename Uart_P::size_t size_t;
      // --------------------------------------------------------------------
      enum ErrorCodes
      {
         SUCCESS = OsModel::SUCCESS,
         ERR_UNSPEC = OsModel::ERR_UNSPEC
      };
      // --------------------------------------------------------------------
      UartPacketExtractor()
         : packet_size_ (0),
            in_process_ (false),
            type_       (0),
            buf_idx_    (0)
      {}
      // -----------------------------------------------------------------------
      int init( Uart& uart )
      {
         uart_ = &uart;
         return OsModel::SUCCESS;
      }
      // -----------------------------------------------------------------------
      int enable_serial_comm()
      {
         uart_->enable_serial_comm();
         uart_->template reg_read_callback<self_type, &self_type::handle_uart_packet>(this);
         return OsModel::SUCCESS;
      }
      // -----------------------------------------------------------------------
      int disable_serial_comm()
      {
         uart_->disable_serial_comm();
         return OsModel::SUCCESS;
      }
      // -----------------------------------------------------------------------
      int write( size_t len, block_data_t *buf )
      {
         return uart_->write( len, buf );
      }
      // -----------------------------------------------------------------
      void handle_uart_packet( block_data_t byte )
      {
         if ( !in_process_ )
         {
            switch (byte)
            {
               case SET_VIRTUAL_LINK:
                  packet_size_ = 10;
                  break;
               case DESTROY_VIRTUAL_LINK:
                  packet_size_ = 10;
                  break;
               case VIRTUAL_LINK_MESSAGE:
                  packet_size_ = 21;
                  break;
               default:
                  return;
            }
            
            buf_idx_ = 0;
            type_ = byte;
            in_process_ = true;
            buf_[buf_idx_++] = byte;
         }
         else
         {
            // 5th byte in virtual message is payload length - add to message
            // header size
            if ( type_ == VIRTUAL_LINK_MESSAGE && buf_idx_ == 5 )
               packet_size_ += byte;
            
            buf_[buf_idx_++] = byte;
         }
         
         if ( buf_idx_ == packet_size_ )
         {
            this->notify_receivers( buf_idx_, buf_ );
            in_process_ = false;
         }
      }

   private:
      int packet_size_;
      bool in_process_;
      uint8_t type_;
      
      block_data_t buf_[BUFFER_SIZE];
      int buf_idx_;
      typename Uart::self_pointer_t uart_;
   };
}

#endif
