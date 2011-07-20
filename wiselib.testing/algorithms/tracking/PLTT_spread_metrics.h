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
#ifndef __PLTT_SPREAD_METRICS_H__
#define __PLTT_SPREAD_METRICS_H__

namespace wiselib
{
	template< typename Integer_P>
	class PLTT_TargetSpreadMetricType
	{
	public:
		typedef Integer_P Integer;
		PLTT_TargetSpreadMetricType() :
			trace_messages_send ( 0 ),
			trace_messages_bytes_send ( 0 )
		{}
		Integer get_trace_messages_send(){ return trace_messages_send; }
		Integer get_trace_messages_bytes_send(){ return trace_messages_bytes_send; }
		void inc_trace_messages_send() { trace_messages_send++; }
		void inc_trace_messages_bytes_send( Integer _bytes_num ) { trace_messages_bytes_send = trace_messages_bytes_send + _bytes_num; }
		void reset()
		{
			trace_messages_send = 0;
			trace_messages_bytes_send = 0;
		}
	private:
		Integer trace_messages_send;
		Integer trace_messages_bytes_send;
	};



	template <	typename node_id_t_P,
				typename Integer_P>
	class PLTT_PassiveSpreadMetricType
	{
	public:
		typedef node_id_t_P node_id_t;
		typedef Integer_P Integer;
		PLTT_PassiveSpreadMetricType(){}
		PLTT_PassiveSpreadMetricType( node_id_t _target_id )
		{
			spread_messages_send = 0;
			spread_messages_received = 0;
			spread_messages_bytes_received = 0;
			spread_messages_inhibited = 0;
			spread_messages_bytes_send = 0;
			spread_messages_bytes_inhibited = 0;
			inhibition_messages_send = 0;
			inhibition_messages_bytes_send = 0;
			inhibition_messages_bytes_inhibited = 0;
			inhibition_messages_inhibited = 0;
#ifndef OPT_TARGET_LIST_AGGREGATION
			inhibition_messages_received = 0;
			inhibition_messages_bytes_received = 0;
#endif
			target_id = _target_id;
		}
		~PLTT_PassiveSpreadMetricType(){}
		void inc_spread_messages_send( void ) { ++spread_messages_send; }
		void inc_spread_messages_bytes_send( Integer _bytes_num ) { spread_messages_bytes_send = spread_messages_bytes_send + _bytes_num; }
		void inc_spread_messages_inhibited( void ) { ++spread_messages_inhibited; }
		void inc_spread_messages_bytes_inhibited( Integer _bytes_num ) { spread_messages_bytes_inhibited = spread_messages_bytes_inhibited + _bytes_num; }
		void inc_spread_messages_received( void ) { ++spread_messages_received; }
		void inc_spread_messages_bytes_received( Integer _bytes_num ) { spread_messages_bytes_received = spread_messages_bytes_received + _bytes_num; }
		void inc_inhibition_messages_send( void ) { ++inhibition_messages_send; }
		void inc_inhibition_messages_bytes_send( Integer _bytes_num ) { inhibition_messages_bytes_send = inhibition_messages_bytes_send + _bytes_num; }
		void inc_inhibition_messages_inhibited( void ) { ++inhibition_messages_inhibited; }
		void inc_inhibition_messages_bytes_inhibited( Integer _bytes_num ) { inhibition_messages_bytes_inhibited = inhibition_messages_bytes_inhibited + _bytes_num; }
		Integer get_spread_messages_send( void ) { return spread_messages_send; }
		Integer get_spread_messages_bytes_send( void ) { return spread_messages_bytes_send; }
		Integer get_spread_messages_received( void ) { return spread_messages_received; }
		Integer get_spread_messages_bytes_received( void ) { return spread_messages_bytes_received; }
		Integer get_spread_messages_inhibited( void ) { return spread_messages_inhibited; }
		Integer get_spread_messages_bytes_inhibited( void ) { return spread_messages_bytes_inhibited; }
		Integer get_inhibition_messages_send( void ) { return inhibition_messages_send; }
		Integer get_inhibition_messages_bytes_send( void ) { return inhibition_messages_bytes_send; }
		Integer get_inhibition_messages_inhibited( void ) { return inhibition_messages_inhibited; }
		Integer get_inhibition_messages_bytes_inhibited( void ) { return inhibition_messages_bytes_inhibited; }
#ifndef OPT_TARGET_LIST_AGGREGATION
		void inc_inhibition_messages_received( void ) { ++inhibition_messages_received; }
		void inc_inhibition_messages_bytes_received( Integer _bytes_num ) { inhibition_messages_bytes_received = inhibition_messages_bytes_received + _bytes_num; }
		Integer get_inhibition_messages_received( void ) { return inhibition_messages_received; }
		Integer get_inhibition_messages_bytes_received( void ) { return inhibition_messages_bytes_received; }
#endif
		node_id_t get_target_id( void )	{ return target_id; }
		void reset( void )
		{
			spread_messages_send = 0;
			spread_messages_bytes_send = 0;
			spread_messages_inhibited = 0;
			spread_messages_bytes_inhibited = 0;
			spread_messages_received = 0;
			spread_messages_bytes_received = 0;
			inhibition_messages_send = 0;
			inhibition_messages_bytes_send = 0;
			inhibition_messages_inhibited = 0;
			inhibition_messages_bytes_inhibited = 0;
#ifndef OPT_TARGET_LIST_AGGREGATION
			inhibition_messages_received = 0;
			inhibition_messages_bytes_received = 0;
#endif
		}
	private:
		node_id_t target_id;
		Integer spread_messages_send;
		Integer spread_messages_bytes_send;
		Integer spread_messages_inhibited;
		Integer spread_messages_bytes_inhibited;
		Integer spread_messages_received;
		Integer spread_messages_bytes_received;
		Integer inhibition_messages_send;
		Integer inhibition_messages_bytes_send;
		Integer inhibition_messages_inhibited;
		Integer inhibition_messages_bytes_inhibited;
#ifndef OPT_TARGET_LIST_AGGREGATION
		Integer inhibition_messages_received;
		Integer inhibition_messages_bytes_received;

#endif
	};

	template <	typename PLTT_PassiveSpreadMetric_P,
				typename PLTT_PassiveSpreadMetricList_P>
	class PLTT_PassiveSpreadMetricsType
	{
	public:
		typedef PLTT_PassiveSpreadMetric_P PLTT_PassiveSpreadMetric;
		typedef typename PLTT_PassiveSpreadMetric::Integer Integer;
		typedef typename PLTT_PassiveSpreadMetric::node_id_t node_id_t;
		typedef PLTT_PassiveSpreadMetricList_P PLTT_PassiveSpreadMetricList;
		typedef typename PLTT_PassiveSpreadMetricList::iterator PLTT_PassiveSpreadMetricListIterator;
		PLTT_PassiveSpreadMetricsType()
		{
#ifdef OPT_TARGET_LIST_AGGREGATION
		inhibition_messages_received = 0;
		inhibition_messages_bytes_received = 0;
#endif
		}
		~PLTT_PassiveSpreadMetricsType()
		{}
		//---------------------------------------------------------------------------------------
		PLTT_PassiveSpreadMetric* find_metrics_of_target( node_id_t _target_id )
		{
			for (PLTT_PassiveSpreadMetricListIterator i = passive_spread_metric_list.begin(); i != passive_spread_metric_list.end(); ++i )
			{
				if ( i->get_target_id() == _target_id )
				{
					return &(*i);
				}
			}
			return NULL;
		}
		//---------------------------------------------------------------------------------------
		Integer get_spread_messages_send_of_target( node_id_t _target_id )
		{
			PLTT_PassiveSpreadMetric* p = find_metrics_of_target( _target_id );
			if ( p != NULL )
			{
				return p->get_spread_messages_send();
			}
		}
		//---------------------------------------------------------------------------------------
		Integer get_spread_messages_bytes_send_of_target( node_id_t _target_id )
		{
			PLTT_PassiveSpreadMetric* p = find_metrics_of_target( _target_id );
			if ( p != NULL )
			{
				return p->get_spread_messages_bytes_send();
			}
		}
		//---------------------------------------------------------------------------------------
		Integer get_spread_messages_inhibited_of_target( node_id_t _target_id )
		{
			PLTT_PassiveSpreadMetric* p = find_metrics_of_target( _target_id );
			if ( p != NULL )
			{
				return p->get_spread_messages_inhibited();
			}
		}
		//---------------------------------------------------------------------------------------
		Integer get_spread_messages_bytes_inhibited_of_target( node_id_t _target_id )
		{
			PLTT_PassiveSpreadMetric* p = find_metrics_of_target( _target_id );
			if ( p != NULL )
			{
				return p->get_spread_messages_bytes_inhibited();
			}
		}
		//---------------------------------------------------------------------------------------
		Integer get_spread_messages_received_of_target( node_id_t _target_id )
		{
			PLTT_PassiveSpreadMetric* p = find_metrics_of_target( _target_id );
			if ( p != NULL )
			{
				return p->get_spread_messages_received();
			}
		}
		//---------------------------------------------------------------------------------------
		Integer get_spread_messages_bytes_received_of_target( node_id_t _target_id )
		{
			PLTT_PassiveSpreadMetric* p = find_metrics_of_target( _target_id );
			if ( p != NULL )
			{
				return p->get_spread_messages_bytes_received();
			}
		}
		//---------------------------------------------------------------------------------------
		void inc_spread_messages_send_of_target( node_id_t _target_id )
		{
			PLTT_PassiveSpreadMetric* p = find_metrics_of_target( _target_id );
			if ( p != NULL )
			{
				p->inc_spread_messages_send();
			}
			else
			{
				passive_spread_metric_list.push_back( PLTT_PassiveSpreadMetric( _target_id ) );
				PLTT_PassiveSpreadMetricListIterator i = passive_spread_metric_list.end() - 1;
				i->inc_spread_messages_send();
			}
		}
		//---------------------------------------------------------------------------------------
		void inc_spread_messages_bytes_send_of_target( node_id_t _target_id, Integer _bytes_num )
		{
			PLTT_PassiveSpreadMetric* p = find_metrics_of_target( _target_id );
			if ( p != NULL )
			{
				p->inc_spread_messages_bytes_send( _bytes_num );
			}
			else
			{
				passive_spread_metric_list.push_back( PLTT_PassiveSpreadMetric( _target_id ) );
				PLTT_PassiveSpreadMetricListIterator i = passive_spread_metric_list.end() - 1;
				i->inc_spread_messages_bytes_send( _bytes_num );
			}
		}
		//---------------------------------------------------------------------------------------
		void inc_spread_messages_inhibited_of_target( node_id_t _target_id )
		{
			PLTT_PassiveSpreadMetric* p = find_metrics_of_target( _target_id );
			if ( p != NULL )
			{
				p->inc_spread_messages_inhibited();
			}
			else
			{
				passive_spread_metric_list.push_back( PLTT_PassiveSpreadMetric( _target_id ) );
				PLTT_PassiveSpreadMetricListIterator i = passive_spread_metric_list.end() - 1;
				i->inc_spread_messages_inhibited();
			}
		}
		//---------------------------------------------------------------------------------------
		void inc_spread_messages_bytes_inhibited_of_target( node_id_t _target_id, Integer _bytes_num )
		{
			PLTT_PassiveSpreadMetric* p = find_metrics_of_target( _target_id );
			if ( p != NULL )
			{
				p->inc_spread_messages_bytes_inhibited( _bytes_num );
			}
			else
			{
				passive_spread_metric_list.push_back( PLTT_PassiveSpreadMetric( _target_id ) );
				PLTT_PassiveSpreadMetricListIterator i = passive_spread_metric_list.end() - 1;
				i->inc_spread_messages_bytes_inhibited( _bytes_num );
			}
		}
		//---------------------------------------------------------------------------------------
		void inc_spread_messages_received_of_target( node_id_t _target_id )
		{
			PLTT_PassiveSpreadMetric* p = find_metrics_of_target( _target_id );
			if ( p != NULL )
			{
				p->inc_spread_messages_received();
			}
			else
			{
				passive_spread_metric_list.push_back( PLTT_PassiveSpreadMetric( _target_id ) );
				PLTT_PassiveSpreadMetricListIterator i = passive_spread_metric_list.end() - 1;
				i->inc_spread_messages_received();
			}
		}
		//---------------------------------------------------------------------------------------
		void inc_spread_messages_bytes_received_of_target( node_id_t _target_id, Integer _bytes_num )
		{
			PLTT_PassiveSpreadMetric* p = find_metrics_of_target( _target_id );
			if ( p != NULL )
			{
				p->inc_spread_messages_bytes_received( _bytes_num );
			}
			else
			{
				passive_spread_metric_list.push_back( PLTT_PassiveSpreadMetric( _target_id ) );
				PLTT_PassiveSpreadMetricListIterator i = passive_spread_metric_list.end() - 1;
				i->inc_spread_messages_bytes_received( _bytes_num );
			}
		}
		//---------------------------------------------------------------------------------------
		Integer get_inhibition_messages_inhibited_of_target( node_id_t _target_id )
		{
			PLTT_PassiveSpreadMetric* p = find_metrics_of_target( _target_id );
			if ( p != NULL )
			{
				return p->get_inhibition_messages_inhibited();
			}
		}
		//---------------------------------------------------------------------------------------
		Integer get_inhibition_messages_bytes_inhibited_of_target( node_id_t _target_id )
		{
			PLTT_PassiveSpreadMetric* p = find_metrics_of_target( _target_id );
			if ( p != NULL )
			{
				return p->get_inhibition_messages_bytes_inhibited();
			}
		}
		//---------------------------------------------------------------------------------------
		Integer get_inhibition_messages_send_of_target( node_id_t _target_id )
		{
			PLTT_PassiveSpreadMetric* p = find_metrics_of_target( _target_id );
			if ( p != NULL )
			{
				return p->get_inhibition_messages_send();
			}
		}
		//---------------------------------------------------------------------------------------
		Integer get_inhibition_messages_bytes_send_of_target( node_id_t _target_id )
		{
			PLTT_PassiveSpreadMetric* p = find_metrics_of_target( _target_id );
			if ( p != NULL )
			{
				return p->get_inhibition_messages_bytes_send();
			}
		}
		//---------------------------------------------------------------------------------------
		void inc_inhibition_messages_send_of_target( node_id_t _target_id )
		{
			PLTT_PassiveSpreadMetric* p = find_metrics_of_target( _target_id );
			if ( p != NULL )
			{
				p->inc_inhibition_messages_send();
			}
			else
			{
				passive_spread_metric_list.push_back( PLTT_PassiveSpreadMetric( _target_id ) );
				PLTT_PassiveSpreadMetricListIterator i = passive_spread_metric_list.end() - 1;
				i->inc_inhibition_messages_send();
			}
		}
		//---------------------------------------------------------------------------------------
		void inc_inhibition_messages_bytes_send_of_target( node_id_t _target_id, Integer _bytes_num )
		{
			PLTT_PassiveSpreadMetric* p = find_metrics_of_target( _target_id );
			if ( p != NULL )
			{
				p->inc_inhibition_messages_bytes_send( _bytes_num );
			}
			else
			{
				passive_spread_metric_list.push_back( PLTT_PassiveSpreadMetric( _target_id ) );
				PLTT_PassiveSpreadMetricListIterator i = passive_spread_metric_list.end() - 1;
				i->inc_inhibition_messages_bytes_send( _bytes_num );
			}
		}
		//---------------------------------------------------------------------------------------
		void inc_inhibition_messages_inhibited_of_target( node_id_t _target_id )
		{
			PLTT_PassiveSpreadMetric* p = find_metrics_of_target( _target_id );
			if ( p != NULL )
			{
				p->inc_inhibition_messages_inhibited();
			}
			else
			{
				passive_spread_metric_list.push_back( PLTT_PassiveSpreadMetric( _target_id ) );
				PLTT_PassiveSpreadMetricListIterator i = passive_spread_metric_list.end() - 1;
				i->inc_inhibition_messages_inhibited();
			}
		}
		//---------------------------------------------------------------------------------------
		void inc_inhibition_messages_bytes_inhibited_of_target( node_id_t _target_id, Integer _bytes_num )
		{
			PLTT_PassiveSpreadMetric* p = find_metrics_of_target( _target_id );
			if ( p != NULL )
			{
				p->inc_inhibition_messages_bytes_inhibited( _bytes_num );
			}
			else
			{
				passive_spread_metric_list.push_back( PLTT_PassiveSpreadMetric( _target_id ) );
				PLTT_PassiveSpreadMetricListIterator i = passive_spread_metric_list.end() - 1;
				i->inc_inhibition_messages_bytes_inhibited( _bytes_num );
			}
		}
		//---------------------------------------------------------------------------------------

#ifndef OPT_TARGET_LIST_AGGREGATION
		Integer get_inhibition_messages_received_of_target( node_id_t _target_id )
		{
			PLTT_PassiveSpreadMetric* p = find_metrics_of_target( _target_id );
			if ( p != NULL )
			{
				return p->get_inhibition_messages_received();
			}
		}
		//---------------------------------------------------------------------------------------
		Integer get_inhibition_messages_bytes_received_of_target( node_id_t _target_id )
		{
			PLTT_PassiveSpreadMetric* p = find_metrics_of_target( _target_id );
			if ( p != NULL )
			{
				return p->get_inhibition_messages_bytes_received();
			}
		}
		void inc_inhibition_messages_received_of_target( node_id_t _target_id )
		{
			PLTT_PassiveSpreadMetric* p = find_metrics_of_target( _target_id );
			if ( p != NULL )
			{
				p->inc_inhibition_messages_received();
			}
			else
			{
				passive_spread_metric_list.push_back( PLTT_PassiveSpreadMetric( _target_id ) );
				PLTT_PassiveSpreadMetricListIterator i = passive_spread_metric_list.end() - 1;
				i->inc_inhibition_messages_received();
			}
		}
		//---------------------------------------------------------------------------------------
		void inc_inhibition_messages_bytes_received_of_target( node_id_t _target_id, Integer _bytes_num )
		{
			PLTT_PassiveSpreadMetric* p = find_metrics_of_target( _target_id );
			if ( p != NULL )
			{
				p->inc_inhibition_messages_bytes_received( _bytes_num );
			}
			else
			{
				passive_spread_metric_list.push_back( PLTT_PassiveSpreadMetric( _target_id ) );
				PLTT_PassiveSpreadMetricListIterator i = passive_spread_metric_list.end() - 1;
				i->inc_inhibition_messages_bytes_received( _bytes_num );
			}
		}
		//---------------------------------------------------------------------------------------
#else
		//---------------------------------------------------------------------------------------
		void inc_inhibition_messages_received(){ ++inhibition_messages_received; }
		//---------------------------------------------------------------------------------------
		void inc_inhibition_messages_bytes_received( Integer _bytes_num ) { inhibition_messages_bytes_received = inhibition_messages_bytes_received + _bytes_num; }
		//---------------------------------------------------------------------------------------
#endif
		//---------------------------------------------------------------------------------------
		void reset( void )
		{
			for (PLTT_PassiveSpreadMetricListIterator i = passive_spread_metric_list.begin(); i != passive_spread_metric_list.end(); ++i )
			{
				i->reset();
			}
		}
		//---------------------------------------------------------------------------------------
		Integer get_inhibition_messages_send()
		{
			Integer num = 0;
			for (PLTT_PassiveSpreadMetricListIterator i = passive_spread_metric_list.begin(); i != passive_spread_metric_list.end(); ++i )
			{
				num = num + i->get_inhibition_messages_send();
			}
			return num;
		}
		Integer get_inhibition_messages_bytes_send()
		{
			Integer num = 0;
			for (PLTT_PassiveSpreadMetricListIterator i = passive_spread_metric_list.begin(); i != passive_spread_metric_list.end(); ++i )
			{
				num = num + i->get_inhibition_messages_bytes_send();
			}
			return num;
		}
		Integer get_inhibition_messages_received()
		{
			#ifndef OPT_TARGET_LIST_AGGREGATION
			Integer num = 0;
			for (PLTT_PassiveSpreadMetricListIterator i = passive_spread_metric_list.begin(); i != passive_spread_metric_list.end(); ++i )
			{
				num = num + i->get_inhibition_messages_received();
			}
			return num;
			#else
			return inhibition_messages_received;
			#endif
		}
		Integer get_inhibition_messages_bytes_received()
		{
			#ifndef OPT_TARGET_LIST_AGGREGATION
			Integer num = 0;
			for (PLTT_PassiveSpreadMetricListIterator i = passive_spread_metric_list.begin(); i != passive_spread_metric_list.end(); ++i )
			{
				num = num + i->get_inhibition_messages_bytes_received();
			}
			return num;
			#else
			return inhibition_messages_bytes_received;
			#endif
		}
		Integer get_inhibition_messages_inhibited()
		{
			Integer num = 0;
			for (PLTT_PassiveSpreadMetricListIterator i = passive_spread_metric_list.begin(); i != passive_spread_metric_list.end(); ++i )
			{
				num = num + i->get_inhibition_messages_inhibited();
			}
			return num;
		}
		Integer get_inhibition_messages_bytes_inhibited()
		{
			Integer num = 0;
			for (PLTT_PassiveSpreadMetricListIterator i = passive_spread_metric_list.begin(); i != passive_spread_metric_list.end(); ++i )
			{
				num = num + i->get_inhibition_messages_bytes_inhibited();
			}
			return num;
		}
		Integer get_spread_messages_send()
		{
			Integer num = 0;
			for (PLTT_PassiveSpreadMetricListIterator i = passive_spread_metric_list.begin(); i != passive_spread_metric_list.end(); ++i )
			{
				num = num + i->get_spread_messages_send();
			}
			return num;
		}
		Integer get_spread_messages_bytes_send()
		{
			Integer num = 0;
			for (PLTT_PassiveSpreadMetricListIterator i = passive_spread_metric_list.begin(); i != passive_spread_metric_list.end(); ++i )
			{
				num = num + i->get_spread_messages_bytes_send();
			}
			return num;
		}
		Integer get_spread_messages_received()
		{
			Integer num = 0;
			for (PLTT_PassiveSpreadMetricListIterator i = passive_spread_metric_list.begin(); i != passive_spread_metric_list.end(); ++i )
			{
				num = num + i->get_spread_messages_received();
			}
			return num;
		}
		Integer get_spread_messages_bytes_received()
		{
			Integer num = 0;
			for (PLTT_PassiveSpreadMetricListIterator i = passive_spread_metric_list.begin(); i != passive_spread_metric_list.end(); ++i )
			{
				num = num + i->get_spread_messages_bytes_received();
			}
			return num;
		}
		Integer get_spread_messages_inhibited()
		{
			Integer num = 0;
			for (PLTT_PassiveSpreadMetricListIterator i = passive_spread_metric_list.begin(); i != passive_spread_metric_list.end(); ++i )
			{
				num = num + i->get_spread_messages_inhibited();
			}
			return num;
		}
		Integer get_spread_messages_bytes_inhibited()
		{
			Integer num = 0;
			for (PLTT_PassiveSpreadMetricListIterator i = passive_spread_metric_list.begin(); i != passive_spread_metric_list.end(); ++i )
			{
				num = num + i->get_spread_messages_bytes_inhibited();
			}
			return num;
		}
		Integer get_messages_send()
		{
			return get_spread_messages_send() + get_inhibition_messages_send();
		}
		Integer get_messages_bytes_send()
		{
			return get_spread_messages_bytes_send() + get_inhibition_messages_bytes_send();
		}
		Integer get_messages_received()
		{
			return get_spread_messages_received() + get_inhibition_messages_received();
		}
		Integer get_messages_bytes_received()
		{
			return get_spread_messages_bytes_received() + get_inhibition_messages_bytes_received();
		}
		Integer get_messages_inhibited()
		{
			return get_spread_messages_inhibited() + get_inhibition_messages_inhibited();
		}
		Integer get_messages_bytes_inhibited()
		{
			return get_spread_messages_bytes_inhibited() + get_inhibition_messages_bytes_inhibited();
		}
		PLTT_PassiveSpreadMetricList* get_passive_spread_metric_list ()
		{
			return &passive_spread_metric_list;
		}
	private:
		PLTT_PassiveSpreadMetricList passive_spread_metric_list;
#ifdef OPT_TARGET_LIST_AGGREGATION
		Integer inhibition_messages_received;
		Integer inhibition_messages_bytes_received;
#endif
	};
}
#endif
