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

#ifndef __PROTOCOL_H__
#define	__PROTOCOL_H__

#include "neighbor.h"
#include "protocol_settings.h"
#include "protocol_payload.h"
#include "util/pstl/vector_static.h"
#include "util/delegates/delegate.hpp"

namespace wiselib
{
	template< 	typename Os_P,
				typename Radio_P,
				typename Clock_P,
				typename Timer_P,
				typename Debug_P>
	class Protocol_Type
	{
	public:
		typedef Os_P Os;
		typedef Radio_P Radio;
		typedef Clock_P Clock;
		typedef Timer_P Timer;
		typedef Debug_P Debug;
		typedef typename Radio::node_id_t node_id_t;
		typedef typename Radio::size_t size_t;
		typedef Neighbor_Type<Os, Radio, Clock, Timer, Debug> Neighbor;
		typedef ProtocolPayload_Type<Os, Radio, Debug> ProtocolPayload;
		typedef ProtocolSettings_Type<Os, Radio, Timer, Debug> ProtocolSettings;
		typedef vector_static<Os, Neighbor, ND_MAX_NEIGHBORS> Neighbor_vector;
		typedef typename Neighbor_vector::iterator Neighbor_vector_iterator;
		typedef vector_static<Os, ProtocolPayload, ND_MAX_REGISTERED_PROTOCOLS> ProtocolPayload_vector;
		typedef typename ProtocolPayload_vector::iterator ProtocolPayload_vector_iterator;
		typedef delegate4<void, uint8_t, node_id_t, size_t, uint8_t*> event_notifier_delegate_t;
		typedef Protocol_Type<Os, Radio, Clock, Timer, Debug> self_type;
#ifdef NEIGHBOR_DISCOVERY_COORD_SUPPORT
		typedef typename Neighbor::Position Position;
#endif
		// --------------------------------------------------------------------
		Protocol_Type() :
			protocol_id					( 0 ),
			event_notifier_callback		( event_notifier_delegate_t::template from_method<Protocol_Type, &Protocol_Type::null_callback > ( this ) )
		{}
		~Protocol_Type()
		{}
		// --------------------------------------------------------------------
		void set_protocol_id( uint8_t _pid )
		{
			protocol_id = _pid;
			settings.get_protocol_payload_ref()->set_protocol_id( _pid );
		}
		// --------------------------------------------------------------------
		uint8_t get_protocol_id()
		{
			return protocol_id;
		}
		// --------------------------------------------------------------------
		event_notifier_delegate_t get_event_notifier_callback()
		{
			return event_notifier_callback;
		}
		// --------------------------------------------------------------------
		template<class T, void(T::*TMethod)(uint8_t, node_id_t, size_t, uint8_t*) >
		void set_event_notifier_callback( T *_obj_pnt )
		{
			event_notifier_callback = event_notifier_delegate_t::template from_method<T, TMethod > ( _obj_pnt );
		}
		// --------------------------------------------------------------------
		void set_event_notifier_callback( event_notifier_delegate_t _enc )
		{
			event_notifier_callback = _enc;
		}
		// --------------------------------------------------------------------
		void reset_event_notifier_callback()
		{
			event_notifier_callback	= event_notifier_delegate_t::template from_method<Protocol_Type, &Protocol_Type::null_callback > ( this );
		}
		// --------------------------------------------------------------------
		ProtocolSettings get_protocol_settings()
		{
			return settings;
		}
		// --------------------------------------------------------------------
		ProtocolSettings* get_protocol_settings_ref()
		{
			return &settings;
		}
		// --------------------------------------------------------------------
		void set_protocol_settings( ProtocolSettings& _ps )
		{
			settings = _ps;
		}
		// --------------------------------------------------------------------
		Neighbor_vector* get_neighborhood_ref()
		{
			return &neighborhood;
		}
		// --------------------------------------------------------------------
		Neighbor_vector get_neighborhood()
		{
			return neighborhood;
		}
		// --------------------------------------------------------------------
		Neighbor_vector fill_active_neighborhood( Neighbor_vector& _nv )
		{
			_nv.clear();
			for ( Neighbor_vector_iterator it = neighborhood.begin(); it != neighborhood.end(); ++it )
			{
				if ( it->get_active() == 1 )
				{
					_nv.push_back( *it );
				}
			}
			return _nv;
		}
		// --------------------------------------------------------------------
		void set_neighborhood( Neighbor_vector& _nv )
		{
			neighborhood = _nv;
		}
		// --------------------------------------------------------------------
		Neighbor* get_neighbor_ref( node_id_t _nid )
		{
			for ( Neighbor_vector_iterator it = neighborhood.begin(); it != neighborhood.end(); ++it )
			{
				if (it->get_id() == _nid )
				{
					return &(*it);
				}
			}
			return NULL;
		}
		// --------------------------------------------------------------------
		Neighbor* get_active_neighbor_ref( node_id_t _nid )
		{
			for ( Neighbor_vector_iterator it = neighborhood.begin(); it != neighborhood.end(); ++it )
			{
				if ( (it->get_id() == _nid ) && ( it->get_active() == 1 ) )
				{
					return &(*it);
				}
			}
			return NULL;
		}
		// --------------------------------------------------------------------
		size_t get_neighborhood_active_size()
		{
			size_t active_size = 0;
			for ( Neighbor_vector_iterator it = neighborhood.begin(); it != neighborhood.end(); ++it )
			{
				if ( it->get_active() == 1 )
				{
					active_size++;
				}
			}
			return active_size;
		}
		// --------------------------------------------------------------------
		void resolve_overflow_strategy( node_id_t _nid )
		{
			Neighbor* n_ref = get_neighbor_ref( _nid );
			if ( n_ref != NULL )
			{
				ProtocolSettings* p_ref = get_protocol_settings_ref();
				if ( n_ref->get_total_beacons() == 0xffffffff || n_ref->get_total_beacons_expected() == 0xffffffff )
				{
					if ( p_ref->get_overflow_strategy() & ProtocolSettings::RESET_TOTAL_BEACONS )
					{
						n_ref->set_total_beacons( 0 );
					}
					if ( p_ref->get_overflow_strategy() & ProtocolSettings::RESET_TOTAL_BEACONS_EXPECTED )
					{
						n_ref->set_total_beacons_expected( 0 );
					}
#ifdef CONFIG_NEIBHBOR_DISCOVERY_H_LQI_FILTERING
					if ( p_ref->get_overflow_strategy() & ProtocolSettings::RESET_AVG_LQI )
					{
						n_ref->set_avg_LQI( 0 );
					}
					if ( p_ref->get_overflow_strategy() & ProtocolSettings::RESET_AVG_LQI_INVERSE )
					{
						n_ref->set_avg_LQI_inverse( 0 );
					}
#endif
#ifdef CONFIG_NEIBHBOR_DISCOVERY_H_RSSI_FILTERING
					if ( p_ref->get_overflow_strategy() & ProtocolSettings::RESET_AVG_RSSI )
					{
						n_ref->set_avg_RSSI( 0 );
					}
					if ( p_ref->get_overflow_strategy() & ProtocolSettings::RESET_AVG_RSSI_INVERSE )
					{
						n_ref->set_avg_RSSI_inverse( 0 );
					}
#endif
					if ( p_ref->get_overflow_strategy() & ProtocolSettings::RESET_STAB )
					{
						n_ref->set_link_stab_ratio( 0 );
					}
					if ( p_ref->get_overflow_strategy() & ProtocolSettings::RESET_STAB_INVERSE )
					{
						n_ref->set_link_stab_ratio_inverse( 0 );
					}
					if ( p_ref->get_overflow_strategy() == ProtocolSettings::RATIO_DIVIDER )
					{
						n_ref->set_total_beacons( n_ref->get_total_beacons() / p_ref->get_ratio_divider() );
						n_ref->set_total_beacons_expected( n_ref->get_total_beacons_expected() / p_ref->get_ratio_divider() );
					}
					else
					{
						n_ref->set_total_beacons( n_ref->get_total_beacons() / p_ref->get_ratio_divider() );
						n_ref->set_total_beacons_expected( n_ref->get_total_beacons_expected() / p_ref->get_ratio_divider() );
					}
				}
			}
		}
		// --------------------------------------------------------------------
		uint32_t resolve_beacon_weight( node_id_t _nid )
		{
			ProtocolSettings* p_ref = get_protocol_settings_ref();
			if ( p_ref->get_ratio_normalization_strategy() == ProtocolSettings::R_NR_NORMAL )
			{
				return 1;
			}
			else if ( p_ref->get_ratio_normalization_strategy() == ProtocolSettings::R_NR_WEIGHTED )
			{
				return p_ref->get_beacon_weight();
			}
			return 1;
		}
		// --------------------------------------------------------------------
		uint32_t resolve_lost_beacon_weight( node_id_t _nid )
		{
			ProtocolSettings* p_ref = get_protocol_settings_ref();
			if ( p_ref->get_ratio_normalization_strategy() == ProtocolSettings::R_NR_NORMAL )
			{
				return 1;
			}
			else if ( p_ref->get_ratio_normalization_strategy() == ProtocolSettings::R_NR_WEIGHTED )
			{
				return p_ref->get_lost_beacon_weight();
			}
			return 1;
		}
		// --------------------------------------------------------------------
		Protocol_Type& operator=( const Protocol_Type& _p )
		{
			protocol_id = _p.protocol_id;
			event_notifier_callback = _p.event_notifier_callback;
			settings = _p.settings;
			neighborhood = _p.neighborhood;
			return *this;
		}
		// --------------------------------------------------------------------
#ifdef DEBUG_PROTOCOL_H
		void print( Debug& _debug, Radio& _radio
#ifdef NEIGHBOR_DISCOVERY_COORD_SUPPORT
				,Position _pos = Position( 0, 0, 0 )
#endif
				)
		{
#ifndef DEBUG_NEIGHBOR_DISCOVERY_STATS
			_debug.debug( "-------------------------------------------------------\n" );
			_debug.debug( "Protocol :\n" );
			_debug.debug( "protocol_id (size %i) : %d\n", sizeof(protocol_id), protocol_id );
			settings.print( _debug, _radio );
			_debug.debug( "neighborhood : \n" );
#endif
			for ( Neighbor_vector_iterator it = neighborhood.begin(); it != neighborhood.end(); ++it )
			{
#ifdef NEIGHBOR_DISCOVERY_COORD_SUPPORT
				it->print( _debug, _radio, _pos );
#else
				it->print( _debug, _radio );
#endif
			}
#ifndef DEBUG_NEIGHBOR_DISCOVERY_STATS
			_debug.debug( "-------------------------------------------------------\n" );
#endif
		}
#endif
		// --------------------------------------------------------------------
		void null_callback( uint8_t _null_event, node_id_t _null_node_id, size_t _null_len, uint8_t* _null_data )
		{}
		// --------------------------------------------------------------------
	private:
		uint8_t protocol_id;
		event_notifier_delegate_t event_notifier_callback;
		ProtocolSettings settings;
		Neighbor_vector neighborhood;
	};
}
#endif
