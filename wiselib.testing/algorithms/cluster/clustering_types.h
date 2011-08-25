/* 
 * File:   clustering_types.h
 * Author: amaxilatis
 *
 * Created on October 12, 2010, 5:34 PM
 */

#ifndef _ALGORITHMS_CLUSTERING_TYPES_H
#define	_ALGORITHMS_CLUSTERING_TYPES_H




namespace wiselib {

    enum msg_ids {
        CLRADIO_MSG = 39,
        JOIN = 43, REJOIN = 43,
        RESUME = 44, CONVERGECAST = 44,
        ATTRIBUTE = 45, FLOOD = 45,
        HEAD_LOST = 46,
        REFORM = 46,
        INFORM = 47,
        JOINM = 43,

        JOIN_ACCEPT = 22,
        JOIN_DENY = 23,
        NEIGHBOR_DISCOVERY = 25,
        NEIGHBOR_REPLY = 26,
        JOIN_REQUEST = 27
    };

    enum node_type {
        UNCLUSTERED = 0,
        SIMPLE = 1,
        HEAD = 2,
        GATEWAY = 3
    };

    enum EventIds {
        ELECTED_CLUSTER_HEAD = 0, //former CLUSTER_HEAD_CHANGED
        NODE_JOINED = 1,
        NODE_LEFT = 2,
        CLUSTER_FORMED = 3,
        GATEWAY_NODE = 4,
        MESSAGE_SENT = 5
    };

    enum clustering_status {
        FORMED = 0,
        FORMING = 1,
        UNFORMED = 2
    };

    enum ClusterIds {
        UNKNOWN_CLUSTER_HEAD = 0xffff
    };

    struct semantics {
        wiselib::OSMODEL::Radio::node_id_t node_id_;
        int semantic_id_;
        int semantic_value_;
//        int semantic_hops_;
        bool cluster_head_;
        bool enabled_;
    };
    typedef struct semantics semantics_t;
}


#include "algorithms/cluster/semantics.h" 


#include "algorithms/cluster/join_message.h"
typedef wiselib::JoinClusterMsg<wiselib::OSMODEL, wiselib::OSMODEL::Radio> JoinClusterMsg_t;
#include "algorithms/cluster/messages/head_lost.h"
typedef wiselib::LostClusterMsg<wiselib::OSMODEL, wiselib::OSMODEL::Radio> LostClusterMsg_t;
#include "algorithms/cluster/messages/join_sema.h"
typedef wiselib::JoinSemanticClusterMsg<wiselib::OSMODEL, wiselib::OSMODEL::Radio> JoinSemanticClusterMsg_t;
#include "algorithms/cluster/join_multiple_message.h"
typedef wiselib::JoinMultipleClusterMsg<wiselib::OSMODEL, wiselib::OSMODEL::Radio> JoinMultipleClusterMsg_t;
#include "algorithms/cluster/convergecast_message.h"
typedef wiselib::ConvergecastMsg<wiselib::OSMODEL, wiselib::OSMODEL::Radio> ConvergecastMsg_t;
#include "algorithms/cluster/join_accept_message.h"
#include "algorithms/cluster/resume_message.h"
typedef wiselib::ResumeClusterMsg<wiselib::OSMODEL, wiselib::OSMODEL::Radio> ResumeMsg_t;
#include "algorithms/cluster/reform_message.h"
#include "algorithms/cluster/attribute_message.h"


#include "algorithms/cluster/messages/sema_attr.h"
typedef wiselib::SemaAttrClusterMsg<wiselib::OSMODEL, wiselib::OSMODEL::Radio> SemaAttributeMsg_t;

#include "algorithms/cluster_radio/cluster_radio_message.h"

#endif	/* _CLUSTERING_TYPES_H */

