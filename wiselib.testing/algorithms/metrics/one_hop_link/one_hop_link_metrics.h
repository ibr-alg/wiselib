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
#ifndef __ALGORITHMS_METRICS_ONE_HOP_LINK_METRICS_H__
#define __ALGORITHMS_METRICS_ONE_HOP_LINK_METRICS_H__

#include "algorithms/metrics/one_hop_link/one_hop_link_metrics_request.h"
#include "algorithms/metrics/one_hop_link/one_hop_link_metrics_reply.h"

#define DEBUG_ONE_HOP_LINK_METRICS

namespace wiselib
{

   template<typename node_id_t>
   struct OneHopLinkMetricsDataItem
   {
      node_id_t id;
      uint16_t packets_received;
   };


   /** \brief Xyz implementation of \ref xyz_concept "Xzy Concept"
    *  \ingroup xyz_concept
    *
    * Xyz implementation of \ref xyz_concept "Xyz concept" ...
    */
   template<typename OsModel_P,
            typename Radio_P,
            typename Timer_P,
            typename Clock_P,
            typename MetricsDataContainer_P, ///< Container with OneHopLinkMetricsDataItem as values!
            typename Debug_P>
   class OneHopLinkMetrics
   {
   public:
      typedef OsModel_P OsModel;
      typedef Radio_P Radio;
      typedef Timer_P Timer;
      typedef Clock_P Clock;
      typedef MetricsDataContainer_P MetricsDataContainer;
      typedef Debug_P Debug;

      typedef typename MetricsDataContainer::iterator MetricsDataContainerIterator;

      typedef OneHopLinkMetricsDataItem<typename Radio::node_id_t> MetricsDataItem;

      typedef OneHopLinkMetrics<OsModel, Radio, Timer, Clock, MetricsDataContainer, Debug> self_type;
      typedef OneHopLinkMetricsRequestMessage<OsModel, Radio> RequestMessage;
      typedef OneHopLinkMetricsReplyMessage<OsModel, Radio> ReplyMessage;

      typedef typename Radio::node_id_t node_id_t;
      typedef typename Radio::size_t size_t;
      typedef typename Radio::block_data_t block_data_t;

      typedef typename Clock::time_t time_t;

      typedef typename Timer::millis_t millis_t;
      // --------------------------------------------------------------------
      enum ErrorCodes
      {
         SUCCESS = OsModel::SUCCESS,
         ERR_UNSPEC = OsModel::ERR_UNSPEC
      };
      // --------------------------------------------------------------------
      ///@name Data
      ///@{
      struct MetricsData {
         MetricsData(): start_time(time_t()), stop_time(time_t()),
            total_received(0) {};

         time_t start_time;
         time_t stop_time;

         uint16_t total_received;

         MetricsDataContainer link_metrics;
      };
      ///@}

      ///@name Construction / Destruction
      ///@{
      OneHopLinkMetrics();
      ~OneHopLinkMetrics();
      ///@}

      ///@name Main Control
      ///@{
      int init( Radio_P& r, Timer_P& t, Clock_P& c, Debug_P& d);
      int init( void );
      int destruct( void );
      void start( millis_t ti, uint16_t pts, uint16_t ps );
      ///@}

      ///@name Experiment Results
      ///@{
      MetricsData& metrics_data( void )
      { return metrics_data_; };
      ///@}

      ///@name Methods called by Timer
      ///@{
      void timer_elapsed( void *userdata );
      ///@}

      ///@name Methods called by RadioModel
      ///@{
      void receive( node_id_t from, size_t len, block_data_t *data );
      ///@}

   private:

      Radio& radio()
      { return *radio_; }

      Timer& timer()
      { return *timer_; }

      Clock& clock()
      { return *clock_; }

      Debug& debug()
      { return *debug_; }

      /** \brief Message IDs
      */
      enum OneHopLinkMetricsMsgIds {
         LinkMetricsRequest = 145,
         LinkMetricsReply   = 146,
      };

      MetricsData metrics_data_;

      node_id_t sink_;
      uint32_t transmit_interval_;
      int packets_to_send_;
      int packet_size_;
      int seq_num_;

      bool running_;

      typename Radio::self_pointer_t radio_;
      typename Timer::self_pointer_t timer_;
      typename Clock::self_pointer_t clock_;
      typename Debug::self_pointer_t debug_;
   };
   // -----------------------------------------------------------------------
   // -----------------------------------------------------------------------
   // -----------------------------------------------------------------------
   template<typename OsModel_P,
            typename Radio_P,
            typename Timer_P,
            typename Clock_P,
            typename MetricsDataContainer_P,
            typename Debug_P>
   OneHopLinkMetrics<OsModel_P, Radio_P, Timer_P, Clock_P, MetricsDataContainer_P, Debug_P>::
   OneHopLinkMetrics()
      : sink_         ( Radio::NULL_NODE_ID ),
         transmit_interval_ ( 100 ),
         packets_to_send_   ( 0 ),
         packet_size_  ( 0 ),
         seq_num_      ( 0 ),
         running_      ( false ),
         radio_(0),
         timer_(0),
         clock_(0),
         debug_(0)
   {};
   // -----------------------------------------------------------------------
   template<typename OsModel_P,
            typename Radio_P,
            typename Timer_P,
            typename Clock_P,
            typename MetricsDataContainer_P,
            typename Debug_P>
   OneHopLinkMetrics<OsModel_P, Radio_P, Timer_P, Clock_P, MetricsDataContainer_P, Debug_P>::
   ~OneHopLinkMetrics()
   {
#ifdef DEBUG_ONE_HOP_LINK_METRICS
      debug().debug( "OneHopLinkMetrics Destroyed\n" );
#endif
   };
   // -----------------------------------------------------------------------
   template<typename OsModel_P,
            typename Radio_P,
            typename Timer_P,
            typename Clock_P,
            typename MetricsDataContainer_P,
            typename Debug_P>
   int
   OneHopLinkMetrics<OsModel_P, Radio_P, Timer_P, Clock_P, MetricsDataContainer_P, Debug_P>::
   init( Radio_P& r, Timer_P& t, Clock_P& c, Debug_P& d)
   {
      radio_ = &r;
      timer_ = &t;
      clock_ = &c;
      debug_ = &d;

#ifdef DEBUG_ONE_HOP_LINK_METRICS
      debug().debug( "OneHopLinkMetrics Boots for %i\n", radio().id() );
#endif
      radio().enable_radio();
      radio().template reg_recv_callback<self_type, &self_type::receive>(this);

      return SUCCESS;
   }
   // -----------------------------------------------------------------------
   template<typename OsModel_P,
            typename Radio_P,
            typename Timer_P,
            typename Clock_P,
            typename MetricsDataContainer_P,
            typename Debug_P>
   int
   OneHopLinkMetrics<OsModel_P, Radio_P, Timer_P, Clock_P, MetricsDataContainer_P, Debug_P>::
   init( void )
   {
      return ERR_UNSPEC;
   }
   // -----------------------------------------------------------------------
   template<typename OsModel_P,
            typename Radio_P,
            typename Timer_P,
            typename Clock_P,
            typename MetricsDataContainer_P,
            typename Debug_P>
   int
   OneHopLinkMetrics<OsModel_P, Radio_P, Timer_P, Clock_P, MetricsDataContainer_P, Debug_P>::
   destruct( void )
   {
      radio().disable_radio();
      return SUCCESS;
   }
   // -----------------------------------------------------------------------
   template<typename OsModel_P,
            typename Radio_P,
            typename Timer_P,
            typename Clock_P,
            typename MetricsDataContainer_P,
            typename Debug_P>
   void
   OneHopLinkMetrics<OsModel_P, Radio_P, Timer_P, Clock_P, MetricsDataContainer_P, Debug_P>::
   start( millis_t ti, uint16_t pts, uint16_t ps )
   {
#ifdef DEBUG_ONE_HOP_LINK_METRICS
      debug().debug( "Start OneHopLinkMetrics at %i\n", radio().id() );
#endif
      metrics_data_.start_time = clock().time();
      metrics_data_.stop_time = clock().time();
      metrics_data_.total_received = 0;
      metrics_data_.link_metrics.clear();

      RequestMessage req( LinkMetricsRequest, radio().id(), pts, ps, ti );
      radio().send( Radio::BROADCAST_ADDRESS, req.buffer_size(), (uint8_t*)&req );
   }
   // -----------------------------------------------------------------------
   template<typename OsModel_P,
            typename Radio_P,
            typename Timer_P,
            typename Clock_P,
            typename MetricsDataContainer_P,
            typename Debug_P>
   void
   OneHopLinkMetrics<OsModel_P, Radio_P, Timer_P, Clock_P, MetricsDataContainer_P, Debug_P>::
   timer_elapsed( void *userdata )
   {
      seq_num_++;
      ReplyMessage reply( LinkMetricsReply, packet_size_ );
      radio().send( sink_, reply.buffer_size(), (uint8_t*)&reply );

      if( seq_num_ < packets_to_send_ )
      {
         timer().template set_timer<self_type, &self_type::timer_elapsed>(
               transmit_interval_, this, 0 );
      }
      else
      {
#ifdef DEBUG_ONE_HOP_LINK_METRICS
         debug().debug( "#fin at %i\n", radio().id() );
#endif
         running_ = false;
      }
   }
   // -----------------------------------------------------------------------
   template<typename OsModel_P,
            typename Radio_P,
            typename Timer_P,
            typename Clock_P,
            typename MetricsDataContainer_P,
            typename Debug_P>
   void
   OneHopLinkMetrics<OsModel_P, Radio_P, Timer_P, Clock_P, MetricsDataContainer_P, Debug_P>::
   receive( node_id_t from, size_t len, block_data_t *data )
   {
      if ( from == radio().id() )
         return;
      uint8_t msg_id = *data;
      if ( msg_id == LinkMetricsRequest && !running_ )
      {
         RequestMessage *message = (RequestMessage *)data;
         sink_ = message->sink();
         packets_to_send_ = message->pts();
         packet_size_ = message->ps();
         transmit_interval_ = message->ti();

         seq_num_ = 0;
         running_ = true;

#ifdef DEBUG_ONE_HOP_LINK_METRICS
         debug().debug( "Received request at %i\n", radio().id() );
         debug().debug( "  -> type: %i\n", (int)message->msg_id() );
         debug().debug( "  -> sink: %i\n", sink_ );
         debug().debug( "  -> pts : %i\n", packets_to_send_ );
         debug().debug( "  -> ps  : %i\n", packet_size_ );
         debug().debug( "  -> ti  : %i\n", transmit_interval_ );
#endif

         timer().template set_timer<self_type, &self_type::timer_elapsed>(
               transmit_interval_, this, 0 );
      }
      else if ( msg_id == LinkMetricsReply )
      {
         metrics_data_.stop_time = clock().time();

         bool found = false;
         for ( MetricsDataContainerIterator
                  it = metrics_data_.link_metrics.begin();
                  it != metrics_data_.link_metrics.end();
                  ++it )
            if ( (*it).id == from )
            {
               (*it).packets_received++;
               found = true;
               break;
            }

         if ( !found )
         {
            MetricsDataItem item;
            item.id = from;
            item.packets_received = 1;
            metrics_data_.link_metrics.push_back( item );
         }

         metrics_data_.total_received++;
      }
   }

}
#endif
