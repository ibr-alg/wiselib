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
#ifndef __ALGORITHMS_METRICS_INDIVIDUAL_LINK_METRICS_H__
#define __ALGORITHMS_METRICS_INDIVIDUAL_LINK_METRICS_H__

#include "algorithms/metrics/individual_link/individual_link_metrics_message.h"

#define DEBUG_INDIVIDUAL_LINK_METRICS

namespace wiselib
{

   /** \brief Xyz implementation of \ref xyz_concept "Xzy Concept"
    *  \ingroup xyz_concept
    *
    * Xyz implementation of \ref xyz_concept "Xyz concept" ...
    */
   template<typename OsModel_P,
            typename Radio_P,
            typename Timer_P,
            typename Clock_P,
            typename Debug_P,
            typename DataItemContainer_P>
   class IndividualLinkMetrics
   {
   public:
      enum {
         MESSAGE_SIZE = 40
      };

      typedef OsModel_P OsModel;
      typedef Radio_P Radio;
      typedef Timer_P Timer;
      typedef Clock_P Clock;
      typedef Debug_P Debug;
      typedef DataItemContainer_P DataItemContainer;

      typedef typename DataItemContainer::iterator DataItemContainerIterator;

      typedef IndividualLinkMetrics<OsModel_P, Radio_P, Timer_P, Clock_P, Debug_P, DataItemContainer_P> self_type;
      typedef IndividualLinkMetricsMessage<OsModel, Radio, MESSAGE_SIZE> LinkMessage;

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
      struct MetricsData
      {
         MetricsData(): total_sent(0), lost(0) {};

         uint16_t total_sent;
         uint16_t lost;

         DataItemContainer link_times;
      };
      ///@}

      ///@name Construction / Destruction
      ///@{
      IndividualLinkMetrics();
      ~IndividualLinkMetrics();
      ///@}

      ///@name Main Control
      ///@{
      int init( Radio_P& r, Timer_P& t, Clock_P& c, Debug_P& d);
      inline int init();
      inline int destruct( void );
      void start( node_id_t receiver, millis_t ti, uint16_t packets_to_send );
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
      enum IndividualLinkMetricsMsgIds {
         LinkMetricsRequest = 143,
         LinkMetricsReply   = 144
      };

      MetricsData metrics_data_;

      node_id_t receiver_;
      millis_t transmit_interval_;
      uint16_t packets_to_send_;

      typename Radio::self_pointer_t radio_;
      typename Timer::self_pointer_t timer_;
      typename Clock::self_pointer_t clock_;
      typename Debug::self_pointer_t debug_;

      bool ack_;
      time_t sent_;
   };
   // -----------------------------------------------------------------------
   // -----------------------------------------------------------------------
   // -----------------------------------------------------------------------
   template<typename OsModel_P,
            typename Radio_P,
            typename Timer_P,
            typename Clock_P,
            typename Debug_P,
            typename DataItemContainer_P>
   IndividualLinkMetrics<OsModel_P, Radio_P, Timer_P, Clock_P, Debug_P, DataItemContainer_P>::
   IndividualLinkMetrics()
      : receiver_     ( Radio::NULL_NODE_ID ),
         transmit_interval_ ( 100 ),
         packets_to_send_   ( 0 ),
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
            typename Debug_P,
            typename DataItemContainer_P>
   IndividualLinkMetrics<OsModel_P, Radio_P, Timer_P, Clock_P, Debug_P, DataItemContainer_P>::
   ~IndividualLinkMetrics()
   {
#ifdef DEBUG_INDIVIDUAL_LINK_METRICS
      debug().debug( "IndividualLinkMetrics Destroyed\n" );
#endif
   };
   // -----------------------------------------------------------------------
   template<typename OsModel_P,
            typename Radio_P,
            typename Timer_P,
            typename Clock_P,
            typename Debug_P,
            typename DataItemContainer_P>
   int
   IndividualLinkMetrics<OsModel_P, Radio_P, Timer_P, Clock_P, Debug_P, DataItemContainer_P>::
   init( Radio_P& r, Timer_P& t, Clock_P& c, Debug_P& d)
   {
      radio_ = &r;
      timer_ = &t;
      clock_ = &c;
      debug_ = &d;

#ifdef DEBUG_INDIVIDUAL_LINK_METRICS
      debug().debug( "IndividualLinkMetrics Boots for %i\n", radio().id() );
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
            typename Debug_P,
            typename DataItemContainer_P>
   int
   IndividualLinkMetrics<OsModel_P, Radio_P, Timer_P, Clock_P, Debug_P, DataItemContainer_P>::
   init( void )
   {
      return ERR_UNSPEC;
   }
   // -----------------------------------------------------------------------
   template<typename OsModel_P,
            typename Radio_P,
            typename Timer_P,
            typename Clock_P,
            typename Debug_P,
            typename DataItemContainer_P>
   int
   IndividualLinkMetrics<OsModel_P, Radio_P, Timer_P, Clock_P, Debug_P, DataItemContainer_P>::
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
            typename Debug_P,
            typename DataItemContainer_P>
   void
   IndividualLinkMetrics<OsModel_P, Radio_P, Timer_P, Clock_P, Debug_P, DataItemContainer_P>::
   start( node_id_t receiver, millis_t ti, uint16_t packets_to_send )
   {
#ifdef DEBUG_INDIVIDUAL_LINK_METRICS
      debug().debug( "Start IndividualLinkMetrics: %i -> %i \n", radio().id(), receiver );
#endif
      receiver_          = receiver;
      transmit_interval_ = ti;
      packets_to_send_   = packets_to_send;

      metrics_data_.link_times = DataItemContainer();
      metrics_data_.total_sent = 1;
      metrics_data_.lost = 0;
      metrics_data_.link_times.clear();

      ack_ = false;
      sent_ = clock().time();
      LinkMessage req( LinkMetricsRequest );
      radio().send( receiver_, req.buffer_size(), (uint8_t*)&req );

      timer().template set_timer<self_type, &self_type::timer_elapsed>(
               transmit_interval_, this, 0 );
   }
   // -----------------------------------------------------------------------
   template<typename OsModel_P,
            typename Radio_P,
            typename Timer_P,
            typename Clock_P,
            typename Debug_P,
            typename DataItemContainer_P>
   void
   IndividualLinkMetrics<OsModel_P, Radio_P, Timer_P, Clock_P, Debug_P, DataItemContainer_P>::
   timer_elapsed( void *userdata )
   {
      if ( !ack_ )
         metrics_data_.lost++;

      if ( metrics_data_.total_sent == packets_to_send_ )
      {
#ifdef DEBUG_INDIVIDUAL_LINK_METRICS
      debug().debug( "IndividualLinkMetrics done\n" );
#endif
         return;
      }

      LinkMessage req( LinkMetricsRequest );
      ack_ = false;
      sent_ = clock().time();
#ifdef DEBUG_INDIVIDUAL_LINK_METRICS
      debug().debug( "+\n" );
#endif
      radio().send( receiver_, req.buffer_size(), (uint8_t*)&req );
      metrics_data_.total_sent++;

      timer().template set_timer<self_type, &self_type::timer_elapsed>(
               transmit_interval_, this, 0 );
   }
   // -----------------------------------------------------------------------
   template<typename OsModel_P,
            typename Radio_P,
            typename Timer_P,
            typename Clock_P,
            typename Debug_P,
            typename DataItemContainer_P>
   void
   IndividualLinkMetrics<OsModel_P, Radio_P, Timer_P, Clock_P, Debug_P, DataItemContainer_P>::
   receive( node_id_t from, size_t len, block_data_t *data )
   {
      uint8_t msg_id = *data;
      if ( msg_id == LinkMetricsRequest )
      {
         LinkMessage reply( LinkMetricsReply );
         radio().send( from, reply.buffer_size(), (uint8_t*)&reply );
      }
      else if ( msg_id == LinkMetricsReply )
      {
         ack_ = true;
         time_t now = clock().time();
         time_t duration = now - sent_;
         metrics_data_.link_times.push_back( duration );
#ifdef DEBUG_INDIVIDUAL_LINK_METRICS
      debug().debug( "#\n" );
#endif
#ifdef DEBUG_INDIVIDUAL_LINK_METRICS
//       debug().debug( "now is %i, sent is %i, dur is %i, size is %i\n", now, sent_, duration, metrics_data_.link_times.size() );
#endif
      }
   }

}
#endif
