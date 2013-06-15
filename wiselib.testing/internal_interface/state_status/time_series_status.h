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
			FrameStateStatus lag_frame;
			for ( size_t j = SERIES_SIZE - FRAME_SIZE + 1; j!= 0; j-- )
			{
				for ( size_t i = FRAME_SIZE; i !=0 ; i-- )
				{
					lag_frame.push( series->get_ss_vector_ref()->at( ( i - 1 ) + ( j - 1 ) ) );
				}
				R.push( lag_frame.autocorellate( *frame ) );
			}
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
