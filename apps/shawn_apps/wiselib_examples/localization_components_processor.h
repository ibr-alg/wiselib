/************************************************************************
 ** This file is part of the network simulator Shawn.                  **
 ** Copyright (C) 2004-2007 by the SwarmNet (www.swarmnet.de) project  **
 ** Shawn is free software; you can redistribute it and/or modify it   **
 ** under the terms of the BSD License. Refer to the shawn-licence.txt **
 ** file in the root of the Shawn source tree for further details.     **
 ************************************************************************/
#ifndef __SHAWN_LEGACYAPPS_WISELIB_EXAMPLES_LOCALIZATION_COMPONENTS_PROCESSOR_H
#define __SHAWN_LEGACYAPPS_WISELIB_EXAMPLES_LOCALIZATION_COMPONENTS_PROCESSOR_H

#include "_legacyapps_enable_cmake.h"
#ifdef ENABLE_WISELIB_EXAMPLES


#include "apps/wiselib/ext_iface_processor.h"
#include "sys/processor.h"
#include "sys/event_scheduler.h"

#include "external_interface/shawn/shawn_os.h"
#include "external_interface/shawn/shawn_radio.h"
#include "external_interface/shawn/shawn_clock.h"
#include "external_interface/shawn/shawn_timer.h"
#include "external_interface/shawn/shawn_debug.h"
#include "external_interface/shawn/shawn_distance.h"
#if 0
#include "algorithms/localization/distance_based/distance_based_localization_base.h"
#include "algorithms/localization/distance_based/modules/localization_nop_module.h"
#include "algorithms/localization/distance_based/modules/distance/localization_dv_hop_module.h"
#include "algorithms/localization/distance_based/modules/distance/localization_sum_dist_module.h"
#include "algorithms/localization/distance_based/modules/distance/localization_euclidean_module.h"
#include "algorithms/localization/distance_based/modules/distance/localization_gpsfree_lcs_module.h"
#include "algorithms/localization/distance_based/modules/position/localization_minmax_module.h"
#include "algorithms/localization/distance_based/modules/position/localization_lateration_module.h"
#include "algorithms/localization/distance_based/modules/position/localization_gpsfree_ncs_module.h"
#include "algorithms/localization/distance_based/modules/refinement/localization_iter_lateration_module.h"
#include "algorithms/localization/distance_based/util/localization_shared_data.h"
#endif
#include "util/pstl/vector_static.h"
#include <vector>
#include <stdint.h>

typedef wiselib::ShawnOsModel Os;
typedef wiselib::ShawnClockModel<Os> Clock;
typedef wiselib::ShawnDistanceModel<Os> Distance;
// typedef wiselib::vector_static<Os, wiselib::OneHopLinkMetricsDataItem<
//    Os::Radio::node_id_t>, 10> MetricsDataItemContainer;
// typedef std::vector<wiselib::OneHopLinkMetricsDataItem<Os::Radio::node_id_t> >
//    MetricsDataItemContainer;

// typedef wiselib::LocalizationSharedData<Os, Os::Radio, Clock> SharedData;
// 
// // no operation - dummy module
// typedef wiselib::LocalizationNopModule<Os, Os::Radio, SharedData> NopModule;
// // distance modules
// typedef wiselib::LocalizationDvHopModule<Os, Os::Radio, Clock, Os::Debug, SharedData> DvHopModule;
// typedef wiselib::LocalizationSumDistModule<Os, Os::Radio, Clock, Distance, Os::Debug, SharedData> SumDistModule;
// typedef wiselib::LocalizationEuclideanModule<Os, Os::Radio, Clock, Distance, Os::Debug, SharedData> EuclideanModule;
// typedef wiselib::LocalizationGpsFreeLcsModule<Os, Os::Radio, Clock, Distance, Os::Debug, SharedData> GpsFreeLcsModule;
// // position modules
// typedef wiselib::LocalizationMinMaxModule<Os, Os::Radio, Os::Debug, SharedData> MinMaxModule;
// typedef wiselib::LocalizationLaterationModule<Os, Os::Radio, Os::Debug, SharedData> LaterationModule;
// typedef wiselib::LocalizationGpsFreeNcsModule<Os, Os::Radio, Clock, Os::Debug, SharedData> GpsFreeNcsModule;
// // refinement modules
// typedef wiselib::LocalizationIterLaterationModule<Os, Os::Radio, Distance, Os::Debug, SharedData> IterLaterationModule;
// 
// typedef wiselib::DistanceBasedLocalization<Os, Os::Radio, Os::Timer, SharedData,
//             GpsFreeLcsModule, GpsFreeNcsModule, NopModule> LocalizationBase;


namespace wiselib_examples
{

   /**
    */
   class LocalizationComponentsProcessor
       : public virtual wiselib::ExtIfaceProcessor
   {
   public:

      ///@name Constructor/Destructor
      ///@{
      LocalizationComponentsProcessor();
      virtual ~LocalizationComponentsProcessor();
      ///@}

      ///@name Inherited from Processor
      ///@{
      /**
       */
      virtual void boot( void ) throw();
      ///@}

      void receive_message( int src_addr, size_t len, unsigned char *buf ) throw();
      void event_evaluation( void* userdata ) throw();
      void actor_off( void* userdata ) throw();

   private:
      int timer_duration_;
      bool timer_started_;
      int event_counter_;
      int dms_threshold_;
      int actor_duration_;
      wiselib::ShawnOs os_;
//       LocalizationBase localization_base_;
   };


}

#endif
#endif
