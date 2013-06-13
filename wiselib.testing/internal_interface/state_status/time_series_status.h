#ifndef TIME_SERIES_STATUS_H_
#define TIME_SERIES_STATUS_H_

#include "state_status.h"

namespace wiselib
{
template	<	typename Os_P,
				typename Radio_P,
				typename Debug_P,
				int FRAME_SIZE,
				int SERIES_SIZE
			>
	class TimeSeries_Type
	{
	public:
		typedef Os_P Os;
		typedef Radio_P Radio;
		typedef Debug_P Debug;
		typedef typename Radio::size_t size_t;
		typedef StateStatus_Type<Os, Radio, Debug, int32_t, FRAME_SIZE> FrameStateStatus;
		typedef typename FrameStateStatus::status_state_vector frame_vector;
		typedef typename FrameStateStatus::status_state_vector_iterator frame_iterator;
		typedef StateStatus_Type<Os, Radio, Debug, int32_t, SERIES_SIZE> SeriesStateStatus;
		typedef typename SeriesStateStatus::status_state_vector series_vector;
		typedef typename SeriesStateStatus::status_state_vector_iterator series_iterator;
		typedef TimeSeries_Type<Os, Radio, Debug, FRAME_SIZE, SERIES_SIZE> self_type;

		TimeSeries_Type() {}
		~TimeSeries_Type() {}
		// --------------------------------------------------------------------
		SeriesStateStatus autocorellate( Debug& _d, Radio& _r)
		{
			SeriesStateStatus R;

			//frame->print( _d, _r );
			//series->print( _d, _r );
			FrameStateStatus lag_frame;
			for ( size_t j = 0; j < SERIES_SIZE - FRAME_SIZE + 1; j++ )
			{
				for ( size_t i = 0; i < FRAME_SIZE; i++ )
				{

					//series->get_ss_vector_ref()->at[j+i];
					lag_frame.push( series->get_ss_vector_ref()->at(i + j ) );
				}
				lag_frame.print( _d, _r );
				_d.debug("-------------------------");
				//R.push( lag_frame.cartesian_product( lag_frame ) );
			}
			_d.debug("-------------------------");
			_d.debug("-------------------------");
			_d.debug("-------------------------");

			//R.print( _d, _r );
			frame->print( _d, _r );
			return R;
		}
		// --------------------------------------------------------------------
		void set_frame( FrameStateStatus* _f )
		{
			frame = _f;
		}
		// --------------------------------------------------------------------
		FrameStateStatus* get_frame(void)
		{
			return frame;
		}
		// --------------------------------------------------------------------
		void set_series( SeriesStateStatus* _s )
		{
			series = _s;
		}
		// --------------------------------------------------------------------
		SeriesStateStatus* get_series(void)
		{
			return series;
		}
		// --------------------------------------------------------------------
	private:
		FrameStateStatus* frame;
		SeriesStateStatus* series;
	};

}

#endif
