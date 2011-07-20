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
#ifndef CONNECTOR_SHAWN_ACTUATOR_TAG_H
#define CONNECTOR_SHAWN_ACTUATOR_TAG_H

#include "external_interface/shawn/shawn_types.h"


namespace wiselib
{

   /** \brief Shawn actuator - setting a tag
    *  \ingroup actuator_concept
    *
    * Shawn implementation of the \ref actor_concept "Actuator Concept" ...
    */
   template<typename OsModel_P,
            typename TagType_P>
   class ShawnActuatorTagModel
   {
   public:
      typedef OsModel_P OsModel;
      typedef TagType_P TagType;

      typedef ShawnActuatorTagModel<OsModel, TagType> self_type;
      typedef self_type* self_pointer_t;

      typedef TagType value_t;
      // --------------------------------------------------------------------
      enum ErrorCodes
      {
         SUCCESS = OsModel::SUCCESS,
         ERR_UNSPEC = OsModel::ERR_UNSPEC,
         ERR_NOTIMPL = OsModel::ERR_NOTIMPL
      };
      // --------------------------------------------------------------------
      enum
      {
         READY = OsModel::READY,
         NO_VALUE = OsModel::NO_VALUE,
         INACTIVE = OsModel::INACTIVE
      };
      // --------------------------------------------------------------------
      ShawnActuatorTagModel( ShawnOs& os )
         : os_       ( os ),
            tagname_ ( "" ),
            value_   ( value_t() )
      {}
      // --------------------------------------------------------------------
      int init()
      {
         return SUCCESS;
      }
      // --------------------------------------------------------------------
      int destruct()
      {
         return SUCCESS;
      }
      // --------------------------------------------------------------------
      int state()
      {
         return READY;
      }
      // --------------------------------------------------------------------
      value_t operator()()
      {
         return value_;
      }
      // --------------------------------------------------------------------
      void set_tagname( std::string tagname )
      {
         tagname_ = tagname;
      }
      // --------------------------------------------------------------------
      std::string tagname()
      {
         return tagname_;
      }
      // --------------------------------------------------------------------
      void set_value( value_t value )
      {
         value_ = value;
         os().proc->owner_w().template write_simple_tag<value_t>( tagname(), value );
      }

   private:
      ShawnOs& os()
      { return os_; }
      // --------------------------------------------------------------------
      ShawnOs& os_;
      std::string tagname_;
      value_t value_;
   };

}

#endif
