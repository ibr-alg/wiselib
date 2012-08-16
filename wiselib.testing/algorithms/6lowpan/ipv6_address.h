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

/*
* File: ipv6_address.h
* Class(es): IPv6Address
* Author: Daniel Gehberger - GSoC 2012 - 6LoWPAN project
*/

#ifndef __ALGORITHMS_6LOWPAN_IPV6_ADDR_H__
#define __ALGORITHMS_6LOWPAN_IPV6_ADDR_H__


namespace wiselib
{
	/** \brief Class for IPv6 address representation
	*
	* This class represents an IPv6 address and the length of the prefix.
	*/
	template<typename Radio_P, 
		typename Debug_P>
	class IPv6Address
	{
	public:
	
		typedef Debug_P Debug;
		typedef Radio_P Radio;
		typedef typename Radio::node_id_t link_layer_node_id_t;
		
		///@name Construction
		///@{
		IPv6Address()
		{
			memset(addr,0, 16);
			prefix_length = 0;
		}
		
		//----------------------------------------------------------------------------
		IPv6Address(const IPv6Address& address)
		{
			memcpy(addr, address.addr, 16);
			prefix_length = address.prefix_length;
		}
		
		//----------------------------------------------------------------------------
		IPv6Address(const uint8_t* addr)
		{
			memcpy(addr, addr, 16);
			prefix_length = 64;
		}
		
		//----------------------------------------------------------------------------
		/// \brief Constructor for static pre defined addresses
		IPv6Address(int type)
		{
			//"Broadcast" (Multicast) - FF02:0:0:0:0:0:0:1
			if ( type == 1 )
			{
				addr[0]=0xFF;
				addr[1]=0x02;
				memset(addr+2,0,13);
				addr[15]=0x01;
				prefix_length = 64;
			}
			//All routers' address - FF02:0:0:0:0:0:0:2
			else if( type == 2 )
			{
				addr[0]=0xFF;
				addr[1]=0x02;
				memset(addr+2,0,13);
				addr[15]=0x02;
				prefix_length = 64;
			}
			//Unspecified address - NULL NODE ID - 0:0:0:0:0:0:0:0
			else
			{
				memset(addr,0, 16);
				prefix_length = 0;
			}
		}
		///@}
		
		#ifdef IPv6_LAYER_DEBUG
		//----------------------------------------------------------------------------
		void set_debug( Debug& debug )
		{
			debug_ = &debug;
		}
		#endif
		
		// --------------------------------------------------------------------
		
		//NOTE This should be a configured address (u bit)
		/** \brief Function to set byte array as an address, with prefix length
		* \param address byte array of the address
		* \param prefix_l length of the prefix in bits
		*/
		void set_address( uint8_t* address, uint8_t prefix_l = 64 )
		{
			memcpy( addr, address, 16);
			prefix_length = prefix_l;
		}
		
		// --------------------------------------------------------------------
		
		/** \brief Function to set a prefix
		* \param prefix byte array of the prefix
		* \param prefix_l length of the prefix in bits
		*/
		void set_prefix( uint8_t* prefix, uint8_t prefix_l = 64 )
		{
			int full_bytes = (int)(prefix_l / 8);
			memcpy( addr, prefix, full_bytes);
			
			//If it is not a full byte
			if( prefix_l % 8 != 0 )
			{
				// copy the last bits
				//NOTE: unused positions must be filled with zeros!
				addr[full_bytes] |= ( prefix[full_bytes] );
			}
			
			prefix_length = prefix_l;
		}
		
		// --------------------------------------------------------------------
		/** \brief Function to set a fix 8 bytes long hostID
		* \param host byte array of the hostID
		*/
		void set_hostID( uint8_t* host )
		{	
			memcpy(&(addr[8]), host, 8);
		}
		
		// --------------------------------------------------------------------
		/** \brief Function to set a long hostID from a MAC address, used for automatic address configuration
		* \param iid_ pointer to the MAC address
		* \param global flag to indicate that global bit (U bit) has to be used or not
		*/
		void set_long_iid( link_layer_node_id_t* iid_, bool global )
		{
			link_layer_node_id_t iid = *iid_;
			//The different operation systems provide different length link_layer_node_id_t-s
			for ( unsigned int i = 0; i < ( sizeof(link_layer_node_id_t) ); i++ )
			{
				addr[15-i] = ( iid & 0xFF );
				iid = iid >> 8;
			}
			
			//Global address: u bit is 1
			if( global )
				addr[8] |= 0x02;
			//Local address: u bit is 0
			else
				addr[8] &= 0xFD;
		}
		
		//NOTE this is not used at the moment
		/** \brief Function to set a short hostID, provided by the PAN coordinator
		* \param iid the received id
		* \param pan_id the ID of the actual PAN
		*/
		void set_short_iid( uint16_t iid, uint16_t pan_id = 0 )
		{
			addr[8] = (pan_id >> 8);
			
			//The u bit has to be 0
			addr[8] |= 0x02;
			
			addr[9] = (pan_id & 0x00FF);
			addr[10] = 0x00;

			addr[13] = 0x00;
			addr[14] = (iid >> 8);
			addr[15] = (iid & 0x00FF);
		}
		
		// --------------------------------------------------------------------
		/** \brief Make the address link-local: prefix and global bit
		*/
		void make_it_link_local()
		{
			uint8_t link_local_prefix[8];
			link_local_prefix[0]=0xFE;
			link_local_prefix[1]=0x80;
			memset(&(link_local_prefix[2]),0, 6);
			set_prefix(link_local_prefix);
			//delete global bit
			addr[8] &= 0xFD;
			prefix_length = 64;
		}
		
		// --------------------------------------------------------------------
		
		/** \brief Function to determinate that the stored address is link-local or not
		* \return true if it is link local, false if it is not link-local
		*/
		bool is_it_link_local( )
		{
			if ( (addr[0] == (uint8_t)0xFE) && (addr[1] == (uint8_t)0x80) )
			{
				for( int i = 2; i < 8; i++)
					if( addr[i] != 0 )
						return false;
			
				return true;
			}
			return false;
		}
		
		// --------------------------------------------------------------------
		
		//NOTE this is not used, and maybe not neccessary
		/** \brief Function to construct a Solicited multicast address
		* \param iid the target ID
		*/
		void make_it_solicited_multicast( link_layer_node_id_t iid )
		{
			addr[0] = 0xFF;
			addr[1] = 0x02;
			memset(&(addr[2]), 0, 9);
			addr[11] = 0x01;
			addr[12] = 0xFF;
			
			if( sizeof( link_layer_node_id_t ) > 3 )
				addr[13] = iid >> 16;
			else
				addr[13] =  0x00;
			addr[14] = iid >> 8;
			addr[15] = iid;
			prefix_length = 104;
		}
		
		// --------------------------------------------------------------------
		
		/** \brief Function to covert dec-->(char)hex
		* This is required because the testbed prints %x as 0x__ and addresses aren't readable
		* \param dec the decimal number [0-15]
		* \return the hex character
		*/
		char get_hex( uint8_t dec )
		{
			char c;
			if( dec < 10 )
				c = dec + 48;
			else
				c = dec + 87;
			
			return c;
		}
		
		/** \brief Function to construt a character array from the address
		* It is required for address printing
		* \param str A pointer to a 43-byte-long character array
		* \return the same pointer
		*/
		char* get_address( char* str)
		{
			uint8_t zeros = 0;
			
			uint8_t act = 0;
			for( int i = 0; i < 16; i++ )
			{
				if( addr[i] == 0 )
					zeros++;
				
				str[act++] = get_hex( addr[i] >> 4 );
				str[act++] = get_hex( addr[i] & 0x0F );
				
				if(i%2==1 && i<15)
					str[act++] = ':';
			}
			
			//Unspecified address
			if( zeros == 16 )
			{
				str[0] = ':';
				str[1] = ':';
				str[2] = '\0';
				return str;
			}
			
			str[act++] = '/';
			str[act++] = (prefix_length / 10) + 48;
			str[act++] = (prefix_length % 10) + 48;
			str[act++] = '\0';
			
			return str;
		}
		
		// --------------------------------------------------------------------
		/** \brief operator == for comparsion
		*/
		bool operator ==(const IPv6Address<Radio, Debug>& b)
		{
			//If every byte is equal, return true
			if( common_prefix_length( b ) == 16 )
			{
				return true;
			}
			return false;
		}
		
		/** \brief operator != for comparsion
		*/
		bool operator !=(const IPv6Address<Radio, Debug>& b)
		{
			//If every byte is equal, return true
			if( common_prefix_length( b ) != 16 )
			{
				return true;
			}
			return false;
		}
		
		// --------------------------------------------------------------------
		
		/** \brief Function to determinate the common prefix length of the actual and another address
		* \param b the compared address
		* \return the length in bytes
		*/
		//Return the size of the same bytes at from the beginning of the address
		uint8_t common_prefix_length(const IPv6Address<Radio, Debug>& b )
		{
			uint8_t same = 0;
			for( int i = 0; i < 16; i++ )
			{
				if( addr[i] == b.addr[i] )
					same++;
				else
					break;
			}
			return same;
		}
		
		///Storage for the address
		uint8_t addr[16];
		///Storage for the prefix length
		uint8_t prefix_length;
		
		#ifdef IPv6_LAYER_DEBUG
	private:
		
		Debug& debug()
		{ return *debug_; }
		
		typename Debug::self_pointer_t debug_;
		#endif
   };
}
#endif
