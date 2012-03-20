#ifndef NEIGHBOR_H
#define	NEIGHBOR_H

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
		typedef Neighbor_Type<Os, Radio, Clock, Timer, Debug> self_type;
		// --------------------------------------------------------------------
		Neighbor_Type()	:
			id								( 0 ),
			total_beacons					( 0 ),
			total_beacons_expected			( 0 ),
			avg_LQI							( 255 ),
			avg_LQI_inverse					( 255 ),
			link_stab_ratio					( 0 ),
			link_stab_ratio_inverse			( 0 ),
			beacon_period					( 0 ),
			beacon_period_update_counter	( 0 ),
			active							( 0 )
		{}
		// --------------------------------------------------------------------
		Neighbor_Type(	node_id_t _id,
						uint32_t _tbeac,
						uint32_t _tbeac_exp,
						uint8_t _alqi,
						uint8_t _alqi_in,
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
			avg_LQI = _alqi;
			avg_LQI_inverse = _alqi_in;
			link_stab_ratio = _lsratio;
			link_stab_ratio_inverse = _lsratio_in;
			beacon_period = _bp;
			beacon_period_update_counter = _bpuc;
			if ( _a != 0 )
			{
				active = 1;
			}
			last_beacon = _lb;
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
			//TODO overflow
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
		uint8_t get_active()
		{
			return active;
		}
		// --------------------------------------------------------------------
		Neighbor_Type& operator=( const Neighbor_Type& _n )
		{
			id = _n.id;
			total_beacons = _n.total_beacons;
			total_beacons_expected = _n.total_beacons_expected;
			avg_LQI = _n.avg_LQI;
			avg_LQI_inverse = _n.avg_LQI_inverse;
			link_stab_ratio = _n.link_stab_ratio;
			link_stab_ratio_inverse = _n.link_stab_ratio_inverse;
			beacon_period = _n.beacon_period;
			beacon_period_update_counter = _n.beacon_period_update_counter;
			last_beacon = _n.last_beacon;
			active = _n.active;
			return *this;
		}
		// --------------------------------------------------------------------
		block_data_t* serialize( block_data_t* _buff, size_t _offset = 0 )
		{
			size_t ID_POS = 0;
			size_t AVG_LQI_POS = ID_POS + sizeof(node_id_t);
			size_t LINK_STAB_RATIO_POS = AVG_LQI_POS + sizeof(uint8_t);
			write<Os, block_data_t, node_id_t>( _buff + ID_POS + _offset, id );
			write<Os, block_data_t, uint8_t>( _buff + AVG_LQI_POS + _offset, avg_LQI );
			write<Os, block_data_t, uint8_t>( _buff + LINK_STAB_RATIO_POS + _offset, link_stab_ratio );
			return _buff;
		}
		// --------------------------------------------------------------------
		void de_serialize( block_data_t* _buff, size_t _offset = 0 )
		{
			size_t ID_POS = 0;
			size_t AVG_LQI_POS = ID_POS + sizeof(node_id_t);
			size_t LINK_STAB_RATIO_POS = AVG_LQI_POS + sizeof(uint8_t);
			id = read<Os, block_data_t, node_id_t>( _buff + ID_POS + _offset );
			avg_LQI = read<Os, block_data_t, uint8_t>( _buff + AVG_LQI_POS + _offset );
			link_stab_ratio = read<Os, block_data_t, uint8_t>( _buff + LINK_STAB_RATIO_POS + _offset );
		}
		// --------------------------------------------------------------------
		size_t serial_size()
		{
			size_t ID_POS = 0;
			size_t AVG_LQI_POS = ID_POS + sizeof(node_id_t);
			size_t LINK_STAB_RATIO_POS = AVG_LQI_POS + sizeof(uint8_t);
			return LINK_STAB_RATIO_POS + sizeof( uint8_t );
		}
		// --------------------------------------------------------------------

		void print( Debug& debug, Radio& radio )
		{
#ifndef NB_DEBUG_STATS
			debug.debug( "-------------------------------------------------------\n");
			debug.debug( "neighbor :\n" );
			debug.debug( "id : %x\n", id );
			debug.debug( "total_beacons : %d\n", total_beacons );
			debug.debug( "total_beacons_expected : %d\n", total_beacons_expected );
			debug.debug( "avg_LQI : %d\n", avg_LQI );
			debug.debug( "avg_LQI_inverse : %i\n", avg_LQI_inverse );
			debug.debug( "link_stab_ratio : %i\n", link_stab_ratio );
			debug.debug( "link_stab_ratio_inverse : %i\n", link_stab_ratio_inverse );
			debug.debug( "beacon_period : %d\n", beacon_period );
			debug.debug( "beacon_period_update_counter : %d\n", beacon_period_update_counter );
			debug.debug( "active : %d\n", active );
			debug.debug( "-------------------------------------------------------\n");
#else
			if ( active == 1 )
			{
				debug.debug( "NB_STATS:%x:%x:%d:%d:%d:%d:%d:%d:%d:%d:%d\n",
					radio.id(),
					id,
					total_beacons,
					total_beacons_expected,
					avg_LQI,
					avg_LQI_inverse,
					link_stab_ratio,
					link_stab_ratio_inverse,
					beacon_period,
					beacon_period_update_counter,
					active );
			}
#endif
		}
		// --------------------------------------------------------------------
	private:
		node_id_t id;
		uint32_t total_beacons;
		uint32_t total_beacons_expected;
		uint8_t avg_LQI;
		uint8_t avg_LQI_inverse;
		uint8_t link_stab_ratio;
		uint8_t link_stab_ratio_inverse;
		millis_t beacon_period;
		uint32_t beacon_period_update_counter;
		uint8_t active;
		time_t last_beacon;
	};
}
#endif
