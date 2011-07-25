/************************************************************************
 ** This file is part of the network simulator Shawn.                  **
 ** Copyright (C) 2004-2007 by the SwarmNet (www.swarmnet.de) project  **
 ** Shawn is free software; you can redistribute it and/or modify it   **
 ** under the terms of the BSD License. Refer to the shawn-licence.txt **
 ** file in the root of the Shawn source tree for further details.     **
 ************************************************************************/
#ifndef __SHAWN_LEGACYAPPS_WISELIB_CLUSTERING_PROCESSOR_H
#define __SHAWN_LEGACYAPPS_WISELIB_CLUSTERING_PROCESSOR_H


#include "_legacyapps_enable_cmake.h"
#ifdef ENABLE_WISELIBCLUSTERING

#include "apps/wiselib/ext_iface_processor.h"
#include "sys/processor.h"
#include "sys/event_scheduler.h"

#include "external_interface/shawn/shawn_os.h"
#include "external_interface/shawn/shawn_radio.h"
#include "external_interface/shawn/shawn_timer.h"
#include "external_interface/shawn/shawn_clock.h"
#include "external_interface/shawn/shawn_debug.h"
#include "internal_interface/routing_table/routing_table_static_array.h"
#include "internal_interface/routing_table/routing_table_stl_map.h"

//UNCOMENT ENABLE TO ACTIVATE ALGORITHM
#define ENABLE_BFS
#define ENABLE_DFS
//#define ENABLE_MBFS
//#define ENABLE_MDFS
//#define ENABLE_MMAXMIND
//#define ENABLE_MMOCA
//#define ENABLE_MAXMIND
#define ENABLE_MCAWT
#define ENABLE_AEEC

// bfsclustering MONOLITHIC
#ifdef ENABLE_BFS
#include "algorithms/cluster/bfscluster/bfsclustering.h"

#endif
// bfsclustering MICROKERNEL
#ifdef ENABLE_MBFS
#include "cluster/modular/cluster_formations/bfs_cluster_formation.h"
#include "algorithms/cluster/modules/chd/attr_chd.h"
#include "cluster/modular/it_modules/simple_it.h"
#include "cluster/modular/jd_modules/bfs_jd.h"
#endif
//dfsclustering MONOLITHIC
#ifdef ENABLE_DFS
#include "algorithms/cluster/dfscluster/dfsclustering.h"
#endif
//dfsclustering MICROKERNEL
#ifdef ENABLE_MDFS
#include "cluster/modular/cluster_formations/dfs_cluster_formation.h"
#include "cluster/modular/chd_modules/probabilistic_chd.h"
#include "cluster/modular/it_modules/simple_it.h"
#include "cluster/modular/jd_modules/dfs_jd.h"
#endif
//maxmind MONOLITHIC
#ifdef ENABLE_MAXMIND
#include "algorithms/cluster/maxmind/maxmind.h"
#endif
//maxmind MICROKERNEL
#ifdef ENABLE_MMAXMIND
#include "algorithms/cluster/maxmind/maxmind.h"
#include "algorithms/cluster/modules/chd/maxmind_chd.h"
#include "algorithms/cluster/modules/it/maxmind_it.h"
#include "algorithms/cluster/modules/jd/maxmind_jd.h"
#endif
//moca
#ifdef ENABLE_MMOCA
#include "algorithms/cluster/moca/moca.h"
#include "algorithms/cluster/modules/chd/prob_chd.h"
#include "algorithms/cluster/modules/it/overlapping_it.h"
#include "algorithms/cluster/modules/jd/moca_jd.h"
#endif
//cawt
#ifdef ENABLE_MCAWT
#include "algorithms/cluster/cores/cawt.h"
#include "algorithms/cluster/modules/chd/wt_chd.h"
#include "algorithms/cluster/modules/jd/cawt_jd.h"
#include "algorithms/cluster/modules/it/generic_normal_it.h"
#endif
#ifdef ENABLE_AEEC
#include "algorithms/cluster/cores/aeec.h"
#include "algorithms/cluster/modules/chd/join_request_chd.h"
#include "algorithms/cluster/modules/jd/nearest_head_jd.h"
#include "algorithms/cluster/modules/it/generic_normal_it.h"
#endif

#include "util/metrics/energy_consumption_radio_wrapper.h"
#include <concept_check.hpp>

typedef wiselib::ShawnOsModel Os;

// clustering
#ifdef ENABLE_BFS
// bfsclustering MONOLITHC
typedef wiselib::BfsClustering<Os, Os::Radio, Os::Timer, Os::Debug> bfsclustering_t;
#endif
#ifdef ENABLE_MBFS
// bfsclustering MICROKERNEL
//typedef wiselib::RandomClusterHeadDecision <Os::Radio, Os::Debug> randomCHD_t;
typedef wiselib::AtributeClusterHeadDecision <Os, Os::Radio> attrCHD_t;
typedef wiselib::SimpleJoinDecision<Os> simpleJD_t;
typedef wiselib::SimpleIterator<Os> simpleIT_t;
typedef wiselib::ClusterFormation<Os, Os::Radio, Os::Timer, Os::Debug, attrCHD_t, simpleJD_t, simpleIT_t> Mbfsclustering_t;
#endif
#ifdef ENABLE_DFS
// dfsclustering MONOLITHIC
typedef wiselib::DfsClustering<Os, Os::Radio, Os::Timer, Os::Debug> dfsclustering_t;
#endif

#ifdef ENABLE_MDFS
#ifndef ENABLE_MBFS
typedef wiselib::SimpleIterator<Os, Os::Radio, Os::Timer, Os::Debug> simpleIT_t;
//typedef wiselib::SimpleIterator<Os, Os::Radio, Os::Timer, Os::Debug> simpleIT_t;
#endif
// dfsclustering MICROKERNEL
typedef wiselib::ProbabilisticClusterHeadDecision <Os::Radio, Os::Debug> probaCHD_t;
typedef wiselib::DfsJoinDecision<Os::Radio, Os::Debug> dfsJD_t;
typedef wiselib::MDfsClusterFormation<Os, Os::Radio, Os::Timer, Os::Debug, probaCHD_t, dfsJD_t, simpleIT_t> Mdfsclustering_t;
#endif
#ifdef ENABLE_MAXMIND
// maxmind MONOLITHIC
typedef wiselib::MaxmindCore<Os, Os::Radio, Os::Timer, Os::Debug> maxmind_t;
#endif
#ifdef ENABLE_MMAXMIND
// maxmind MICROKERNEL
typedef wiselib::MaxmindClusterHeadDecision<Os> maxmindCHD_t;
typedef wiselib::MaxmindJoinDecision<Os> maxmindJD_t;
typedef wiselib::MaxmindIterator<Os> maxmindIT_t;
typedef wiselib::MaxmindCore<Os, maxmindCHD_t, maxmindJD_t, maxmindIT_t> Mmaxmindclustering_t;
#endif
#ifdef ENABLE_MMOCA
typedef wiselib::ProbabilisticClusterHeadDecision <Os, Os::Radio> probabilisticCHD_t;
typedef wiselib::MocaJoinDecision<Os> mocaJD_t;
typedef wiselib::OverlappingIterator<Os> overlappingIT_t;
typedef wiselib::MocaCore<Os, probabilisticCHD_t, mocaJD_t, overlappingIT_t> Mmocaclustering_t;
#endif
//#include "util/metrics/energy_consumption_radio_wrapper.h"
//typedef wiselib::EnergyConsumptionRadioWrapperModel<Os, Os::Radio, Os::Clock> EnergyRadio;
#ifdef ENABLE_MCAWT
typedef wiselib::WaitingTimerClusterHeadDecision<Os> wtCHD_t;
typedef wiselib::CawtJoinDecision<Os> cawtJD_t;
typedef wiselib::GenericNormalIterator<Os, Os::Radio, Os::Timer, Os::Debug, Os::Radio::node_id_t> normIT_t;
typedef wiselib::CawtCore<Os, wtCHD_t, cawtJD_t, normIT_t> Mcawtclustering_t;
#endif
#ifdef ENABLE_AEEC
typedef wiselib::JoinRequestClusterHeadDecision<Os, Os::Radio> jrCHD_t;
typedef wiselib::NearestHeadJoinDecision<Os, Os::Radio> nearestJD_t;
typedef wiselib::GenericNormalIterator<Os, Os::Radio, Os::Timer, Os::Debug, wiselib::AeecNeighbourInfo<Os> > aeec_normIT_t;
typedef wiselib::AeecCore<Os, jrCHD_t, nearestJD_t, aeec_normIT_t> Aeecclustering_t;
#endif


//typedef wiselib::ClusterFormation<Os, RandomClusterHeadDecision, SimpleJoinDecision, BfsIterator, BfsGroupKeyFormation>

//typedef wiselib::ClusterRadio<bfs_clustering_t> cluster_radio_t;

namespace wiselib {

    //typedef ClusterFormation<Os, RandomClusterHeadDecision, SimpleJoinDecision, BfsIterator, BfsGroupKeyFormation> cluster_formation_t;

    /**
     */
    class WiselibClusteringProcessor
    : public virtual ExtIfaceProcessor {
    public:

        ///@name Constructor/Destructor
        ///@{
        WiselibClusteringProcessor();
        virtual ~WiselibClusteringProcessor();
        ///@}

        ///@name Inherited from Processor
        ///@{
        /**
         */
        virtual void boot(void) throw ();
        ///@}

        virtual void work(void) throw ();
        void cluster_state_changed(int event) throw ();
    private:
        std::string cluster_algo_;


        //testreliable_t test_reliable_;
        //testvolume_t test_volume_;
        //echo_t echo_module_;




        //cluster_radio_t cr_;

        //cluster_formation_t cf_;
        ShawnOs os_;
        Os::Radio wiselib_radio_;
        Os::Timer wiselib_timer_;
        Os::Debug wiselib_debug_;
	Os::Clock wiselib_clock_;
	Os::Rand wiselib_rand_;
	Os::Distance* wiselib_distance_;
        //ReliableRadio_t ReliableRadio_;
        //VolumeRadio_t VolumeRadio_;

        //echo_t echo_;

        int theta_;

#ifdef ENABLE_BFS
        // bfsclustering MONOLITHC
        bfsclustering_t bfsclustering_;
#endif
#ifdef ENABLE_MBFS
        // bfsclustering MICROKERNEL
        //randomCHD_t randomCHD_;
        attrCHD_t attrCHD_;
        simpleJD_t simpleJD_;
        simpleIT_t simpleIT_;
        Mbfsclustering_t Mbfsclustering_;
#endif
#ifdef ENABLE_DFS
        dfsclustering_t dfsclustering_;
#endif
#ifdef ENABLE_MDFS
#ifndef ENABLE_MBFS
        simpleIT_t simpleIT_;
#endif
        //attrCHD_t attrCHD_;
        probaCHD_t probaCHD_;
        dfsJD_t dfsJD_;
        Mdfsclustering_t Mdfsclustering_;
#endif
#ifdef ENABLE_MAXMIND
        // maxmindclustering MONOLITHC
        maxmind_t maxmindclustering_;
#endif
#ifdef ENABLE_MMAXMIND
        // maxmindcustering MICROKERNEL
        maxmindCHD_t maxmindCHD_;
        maxmindJD_t maxmindJD_;
        maxmindIT_t maxmindIT_;
        Mmaxmindclustering_t Mmaxmindclustering_;
#endif
#ifdef ENABLE_MMOCA
        probabilisticCHD_t probabilisticCHD_;
        mocaJD_t mocaJD_;
        overlappingIT_t overlappingIT_;
        Mmocaclustering_t Mmocaclustering_;
#endif
#ifdef ENABLE_MCAWT
	wtCHD_t wtCHD_;
	cawtJD_t cawtJD_;
	normIT_t normIT_;
	Mcawtclustering_t Mcawtclustering_;
	int cawt_max_round_;
#endif
#ifdef ENABLE_AEEC
	jrCHD_t jrCHD_;
	nearestJD_t aeecJD_;
	aeec_normIT_t aeecIT_;
	Aeecclustering_t Aeecclustering_;
#endif
    };


}

#endif
#endif
