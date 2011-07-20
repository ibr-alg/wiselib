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
#ifndef CONNECTOR_SHAWN_STRINGTAG_UART_H
#define CONNECTOR_SHAWN_STRINGTAG_UART_H

#include "util/delegates/delegate.hpp"
#include "util/base_classes/uart_base.h"
#include "external_interface/shawn/shawn_types.h"
#include "external_interface/shawn/shawn_timer.h"
#include "external_interface/shawn/shawn_os.h"

#include "sys/node.h"
#include "sys/simulation/simulation_controller.h"
#include "sys/simulation/simulation_environment.h"
#include "sys/taggings/basic_tags.h"
#include "sys/tag.h"

#include <string>

namespace wiselib
{

   /** \brief Shawn UART implementation using String Tags
   *
   * Uart implementation for Shawn that writes each buffer received via
   * "write" to a string tag.
   */
   template<typename OsModel_P>
   class ShawnStringTagUartModel
      : public UartBase<OsModel_P, uint8_t, uint8_t>
   {
   public:
      typedef OsModel_P OsModel;

      typedef ShawnStringTagUartModel<OsModel> self_type;
      typedef self_type* self_pointer_t;

      typedef uint8_t block_data_t;
      typedef uint8_t size_t;
      // --------------------------------------------------------------------
      enum ErrorCodes
      {
         SUCCESS = OsModel::SUCCESS,
         ERR_UNSPEC = OsModel::ERR_UNSPEC
      };
      // -----------------------------------------------------------------------
      ShawnStringTagUartModel( ShawnOs& os )
         : os_      ( os ),
            enabled_( false ),
            tagname_( "wiselib_uart_out" )
      {
         os_.proc->owner_w().template write_simple_tag<std::string>( tagname_, "" );
      }
      // -----------------------------------------------------------------------
      int enable_serial_comm()
      {
          enabled_ = true;
          return SUCCESS;
      }
      // -----------------------------------------------------------------------
      int disable_serial_comm()
      {
          enabled_ = false;
          return SUCCESS;
      }
      // -----------------------------------------------------------------------
      int write( size_t len, block_data_t *buf )
      {
         std::string content( (char*)(buf + 19) );
         content = content.substr(0, content.size() - 1);
         os().proc->owner_w().template write_simple_tag<std::string>( tagname_, content );
         return SUCCESS;
      }
      // -----------------------------------------------------------------------
      void receive(size_t len, block_data_t *buf)
      {
          self_type::notify_receivers( len, buf );
      }
      // -----------------------------------------------------------------------
      void set_tagname( std::string tagname )
      {
          tagname_ = tagname;
      }
      // -----------------------------------------------------------------------
      std::string tagname()
      {
          return tagname_;
      }

   private:
      ShawnOs& os()
      { return os_; }
      // --------------------------------------------------------------------
      ShawnOs& os_;
      bool enabled_;
      std::string tagname_;
   };
}

#endif
