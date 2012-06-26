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
#ifndef __ALGORITHMS_6LOWPAN_CONTEXT_MANAGER_H__
#define __ALGORITHMS_6LOWPAN_CONTEXT_MANAGER_H__

#include "algorithms/6lowpan/context_type.h"

#define LOWPAN_CONTEXTS_NUMBER 16

namespace wiselib
{
	template<typename Radio_P,
		typename Debug_P>
	class LoWPANContextManager
	{
	public:
		typedef Radio_P Radio;
		typedef Debug_P Debug;
		typedef typename Radio::node_id_t node_id_t;
		
		typedef LoWPANContextType<Radio> ContextType_t;
		
		// -----------------------------------------------------------------
		LoWPANContextManager()
			{
				for( uint8_t i = 0; i < LOWPAN_CONTEXTS_NUMBER; i++ )
				{
					contexts[i].valid_lifetime = 0;
				}
			}

		// -----------------------------------------------------------------

		/**
		* Search for a context by prefix
		* \param address the address with the required prefix
		* \return number of the context
		*/
		uint8_t get_context_number_by_prefix( node_id_t address )
		{
			uint8_t selected = LOWPAN_CONTEXTS_NUMBER;
			
			for( uint8_t i = 0; i < LOWPAN_CONTEXTS_NUMBER; i++ )
			{
				//If no selected context at the moment
				//and the context is valid + the stored prefix matches the address' prefix
				if( selected == LOWPAN_CONTEXTS_NUMBER &&
				 	contexts[i].valid_lifetime > 0 &&
					memcmp( address.addr, contexts[i].prefix.addr, contexts[i].prefix.prefix_length ) == 0 )
				{
					selected = i;
					//reset selected context's lifetime
					contexts[i].valid_lifetime = 0xFFFF;
				}
				else
				{
					if( contexts[i].valid_lifetime > 0 )
						//Decrement unselected context's lifetime
						contexts[i].valid_lifetime--;
				}
			}
			return selected;
		}
		
		// -----------------------------------------------------------------

		/**
		* Get a pointer to a defined context
		* \param num the number of the context
		* \return Pointer to the prefix or NULL if it isn't valid
		*/
		node_id_t* get_prefix_by_number( uint8_t num )
		{

			if( contexts[num].valid_lifetime > 0 )
			{
				contexts[num].valid_lifetime = 0xFFFF;
				return &(contexts[num].prefix);
			}
			else
				return NULL;
		}
		
		// -----------------------------------------------------------------
		
		/**
		* Set a context
		* It will be placed at the place with the lowest lifetime
		* \return number of the new context
		*/
		uint8_t set_context( node_id_t context )
		{
			uint16_t lowest_life = 0xFFFF;
			uint8_t selected = 0;
			//Search for a context with the lowest life time
			for( uint8_t i = 0; i < LOWPAN_CONTEXTS_NUMBER; i++ )
			{
				if( contexts[i].valid_lifetime < lowest_life )
				{
					lowest_life = contexts[i].valid_lifetime;
					selected = i;
				}
			}
			 contexts[selected].valid_lifetime = 0xFFFF;
			contexts[selected].prefix = context;
			
			return selected;
		}
		
		
		/**
		* Array with defined size
		*/
		ContextType_t contexts[LOWPAN_CONTEXTS_NUMBER];
		
	};

}
#endif
