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
#ifndef __ALGORITHMS_6LOWPAN_ND_STORAGE_H__
#define __ALGORITHMS_6LOWPAN_ND_STORAGE_H__

namespace wiselib
{
	template<typename LinkLayerAddress_P,
		typename IPv6Addr_P>
	class NeighborCacheEntryType
	{
	public:
		typedef LinkLayerAddress_P node_id_t;
		typedef IPv6Addr_P IPv6Addr_t;
		
		enum NeighborState
		{
			GARBAGECOLLECTIBLE = 1,
			REGISTERED = 2,
			TENTATIVE = 3
		};
		
		NeighborCacheEntryType():
			link_layer_address( 0 ),
			status( GARBAGECOLLECTIBLE ),
			is_router( false ),
			lifetime( 0 )
			{}
		
		uint64_t link_layer_address;
		
		int status;
		
		bool is_router;
		
		uint16_t lifetime;
		
		IPv6Addr_t ip_address;
	};

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
	
	template<typename NeighborCacheEntryType_P>
	class DefaultRouterEntryType
	{
	public:
		typedef NeighborCacheEntryType_P NeighborCacheEntryType;
		
		DefaultRouterEntryType():
			neighbor_pointer( NULL ),
			own_registration_lifetime( 0 )
			{}
		
		NeighborCacheEntryType* neighbor_pointer;
		
		uint16_t own_registration_lifetime;
	};

	
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
	
	template<typename LinkLayerAddress_P,
		typename IPv6Addr_P>
	class NeighborCache_DefaultRouters
	{
		public:
			typedef LinkLayerAddress_P node_id_t;
			typedef IPv6Addr_P IPv6Addr_t;
			
			typedef NeighborCacheEntryType<LinkLayerAddress_P, IPv6Addr_P> NeighborCacheEntryType_t;
			typedef DefaultRouterEntryType<NeighborCacheEntryType_t> DefaultRouterEntryType_t;
			
			enum UpdateReturnValues
			{
				SUCCESS = 0,
				DUPLICATE_ADDRESS = 1,
				NEIGHBOR_CACHE_FULL = 2,
				ROUTER_LIST_FULL = 3
			};
			
			enum NeighborState
			{
				GARBAGECOLLECTIBLE = NeighborCacheEntryType_t::GARBAGECOLLECTIBLE,
				REGISTERED = NeighborCacheEntryType_t::REGISTERED,
				TENTATIVE = NeighborCacheEntryType_t::TENTATIVE
			};
			
			/**
			* Updates a Neighbor Cache entry or creates a new one.
			*
			* \param	address		the IPv6 address
			* \param 	ll_address	the link-layer address
			* \param	lifetime	the time this entry is valid
			*
			* \return UpdateNeighborReturnValues
			*/
			uint8_t update_neighbor( uint8_t& number_of_neighbor, IPv6Addr_t* ip_address, node_id_t ll_address, uint16_t lifetime )
			{
				uint8_t selected_place = LOWPAN_MAX_OF_NEIGHBORS;
				number_of_neighbor = LOWPAN_MAX_OF_NEIGHBORS;
				
				//Search for the IP address in the list
				for( int i = 0; i < LOWPAN_MAX_OF_NEIGHBORS; i++ )
				{
					//If there is an entry with this IP
					if( neighbors_[i].ip_address == *(ip_address) )
					{
						//If this is for the same node, update the entry
						if( neighbors_[i].link_layer_address == (uint64_t)ll_address )
						{
							if( lifetime == 0 )
							{
								//This is a message to delete this entry
								neighbors_[i].status = GARBAGECOLLECTIBLE;
							}
							else
							{
								//Update an existing entry
								neighbors_[i].status = REGISTERED;
								neighbors_[i].lifetime = lifetime;
								number_of_neighbor = i;
							}
							return SUCCESS;
						}
						else
						{
							return DUPLICATE_ADDRESS;
						}
					}
					if( selected_place == LOWPAN_MAX_OF_NEIGHBORS &&
						neighbors_[i].status == GARBAGECOLLECTIBLE )
						selected_place = i;
				}
				
				if( selected_place == LOWPAN_MAX_OF_NEIGHBORS )
					return NEIGHBOR_CACHE_FULL;
				
				//Something wrong: new registration with 0 lifetime
				if( lifetime == 0 )
					return SUCCESS;
				
				neighbors_[selected_place].ip_address = *(ip_address);
				neighbors_[selected_place].link_layer_address = (uint64_t)ll_address;
				neighbors_[selected_place].status = REGISTERED;
				neighbors_[selected_place].lifetime = lifetime;
				neighbors_[selected_place].is_router = false;
				number_of_neighbor = selected_place;
				
				return SUCCESS;
			}
			
			/**
			* Returns the instance of Neighbor which owns the given IPv6 address.
			*
			* \param 	address		the IPv6 address
			*
			* \return	the neighbor with the IPv6 address or NULL if no matching entry was found
			*/
			NeighborCacheEntryType_t* get_neighbor( IPv6Addr_t* ip_address )
			{
				for( int i = 0; i < LOWPAN_MAX_OF_NEIGHBORS; i++ )
					if( neighbors_[i].ip_address == *(ip_address) )
						return &(neighbors_[i]);
				return NULL;
			}
			
			/**
			* Updates a router entry in the default router list or creates a new one.
			*
			* \param	addr			the IPv6 address of the router
			* \param	ll_addr			the link layer address of the router
			* \param	valid_lifetime	the advertised valid lifetime of the router
			* \param	own_lifetime	this node's registered lifetime at the router
			*
			* \return	UpdateReturnValues
			*/
			uint8_t update_router( IPv6Addr_t* ip_address, node_id_t* ll_address, uint16_t valid_lifetime, uint16_t own_lifetime )
			{
				uint8_t selected_place = LOWPAN_MAX_OF_ROUTERS;
				
				//Call the update_neighbor function, it inserts or updates the router to the cache
				//If there was an error, the function returns
				uint8_t neighbor_number = LOWPAN_MAX_OF_NEIGHBORS;
				int result = update_neighbor( selected_place, ip_address, ll_address, valid_lifetime );
				if( (result == NEIGHBOR_CACHE_FULL) ||
					(result == DUPLICATE_ADDRESS) ||
					(valid_lifetime == 0) )
					return result;
				
				//Search for an entry for this IP in the default routers' lis
				//and discover a free space if it will be a  new one
				for( int i = 0; i < LOWPAN_MAX_OF_ROUTERS; i++ )
				{
					if( routers_[i].neighbor_pointer == NULL )
						selected_place = i;
					else if( routers_[i].neighbor_pointer->ip_address == *(ip_address) )
					{
						routers_[i].own_registration_lifetime = own_lifetime;
						return SUCCESS;
					}
				}
				
				//No free space for this router
				if( selected_place == LOWPAN_MAX_OF_ROUTERS )
					return ROUTER_LIST_FULL;
				//Insert the new entry into the default routers list
				//The entry in the neighbor cache is from the update_neighbor function call
				else
				{
					routers_[selected_place].neighbor_pointer = &(neighbors_[neighbor_number]);
					routers_[selected_place].own_registration_lifetime = own_lifetime;
					return SUCCESS;
				}
			}
			
			/**
			* Returns the router with the given IPv6 address.
			*
			* \param	addr	IPv6 address of the router
			*
			* \return	the router entry or NULL if no entry was found
			*/
			DefaultRouterEntryType_t* get_router( IPv6Addr_t* ip_address )
			{
				for( int i = 0; i < LOWPAN_MAX_OF_ROUTERS; i++ )
					if( routers_[i].neighbor_pointer->ip_address == *(ip_address) )
						return &(routers_[i]);
				return NULL;
			}
			
			/**
			* Returns the router with the given index in the router list.
			*
			* \param	index	index of the router entry
			*
			* \return	the router entry
			*/
			DefaultRouterEntryType_t* get_router( uint8_t index )
			{
				return &(routers_[index]);
			}
			
		private:
			NeighborCacheEntryType_t neighbors_[LOWPAN_MAX_OF_NEIGHBORS];
			DefaultRouterEntryType_t routers_[LOWPAN_MAX_OF_ROUTERS];
			
	};

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
	
	template<typename Radio_P,
		typename Debug_P>
	class NDStorage
	{
	public:
		typedef Radio_P Radio;
		typedef Debug_P Debug;
		
		typedef typename Radio::node_id_t node_id_t;
		typedef IPv6Address<Radio, Debug> IPv6Addr_t;
		
		typedef NeighborCache_DefaultRouters<node_id_t, IPv6Addr_t> NeighborCache_DefaultRouters_t;

		// -----------------------------------------------------------------
		NDStorage( )
			: is_router( false ),
			adv_managed_flag( false ),
			adv_other_config_flag( false ),
			adv_link_mtu( 0 ),
			adv_reachable_time( 0 ),
			adv_retrans_timer( 0 ),
			adv_cur_hop_limit( 255 ),
			adv_default_lifetime( 1800 )
			{}
		// -----------------------------------------------------------------
		
		/**
		* A flag indicating whether routing is enabled on this interface. Enabling routing on the interface
		* would imply that a router can forward packets to or from the interface.
		*
		* Default: FALSE
		*/
		bool is_router;
		
		/**
		* The TRUE/FALSE value to be placed in the "Managed address configuration" flag field in the Router
		* Advertisement. See [ADDRCONF].
		*
		* Default: FALSE
		*/
		bool adv_managed_flag;
		
		/**
		* The TRUE/FALSE value to be placed in the "Other configuration" flag field in the Router
		* Advertisement. See [ADDRCONF].
		*
		* Default: FALSE
		*/
		bool adv_other_config_flag;
		
		/**
		* The value to be placed in MTU options sent by the router. A value of zero indicates that no MTU
		* options are sent.
		*
		* Default: 0
		*/
		uint16_t adv_link_mtu;
		
		/**
		* The value to be placed in the Reachable Time field in the Router Advertisement messages sent by the
		* router. The value zero means unspecified (by this router). MUST be no greater than 3,600,000
		* milliseconds (1 hour).
		*
		* Default: 0
		*/
		uint32_t adv_reachable_time;
		
		/**
		* The value to be placed in the Retrans Timer field in the Router Advertisement messages sent by the
		* router. The value zero means unspecified (by this router).
		*
		* Default: 0
		*/
		uint32_t adv_retrans_timer;
		
		/**
		* The default value to be placed in the Cur Hop Limit field in the Router Advertisement messages sent by
		* the router. The value should be set to the current diameter of the Internet. The value zero means
		* unspecified (by this router).
		*
		* Default: The value specified in the "Assigned Numbers" [ASSIGNED] that was in effect at the time
		* of implementation.
		*/
		uint8_t adv_cur_hop_limit;
		
		/**
		* The value to be placed in the Router Lifetime field of Router Advertisements sent from the interface,
		* in seconds. MUST be either zero or between MaxRtrAdvInterval and 9000 seconds. A value of
		* zero indicates that the router is not to be used as a default router. These limits may be overridden
		* by specific documents that describe how IPv6 operates over different link layers. For instance,
		* in a point-to-point link the peers may have enough information about the number and status of devices
		* at the other end so that advertisements are needed less frequently.
		*
		* Default: 3 * MaxRtrAdvInterval [3 * 600 = 1800 sec]
		*/
		uint16_t adv_default_lifetime;
		
		/*
		* Is this a border router?
		* Default: FALSE
		
		bool is_border_router;
		

		* The address of the border router
		
		IPv6Addr_t border_router_address;
		

		* Version number from the border router
		
		uint16_t border_router_version_number;*/
		
		/**
		* Instance of the NeighborCache_DefaultRouters
		*/
		NeighborCache_DefaultRouters_t neighbor_cache;
	};
	
	
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
	
	template<typename Radio_P,
	typename Debug_P>
	class LoWPANContextType
	{
		public:
			typedef Radio_P Radio;
			typedef Debug_P Debug;
			typedef IPv6Address<Radio, Debug> node_id_t;
			
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
			* If the context is valid it can be used for compression
			*/
			bool valid;
			
			/**
			* Prefix stored within this context
			* This is an IPv6 address
			* Prefix length inside
			*/
			node_id_t prefix;	  
	};

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
	
	
	template<typename Radio_P,
	typename Debug_P>
	class LoWPANContextManager
	{
		public:
			typedef Radio_P Radio;
			typedef Debug_P Debug;
			typedef IPv6Address<Radio, Debug> node_id_t;
			
			typedef LoWPANContextType<Radio, Debug> ContextType_t;
			
			// -----------------------------------------------------------------
			LoWPANContextManager()
			{
				for( int i = 0; i < LOWPAN_CONTEXTS_NUMBER; i++ )
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
				
				for( int i = 0; i < LOWPAN_CONTEXTS_NUMBER; i++ )
				{
					//If no selected context at the moment
					//and the context is valid + the stored prefix matches the address' prefix
					if( selected == LOWPAN_CONTEXTS_NUMBER &&
						contexts[i].valid_lifetime > 0 &&
						( memcmp( address.addr, contexts[i].prefix.addr, contexts[i].prefix.prefix_length ) == 0 ) )
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
				for( int i = 0; i < LOWPAN_CONTEXTS_NUMBER; i++ )
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