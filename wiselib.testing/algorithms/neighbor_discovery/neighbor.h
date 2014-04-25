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

#ifndef __NEIGHBOR_H__
#define	__NEIGHBOR_H__

#include "neighbor_discovery_source_config.h"
#ifdef CONFIG_NEIGHBOR_DISCOVERY_H_COORD_SUPPORT
#include "../../internal_interface/position/position_new.h"
#endif

namespace wiselib
{
	template< 	typename Os_P,
				typename Radio_P,
				typename Clock_P,
				typename Timer_P,
				typename Debug_P>
	class Neighbor_Type
	{
	public:
		typedef Os_P Os;
		typedef Radio_P Radio;
		typedef Timer_P Timer;
		typedef Clock_P Clock;
		typedef Debug_P Debug;
		typedef typename Radio::node_id_t node_id_t;
		typedef typename Radio::size_t size_t;
		typedef typename Radio::block_data_t block_data_t;
		typedef typename Timer::millis_t millis_t;
		typedef typename Clock::time_t time_t;
#ifdef CONFIG_NEIGHBOR_DISCOVERY_H_COORD_SUPPORT
#ifdef CONFIG_NEIGHBOR_DISCOVERY_H_COORD_SUPPORT_2D
		typedef Position2DType<Os, Radio, PositionNumber, Debug> Position;
#endif
#ifdef CONFIG_NEIGHBOR_DISCOVERY_H_COORD_SUPPORT_3D
		typedef Position3DType<Os, Radio, PositionNumber, Debug> Position;
#endif
#endif

		typedef Neighbor_Type<Os, Radio, Clock, Timer, Debug> self_type;

		// --------------------------------------------------------------------
		Neighbor_Type()	:
			id								( 0 ),
			total_beacons					( 0 ),
			total_beacons_expected			( 0 ),
#ifdef CONFIG_NEIGHBOR_DISCOVERY_H_LQI_FILTERING
			avg_LQI							( ND_MAX_AVG_LQI_THRESHOLD / 2 ),
			avg_LQI_inverse					( ND_MAX_AVG_LQI_INVERSE_THRESHOLD / 2 ),
#endif
#ifdef CONFIG_NEIGHBOR_DISCOVERY_H_RSSI_FILTERING
			avg_RSSI						( ND_MAX_AVG_RSSI_THRESHOLD / 2 ),
			avg_RSSI_inverse				( ND_MAX_AVG_RSSI_INVERSE_THRESHOLD / 2 ),
#endif
#ifdef CONFIG_NEIGHBOR_DISCOVERY_H_ACTIVE_CONNECTIVITY_FILTERING
			active_connectivity				( 0 ),
#endif
			link_stab_ratio					( 0 ),
			link_stab_ratio_inverse			( 0 ),
			beacon_period					( 0 ),
			beacon_period_update_counter	( 0 ),
			active							( 0 )
#ifdef CONFIG_NEIGHBOR_DISCOVERY_H_TRUST_FILTERING
			,trust_counter					( ND_MIN_TRUST_COUNTER ),
			trust_counter_inverse			( ND_MIN_TRUST_COUNTER_INVERSE )
#endif
#ifdef CONFIG_NEIGHBOR_DISCOVERY_H_TRANSMISSION_POWER_PIGGY
			,transmission_power_dB			( 0 )
#endif
		{}
		// --------------------------------------------------------------------
		Neighbor_Type(	node_id_t _id,
						uint32_t _tbeac,
						uint32_t _tbeac_exp,
#ifdef CONFIG_NEIGHBOR_DISCOVERY_H_LQI_FILTERING
						uint8_t _alqi,
						uint8_t _alqi_in,
#endif
#ifdef CONFIG_NEIGHBOR_DISCOVERY_H_RSSI_FILTERING
						uint8_t _arssi,
						uint8_t	_arssi_in,
#endif
#ifdef CONFIG_NEIGHBOR_DISCOVERY_H_ACTIVE_CONNECTIVITY_FILTERING
						uint8_t _ac,
#endif
						uint8_t _lsratio,
						uint8_t _lsratio_in,
						millis_t _bp,
						uint32_t _bpuc,
						uint8_t _a,
						time_t _lb )
		{
			id = _id;
			total_beacons = _tbeac;
			total_beacons_expected = _tbeac_exp;
#ifdef CONFIG_NEIGHBOR_DISCOVERY_H_LQI_FILTERING
			avg_LQI = _alqi;
			avg_LQI_inverse = _alqi_in;
#endif
#ifdef CONFIG_NEIGHBOR_DISCOVERY_H_RSSI_FILTERING
			avg_RSSI = _arssi;
			avg_RSSI_inverse = _arssi_in;
#endif
#ifdef CONFIG_NEIGHBOR_DISCOVERY_H_ACTIVE_CONNECTIVITY_FILTERING
			active_connectivity = _ac;
#endif
			link_stab_ratio = _lsratio;
			link_stab_ratio_inverse = _lsratio_in;
			beacon_period = _bp;
			beacon_period_update_counter = _bpuc;
			if ( _a != 0 )
			{
				active = 1;
			}
			last_beacon = _lb;
#ifdef CONFIG_NEIGHBOR_DISCOVERY_H_TRUST_FILTERING
			trust_counter = ND_MIN_TRUST_COUNTER;
			trust_counter_inverse = ND_MIN_TRUST_COUNTER_INVERSE;
#endif
		}
		// --------------------------------------------------------------------
		~Neighbor_Type()
		{}
		// --------------------------------------------------------------------
		node_id_t get_id()
		{
			return id;
		}
		// --------------------------------------------------------------------
		void set_id( node_id_t _id )
		{
			id = _id;
		}
		// --------------------------------------------------------------------
		uint32_t get_total_beacons()
		{
			return total_beacons;
		}
		// --------------------------------------------------------------------
		void inc_total_beacons( uint32_t _tbeac = 1 )
		{
			//TODO overflow
			total_beacons = total_beacons + _tbeac;
		}
		// --------------------------------------------------------------------
		void set_total_beacons( uint32_t _tbeac )
		{
			total_beacons = _tbeac;
		}
		// --------------------------------------------------------------------
		uint32_t get_total_beacons_expected()
		{
			return total_beacons_expected;
		}
		// --------------------------------------------------------------------
		void inc_total_beacons_expected( uint32_t _tbeac_exp = 1)
		{
			//TODO overflow
			total_beacons_expected = total_beacons_expected + _tbeac_exp;
		}
		// --------------------------------------------------------------------
		void set_total_beacons_expected( uint32_t _tbeac_exp = 1)
		{
			//TODO overflow
			total_beacons_expected = _tbeac_exp;
		}
		// --------------------------------------------------------------------
#ifdef CONFIG_NEIGHBOR_DISCOVERY_H_LQI_FILTERING
		uint8_t get_avg_LQI()
		{
			return avg_LQI;
		}
		// --------------------------------------------------------------------
		void set_avg_LQI( uint8_t _alqi )
		{
			avg_LQI = _alqi;
		}
		// --------------------------------------------------------------------
		void update_avg_LQI( uint8_t _lqi, uint32_t _lqi_w = 1 )
		{
			avg_LQI = ( ( avg_LQI * total_beacons ) + _lqi * _lqi_w ) / ( total_beacons + _lqi_w );
		}
		// --------------------------------------------------------------------
		uint8_t get_avg_LQI_inverse()
		{
			return avg_LQI_inverse;
		}
		// --------------------------------------------------------------------
		void set_avg_LQI_inverse( uint8_t _alqi_in )
		{
			avg_LQI_inverse = _alqi_in;
		}
#endif
		// --------------------------------------------------------------------
#ifdef CONFIG_NEIGHBOR_DISCOVERY_H_RSSI_FILTERING
		uint8_t get_avg_RSSI()
		{
			return avg_RSSI;
		}
		// --------------------------------------------------------------------
		void set_avg_RSSI( uint8_t _arssi )
		{
			avg_RSSI = _arssi;
		}
		// --------------------------------------------------------------------
		void update_avg_RSSI( uint8_t _rssi, uint32_t _rssi_w = 1 )
		{
			avg_RSSI = ( ( avg_RSSI * total_beacons ) + _rssi * _rssi_w ) / ( total_beacons + _rssi_w );
		}
		// --------------------------------------------------------------------
		uint8_t get_avg_RSSI_inverse()
		{
			return avg_RSSI_inverse;
		}
		// --------------------------------------------------------------------
		void set_avg_RSSI_inverse( uint8_t _arssi_in )
		{
			avg_RSSI_inverse = _arssi_in;
		}
#endif
		// --------------------------------------------------------------------
		uint8_t get_link_stab_ratio()
		{
			return link_stab_ratio;
		}
		// --------------------------------------------------------------------
		void update_link_stab_ratio()
		{
			if ( total_beacons >= total_beacons_expected )
			{
				link_stab_ratio = 100;
			}
			else if ( total_beacons_expected == 0 )
			{
				link_stab_ratio = 0;
			}
			link_stab_ratio = ( total_beacons * 100 ) / total_beacons_expected;
		}
		// --------------------------------------------------------------------
		void update_link_stab_ratio_inverse( uint8_t _b_w, uint8_t _lb_w )
		{
			int32_t tmp_link_stab_ratio_inverse = ( ( 100 * _b_w - ( 100 - link_stab_ratio_inverse ) * _lb_w ) * 100 ) / ( ( link_stab_ratio_inverse * _b_w ) + ( ( 100 - link_stab_ratio_inverse ) * _lb_w ) );
			if ( tmp_link_stab_ratio_inverse >= 100 )
			{
				link_stab_ratio_inverse = 100;
			}
			else if ( tmp_link_stab_ratio_inverse < 0 )
			{
				link_stab_ratio_inverse = 0;
			}
			else
			{
				link_stab_ratio_inverse = tmp_link_stab_ratio_inverse;
			}
		}
		// --------------------------------------------------------------------
		void set_link_stab_ratio( uint8_t _lsratio )
		{
			link_stab_ratio = _lsratio;
		}
		// --------------------------------------------------------------------
		uint8_t get_link_stab_ratio_inverse()
		{
			return link_stab_ratio_inverse;
		}
		// --------------------------------------------------------------------
		void set_link_stab_ratio_inverse( uint8_t _lsratio_in )
		{
			link_stab_ratio_inverse = _lsratio_in;
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
		time_t get_last_beacon()
		{
			return last_beacon;
		}
		// --------------------------------------------------------------------
		void set_last_beacon( time_t _lb )
		{
			last_beacon = _lb;
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
		void inc_beacon_period_update_counter( uint32_t _bpuc = 1)
		{
			//TODO overflow
			beacon_period_update_counter = beacon_period_update_counter + _bpuc;
		}
		// --------------------------------------------------------------------
		void set_active( uint8_t _a = 1 )
		{
			if ( _a != 0 )
			{
				active = 1;
			}
			else
			{
				active = 0;
			}
		}
		// --------------------------------------------------------------------
		int8_t get_active()
		{
			return active;
		}
		// --------------------------------------------------------------------
#ifdef CONFIG_NEIGHBOR_DISCOVERY_H_TRUST_FILTERING
		int8_t get_trust_counter()
		{
			return trust_counter;
		}
		// --------------------------------------------------------------------
		void set_trust_counter( int8_t _tc )
		{
			trust_counter = _tc;
		}
		// --------------------------------------------------------------------
		void inc_trust_counter()
		{
			trust_counter = trust_counter + 1;
			if ( trust_counter > ND_MAX_TRUST_COUNTER )
			{
				trust_counter = ND_MAX_TRUST_COUNTER;
			}
		}
		// --------------------------------------------------------------------
		void dec_trust_counter()
		{
			trust_counter = trust_counter - 1;
			if ( trust_counter < ND_MIN_TRUST_COUNTER )
			{
				trust_counter = ND_MIN_TRUST_COUNTER;
			}
		}
		// --------------------------------------------------------------------
		int8_t get_trust_counter_inverse()
		{
			return trust_counter_inverse;
		}
		// --------------------------------------------------------------------
		void set_trust_counter_inverse( int8_t _itc )
		{
			trust_counter_inverse = _itc;
		}
		// --------------------------------------------------------------------
		void inc_trust_counter_inverse()
		{
			trust_counter_inverse = trust_counter_inverse + 1;
			if ( trust_counter_inverse > ND_MAX_TRUST_COUNTER_INVERSE )
			{
				trust_counter_inverse = ND_MAX_TRUST_COUNTER_INVERSE;
			}
		}
		// --------------------------------------------------------------------
		void dec_trust_counter_inverse()
		{
			trust_counter_inverse = trust_counter_inverse - 1;
			if ( trust_counter_inverse < ND_MIN_TRUST_COUNTER_INVERSE )
			{
				trust_counter_inverse = ND_MIN_TRUST_COUNTER_INVERSE;
			}
		}
#endif
		// --------------------------------------------------------------------
#ifdef CONFIG_NEIGHBOR_DISCOVERY_H_ACTIVE_CONNECTIVITY_FILTERING
		uint8_t get_active_connectivity()
		{
			return active_connectivity;
		}
		// --------------------------------------------------------------------
		void set_active_connectivity( uint8_t _ac )
		{
			active_connectivity = _ac;
		}
		// --------------------------------------------------------------------
#endif
#ifdef CONFIG_NEIGHBOR_DISCOVERY_H_COORD_SUPPORT
		Position get_position()
		{
			return position;
		}
		// --------------------------------------------------------------------
		void set_position( Position _p )
		{
			position = _p;
		}
#endif
		// --------------------------------------------------------------------
#ifdef CONFIG_NEIGHBOR_DISCOVERY_H_TRANSMISSION_POWER_PIGGY
		int8_t get_transmission_power_dB()
		{
			return transmission_power_dB;
		}
		// --------------------------------------------------------------------
		void set_transmission_power_dB( int8_t _tp_dB )
		{
			transmission_power_dB = _tp_dB;
		}
#endif
		Neighbor_Type& operator=( const Neighbor_Type& _n )
		{
			id = _n.id;
			total_beacons = _n.total_beacons;
			total_beacons_expected = _n.total_beacons_expected;
#ifdef CONFIG_NEIGHBOR_DISCOVERY_H_LQI_FILTERING
			avg_LQI = _n.avg_LQI;
			avg_LQI_inverse = _n.avg_LQI_inverse;
#endif
#ifdef CONFIG_NEIGHBOR_DISCOVERY_H_RSSI_FILTERING
			avg_RSSI = _n.avg_RSSI;
			avg_RSSI_inverse = _n.avg_RSSI_inverse;
#endif
#ifdef CONFIG_NEIGHBOR_DISCOVERY_H_ACTIVE_CONNECTIVITY_FILTERING
			active_connectivity = _n.active_connectivity;
#endif
			link_stab_ratio = _n.link_stab_ratio;
			link_stab_ratio_inverse = _n.link_stab_ratio_inverse;
			beacon_period = _n.beacon_period;
			beacon_period_update_counter = _n.beacon_period_update_counter;
			last_beacon = _n.last_beacon;
			active = _n.active;
#ifdef CONFIG_NEIGHBOR_DISCOVERY_H_TRUST_FILTERING
			trust_counter = _n.trust_counter;
			trust_counter_inverse = _n.trust_counter_inverse;
#endif
#ifdef CONFIG_NEIGHBOR_DISCOVERY_H_COORD_SUPPORT
			position = _n.position;
#endif
#ifdef CONFIG_NEIGHBOR_DISCOVERY_H_TRANSMISSION_POWER_PIGGY
			transmission_power_dB = _n.transmission_power_dB;
#endif
			return *this;
		}
		// --------------------------------------------------------------------
		block_data_t* serialize( block_data_t* _buff, size_t _offset = 0 )
		{
			size_t ID_POS = 0;
#ifdef CONFIG_NEIGHBOR_DISCOVERY_H_LQI_FILTERING
#ifdef CONFIG_NEIGHBOR_DISCOVERY_H_RSSI_FILTERING
			size_t AVG_LQI_POS = ID_POS + sizeof(node_id_t);
			size_t AVG_RSSI_POS = AVG_LQI_POS + sizeof( uint8_t);
			size_t LINK_STAB_RATIO_POS = AVG_RSSI_POS + sizeof(uint8_t);
#else
			size_t AVG_LQI_POS = ID_POS + sizeof(node_id_t);
			size_t LINK_STAB_RATIO_POS = AVG_LQI_POS + sizeof(uint8_t);
#endif
#else
#ifdef CONFIG_NEIGHBOR_DISCOVERY_H_RSSI_FILTERING
			size_t AVG_RSSI_POS = ID_POS + sizeof( node_id_t);
			size_t LINK_STAB_RATIO_POS = AVG_RSSI_POS + sizeof(uint8_t);
#else
			size_t LINK_STAB_RATIO_POS = ID_POS + sizeof(node_id_t);
#endif
#endif
			write<Os, block_data_t, node_id_t> ( _buff + ID_POS + _offset, id );
#ifdef CONFIG_NEIGHBOR_DISCOVERY_H_LQI_FILTERING
			write<Os, block_data_t, uint8_t> ( _buff + AVG_LQI_POS + _offset, avg_LQI );
#endif
#ifdef CONFIG_NEIGHBOR_DISCOVERY_H_RSSI_FILTERING
			write<Os, block_data_t, uint8_t> ( _buff + AVG_RSSI_POS + _offset, avg_RSSI );
#endif
			write<Os, block_data_t, uint8_t> ( _buff + LINK_STAB_RATIO_POS + _offset, link_stab_ratio );
			return _buff;
		}
		// --------------------------------------------------------------------
		void de_serialize( block_data_t* _buff, size_t _offset = 0 )
		{
			size_t ID_POS = 0;
#ifdef CONFIG_NEIGHBOR_DISCOVERY_H_LQI_FILTERING
#ifdef CONFIG_NEIGHBOR_DISCOVERY_H_RSSI_FILTERING
			size_t AVG_LQI_POS = ID_POS + sizeof(node_id_t);
			size_t AVG_RSSI_POS = AVG_LQI_POS + sizeof( uint8_t);
			size_t LINK_STAB_RATIO_POS = AVG_RSSI_POS + sizeof(uint8_t);
#else
			size_t AVG_LQI_POS = ID_POS + sizeof(node_id_t);
			size_t LINK_STAB_RATIO_POS = AVG_LQI_POS + sizeof(uint8_t);
#endif
#else
#ifdef CONFIG_NEIGHBOR_DISCOVERY_H_RSSI_FILTERING
			size_t AVG_RSSI_POS = ID_POS + sizeof( node_id_t);
			size_t LINK_STAB_RATIO_POS = AVG_RSSI_POS + sizeof(uint8_t);
#else
			size_t LINK_STAB_RATIO_POS = ID_POS + sizeof(node_id_t);
#endif
#endif
			id = read<Os, block_data_t, node_id_t> ( _buff + ID_POS + _offset );
#ifdef CONFIG_NEIGHBOR_DISCOVERY_H_LQI_FILTERING
			avg_LQI = read<Os, block_data_t, uint8_t> ( _buff + AVG_LQI_POS + _offset );
#endif
#ifdef CONFIG_NEIGHBOR_DISCOVERY_H_RSSI_FILTERING
			avg_RSSI = read<Os, block_data_t, uint8_t> ( _buff + AVG_RSSI_POS + _offset );
#endif

			link_stab_ratio = read<Os, block_data_t, uint8_t> ( _buff + LINK_STAB_RATIO_POS + _offset );
		}
		// --------------------------------------------------------------------
		size_t serial_size()
		{
			size_t ID_POS = 0;
#ifdef CONFIG_NEIGHBOR_DISCOVERY_H_LQI_FILTERING
#ifdef CONFIG_NEIGHBOR_DISCOVERY_H_RSSI_FILTERING
			size_t AVG_LQI_POS = ID_POS + sizeof(node_id_t);
			size_t AVG_RSSI_POS = AVG_LQI_POS + sizeof( uint8_t);
			size_t LINK_STAB_RATIO_POS = AVG_RSSI_POS + sizeof(uint8_t);
			return LINK_STAB_RATIO_POS + sizeof( uint8_t );
#else
			size_t AVG_LQI_POS = ID_POS + sizeof(node_id_t);
			size_t LINK_STAB_RATIO_POS = AVG_LQI_POS + sizeof(uint8_t);
#endif
#else
#ifdef CONFIG_NEIGHBOR_DISCOVERY_H_RSSI_FILTERING
			size_t AVG_RSSI_POS = ID_POS + sizeof( node_id_t);
			size_t LINK_STAB_RATIO_POS = AVG_RSSI_POS + sizeof(uint8_t);
#else
			size_t LINK_STAB_RATIO_POS = ID_POS + sizeof(node_id_t);
#endif
#endif
			return LINK_STAB_RATIO_POS + sizeof( uint8_t );
		}
		// --------------------------------------------------------------------
#ifdef DEBUG_NEIGHBOR_H
		void print( Debug& debug, Radio& radio, uint32_t counter
#ifdef CONFIG_NEIGHBOR_DISCOVERY_H_COORD_SUPPORT
				,Position pos = Position( 0, 0, 0 )
#endif
				)
		{
#ifndef DEBUG_NEIGHBOR_DISCOVERY_STATS
			debug.debug( "-------------------------------------------------------\n" );
			debug.debug( "Neighbor : \n" );
			debug.debug( "id (size %i) : %x\n", sizeof(node_id_t), id );
			debug.debug( "total_beacons (size %i) : %d\n", sizeof(total_beacons), total_beacons );
			debug.debug( "total_beacons_expected (size %i) : %d\n", sizeof(total_beacons_expected), total_beacons_expected );
#ifdef CONFIG_NEIGHBOR_DISCOVERY_H_LQI_FILTERING
			debug.debug( "avg_LQI (size %i) : %d\n", sizeof(avg_LQI), avg_LQI );
			debug.debug( "avg_LQI_inverse (size %i) : %i\n", sizeof(avg_LQI_inverse), avg_LQI_inverse );
#endif
#ifdef CONFIG_NEIGHBOR_DISCOVERY_H_RSSI_FILTERING
			debug.debug( "avg_RSSI (size %i) : %d\n", sizeof(avg_RSSI), avg_RSSI );
			debug.debug( "avg_RSSI_inverse (size %i) : %i\n", sizeof(avg_RSSI_inverse), avg_RSSI_inverse );
#endif
#ifdef CONFIG_NEIGHBOR_DISCOVERY_H_ACTIVE_CONNECTIVITY_FILTERING
			debug.debug( "active_connectivity (size %i) : %i\n", sizeof(active_connectivity), active_connectivity );
#endif
			debug.debug( "link_stab_ratio (size %i) : %i\n", sizeof(link_stab_ratio), link_stab_ratio );
			debug.debug( "link_stab_ratio_inverse (size %i) : %i\n", sizeof(link_stab_ratio_inverse), link_stab_ratio_inverse );
			debug.debug( "beacon_period (size %i) : %d\n", sizeof(beacon_period), beacon_period );
			debug.debug( "beacon_period_update_counter (size %i) : %d\n", sizeof(beacon_period_update_counter), beacon_period_update_counter );
			debug.debug( "trust_counter (size %i) : %d\n", sizeof(trust_counter), trust_counter );
			debug.debug( "trust_counter_inverse (size %i) : %d\n", sizeof(trust_counter_inverse), trust_counter_inverse );
			debug.debug( "active (size %i) : %d\n", sizeof(active), active );
			debug.debug( "-------------------------------------------------------\n" );
#else
//			if ( ( radio.id() != id ) && ( active ) )
                        if ( ( radio.id() != id ) )
			{
#ifdef CONFIG_NEIGHBOR_DISCOVERY_H_COORD_SUPPORT
#ifdef CONFIG_NEIGHBOR_DISCOVERY_H_COORD_SUPPORT_SHAWN
				debug.debug( "DDDDDD:%d:%d:%d:%d:%d:%d:%d:%d:%d:%d:%d:%d:%d:%d:%f:%f:%f:%f:%d:%d:%d\n",
#endif
#ifdef CONFIG_NEIGHBOR_DISCOVERY_H_COORD_SUPPORT_ISENSE
				debug.debug( "DDDDDD:%x:%x:%d:%d:%d:%d:%d:%d:%d:%d:%d:%d:%d:%d:%d:%d:%d:%d:%d:%d:%d\n",
#endif
#else
				debug.debug( "DDDDDD:%x:%x:%d:%d:%d:%d:%d:%d:%d:%d:%d:%d:%d:%d:%d:%d:%d\n",
#endif
					radio.id(),
					id,
					total_beacons,
					total_beacons_expected,
#ifdef CONFIG_NEIGHBOR_DISCOVERY_H_LQI_FILTERING
					avg_LQI,
					avg_LQI_inverse,
#else
					0,0,
#endif
#ifdef CONFIG_NEIGHBOR_DISCOVERY_H_RSSI_FILTERING
					avg_RSSI,
					avg_RSSI_inverse,
#else
					0,0,
#endif
#ifdef CONFIG_NEIGHBOR_DISCOVERY_H_ACTIVE_CONNECTIVITY_FILTERING
					active_connectivity,
#else
					0,
#endif
					link_stab_ratio,
					link_stab_ratio_inverse,
					beacon_period,
					beacon_period_update_counter,
					active
#ifdef CONFIG_NEIGHBOR_DISCOVERY_H_COORD_SUPPORT
					,position.get_x(), position.get_y(), position.get_z(),
					( ( position.get_x() - pos.get_x() ) * ( position.get_x() - pos.get_x() ) +
					( position.get_y() - pos.get_y() ) * ( position.get_y() - pos.get_y() ) +
					( position.get_z() - pos.get_z() ) * ( position.get_z() - pos.get_z() ) )
#endif
#ifdef CONFIG_NEIGHBOR_DISCOVERY_H_TRUST_FILTERING
					,trust_counter
					,trust_counter_inverse, counter
#endif
				);
			}
#endif
		}
#endif
		// --------------------------------------------------------------------
	private:
		node_id_t id;
		uint32_t total_beacons;
		uint32_t total_beacons_expected;
#ifdef CONFIG_NEIGHBOR_DISCOVERY_H_LQI_FILTERING
		uint8_t avg_LQI;
		uint8_t avg_LQI_inverse;
#endif
#ifdef CONFIG_NEIGHBOR_DISCOVERY_H_RSSI_FILTERING
		uint8_t avg_RSSI;
		uint8_t avg_RSSI_inverse;
#endif
#ifdef CONFIG_NEIGHBOR_DISCOVERY_H_ACTIVE_CONNECTIVITY_FILTERING
		uint8_t active_connectivity;
#endif
		uint8_t link_stab_ratio;
		uint8_t link_stab_ratio_inverse;
		millis_t beacon_period;
		uint32_t beacon_period_update_counter;
		uint8_t active;
		time_t last_beacon;
#ifdef CONFIG_NEIGHBOR_DISCOVERY_H_TRUST_FILTERING
		int8_t trust_counter;
		int8_t trust_counter_inverse;
#endif
#ifdef CONFIG_NEIGHBOR_DISCOVERY_H_COORD_SUPPORT
		Position position;
#endif
#ifdef CONFIG_NEIGHBOR_DISCOVERY_H_TRANSMISSION_POWER_PIGGY
		int8_t transmission_power_dB;
#endif
	};
}
#endif
