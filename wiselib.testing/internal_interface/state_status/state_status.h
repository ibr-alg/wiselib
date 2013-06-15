#ifndef __STATE_STATUS_H_
#define __STATE_STATUS_H_

#include "util/pstl/vector_static.h"
#include "state_status_source_config.h"
#include "state_status_default_values_config.h"

namespace wiselib
{
	template	<	typename Os_P,
					typename Radio_P,
					typename Debug_P,
					typename Numeric_P,
					int SS_VECTOR_SIZE
				>
	class StateStatus_Type
	{
	public:
		typedef Os_P Os;
		typedef Radio_P Radio;
		typedef Debug_P Debug;
		typedef Numeric_P Numeric;
		typedef vector_static<Os, Numeric, SS_VECTOR_SIZE> status_state_vector;
		typedef typename status_state_vector::iterator status_state_vector_iterator;
		typedef StateStatus_Type<Os, Radio, Debug, Numeric, SS_VECTOR_SIZE> self_type;

		StateStatus_Type() :
			lock_flag ( 1 )
		{}
		// --------------------------------------------------------------------
		~StateStatus_Type()
		{}
		// --------------------------------------------------------------------
		uint8_t check_mono_inc( uint8_t size_flag = 0 )
		{
			if ( ( ss_vector.size() != ss_vector.max_size() ) && ( !size_flag ) )
			{
				return 0;
			}
			status_state_vector_iterator previous;
			for ( status_state_vector_iterator i = ss_vector.begin(); i != ss_vector.end(); ++i )
			{
				if ( ( i != ss_vector.begin() ) && ( i != previous ) )
				{
					if ( *i > *previous )
					{
						return 0;
					}
				}
				previous = i;
			}
			return 1;
		}
		// --------------------------------------------------------------------
		uint8_t check_mono_dec( uint8_t size_flag = 0 )
		{
			if ( ( ss_vector.size() != ss_vector.max_size() ) && ( !size_flag ) )
			{
				return 0;
			}
			status_state_vector_iterator previous;
			for ( status_state_vector_iterator i = ss_vector.begin(); i != ss_vector.end(); ++i )
			{
				if ( ( i != ss_vector.begin() ) && ( i != previous ) )
				{
					if ( *i < *previous )
					{
						return 0;
					}
				}
				previous = i;
			}
			return 1;
		}
		// --------------------------------------------------------------------
		uint8_t check_constant( uint8_t size_flag = 0 )
		{
			if ( ( ss_vector.size() != ss_vector.max_size() ) && ( !size_flag ) )
			{
				return 0;
			}
			status_state_vector_iterator previous;
			for ( status_state_vector_iterator i = ss_vector.begin(); i != ss_vector.end(); ++i )
			{
				if ( ( i != ss_vector.begin() ) && ( i != previous ) )
				{
					if ( *i != *previous )
					{
						return 0;
					}
				}
				previous = i;
			}
			return 1;
		}
		// --------------------------------------------------------------------
		uint16_t stdev_10()
		{
			return sqrt32( var_10() );
		}
		// --------------------------------------------------------------------
		Numeric mean_10()
		{
			if ( ss_vector.size() == 0 )
			{
				return 0;
			}
			Numeric sum = 0;
			for ( size_t i = 0; i < ss_vector.size(); i++ )
			{
				sum = sum + ss_vector.at( i );
			}
			if ( sum < 0 )
			{
				sum = sum * ( -1 );
				return ( ( sum * 10 ) / ss_vector.size() ) * ( -1 );
			}
			return sum * 10 / ss_vector.size();
		}
		// --------------------------------------------------------------------
		Numeric var_10()
		{
			if ( ss_vector.size() < 1 )
			{
				return 0;
			}
			Numeric sum = 0;
			for ( size_t i = 0; i < ss_vector.size(); i++ )
			{
				sum = sum + ( ( ss_vector.at( i ) * 10 ) - mean_10() ) * ( ( ss_vector.at( i ) * 10 ) - mean_10() );
			}
			return sum / ( ( ss_vector.size() - 1 ) * 100 );
		}
		// --------------------------------------------------------------------
		uint8_t check_oscillation()
		{
			self_type peaks;
			Numeric stdv = stdev_10();
			if ( stdv > 50 )
			{
				for ( size_t i = 1; i < ss_vector.size() - 1; i++ )
				{
					if 	( 	( ( ss_vector[i] > 0 ) && ( ss_vector[i] > stdv ) ) ||
							( ( ss_vector[i] < 0 ) && ( (-1) * ss_vector[i] > stdv ) )
						)
					{
						peaks.push( ss_vector[i] );
					}
				}
			}
			if ( peaks.ss_vector.size() <= PEAK_THRESHOLD )
			{
				return 0;
			}
			else if ( ( peaks.check_mono_inc(1) ) && ( !peaks.check_constant(1) ) )
			{
				return 0;
			}
			return 1;
		}
		// --------------------------------------------------------------------

		Numeric autocorellate( self_type _v )
		{
			if ( ( ss_vector.size() < 5 ) || ( get_ss_vector_ref()->size() < 5) )
			{
				return 0;
			}
			Numeric sum_x = 0;
			Numeric sum_y = 0;
			Numeric sum_xx = 0;
			Numeric sum_yy = 0;
			Numeric sum_xy = 0;
			for ( size_t i = 0; i < SS_VECTOR_SIZE; i++ )
			{
				sum_x = sum_x + ss_vector[i] * 10;
				sum_y = sum_y + _v.get_ss_vector_ref()->at(i) * 10;
				sum_xx = sum_xx + ss_vector[i] * ss_vector[i] * 100;
				sum_yy = sum_yy + _v.get_ss_vector_ref()->at(i) * _v.get_ss_vector_ref()->at(i) * 100;
				sum_xy = sum_xy + _v.get_ss_vector_ref()->at(i) * ss_vector[i] * 100;
			}
			return ( ( SS_VECTOR_SIZE * sum_xy ) - ( sum_x * sum_y ) ) * 100 / sqrt32( ( SS_VECTOR_SIZE * sum_xx - ( sum_x ) * ( sum_x ) ) * ( SS_VECTOR_SIZE * sum_yy - ( sum_y ) * ( sum_y ) ) );
		}
		// --------------------------------------------------------------------
		uint16_t sqrt32(uint32_t n)
		{
			uint32_t  c = 0x8000;
			uint32_t  g = 0x8000;

		    for(;;) {
		        if(g*g > n)
		            g ^= c;
		        c >>= 1;
		        if(c == 0)
		            return g;
		        g |= c;
		    }
		    return 0;
		}
		// --------------------------------------------------------------------
		void push( Numeric _v )
		{
			status_state_vector tmp;
			tmp.push_back( _v );
			for ( status_state_vector_iterator i = ss_vector.begin(); i != ss_vector.end(); ++i )
			{
				tmp.push_back( *i );
			}
			ss_vector = tmp;
		}
		// --------------------------------------------------------------------
		Numeric first()
		{
			return ss_vector.back();
		}
		// --------------------------------------------------------------------
		Numeric last()
		{
			return ss_vector.front();
		}
		// --------------------------------------------------------------------
		void lock()
		{
			lock_flag = 0;
		}
		// --------------------------------------------------------------------
		void unlock()
		{
			lock_flag = 1;
		}
		// --------------------------------------------------------------------
		uint8_t try_lock()
		{
			return lock_flag;
		}
		// --------------------------------------------------------------------
		uint8_t full()
		{
			if ( ss_vector.size() == ss_vector.max_size() )
			{
				return 1;
			}
			return 0;
		}
		// --------------------------------------------------------------------
		void clear()
		{
			ss_vector.clear();
			lock_flag = 0;
		}
		// --------------------------------------------------------------------
		status_state_vector* get_ss_vector_ref()
		{
			return &ss_vector;
		}
		// --------------------------------------------------------------------
#ifdef DEBUG_STATE_STATUS_H
		void print( Debug& _debug, Radio& _radio )
		{
			size_t c=0;
			for ( status_state_vector_iterator i = ( ss_vector.end() - 1 ); i != ss_vector.begin(); --i )
			{
				_debug.debug("%x:%d:%i\n", _radio.id(), c, *i );
				c++;
			}
			_debug.debug("%x:%d:%i\n", _radio.id(), c, *ss_vector.begin() );
		}
		// --------------------------------------------------------------------
#endif
	private :
		status_state_vector ss_vector;
		uint8_t lock_flag;
	};

}
#endif
