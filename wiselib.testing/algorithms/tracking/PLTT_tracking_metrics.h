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
#ifndef __PLTT_TRACKING_METRICS_H__
#define __PLTT_TRACKING_METRICS_H__
#include "PLTT_config.h"
namespace wiselib
{
	template <	typename node_id_t_P,
				typename Integer_P>
	class PLTT_TrackerTrackingMetricType
	{
	public:
		typedef Integer_P Integer;
		typedef node_id_t_P node_id_t;
		PLTT_TrackerTrackingMetricType(){}
		PLTT_TrackerTrackingMetricType( node_id_t _target_id )
		{
			query_messages_send = 0;
			query_messages_bytes_send = 0;
			report_messages_received = 0;
			report_messages_bytes_received = 0;
			echo_messages_send = 0;
			echo_messages_bytes_send = 0;
			echo_messages_received = 0;
			echo_messages_bytes_received = 0;
			target_id = _target_id;
		}
		~PLTT_TrackerTrackingMetricType( void ){}
		void inc_query_messages_send( void ) { ++query_messages_send; }
		void inc_query_messages_bytes_send( Integer _bytes_num ) { query_messages_bytes_send = query_messages_bytes_send + _bytes_num; }
		void inc_report_messages_received( void ) { ++report_messages_received; }
		void inc_report_messages_bytes_received( Integer _bytes_num ) { report_messages_bytes_received = report_messages_bytes_received + _bytes_num; }
		void inc_echo_messages_send( void ) { ++echo_messages_send; }
		void inc_echo_messages_bytes_send( Integer _bytes_num ) { echo_messages_bytes_send = echo_messages_bytes_send + _bytes_num; }
		void inc_echo_messages_received( void ) { ++echo_messages_received; }
		void inc_echo_messages_bytes_received( Integer _bytes_num ) { echo_messages_bytes_received = echo_messages_bytes_received + _bytes_num; }
		Integer get_query_messages_send( void ) { return query_messages_send; }
		Integer get_query_messages_bytes_send( void ) { return query_messages_bytes_send; }
		Integer get_report_messages_received( void ) { return report_messages_received; }
		Integer get_report_messages_bytes_received( void ) { return report_messages_bytes_received; }
		Integer get_echo_messages_send( void ) { return echo_messages_send; }
		Integer get_echo_messages_bytes_send( void ) { return echo_messages_bytes_send; }
		Integer get_echo_messages_received( void ) { return echo_messages_received; }
		Integer get_echo_messages_bytes_received( void ) {return echo_messages_bytes_received; }
		node_id_t get_target_id( void ) { return target_id; }
		void set_target_id( node_id_t _tar_id ){ target_id = _tar_id; }
		void reset()
		{
			query_messages_send = 0;
			query_messages_bytes_send = 0;
			report_messages_received = 0;
			report_messages_bytes_received = 0;
			echo_messages_send = 0;
			echo_messages_bytes_send = 0;
			echo_messages_received = 0;
			echo_messages_bytes_received = 0;
		}
	private:
		node_id_t target_id;
		Integer report_messages_received;
		Integer report_messages_bytes_received;
		Integer query_messages_send;
		Integer query_messages_bytes_send;
		Integer echo_messages_send;
		Integer echo_messages_bytes_send;
		Integer echo_messages_received;
		Integer echo_messages_bytes_received;
	};

	template <	typename node_id_t_P,
				typename Integer_P>
	class PLTT_PassiveTrackingMetricType
	{
	public:
		typedef Integer_P Integer;
		typedef node_id_t_P node_id_t;
		PLTT_PassiveTrackingMetricType(){}
		PLTT_PassiveTrackingMetricType( node_id_t _tracker_id )
		{
			report_messages_send = 0;
			report_messages_bytes_send = 0;
			report_messages_received = 0;
			report_messages_bytes_received = 0;
			query_messages_send = 0;
			query_messages_bytes_send = 0;
			query_messages_received = 0;
			query_messages_bytes_received = 0;
			echo_messages_send = 0;
			echo_messages_bytes_send = 0;
			echo_messages_received = 0;
			echo_messages_bytes_received = 0;
			tracker_id = _tracker_id;
		}
		~PLTT_PassiveTrackingMetricType( void ){}
		void inc_report_messages_send( void ) { ++report_messages_send; }
		void inc_report_messages_bytes_send( Integer _bytes_num ) { report_messages_bytes_send = report_messages_bytes_send + _bytes_num; }
		void inc_report_messages_received( void ) { ++report_messages_received; }
		void inc_report_messages_bytes_received( Integer _bytes_num ) { report_messages_bytes_received = report_messages_bytes_received + _bytes_num; }
		void inc_query_messages_send( void ) { ++query_messages_send; }
		void inc_query_messages_bytes_send( Integer _bytes_num ) { query_messages_bytes_send = query_messages_bytes_send + _bytes_num; }
		void inc_query_messages_received( void ) { ++query_messages_received; }
		void inc_query_messages_bytes_received( Integer _bytes_num ) { query_messages_bytes_received = query_messages_bytes_received + _bytes_num; }
		void inc_echo_messages_send( void ) { ++echo_messages_send; }
		void inc_echo_messages_bytes_send( Integer _bytes_num ) { echo_messages_bytes_send = echo_messages_bytes_send + _bytes_num; }
		void inc_echo_messages_received( void ) { ++echo_messages_received; }
		void inc_echo_messages_bytes_received( Integer _bytes_num ) { echo_messages_bytes_received = echo_messages_bytes_received + _bytes_num; }
		Integer get_report_messages_send( void ) { return report_messages_send; }
		Integer get_report_messages_bytes_send( void ) { return report_messages_bytes_send; }
		Integer get_query_messages_send( void ) { return query_messages_send; }
		Integer get_query_messages_bytes_send( void ) { return query_messages_bytes_send; }
		Integer get_query_messages_received( void ) { return query_messages_received; }
		Integer get_query_messages_bytes_received( void ) { return query_messages_bytes_received; }
		Integer get_report_messages_received( void ) { return report_messages_received; }
		Integer get_report_messages_bytes_received( void ) { return report_messages_bytes_received; }
		Integer get_echo_messages_send( void ) { return echo_messages_send; }
		Integer get_echo_messages_bytes_send( void ) { return echo_messages_bytes_send; }
		Integer get_echo_messages_received( void ) { return echo_messages_received; }
		Integer get_echo_messages_bytes_received( void ) {return echo_messages_bytes_received; }
		node_id_t get_tracker_id( void ) { return tracker_id; }
		void reset()
		{
			report_messages_send = 0;
			report_messages_bytes_send = 0;
			report_messages_received = 0;
			report_messages_bytes_received = 0;
			query_messages_send = 0;
			query_messages_bytes_send = 0;
			query_messages_received = 0;
			query_messages_bytes_received = 0;
			echo_messages_send = 0;
			echo_messages_bytes_send = 0;
			echo_messages_received = 0;
			echo_messages_bytes_received = 0;
		}
		private:
			node_id_t tracker_id;
			Integer report_messages_send;
			Integer report_messages_bytes_send;
			Integer report_messages_received;
			Integer report_messages_bytes_received;
			Integer query_messages_send;
			Integer query_messages_bytes_send;
			Integer query_messages_received;
			Integer query_messages_bytes_received;
			Integer echo_messages_send;
			Integer echo_messages_bytes_send;
			Integer echo_messages_received;
			Integer echo_messages_bytes_received;
	};

	template <	typename PLTT_PassiveTrackingMetric_P,
				typename PLTT_PassiveTrackingMetricList_P>
	class PLTT_PassiveTrackingMetricsType
	{
	public:
		typedef PLTT_PassiveTrackingMetric_P PLTT_PassiveTrackingMetric;
		typedef typename PLTT_PassiveTrackingMetric::Integer Integer;
		typedef typename PLTT_PassiveTrackingMetric::node_id_t node_id_t;
		typedef PLTT_PassiveTrackingMetricList_P PLTT_PassiveTrackingMetricList;
		typedef typename PLTT_PassiveTrackingMetricList::iterator PLTT_PassiveTrackingMetricListIterator;
		PLTT_PassiveTrackingMetricsType()
		{}
		~PLTT_PassiveTrackingMetricsType()
		{}
		//----------------------------------------------------------------------------------------------------
		PLTT_PassiveTrackingMetric* find_metrics_of_agent_for_tracker( node_id_t _tracker_id )
		{
			for (PLTT_PassiveTrackingMetricListIterator i = passive_tracking_metric_list.begin(); i != passive_tracking_metric_list.end(); ++i )
			{
				if  ( i->get_tracker_id() == _tracker_id )
				{
					return &(*i);
				}
			}
			return NULL;
		}
		//----------------------------------------------------------------------------------------------------
		Integer get_query_messages_send_of_tracker( node_id_t _tracker_id )
		{
			PLTT_PassiveTrackingMetric* p = find_metrics_of_agent_for_tracker( _tracker_id );
			if ( p != NULL )
			{
				return p->get_query_messages_send();
			}
		}
		//----------------------------------------------------------------------------------------------------
		Integer get_query_messages_bytes_send_of_tracker( node_id_t _tracker_id )
		{
			PLTT_PassiveTrackingMetric* p = find_metrics_of_agent_for_tracker( _tracker_id );
			if ( p != NULL )
			{
				return p->get_query_messages_bytes_send();
			}
		}
		//----------------------------------------------------------------------------------------------------
		Integer get_query_messages_received_of_tracker( node_id_t _tracker_id )
		{
			PLTT_PassiveTrackingMetric* p = find_metrics_of_agent_for_tracker( _tracker_id );
			if ( p != NULL )
			{
				return p->get_query_messages_received();
			}
		}
		//----------------------------------------------------------------------------------------------------
		Integer get_query_messages_bytes_received_of_tracker( node_id_t _tracker_id )
		{
			PLTT_PassiveTrackingMetric* p = find_metrics_of_agent_for_tracker( _tracker_id );
			if ( p != NULL )
			{
				return p->get_query_messages_bytes_received_send();
			}
		}
		//----------------------------------------------------------------------------------------------------
		Integer get_report_messages_send_of_tracker( node_id_t _tracker_id )
		{
			PLTT_PassiveTrackingMetric* p = find_metrics_of_agent_for_tracker( _tracker_id );
			if ( p != NULL )
			{
				return p->get_report_messages_send();
			}
		}
		//----------------------------------------------------------------------------------------------------
		Integer get_report_messages_bytes_send_of_tracker( node_id_t _tracker_id )
		{
			PLTT_PassiveTrackingMetric* p = find_metrics_of_agent_for_tracker( _tracker_id );
			if ( p != NULL )
			{
				return p->get_report_messages_bytes_send();
			}
		}
		//----------------------------------------------------------------------------------------------------
		Integer get_report_messages_received_of_tracker( node_id_t _tracker_id )
		{
			PLTT_PassiveTrackingMetric* p = find_metrics_of_agent_for_tracker( _tracker_id );
			if ( p != NULL )
			{
				return p->get_report_messages_received();
			}
		}
		//----------------------------------------------------------------------------------------------------
		Integer get_report_messages_bytes_received_of_tracker( node_id_t _tracker_id )
		{
			PLTT_PassiveTrackingMetric* p = find_metrics_of_agent_for_tracker( _tracker_id );
			if ( p != NULL )
			{
				return p->get_report_messages_bytes_received_send();
			}
		}
		//----------------------------------------------------------------------------------------------------
		Integer get_echo_messages_send_of_tracker( node_id_t _tracker_id )
		{
			PLTT_PassiveTrackingMetric* p = find_metrics_of_agent_for_tracker( _tracker_id );
			if ( p != NULL )
			{
				return p->get_echo_messages_send();
			}
		}
		//----------------------------------------------------------------------------------------------------
		Integer get_echo_messages_bytes_send_of_tracker( node_id_t _tracker_id )
		{
			PLTT_PassiveTrackingMetric* p = find_metrics_of_agent_for_tracker( _tracker_id );
			if ( p != NULL )
			{
				return p->get_echo_messages_bytes_send();
			}
		}
		//----------------------------------------------------------------------------------------------------
		Integer get_echo_messages_received_of_tracker( node_id_t _tracker_id )
		{
			PLTT_PassiveTrackingMetric* p = find_metrics_of_agent_for_tracker( _tracker_id );
			if ( p != NULL )
			{
				return p->get_echo_messages_received();
			}
		}
		//----------------------------------------------------------------------------------------------------
		Integer get_echo_messages_bytes_received_of_tracker( node_id_t _tracker_id )
		{
			PLTT_PassiveTrackingMetric* p = find_metrics_of_agent_for_tracker( _tracker_id );
			if ( p != NULL )
			{
				return p->get_echo_messages_bytes_received_send();
			}
		}
		//----------------------------------------------------------------------------------------------------
		void  inc_query_messages_bytes_send_of_tracker( node_id_t _tracker_id, Integer _bytes_num )
		{
			PLTT_PassiveTrackingMetric* p = find_metrics_of_agent_for_tracker( _tracker_id );
			if ( p != NULL )
			{
				return p->inc_query_messages_bytes_received( _bytes_num );
			}
			else
			{
				passive_tracking_metric_list.push_back( PLTT_PassiveTrackingMetric( _tracker_id ) );
				PLTT_PassiveTrackingMetricListIterator i = passive_tracking_metric_list.end() - 1;
				i->inc_query_messages_bytes_received( _bytes_num );
			}
		}
		//----------------------------------------------------------------------------------------------------
		void  inc_query_messages_send_of_tracker( node_id_t _tracker_id )
		{
			PLTT_PassiveTrackingMetric* p = find_metrics_of_agent_for_tracker( _tracker_id );
			if ( p != NULL )
			{
				return p->inc_query_messages_send();
			}
			else
			{
				passive_tracking_metric_list.push_back( PLTT_PassiveTrackingMetric( _tracker_id ) );
				PLTT_PassiveTrackingMetricListIterator i = passive_tracking_metric_list.end() - 1;
				i->inc_query_messages_send();
			}
		}
		//----------------------------------------------------------------------------------------------------
		void inc_query_messages_received_of_tracker( node_id_t _tracker_id )
		{
			PLTT_PassiveTrackingMetric* p = find_metrics_of_agent_for_tracker( _tracker_id );
			if ( p != NULL )
			{
				return p->inc_query_messages_received();
			}
			else
			{
				passive_tracking_metric_list.push_back( PLTT_PassiveTrackingMetric( _tracker_id ) );
				PLTT_PassiveTrackingMetricListIterator i = passive_tracking_metric_list.end() - 1;
				i->inc_query_messages_received();
			}
		}
		//----------------------------------------------------------------------------------------------------
		void inc_query_messages_bytes_received_of_tracker( node_id_t _tracker_id, Integer _bytes_num )
		{
			PLTT_PassiveTrackingMetric* p = find_metrics_of_agent_for_tracker( _tracker_id );
			if ( p != NULL )
			{
				return p->inc_query_messages_bytes_received( _bytes_num );
			}
			else
			{
				passive_tracking_metric_list.push_back( PLTT_PassiveTrackingMetric( _tracker_id ) );
				PLTT_PassiveTrackingMetricListIterator i = passive_tracking_metric_list.end() - 1;
				i->inc_query_messages_bytes_received( _bytes_num );
			}
		}
		//----------------------------------------------------------------------------------------------------
		void inc_report_messages_send_of_tracker( node_id_t _tracker_id )
		{
			PLTT_PassiveTrackingMetric* p = find_metrics_of_agent_for_tracker( _tracker_id );
			if ( p != NULL )
			{
				return p->inc_report_messages_send();
			}
			else
			{
				passive_tracking_metric_list.push_back( PLTT_PassiveTrackingMetric( _tracker_id ) );
				PLTT_PassiveTrackingMetricListIterator i = passive_tracking_metric_list.end() - 1;
				i->inc_report_messages_send();
			}
		}
		//----------------------------------------------------------------------------------------------------
		void inc_report_messages_bytes_send_of_tracker( node_id_t _tracker_id, Integer _bytes_num )
		{
			PLTT_PassiveTrackingMetric* p = find_metrics_of_agent_for_tracker( _tracker_id );
			if ( p != NULL )
			{
				return p->inc_report_messages_bytes_send( _bytes_num );
			}
			else
			{
				passive_tracking_metric_list.push_back( PLTT_PassiveTrackingMetric( _tracker_id ) );
				PLTT_PassiveTrackingMetricListIterator i = passive_tracking_metric_list.end() - 1;
				i->inc_report_messages_bytes_send( _bytes_num );
			}
		}
		//----------------------------------------------------------------------------------------------------
		void inc_report_messages_received_of_tracker( node_id_t _tracker_id )
		{
			PLTT_PassiveTrackingMetric* p = find_metrics_of_agent_for_tracker( _tracker_id );
			if ( p != NULL )
			{
				return p->inc_report_messages_received();
			}
			else
			{
				passive_tracking_metric_list.push_back( PLTT_PassiveTrackingMetric( _tracker_id ) );
				PLTT_PassiveTrackingMetricListIterator i = passive_tracking_metric_list.end() - 1;
				i->inc_report_messages_received();
			}
		}
		//----------------------------------------------------------------------------------------------------
		void inc_report_messages_bytes_received_of_tracker( node_id_t _tracker_id, Integer _bytes_num )
		{
			PLTT_PassiveTrackingMetric* p = find_metrics_of_agent_for_tracker( _tracker_id );
			if ( p != NULL )
			{
				return p->inc_report_messages_bytes_received( _bytes_num );
			}
			else
			{
				passive_tracking_metric_list.push_back( PLTT_PassiveTrackingMetric( _tracker_id ) );
				PLTT_PassiveTrackingMetricListIterator i = passive_tracking_metric_list.end() - 1;
				i->inc_report_messages_bytes_received( _bytes_num );
			}
		}
		//----------------------------------------------------------------------------------------------------
		void inc_echo_messages_send_of_tracker( node_id_t _tracker_id )
		{
			PLTT_PassiveTrackingMetric* p = find_metrics_of_agent_for_tracker( _tracker_id );
			if ( p != NULL )
			{
				return p->inc_echo_messages_send();
			}
			else
			{
				passive_tracking_metric_list.push_back( PLTT_PassiveTrackingMetric( _tracker_id ) );
				PLTT_PassiveTrackingMetricListIterator i = passive_tracking_metric_list.end() - 1;
				i->inc_echo_messages_send();
			}
		}
		//----------------------------------------------------------------------------------------------------
		void inc_echo_messages_bytes_send_of_tracker( node_id_t _tracker_id, Integer _bytes_num )
		{
			PLTT_PassiveTrackingMetric* p = find_metrics_of_agent_for_tracker( _tracker_id );
			if ( p != NULL )
			{
				return p->inc_echo_messages_bytes_send( _bytes_num );
			}
			else
			{
				passive_tracking_metric_list.push_back( PLTT_PassiveTrackingMetric( _tracker_id ) );
				PLTT_PassiveTrackingMetricListIterator i = passive_tracking_metric_list.end() - 1;
				i->inc_echo_messages_bytes_send( _bytes_num );
			}
		}
		//----------------------------------------------------------------------------------------------------
		void inc_echo_messages_received_of_tracker( node_id_t _tracker_id )
		{
			PLTT_PassiveTrackingMetric* p = find_metrics_of_agent_for_tracker( _tracker_id );
			if ( p != NULL )
			{
				return p->inc_echo_messages_received();
			}
			else
			{
				passive_tracking_metric_list.push_back( PLTT_PassiveTrackingMetric( _tracker_id ) );
				PLTT_PassiveTrackingMetricListIterator i = passive_tracking_metric_list.end() - 1;
				i->inc_echo_messages_received();
			}
		}
		//----------------------------------------------------------------------------------------------------
		void inc_echo_messages_bytes_received_of_tracker( node_id_t _tracker_id, Integer _bytes_num )
		{
			PLTT_PassiveTrackingMetric* p = find_metrics_of_agent_for_tracker( _tracker_id );
			if ( p != NULL )
			{
				return p->inc_echo_messages_bytes_received( _bytes_num );
			}
			else
			{
				passive_tracking_metric_list.push_back( PLTT_PassiveTrackingMetric( _tracker_id ) );
				PLTT_PassiveTrackingMetricListIterator i = passive_tracking_metric_list.end() - 1;
				i->inc_echo_messages_bytes_received( _bytes_num );
			}
		}
		//----------------------------------------------------------------------------------------------------
		Integer get_echo_messages_send( void )
		{
			Integer num = 0;
			for (PLTT_PassiveTrackingMetricListIterator i = passive_tracking_metric_list.begin(); i != passive_tracking_metric_list.end(); ++i )
			{
				num = num + i->get_echo_messages_send();
			}
			return num;
		}
		//----------------------------------------------------------------------------------------------------
		Integer get_echo_messages_bytes_send( void )
		{
			Integer num = 0;
			for (PLTT_PassiveTrackingMetricListIterator i = passive_tracking_metric_list.begin(); i != passive_tracking_metric_list.end(); ++i )
			{
				num = num + i->get_echo_messages_bytes_send();
			}
			return num;
		}
		//----------------------------------------------------------------------------------------------------
		Integer get_echo_messages_received( void )
		{
			Integer num = 0;
			for (PLTT_PassiveTrackingMetricListIterator i = passive_tracking_metric_list.begin(); i != passive_tracking_metric_list.end(); ++i )
			{
				num = num + i->get_echo_messages_received();
			}
			return num;
		}
		//----------------------------------------------------------------------------------------------------
		Integer get_echo_messages_bytes_received( void )
		{
			Integer num = 0;
			for (PLTT_PassiveTrackingMetricListIterator i = passive_tracking_metric_list.begin(); i != passive_tracking_metric_list.end(); ++i )
			{
				num = num + i->get_echo_messages_bytes_received();
			}
			return num;
		}
		//----------------------------------------------------------------------------------------------------
		Integer get_query_messages_send( void )
		{
			Integer num = 0;
			for (PLTT_PassiveTrackingMetricListIterator i = passive_tracking_metric_list.begin(); i != passive_tracking_metric_list.end(); ++i )
			{
				num = num + i->get_query_messages_send();
			}
			return num;
		}
		//----------------------------------------------------------------------------------------------------
		Integer get_query_messages_bytes_send( void )
		{
			Integer num = 0;
			for (PLTT_PassiveTrackingMetricListIterator i = passive_tracking_metric_list.begin(); i != passive_tracking_metric_list.end(); ++i )
			{
				num = num + i->get_query_messages_bytes_send();
			}
			return num;
		}
		//----------------------------------------------------------------------------------------------------
		Integer get_query_messages_received( void )
		{
			Integer num = 0;
			for (PLTT_PassiveTrackingMetricListIterator i = passive_tracking_metric_list.begin(); i != passive_tracking_metric_list.end(); ++i )
			{
				num = num + i->get_query_messages_received();
			}
			return num;
		}
		//----------------------------------------------------------------------------------------------------
		Integer get_query_messages_bytes_received( void )
		{
			Integer num = 0;
			for (PLTT_PassiveTrackingMetricListIterator i = passive_tracking_metric_list.begin(); i != passive_tracking_metric_list.end(); ++i )
			{
				num = num + i->get_query_messages_bytes_received();
			}
			return num;
		}
		//----------------------------------------------------------------------------------------------------
		Integer get_report_messages_send( void )
		{
			Integer num = 0;
			for (PLTT_PassiveTrackingMetricListIterator i = passive_tracking_metric_list.begin(); i != passive_tracking_metric_list.end(); ++i )
			{
				num = num + i->get_report_messages_send();
			}
			return num;
		}
		//----------------------------------------------------------------------------------------------------
		Integer get_report_messages_bytes_send( void )
		{
			Integer num = 0;
			for (PLTT_PassiveTrackingMetricListIterator i = passive_tracking_metric_list.begin(); i != passive_tracking_metric_list.end(); ++i )
			{
				num = num + i->get_report_messages_bytes_send();
			}
			return num;
		}
		//----------------------------------------------------------------------------------------------------
		Integer get_report_messages_received( void )
		{
			Integer num = 0;
			for (PLTT_PassiveTrackingMetricListIterator i = passive_tracking_metric_list.begin(); i != passive_tracking_metric_list.end(); ++i )
			{
				num = num + i->get_report_messages_received();
			}
			return num;
		}
		//----------------------------------------------------------------------------------------------------
		Integer get_report_messages_bytes_received( void )
		{
			Integer num = 0;
			for (PLTT_PassiveTrackingMetricListIterator i = passive_tracking_metric_list.begin(); i != passive_tracking_metric_list.end(); ++i )
			{
				num = num + i->get_report_messages_bytes_received();
			}
			return num;
		}
		//----------------------------------------------------------------------------------------------------
		Integer get_messages_send( void )
		{
			Integer num = 0;
			for (PLTT_PassiveTrackingMetricListIterator i = passive_tracking_metric_list.begin(); i != passive_tracking_metric_list.end(); ++i )
			{
				num = i->get_echo_messages_send() + i->get_query_messages_send() + i->get_report_messages_send() + num;
			}
			return num;
		}
		//----------------------------------------------------------------------------------------------------
		Integer get_messages_bytes_send( void )
		{
			Integer num = 0;
			for (PLTT_PassiveTrackingMetricListIterator i = passive_tracking_metric_list.begin(); i != passive_tracking_metric_list.end(); ++i )
			{
				num = i->get_echo_messages_bytes_send() + i->get_query_messages_bytes_send() + i->get_report_messages_bytes_send() + num;
			}
			return num;
		}
		//----------------------------------------------------------------------------------------------------
		Integer get_messages_received( void )
		{
			Integer num = 0;
			for (PLTT_PassiveTrackingMetricListIterator i = passive_tracking_metric_list.begin(); i != passive_tracking_metric_list.end(); ++i )
			{
				num = i->get_echo_messages_received() + i->get_query_messages_received() + i->get_report_messages_received() + num;
			}
			return num;
		}
		//----------------------------------------------------------------------------------------------------
		Integer get_messages_bytes_received( void )
		{
			Integer num = 0;
			for (PLTT_PassiveTrackingMetricListIterator i = passive_tracking_metric_list.begin(); i != passive_tracking_metric_list.end(); ++i )
			{
				num = i->get_echo_messages_bytes_received() + i->get_query_messages_bytes_received() + i->get_report_messages_bytes_received() + num;
			}
			return num;
		}
		//----------------------------------------------------------------------------------------------------
		PLTT_PassiveTrackingMetricList* get_passive_tracking_metric_list()
		{
			return &passive_tracking_metric_list;
		}
		void reset( void )
		{
			for (PLTT_PassiveTrackingMetricListIterator i = passive_tracking_metric_list.begin(); i != passive_tracking_metric_list.end(); ++i )
			{
				i->reset();
			}
		}
	private:
		PLTT_PassiveTrackingMetricList passive_tracking_metric_list;
	};
}

#endif
