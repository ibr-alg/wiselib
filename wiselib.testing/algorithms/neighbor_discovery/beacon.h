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

#ifndef __BEACON_H__
#define __BEACON_H__

#include "neighbor.h"
#include "protocol_settings.h"
#include "protocol_payload.h"
#include "protocol.h"
#include "util/pstl/vector_static.h"

namespace wiselib
{
	template< 	typename Os_P,
				typename Radio_P,
				typename Clock_P,
				typename Timer_P,
				typename Debug_P>
	class Beacon_Type
	{
	public:
		typedef Os_P Os;
		typedef Radio_P Radio;
		typedef Clock_P Clock;
		typedef Timer_P Timer;
		typedef Debug_P Debug;
		typedef typename Radio::node_id_t node_id_t;
		typedef typename Radio::size_t size_t;
		typedef typename Radio::block_data_t block_data_t;
		typedef typename Timer::millis_t millis_t;
		typedef Neighbor_Type<Os, Radio, Clock, Timer, Debug> Neighbor;
#ifdef CONFIG_NEIGHBOR_DISCOVERY_H_COORD_SUPPORT
		typedef typename Neighbor::Position Position;
#endif
		typedef ProtocolPayload_Type<Os, Radio, Debug> ProtocolPayload;
		typedef ProtocolSettings_Type<Os, Radio, Timer, Debug> ProtocolSettings;
		typedef Protocol_Type<Os, Radio, Clock, Timer, Debug> Protocol;
		typedef vector_static<Os, Neighbor, ND_MAX_NEIGHBORS> Neighbor_vector;
		typedef typename Neighbor_vector::iterator Neighbor_vector_iterator;
		typedef vector_static<Os, ProtocolPayload, ND_MAX_REGISTERED_PROTOCOLS> ProtocolPayload_vector;
		typedef typename ProtocolPayload_vector::iterator ProtocolPayload_vector_iterator;
		typedef vector_static<Os, Protocol, ND_MAX_REGISTERED_PROTOCOLS> Protocol_vector;
		typedef typename Protocol_vector::iterator Protocol_vector_iterator;
		typedef Beacon_Type<Os, Radio, Clock, Timer, Debug> self_type;
		// --------------------------------------------------------------------
		Beacon_Type() :
#ifdef CONFIG_NEIGHBOR_DISCOVERY_H_ACTIVE_SCLD
			SCLD							( 0 ),
#endif
			beacon_period					( 0 ),
			beacon_period_update_counter	( 0 )
		{}
		// --------------------------------------------------------------------
		~Beacon_Type()
		{}
		// --------------------------------------------------------------------
		Neighbor_vector get_neighborhood()
		{
			return neighborhood;
		}
		// --------------------------------------------------------------------
		Neighbor_vector* get_neighborhood_ref()
		{
			return &neighborhood;
		}
		// --------------------------------------------------------------------
		void set_neighborhood( Neighbor_vector& _nv, node_id_t _nid )
		{
			neighborhood.clear();
			for (Neighbor_vector_iterator it = _nv.begin(); it != _nv.end(); ++it )
			{
				if ( it->get_id() != _nid )
				{
					neighborhood.push_back( *it );
				}
			}
		}
		// --------------------------------------------------------------------
		millis_t get_beacon_period()
		{
			return beacon_period;
		}
		// --------------------------------------------------------------------
		void set_beacon_period( millis_t _bp )
		{
			beacon_period = _bp;
		}
		// --------------------------------------------------------------------
		uint32_t get_beacon_period_update_counter()
		{
			return beacon_period_update_counter;
		}
		// --------------------------------------------------------------------
		void set_beacon_period_update_counter( uint32_t _bpuc )
		{
			beacon_period_update_counter = _bpuc;
		}
		// --------------------------------------------------------------------
		ProtocolPayload_vector get_protocol_payloads()
		{
			return protocol_payloads;
		}
		// --------------------------------------------------------------------
		ProtocolPayload_vector* get_protocol_payloads_ref()
		{
			return &protocol_payloads;
		}
		// --------------------------------------------------------------------
		void set_protocol_payloads( ProtocolPayload_vector& _ppv )
		{
			protocol_payloads = _ppv;
		}
		// --------------------------------------------------------------------
		void set_protocol_payloads( Protocol_vector& _pv )
		{
			for ( Protocol_vector_iterator it = _pv.begin(); it != _pv.end(); ++it )
			{
				if ( it->get_protocol_settings_ref()->get_protocol_payload_ref()->get_payload_size() > 0 )
				{
					protocol_payloads.push_back( it->get_protocol_settings_ref()->get_protocol_payload() );
				}
			}
		}
		// --------------------------------------------------------------------
#ifdef CONFIG_NEIGHBOR_DISCOVERY_H_ACTIVE_CONNECTIVITY_FILTERING
		void q_sort_neigh_active_con( int8_t _left, int8_t _right )
		{
			int8_t i = _left;
			int8_t j = _right;
			Neighbor tmp;
			Neighbor pivot = neighborhood[ ( _left + _right ) / 2 ];
			while (i <= j)
			{
				while ( neighborhood[i].get_active_connectivity() < pivot.get_active_connectivity() )
				{
					i++;
				}
				while ( neighborhood[j].get_active_connectivity() > pivot.get_active_connectivity() )
				{
					j--;
				}
				if ( i <= j )
				{
					tmp = neighborhood[i];
					neighborhood[i] = neighborhood[j];
					neighborhood[j] = tmp;
					i++;
					j--;
				}
			};
			if ( _left < j )
			{
				q_sort_neigh_active_con( _left, j );
			}
			if ( i < _right )
			{
				q_sort_neigh_active_con( i, _right);
			}
		}
#endif
		// --------------------------------------------------------------------
#ifdef CONFIG_NEIGHBOR_DISCOVERY_H_ACTIVE_SCLD
		uint8_t get_SCLD()
		{
			return SCLD;
		}
		// --------------------------------------------------------------------
		void set_SCLD( uint8_t _s)
		{
			SCLD = _s;
		}
#endif
		// --------------------------------------------------------------------
#ifdef CONFIG_NEIGHBOR_DISCOVERY_H_COORD_SUPPORT
		void set_position( Position _p )
		{
			position = _p;
		}
		// --------------------------------------------------------------------
		Position get_position()
		{
			return position;
		}
#endif
		// --------------------------------------------------------------------
		Beacon_Type& operator=( const Beacon_Type& _b )
		{
#ifdef CONFIG_NEIGHBOR_DISCOVERY_H_ACTIVE_SCLD
			SCLD = _b.SCLD;
#endif
#ifdef CONFIG_NEIGHBOR_DISCOVERY_H_COORD_SUPPORT
			position = _b.position;
#endif
			beacon_period_update_counter = _b.beacon_period_update_counter;
			beacon_period = _b.beacon_period;
			neighborhood = _b.neighborhood;
			protocol_payloads = _b.protocol_payloads;
			return *this;
		}
		// --------------------------------------------------------------------
#ifdef DEBUG_BEACON_H
		void print( Debug& debug, Radio& radio
#ifdef CONFIG_NEIGHBOR_DISCOVERY_H_COORD_SUPPORT
				,Position pos = Position( 0, 0, 0 )
#endif
				)
		{
			debug.debug( "-------------------------------------------------------\n" );
			debug.debug( "Beacon : \n");
			debug.debug( "serial_size : %d\n", serial_size() );
#ifdef CONFIG_NEIGHBOR_DISCOVERY_H_ACTIVE_SCLD
			debug.debug( "SCLD (size %i) : %d\n,", sizeof(uint8_t), SCLD);
#endif
			for ( ProtocolPayload_vector_iterator it = protocol_payloads.begin(); it != protocol_payloads.end(); ++it )
			{
				it->print( debug, radio );
			}
			debug.debug( "neighborhood size: %d\n", neighborhood.size() );
			for ( Neighbor_vector_iterator it = neighborhood.begin(); it != neighborhood.end(); ++it )
			{
#ifdef CONFIG_NEIGHBOR_DISCOVERY_H_COORD_SUPPORT
				it->print( debug, radio, pos );
#else
				it->print( debug, radio );
#endif
			}
			debug.debug( "beacon_period (size %i) : %d\n", sizeof(beacon_period), beacon_period );
			debug.debug( "beacon_period_update_counter (size %i) : %d\n", sizeof(beacon_period_update_counter), beacon_period_update_counter );
			debug.debug( "active_connectivity (size %i) : %d\n", sizeof( SCLD ), SCLD );
			debug.debug( "-------------------------------------------------------\n" );
		}
#endif
		// --------------------------------------------------------------------
		block_data_t* serialize( block_data_t* _buff, size_t _offset = 0 )
		{
#ifdef CONFIG_NEIGHBOR_DISCOVERY_H_ACTIVE_SCLD
			size_t SCLD_POS = 0;
			write<Os, block_data_t, uint8_t>( _buff + SCLD_POS + _offset, SCLD );
			size_t PROTOCOL_PAYLOADS_SIZE_POS = SCLD_POS + sizeof(uint8_t);
#else
			size_t PROTOCOL_PAYLOADS_SIZE_POS = 0;
#endif
			size_t PROTOCOL_PAYLOADS_POS = PROTOCOL_PAYLOADS_SIZE_POS + sizeof(size_t);
			size_t pps_size = protocol_payloads.size();
			write<Os, block_data_t, size_t>( _buff + PROTOCOL_PAYLOADS_SIZE_POS + _offset, pps_size );
			size_t pp_size = 0;
			for ( ProtocolPayload_vector_iterator it = protocol_payloads.begin(); it != protocol_payloads.end(); ++it )
			{
				it->serialize( _buff + PROTOCOL_PAYLOADS_POS + pp_size, _offset );
				pp_size = it->serial_size() + pp_size;
			}
#ifdef CONFIG_NEIGHBOR_DISCOVERY_H_COORD_SUPPORT
			size_t POSITION_POS = PROTOCOL_PAYLOADS_POS + pp_size;
			position.serialize( _buff + POSITION_POS, _offset );
			size_t NEIGHBORHOOD_SIZE_POS = POSITION_POS + position.serial_size();;
#else
			size_t NEIGHBORHOOD_SIZE_POS = PROTOCOL_PAYLOADS_POS + pp_size;
#endif
			size_t NEIGHBORHOOD_POS = NEIGHBORHOOD_SIZE_POS + sizeof(size_t);
			size_t ns_size = neighborhood.size();
			write<Os, block_data_t, size_t>( _buff + NEIGHBORHOOD_SIZE_POS + _offset, ns_size );
			size_t n_size = 0;
			for ( Neighbor_vector_iterator it = neighborhood.begin(); it != neighborhood.end(); ++it )
			{
				it->serialize( _buff + NEIGHBORHOOD_POS + n_size, _offset );
				n_size = it->serial_size() + n_size;
			}
			size_t BEACON_PERIOD_POS = NEIGHBORHOOD_POS + n_size;
			size_t BEACON_PERIOD_UPDATE_COUNTER_POS = BEACON_PERIOD_POS + sizeof(millis_t);
			write<Os, block_data_t, millis_t>( _buff + BEACON_PERIOD_POS + _offset, beacon_period );
			write<Os, block_data_t, uint32_t>( _buff + BEACON_PERIOD_UPDATE_COUNTER_POS + _offset, beacon_period_update_counter );
			return _buff;
		}
		// --------------------------------------------------------------------
		void de_serialize( block_data_t* _buff, size_t _offset = 0 )
		{
#ifdef CONFIG_NEIGHBOR_DISCOVERY_H_ACTIVE_SCLD
			size_t SCLD_POS = 0;
			SCLD = read<Os, block_data_t, size_t>( _buff + SCLD_POS + _offset );
			size_t PROTOCOL_PAYLOADS_SIZE_POS = SCLD_POS + sizeof(uint8_t);
#else
			size_t PROTOCOL_PAYLOADS_SIZE_POS = 0;
#endif
			size_t PROTOCOL_PAYLOADS_POS = PROTOCOL_PAYLOADS_SIZE_POS + sizeof(size_t);
			size_t pps_size = read<Os, block_data_t, size_t>( _buff + PROTOCOL_PAYLOADS_SIZE_POS + _offset );
			protocol_payloads.clear();
			for ( size_t i = 0; i < pps_size; i++ )
			{
				ProtocolPayload pp;
				pp.de_serialize( _buff + PROTOCOL_PAYLOADS_POS, _offset );
				protocol_payloads.push_back( pp );
				PROTOCOL_PAYLOADS_POS = pp.serial_size() + PROTOCOL_PAYLOADS_POS;
			}
#ifdef CONFIG_NEIGHBOR_DISCOVERY_H_COORD_SUPPORT
			size_t POSITION_POS = PROTOCOL_PAYLOADS_POS;
			position.de_serialize( _buff + POSITION_POS, _offset );
			size_t NEIGHBORHOOD_SIZE_POS = POSITION_POS + position.serial_size();
#else
			size_t NEIGHBORHOOD_SIZE_POS = PROTOCOL_PAYLOADS_POS;
#endif
			size_t NEIGHBORHOOD_POS = NEIGHBORHOOD_SIZE_POS + sizeof(size_t);
			size_t ns_size = read<Os, block_data_t, size_t>( _buff + NEIGHBORHOOD_SIZE_POS + _offset );
			neighborhood.clear();
			for ( size_t i = 0; i < ns_size; i++ )
			{
				Neighbor n;
				n.de_serialize( _buff + NEIGHBORHOOD_POS , _offset );
				neighborhood.push_back( n );
				NEIGHBORHOOD_POS = n.serial_size() + NEIGHBORHOOD_POS;
			}
			size_t BEACON_PERIOD_POS = NEIGHBORHOOD_POS;
			size_t BEACON_PERIOD_UPDATE_COUNTER_POS = BEACON_PERIOD_POS + sizeof(millis_t);
			beacon_period = read<Os, block_data_t, millis_t>( _buff + BEACON_PERIOD_POS + _offset );
			beacon_period_update_counter = read<Os, block_data_t, uint32_t>( _buff + BEACON_PERIOD_UPDATE_COUNTER_POS + _offset );
		}
		// --------------------------------------------------------------------
		size_t serial_size()
		{
			size_t pp_size = 0;
			for ( ProtocolPayload_vector_iterator it = protocol_payloads.begin(); it != protocol_payloads.end(); ++it )
			{
				pp_size = it->serial_size() + pp_size;
			}
			size_t n_size = 0;
			for ( Neighbor_vector_iterator it = neighborhood.begin(); it != neighborhood.end(); ++it )
			{
				n_size = it->serial_size() + n_size;
			}
#ifdef CONFIG_NEIGHBOR_DISCOVERY_H_ACTIVE_SCLD
			size_t SCLD_POS = 0;
			size_t PROTOCOL_PAYLOADS_SIZE_POS = SCLD_POS + sizeof(uint8_t);
#else
			size_t PROTOCOL_PAYLOADS_SIZE_POS = 0;
#endif
			size_t PROTOCOL_PAYLOADS_POS = PROTOCOL_PAYLOADS_SIZE_POS + sizeof(size_t);
#ifdef CONFIG_NEIGHBOR_DISCOVERY_H_COORD_SUPPORT
			size_t POSITION_POS = PROTOCOL_PAYLOADS_POS + pp_size;
			size_t NEIGHBORHOOD_SIZE_POS = POSITION_POS + position.serial_size();;
#else
			size_t NEIGHBORHOOD_SIZE_POS = PROTOCOL_PAYLOADS_POS + pp_size;
#endif
			size_t NEIGHBORHOOD_POS = NEIGHBORHOOD_SIZE_POS + sizeof(size_t);
			size_t BEACON_PERIOD_POS = NEIGHBORHOOD_POS + n_size;
			size_t BEACON_PERIOD_UPDATE_COUNTER_POS = BEACON_PERIOD_POS + sizeof(millis_t);
			return BEACON_PERIOD_UPDATE_COUNTER_POS + sizeof(uint32_t);
		}
		// --------------------------------------------------------------------
	private:
#ifdef CONFIG_NEIGHBOR_DISCOVERY_H_ACTIVE_SCLD
		uint8_t SCLD;
#endif
#ifdef CONFIG_NEIGHBOR_DISCOVERY_H_COORD_SUPPORT
		Position position;
#endif
		ProtocolPayload_vector protocol_payloads;
		Neighbor_vector neighborhood;
		millis_t beacon_period;
		uint32_t beacon_period_update_counter;
	};
}
#endif
