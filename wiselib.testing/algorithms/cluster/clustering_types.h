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


}


#include "algorithms/cluster/messages/join_message.h"
typedef wiselib::JoinClusterMsg<wiselib::OSMODEL, wiselib::OSMODEL::Radio> JoinClusterMsg_t;
#include "algorithms/cluster/messages/head_lost.h"
typedef wiselib::LostClusterMsg<wiselib::OSMODEL, wiselib::OSMODEL::Radio> LostClusterMsg_t;
#include "algorithms/cluster/messages/join_multiple_message.h"
typedef wiselib::JoinMultipleClusterMsg<wiselib::OSMODEL, wiselib::OSMODEL::Radio> JoinMultipleClusterMsg_t;
#include "algorithms/cluster/messages/convergecast_message.h"
typedef wiselib::ConvergecastMsg<wiselib::OSMODEL, wiselib::OSMODEL::Radio> ConvergecastMsg_t;
#include "algorithms/cluster/messages/join_accept_message.h"
typedef wiselib::JoinAccClusterMsg<wiselib::OSMODEL, wiselib::OSMODEL::Radio> JoinAccClusterMsg_t;
#include "algorithms/cluster/messages/resume_message.h"
typedef wiselib::ResumeClusterMsg<wiselib::OSMODEL, wiselib::OSMODEL::Radio> ResumeMsg_t;
#include "algorithms/cluster/messages/reform_message.h"
typedef wiselib::ResumeClusterMsg<wiselib::OSMODEL, wiselib::OSMODEL::Radio> ReformClusterMsg_t;
#include "algorithms/cluster/messages/attribute_message.h"
typedef wiselib::AttributeClusterMsg<wiselib::OSMODEL, wiselib::OSMODEL::Radio> AttributeClusterMsg_t;

#include "algorithms/cluster/messages/sema_attr.h"
typedef wiselib::SemaAttrClusterMsg<wiselib::OSMODEL, wiselib::OSMODEL::Radio> SemaAttributeMsg_t;
#include "algorithms/cluster/messages/sema_groups.h"
typedef wiselib::SemaGroupsClusterMsg<wiselib::OSMODEL, wiselib::OSMODEL::Radio> SemaGroupsMsg_t;
#include "algorithms/cluster/messages/join_sema.h"
typedef wiselib::JoinSemanticClusterMsg<wiselib::OSMODEL, wiselib::OSMODEL::Radio> JoinSemanticClusterMsg_t;

#include "algorithms/cluster/messages/sema_resu.h"
typedef wiselib::SemaResuClusterMsg<wiselib::OSMODEL, wiselib::OSMODEL::Radio> SemaResumeMsg_t;

//#include "algorithms/cluster_radio/cluster_radio_message.h"

#endif	/* _CLUSTERING_TYPES_H */

