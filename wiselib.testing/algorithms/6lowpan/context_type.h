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
#ifndef __ALGORITHMS_6LOWPAN_CONTEXT_TYPE_H__
#define __ALGORITHMS_6LOWPAN_CONTEXT_TYPE_H__

namespace wiselib
{
	template<typename Radio_P>
	class LoWPANContextType
	{
	public:
		typedef Radio_P Radio;
		typedef typename Radio::node_id_t node_id_t;
		
		// -----------------------------------------------------------------
		LoWPANContextType()
			: valid_lifetime( 0 ),
			prefix( Radio::NULL_NODE_ID )
			{}

		// -----------------------------------------------------------------

		LoWPANContextType( uint16_t life, node_id_t pref )
			: valid_lifetime( life ),
			prefix( pref )
			{}
			
			
		/**
		* Maximum lifetime
		*/
		uint16_t valid_lifetime;
	
		/*
		*  Seuencenumber of this context
		
		uint16_t sequence_number;
		
		
		* Determines if this context is onlink
		
		bool onlink_;*/
	
		/**
		* Prefix stored within this context
		* This is an IPv6 address
		* Prefix length inside
		*/
		node_id_t prefix;	  
	};

}
#endif
