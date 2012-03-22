#ifndef PROTOCOL_SETTINGS_H
#define	PROTOCOL_SETTINGS_H

#include "neighbor_discovery_config.h"
#include "protocol_payload.h"

namespace wiselib
{
	template< 	typename Os_P,
				typename Radio_P,
				typename Timer_P,
				typename Debug_P>
	class ProtocolSettings_Type
	{
	public:
		typedef Os_P Os;
		typedef Radio_P Radio;
		typedef Timer_P Timer;
		typedef Debug_P Debug;
		typedef typename Radio::node_id_t node_id_t;
		typedef typename Radio::size_t size_t;
		typedef typename Timer::millis_t millis_t;
		typedef ProtocolPayload_Type<Os, Radio, Debug> ProtocolPayload;
		typedef ProtocolSettings_Type<Os, Radio, Timer, Debug> self_type;
		// --------------------------------------------------------------------
		ProtocolSettings_Type()	:
			max_avg_LQI_threshold 					( NB_MAX_AVG_LQI_THRESHOLD ),
			min_avg_LQI_threshold 					( NB_MIN_AVG_LQI_THRESHOLD ),
			max_avg_LQI_inverse_threshold 			( NB_MAX_AVG_LQI_INVERSE_THRESHOLD ),
			min_avg_LQI_inverse_threshold 			( NB_MIN_AVG_LQI_INVERSE_THRESHOLD ),
			max_link_stab_ratio_threshold 			( NB_MAX_LINK_STAB_RATIO_THRESHOLD ),
			min_link_stab_ratio_threshold 			( NB_MIN_LINK_STABILITY_RATIO_THRESHOLD ),
			max_link_stab_ratio_inverse_threshold	( NB_MAX_LINK_STAB_RATIO_INVERSE_THRESHOLD ),
			min_link_stab_ratio_inverse_threshold	( NB_MIN_LINK_STAB_RATIO_INVERSE_THRESHOLD ),
			events_flag								( NEW_NB | UPDATE_NB | NEW_PAYLOAD | LOST_NB | TRANS_DB_UPDATE | BEACON_PERIOD_UPDATE | NEIGHBOR_REMOVED ),
			proposed_transmission_power_dB			( NB_PROPOSED_TRANSMISSION_POWER_DB ),
			proposed_transmission_power_dB_weight	( NB_PROPOSED_TRANSMISSION_POWER_DB_WEIGHT ),
			proposed_beacon_period					( NB_PROPOSED_BEACON_PERIOD ),
			proposed_beacon_period_weight			( NB_PROPOSED_BEACON_PERIOD_WEIGHT ),
			overflow_strategy						( RATIO_DIVIDER ),
			ratio_divider							( NB_RATIO_DIVIDER ),
			dead_time_strategy						( MEAN_DEAD_TIME_PERIOD ),
			old_dead_time_period_weight				( NB_OLD_DEAD_TIME_PERIOD_WEIGHT ),
			new_dead_time_period_weight				( NB_NEW_DEAD_TIME_PERIOD_WEIGHT ),
			ratio_normalization_strategy			( R_NR_NORMAL ),
			beacon_weight							( NB_BEACON_WEIGHT ),
			lost_beacon_weight						( NB_LOST_BEACON_WEIGHT )
		{}
		// --------------------------------------------------------------------
		ProtocolSettings_Type(	uint8_t _maxLQI,
								uint8_t _minLQI,
								uint8_t _maxLQI_in,
								uint8_t _minLQI_in,
								uint8_t _maxlsr,
								uint8_t _minlsr,
								uint8_t _maxlsr_in,
								uint8_t _minlsr_in,
								uint8_t _ef,
								int8_t _tp_dB,
								uint8_t _tp_dB_w,
								millis_t _pb,
								uint8_t _pb_w,
								uint8_t _ofs,
								uint32_t _rd,
								uint8_t _dt_s,
								uint8_t _odtp_w,
								uint8_t _ndtp_w,
								uint8_t _rn_s,
								uint32_t _b_w,
								uint32_t _lb_w,
								ProtocolPayload _pp )
		{
			max_avg_LQI_threshold = _maxLQI;
			min_avg_LQI_threshold = _minLQI;
			max_avg_LQI_inverse_threshold = _maxLQI_in;
			min_avg_LQI_inverse_threshold = _minLQI_in;
			max_link_stab_ratio_threshold = _maxlsr;
			min_link_stab_ratio_threshold = _minlsr;
			max_link_stab_ratio_inverse_threshold = _maxlsr_in;
			min_link_stab_ratio_inverse_threshold = _minlsr_in,
			events_flag = _ef;
			proposed_transmission_power_dB = _tp_dB;
			proposed_transmission_power_dB_weight = _tp_dB_w;
			proposed_beacon_period = _pb;
			proposed_beacon_period_weight = _pb_w;
			overflow_strategy = _ofs;
			ratio_divider = _rd;
			dead_time_strategy = _dt_s;
			old_dead_time_period_weight = _odtp_w;
			new_dead_time_period_weight = _ndtp_w;
			ratio_normalization_strategy = _rn_s;
			beacon_weight = _b_w;
			lost_beacon_weight = _lb_w;
			protocol_payload = _pp;
		}
		// --------------------------------------------------------------------
		~ProtocolSettings_Type()
		{}
		// --------------------------------------------------------------------
		uint8_t get_max_avg_LQI_threshold()
		{
			return max_avg_LQI_threshold;
		}
		// --------------------------------------------------------------------
		void set_max_avg_LQI_threshold( uint8_t _maxLQI )
		{
			max_avg_LQI_threshold = _maxLQI;
		}
		// --------------------------------------------------------------------
		uint8_t get_min_avg_LQI_threshold()
		{
			return min_avg_LQI_threshold;
		}
		// --------------------------------------------------------------------
		void set_min_avg_LQI_threshold( uint8_t _minLQI )
		{
			min_avg_LQI_threshold = _minLQI;
		}
		// --------------------------------------------------------------------
		uint8_t get_max_avg_LQI_inverse_threshold()
		{
			return max_avg_LQI_inverse_threshold;
		}
		// --------------------------------------------------------------------
		void set_max_avg_LQI_inverse_threshold( uint8_t _maxLQI_in )
		{
			max_avg_LQI_inverse_threshold = _maxLQI_in;
		}
		// --------------------------------------------------------------------
		uint8_t get_min_avg_LQI_inverse_threshold()
		{
			return min_avg_LQI_inverse_threshold;
		}
		// --------------------------------------------------------------------
		void set_min_avg_LQI_inverse_threshold( uint8_t _minLQI_in )
		{
			min_avg_LQI_inverse_threshold = _minLQI_in;
		}
		// --------------------------------------------------------------------
		uint8_t get_max_link_stab_ratio_threshold()
		{
			return max_link_stab_ratio_threshold;
		}
		// --------------------------------------------------------------------
		void set_max_link_stab_ratio_threshold( uint8_t _maxlsr )
		{
			max_link_stab_ratio_threshold = _maxlsr;
		}
		// --------------------------------------------------------------------
		uint8_t get_min_link_stab_ratio_threshold()
		{
			return min_link_stab_ratio_threshold;
		}
		// --------------------------------------------------------------------
		void set_min_link_stab_ratio_threshold( uint8_t _minlsr )
		{
			min_link_stab_ratio_threshold = _minlsr;
		}
		// --------------------------------------------------------------------
		uint8_t get_max_link_stab_ratio_inverse_threshold()
		{
			return max_link_stab_ratio_inverse_threshold;
		}
		// --------------------------------------------------------------------
		void set_max_link_stab_ratio_inverse_threshold( uint8_t _maxlsr_in )
		{
			max_link_stab_ratio_inverse_threshold = _maxlsr_in;
		}
		// --------------------------------------------------------------------
		uint8_t get_min_link_stab_ratio_inverse_threshold()
		{
			return min_link_stab_ratio_inverse_threshold;
		}
		// --------------------------------------------------------------------
		void set_min_link_stab_ratio_inverse_threshold( uint8_t _minlsr_in )
		{
			min_link_stab_ratio_inverse_threshold = _minlsr_in;
		}
		// --------------------------------------------------------------------
		uint8_t get_events_flag()
		{
			return events_flag;
		}
		// --------------------------------------------------------------------
		void set_events_flag( uint8_t _ef )
		{
			events_flag = _ef;
		}
		// --------------------------------------------------------------------
		int8_t get_proposed_transmission_power_dB()
		{
			return proposed_transmission_power_dB;
		}
		// --------------------------------------------------------------------
		void set_proposed_transmission_power_dB( int8_t _tp_dB )
		{
			proposed_transmission_power_dB = _tp_dB;
		}
		// --------------------------------------------------------------------
		uint8_t get_proposed_transmission_power_dB_weight()
		{
			return proposed_transmission_power_dB_weight;
		}
		// --------------------------------------------------------------------
		void set_proposed_transmission_power_dB_weight( uint8_t _tp_dB_w )
		{
			proposed_transmission_power_dB_weight = _tp_dB_w;
		}
		// --------------------------------------------------------------------
		millis_t get_proposed_beacon_period()
		{
			return proposed_beacon_period;
		}
		// --------------------------------------------------------------------
		void set_proposed_beacon_period( millis_t _pbp )
		{
			proposed_beacon_period = _pbp;
		}
		// --------------------------------------------------------------------
		uint8_t get_proposed_beacon_period_weight()
		{
			return proposed_beacon_period_weight;
		}
		// --------------------------------------------------------------------
		void set_proposed_beacon_period_weight( uint8_t _pbp_w )
		{
			proposed_beacon_period_weight = _pbp_w;
		}
		// --------------------------------------------------------------------
		uint8_t get_overflow_strategy()
		{
			return overflow_strategy;
		}
		// --------------------------------------------------------------------
		void set_overflow_strategy( uint8_t _ofs )
		{
			overflow_strategy = _ofs;
		}
		// --------------------------------------------------------------------
		uint32_t get_ratio_divider()
		{
			return ratio_divider;
		}
		// --------------------------------------------------------------------
		void set_ratio_divider( uint32_t _rd )
		{
			if ( _rd == 0 )
			{
				_rd = 1;
			}
			ratio_divider = _rd;
		}
		// --------------------------------------------------------------------
		void set_dead_time_strategy( uint8_t _dd_s )
		{
			if ( _dd_s >= DT_STRATEGY_NUM_VALUES )
			{
				dead_time_strategy = MEAN_DEAD_TIME_PERIOD;
			}
			else
			{
				dead_time_strategy = _dd_s;
			}
		}
		// --------------------------------------------------------------------
		uint8_t get_dead_time_strategy()
		{
			return dead_time_strategy;
		}
		// --------------------------------------------------------------------
		void set_old_dead_time_period_weight( uint8_t _odtp_w )
		{
			old_dead_time_period_weight = _odtp_w;
		}
		// --------------------------------------------------------------------
		uint8_t get_old_dead_time_period_weight()
		{
			return old_dead_time_period_weight;
		}
		// --------------------------------------------------------------------
		void set_new_dead_time_period_weight( uint8_t _ndtp_w )
		{
			new_dead_time_period_weight = _ndtp_w;
		}
		// --------------------------------------------------------------------
		uint8_t get_new_dead_time_period_weight()
		{
			return new_dead_time_period_weight;
		}
		// --------------------------------------------------------------------
		ProtocolPayload get_protocol_payload()
		{
			return protocol_payload;
		}
		// --------------------------------------------------------------------
		ProtocolPayload* get_protocol_payload_ref()
		{
			return &protocol_payload;
		}
		// --------------------------------------------------------------------
		void set_protocol_payload( ProtocolPayload& _pp )
		{
			protocol_payload = _pp;
		}
		// --------------------------------------------------------------------
		uint8_t get_ratio_normalization_strategy()
		{
			return ratio_normalization_strategy;
		}
		// --------------------------------------------------------------------
		void set_ratio_normalization_strategy( uint8_t _rn_s )
		{
			if ( _rn_s >= R_NR_STRATEGY_NUM_VALUES )
			{
				ratio_normalization_strategy = R_NR_NORMAL;
			}
			else
			{
				ratio_normalization_strategy = _rn_s;
			}
		}
		// --------------------------------------------------------------------
		uint32_t get_beacon_weight()
		{
			return beacon_weight;
		}
		// --------------------------------------------------------------------
		void set_beacon_weight( uint32_t _b_w )
		{
			beacon_weight = _b_w;
		}
		// --------------------------------------------------------------------
		uint32_t get_lost_beacon_weight()
		{
			return lost_beacon_weight;
		}
		// --------------------------------------------------------------------
		void set_lost_beacon_weight( uint32_t _bl_w )
		{
			lost_beacon_weight = _bl_w;
		}
		// --------------------------------------------------------------------
		ProtocolSettings_Type& operator=( const ProtocolSettings_Type& _psett )
		{
			max_avg_LQI_threshold = _psett.max_avg_LQI_threshold;
			min_avg_LQI_threshold = _psett.min_avg_LQI_threshold;
			max_avg_LQI_inverse_threshold = _psett.max_avg_LQI_inverse_threshold;
			min_avg_LQI_inverse_threshold = _psett.min_avg_LQI_inverse_threshold;
			max_link_stab_ratio_threshold = _psett.max_link_stab_ratio_threshold;
			min_link_stab_ratio_threshold = _psett.min_link_stab_ratio_threshold;
			max_link_stab_ratio_inverse_threshold = _psett.max_link_stab_ratio_inverse_threshold;
			min_link_stab_ratio_inverse_threshold = _psett.min_link_stab_ratio_inverse_threshold;
			events_flag = _psett.events_flag;
			proposed_transmission_power_dB = _psett.proposed_transmission_power_dB;
			proposed_transmission_power_dB_weight = _psett.proposed_transmission_power_dB_weight;
			proposed_beacon_period = _psett.proposed_beacon_period;
			proposed_beacon_period_weight = _psett.proposed_beacon_period_weight;
			overflow_strategy = _psett.overflow_strategy;
			ratio_divider = _psett.ratio_divider;
			dead_time_strategy	= _psett.dead_time_strategy;
			old_dead_time_period_weight	= _psett.old_dead_time_period_weight;
			new_dead_time_period_weight	= _psett.new_dead_time_period_weight;
			protocol_payload = _psett.protocol_payload;
			ratio_normalization_strategy = _psett.ratio_normalization_strategy;
			beacon_weight = _psett.beacon_weight;
			lost_beacon_weight = _psett.lost_beacon_weight;
			return *this;
		}
		// --------------------------------------------------------------------
#ifdef NB_DEBUG
		void print( Debug& debug, Radio& radio )
		{
			debug.debug( "-------------------------------------------------------\n");
			debug.debug( "protocol_settings :\n");
			debug.debug( "max_avg_LQI_threshold : %d\n", max_avg_LQI_threshold );
			debug.debug( "min_avg_LQI_threshold : %d\n", min_avg_LQI_threshold );
			debug.debug( "max_avg_LQI_inverse_threshold : %d\n", max_avg_LQI_inverse_threshold );
			debug.debug( "min_avg_LQI_inverse_threshold : %d\n", min_avg_LQI_inverse_threshold );
			debug.debug( "max_link_stab_ratio_threshold : %d\n", max_link_stab_ratio_threshold );
			debug.debug( "min_link_stab_ratio_threshold : %d\n", min_link_stab_ratio_threshold );
			debug.debug( "max_link_stab_ratio_inverse_threshold : %d\n", max_link_stab_ratio_inverse_threshold );
			debug.debug( "min_link_stab_ratio_inverse_threshold : %d\n", min_link_stab_ratio_inverse_threshold );
			debug.debug( "events_flag : %d\n", events_flag );
			debug.debug( "proposed transmission_power_dB : %i\n", proposed_transmission_power_dB );
			debug.debug( "proposed transmission_power_dB_weight : %d\n", proposed_transmission_power_dB_weight );
			debug.debug( "proposed_beacon_period : %d\n", proposed_beacon_period );
			debug.debug( "proposed_beacon_period_weight : %d\n", proposed_beacon_period_weight );
			debug.debug( "overflow_strategy : %d\n", overflow_strategy );
			debug.debug( "ratio_divider : %d\n", ratio_divider );
			debug.debug( "dead_time_strategy : %d\n", dead_time_strategy );
			debug.debug( "old_dead_time_period_weight : %d\n", old_dead_time_period_weight );
			debug.debug( "new_dead_time_period_weight : %d\n", new_dead_time_period_weight );
			debug.debug( "ratio_normalization_strategy : %d\n", ratio_normalization_strategy );
			debug.debug( "beacon_weight : %d\n", beacon_weight );
			debug.debug( "lost_beacon_weight : %d\n", lost_beacon_weight );
			protocol_payload.print( debug, radio );
			debug.debug( "-------------------------------------------------------");
		}
#endif
		// --------------------------------------------------------------------
		enum overflow_strategy
		{
			RESET_TOTAL_BEACONS = 1,
			RESET_TOTAL_BEACONS_EXPECTED = 2,
			RESET_STAB = 4,
			RESET_STAB_INVERSE = 8,
			RESET_AVG_LQI = 16,
			RESET_AVG_LQI_INVERSE = 32,
			RATIO_DIVIDER = 64,
		};
		// --------------------------------------------------------------------
		enum event_codes
		{
			NEW_NB = 1,
			UPDATE_NB = 2,
			NEW_PAYLOAD = 4,
			LOST_NB = 8,
			TRANS_DB_UPDATE = 16,
			BEACON_PERIOD_UPDATE = 32,
			NEIGHBOR_REMOVED = 64
		};
		// --------------------------------------------------------------------
		enum dead_time_strategies
		{
			NEW_DEAD_TIME_PERIOD,
			OLD_DEAD_TIME_PERIOD,
			MEAN_DEAD_TIME_PERIOD,
			WEIGHTED_MEAN_DEAD_TIME_PERIOD,
			DT_STRATEGY_NUM_VALUES
		};
		// --------------------------------------------------------------------
		enum ratio_normalization_strategies
		{
			R_NR_NORMAL,
			R_NR_WEIGHTED,
			R_NR_STRATEGY_NUM_VALUES
		};
	private:
		uint8_t max_avg_LQI_threshold;
		uint8_t min_avg_LQI_threshold;
		uint8_t max_avg_LQI_inverse_threshold;
		uint8_t min_avg_LQI_inverse_threshold;
		uint8_t max_link_stab_ratio_threshold;
		uint8_t min_link_stab_ratio_threshold;
		uint8_t max_link_stab_ratio_inverse_threshold;
		uint8_t min_link_stab_ratio_inverse_threshold;
		uint8_t events_flag;
		int8_t proposed_transmission_power_dB;
		uint8_t proposed_transmission_power_dB_weight;
		millis_t proposed_beacon_period;
		uint8_t proposed_beacon_period_weight;
		uint8_t overflow_strategy;
		uint32_t ratio_divider;
		uint8_t dead_time_strategy;
		uint8_t old_dead_time_period_weight;
		uint8_t new_dead_time_period_weight;
		uint8_t ratio_normalization_strategy;
		uint32_t beacon_weight;
		uint32_t lost_beacon_weight;
		ProtocolPayload protocol_payload;
	};
}
#endif







