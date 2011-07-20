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
#include "external_interface/tinyos/tinyos_radio.h"

#ifdef TINYOS_TOSSIM
#include <vector>
#include <map>
#include <iostream>
#endif

namespace wiselib
{

   namespace tinyos
   {

      static const int MAX_REGISTERED_RECEIVERS = 10;
#ifdef TINYOS_TOSSIM
      typedef std::vector<radio_delegate_t> RadioItemVector;
      typedef RadioItemVector::iterator RadioItemVectorIterator;
      static std::map<int, RadioItemVector> receivers;

      typedef std::vector<extended_radio_delegate_t> ExtendedRadioItemVector;
      typedef ExtendedRadioItemVector::iterator ExtendedRadioItemVectorIterator;
      static std::map<int, ExtendedRadioItemVector> extended_receivers;
#else
      static radio_delegate_t receivers[MAX_REGISTERED_RECEIVERS];
      static extended_radio_delegate_t extended_receivers[MAX_REGISTERED_RECEIVERS];
#endif
      // --------------------------------------------------------------------
      extern "C" void tinyos_external_receive( uint16_t from, uint8_t len,
                                               uint8_t *data, uint8_t lqi )
      {
         tinyos::ExtendedData extdata;
         extdata.set_link_metric( lqi );

#ifdef TINYOS_TOSSIM
         for ( RadioItemVectorIterator
                  it = receivers[tinyos_get_nodeid()].begin();
                  it != receivers[tinyos_get_nodeid()].end();
                  ++it )
            if ( *it )
               (*it)( from, len, data );

         for ( ExtendedRadioItemVectorIterator
                  it = extended_receivers[tinyos_get_nodeid()].begin();
                  it != extended_receivers[tinyos_get_nodeid()].end();
                  ++it )
            if ( *it )
               (*it)( from, len, data, extdata );
#else
         for ( int i = 0; i < MAX_REGISTERED_RECEIVERS; i++ )
            if ( receivers[i] )
               receivers[i]( from, len, data );

         for ( int i = 0; i < MAX_REGISTERED_RECEIVERS; i++ )
            if ( extended_receivers[i] )
               extended_receivers[i]( from, len, data, extdata );
#endif
      }
      // -----------------------------------------------------------------------
      int tinyos_radio_add_receiver( radio_delegate_t& d )
      {
#ifdef TINYOS_TOSSIM
         for ( int i = 0; i < MAX_REGISTERED_RECEIVERS; i++ )
         {
            if ( !receivers[tinyos_get_nodeid()].at(i) )
            {
               receivers[tinyos_get_nodeid()].at(i) = d;
               return i;
            }
         }
#else
         for ( int i = 0; i < MAX_REGISTERED_RECEIVERS; i++ )
            if ( !receivers[i] )
            {
               receivers[i] = d;
               return i;
            }
#endif

         return -1;
      }
      // -----------------------------------------------------------------------
      int tinyos_radio_add_extended_receiver( extended_radio_delegate_t& d )
      {
#ifdef TINYOS_TOSSIM
         for ( int i = 0; i < MAX_REGISTERED_RECEIVERS; i++ )
         {
            if ( !extended_receivers[tinyos_get_nodeid()].at(i) )
            {
               extended_receivers[tinyos_get_nodeid()].at(i) = d;
               return i + MAX_REGISTERED_RECEIVERS;
            }
         }
#else
         for ( int i = 0; i < MAX_REGISTERED_RECEIVERS; i++ )
            if ( !extended_receivers[i] )
            {
               extended_receivers[i] = d;
               return i + MAX_REGISTERED_RECEIVERS;
            }
#endif

         return -1;
      }
      // -----------------------------------------------------------------------
      bool tinyos_radio_del_receiver( int idx )
      {
         if ( idx < 0 || idx >= 2*MAX_REGISTERED_RECEIVERS )
            return false;

#ifdef TINYOS_TOSSIM
         if ( idx >= MAX_REGISTERED_RECEIVERS )
            extended_receivers[tinyos_get_nodeid()].at(idx - MAX_REGISTERED_RECEIVERS)
               = extended_radio_delegate_t();
         else
            receivers[tinyos_get_nodeid()].at(idx) = radio_delegate_t();
#else
         if ( idx >= MAX_REGISTERED_RECEIVERS )
            extended_receivers[idx - MAX_REGISTERED_RECEIVERS]
               = extended_radio_delegate_t();
         else
            receivers[idx] = radio_delegate_t();
#endif
         return true;
      }
      // -----------------------------------------------------------------------
      void tinyos_init_wiselib_radio( void )
      {
         // call init of TinyOsRadio.nc-module
         tinyos_init_radio_module();

#ifdef TINYOS_TOSSIM
         for ( int i = 0; i < MAX_REGISTERED_RECEIVERS; i++ )
         {
            receivers[tinyos_get_nodeid()].push_back( radio_delegate_t() );
            extended_receivers[tinyos_get_nodeid()].push_back( extended_radio_delegate_t() );
         }
#else
         for ( int i = 0; i < MAX_REGISTERED_RECEIVERS; i++ )
         {
            receivers[i] = radio_delegate_t();
            extended_receivers[i] = extended_radio_delegate_t();
         }
#endif
      }

   }
}
