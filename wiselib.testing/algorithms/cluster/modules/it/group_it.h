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
 * File:   group_it.h
 * Author: amaxilat
 */

#ifndef __GROUP_IT_H_
#define __GROUP_IT_H_

#include "util/pstl/vector_static.h"
#include "util/pstl/map_static_vector.h"
#include "util/delegates/delegate.hpp"
#include "util/pstl/iterator.h"
#include "util/pstl/pair.h"

namespace wiselib {

    /**
     * \ingroup it_concept
     *
     * Group iterator module.
     */
    template<typename OsModel_P, typename Radio_P, typename Semantics_P>
    class GroupIterator {
    public:
        //TYPEDEFS
        typedef OsModel_P OsModel;
        // os modules
        typedef Radio_P Radio;
        //typedef typename OsModel::Radio Radio;
        typedef typename OsModel::Timer Timer;
        typedef typename OsModel::Debug Debug;
        typedef Semantics_P Semantics_t;
        typedef typename Semantics_t::predicate_t semantic_id_t; //need to remove it
        typedef typename Semantics_t::predicate_t predicate_t;
        typedef typename Semantics_t::value_t value_t;
        typedef typename Semantics_t::group_container_t group_container_t;
        typedef typename Semantics_t::value_container_t value_container_t;
        typedef typename Semantics_t::predicate_container_t predicate_container_t;
#ifdef ANSWERING
        typedef typename Semantics_t::predicate_container_t predicate_container_t;
#endif
        typedef typename Semantics_t::group_entry_t group_entry_t;



        typedef GroupIterator<OsModel_P, Radio_P, Semantics_P> self_t;

        // data types
        typedef int cluster_id_t;
        typedef typename Radio::node_id_t node_id_t;
        typedef typename Radio::block_data_t block_data_t;
        typedef wiselib::vector_static<OsModel, node_id_t, 6 > groupMembers_t;
        typedef wiselib::pair<predicate_t, value_t> groupValuesEntry_t;
        typedef wiselib::vector_static<OsModel, groupValuesEntry_t, 6 > groupValues_t;

        struct groupsJoinedEntry {
            group_entry_t group;
            node_id_t parent;
            groupMembers_t groupMembers;
            groupValues_t groupValues;
        };
        typedef struct groupsJoinedEntry groupsJoinedEntry_t;
        typedef wiselib::vector_static<OsModel, groupsJoinedEntry_t, 6 > groupsVector_t;
        typedef typename groupsVector_t::iterator groupsVectorIterator_t;

        typedef wiselib::pair<group_entry_t, node_id_t> neighborsVectorEntry_t;
        typedef wiselib::vector_static<OsModel, neighborsVectorEntry_t, 6 > neighborsVector_t;
        typedef typename neighborsVector_t::iterator neighborsVectorIterator_t;


        typedef GroupIterator<OsModel_P, Radio_P, Semantics_P> self_type;

        /*
         * Constructor
         */
        GroupIterator() :
        node_type_(UNCLUSTERED),
        isGateway_(false) {
            groupsVector_.clear();
            lastNotified_ = groupsVector_.begin();
        }

        /*
         * Destructor
         * */
        ~GroupIterator() {
        }

        /**
         *
         * @param radiot
         * wiselib radio
         * @param timert
         * wiselib timer
         * @param debugt
         * wiselib debug
         * @param semantics
         * wiselib semantic storage
         */
        void init(Radio& radio, Timer& timer, Debug& debug, Semantics_t &semantics) {
            radio_ = &radio;
            debug_ = &debug;
            isGateway_ = false;
            semantics_ = &semantics;

        }

        /**
         * 
         * @param nodeType
         */
        inline void set_node_type(int nodeType) {
            node_type_ = nodeType;
        }

        /**
         * 
         * @param gi
         * @return
         */
        inline node_id_t parent(group_entry_t gi) {
            if (!groupsVector_.empty()) {
                for (groupsVectorIterator_t it = groupsVector_.begin(); it != groupsVector_.end(); ++it) {
                    //if of the same size maybe the same                    
                    if (gi == it->group) {
                        //                        debug().debug("found parent %x", it->parent);
                        return it->parent;
                    }
                }
            }
            return Radio::NULL_NODE_ID;
        }

        /**
         *       
         * @return
         */
        inline node_id_t parent() {
            if (!groupsVector_.empty()) {
                return groupsVector_.begin()->parent;
            }
            return Radio::NULL_NODE_ID;
        }

        /**
         * 
         * @return 
         */
        inline int node_type() {
            return node_type_;
        }

        /**
         * 
         */
        void reset() {
            node_type_ = UNCLUSTERED;
            isGateway_ = false;
            groupsVector_.clear();
            group_container_t mygroups = semantics_->get_groups();

            for (typename group_container_t::iterator gi = mygroups.begin(); gi != mygroups.end(); ++gi) {
                add_group(*gi, radio().id());
            }
            semantics_->template reg_semantic_updated_callback<self_type, &self_type::updated_semantic > (this);
        }

        /**
         *
         * @return
         * true if a gateway node
         */
        bool is_gateway() {
            return isGateway_;
        }

        /**
         *
         * @param data
         * pointer to the Join message
         * @param len
         * size of the join message
         */
        void parse_join(block_data_t * data, size_t len) {
            SemaGroupsMsg_t * msg = (SemaGroupsMsg_t *) data;
            uint8_t group_count = msg->contained();

            for (uint8_t i = 0; i < group_count; i++) {
                group_entry_t gi = group_entry_t(msg->get_statement_data(i), msg->get_statement_size(i));
                if (semantics_-> has_group(gi)) {
                    addNode2Members(gi, msg->node_id());
                } else {
                    addNode2Neighbors(gi, msg->node_id());
                }
            }
        }

        /**
         *
         * @param gi
         * @param node
         */
        void addNode2Members(group_entry_t gi, node_id_t node) {
            for (groupsVectorIterator_t it = groupsVector_.begin(); it != groupsVector_.end(); ++it) {
                if (it->group == gi) {
                    for (typename groupMembers_t::iterator gmit = it->groupMembers.begin(); gmit != it->groupMembers.end(); ++gmit) {
                        if (node == *gmit) {
                            return;
                        }
                    }
                    it->groupMembers.push_back(node);
                }
            }
        }

        /**
         * Adds an entry to the Neighbors vector for routing over semantics
         * @param gi    the group of the node
         * @param node  the id of the node
         */
        void addNode2Neighbors(group_entry_t gi, node_id_t node) {
            for (neighborsVectorIterator_t it = neighborsVector_.begin(); it != neighborsVector_.end(); ++it) {
                if (it->first == gi) {
                    it->second = node;
                    return;
                }
            }
            //TODO: add new entry
            neighborsVectorEntry_t newEntry;
            newEntry.first = gi;
            newEntry.second = node;
            neighborsVector_.push_back(newEntry);
        }

        /**
         * 
         * @param type
         * @return
         */
        inline size_t node_count(int type) {
            if (type == 1) {
                //inside cluster
                //                return cluster_neighbors_.size();
            } else if (type == 0) {
                //outside cluster
                return 0;
                //                return non_cluster_neighbors_.size();
            }
            return 0;
        }

        /**
         * 
         * @return
         */
        SemaResumeMsg_t get_resume_payload() {
            SemaResumeMsg_t msg;
            msg.set_node_id(radio().id());
            msg.set_group((block_data_t*) (lastNotified_->group.data()), lastNotified_->group.size());
            for (typename groupValues_t::iterator it = lastNotified_->groupValues.begin(); it != lastNotified_->groupValues.end(); ++it) {
                //                debug().debug("adding to the message %s|%s", it->first.c_str(), it->second.c_str());
                msg.add_predicate(it->first.data(), it->first.size(), it->second.data(), it->second.size());
            }
            lastNotified_++;
            if (lastNotified_ == groupsVector_.end()) {
                lastNotified_ = groupsVector_.begin();
            }
            return msg;
        }

        /**
         *
         * @param msg
         */
        void eat_resume(SemaResumeMsg_t * msg) {

            group_entry_t gi = group_entry_t(msg->group_data(), msg->group_size());

            if (!semantics_->has_group(gi)) return;

            //            debug().debug("received resume for %s from %x contains %d - size %d", gi.c_str(), sender, msg->contained(), msg->length());

            size_t count = msg->contained();
            for (size_t i = 0; i < count; i++) {
                block_data_t * pdata = msg->get_predicate_data(i);
                uint8_t psize = msg->get_predicate_size(i);
                predicate_t predicate = predicate_t(pdata, psize, semantics_->get_allocator());
                block_data_t * vdata = msg->get_value_data(i);
                uint8_t vsize = msg->get_value_size(i);
                value_t value = value_t(vdata, vsize, semantics_->get_allocator());

                aggregate_data(gi, predicate, value);
            }
            //            node_joined(sender);
        }

        /**
         *
         * @param group
         * @param predicate
         * @param value
         */
        void aggregate_data(group_entry_t group, predicate_t predicate, value_t value) {
            for (groupsVectorIterator_t gvit = groupsVector_.begin(); gvit != groupsVector_.end(); ++gvit) {

                //if of the same size maybe the same
                if (group == gvit->group) {
                    for (typename groupValues_t::iterator gvalit = gvit->groupValues.begin(); gvalit != gvit->groupValues.end(); ++gvalit) {
                        if (gvalit->first == predicate) {

                            semantics_->aggregate(&gvalit->second, &value, predicate);
                            //                            debug().debug("Sval:%s-%s:%s", gvit->group.c_str(), predicate.c_str(), gvalit->second.c_str());
                            return;
                        }
                    }
                    groupValuesEntry_t newgoupValueEntry;
                    newgoupValueEntry.first = predicate;
                    newgoupValueEntry.second = value;
                    gvit->groupValues.push_back(newgoupValueEntry);
                }
            }
        }

        /**
         *
         * @param group
         * @param predicate
         * @return
         */
        value_t get_aggregate_data(group_entry_t group, predicate_t predicate) {
            for (groupsVectorIterator_t gvit = groupsVector_.begin(); gvit != groupsVector_.end(); ++gvit) {
                //if of the same size maybe the same
                if (group == gvit->group) {
                    for (typename groupValues_t::iterator gvalit = gvit->groupValues.begin(); gvalit != gvit->groupValues.end(); ++gvalit) {
                        if (gvalit->first == predicate) {
                            return gvalit->second;
                        }
                    }
                }
            }
        }

        /**
         *
         * @return
         * the number of groups joined so far
         */
        size_t clusters_joined() {
            return groupsVector_.size();
        }

        /**
         *
         * @param from
         * @return
         */
        bool node_lost(node_id_t from) {
            bool changed = false;
            {
                groupsVectorIterator_t it = groupsVector_.begin();
                while (it != groupsVector_.end()) {
                    //if in this group the lost node is my parent
                    if (it->parent == from) {
                        changed = true;
                        it->parent = radio().id();
                    }
                    it++;
                }
            }
            {
                neighborsVectorIterator_t it = neighborsVector_.begin();
                while (it != neighborsVector_.end()) {
                    //if in this group the lost node is my parent
                    if (it->second == from) {
                        changed = true;
                        neighborsVector_.erase(it);

                    } else {
                        it++;
                    }
                }
            }
            return changed;
        }

        /**
         * 
         */
        void present_neighbors() {

        }

        /**
         *
         * @param gi
         * @param parent
         * @return
         */
        bool add_group(group_entry_t gi, node_id_t parent) {
            bool same = false;
            if (!groupsVector_.empty()) {
                for (groupsVectorIterator_t it = groupsVector_.begin(); it != groupsVector_.end(); ++it) {
                    //if of the same size maybe the same
                    if (gi == it->group) {
                        same = true;
                        return !same;
                    }
                }
            }
            groupsJoinedEntry_t newgroup;
            newgroup.group = gi;
            newgroup.parent = parent;
            newgroup.groupMembers.clear();
            newgroup.groupValues.clear();
            if (parent != radio().id()) {
                newgroup.groupMembers.push_back(parent);
            }

            predicate_container_t my_predicates = semantics_->get_predicates();

            for (typename predicate_container_t::iterator it = my_predicates.begin(); it != my_predicates.end(); ++it) {
                value_container_t myvalues = semantics_->get_values(*it);
                if (myvalues.size() > 0) {
                    typename value_container_t::iterator vi = myvalues.begin();
                    groupValuesEntry_t newGroupValuesEntry;
                    newGroupValuesEntry.first = *it;
                    newGroupValuesEntry.second = *vi;
                    newgroup.groupValues.push_back(newGroupValuesEntry);
                }
            }

            groupsVector_.push_back(newgroup);
            return !same;
        }

    private:

        /**
         *
         * Registered to the semantic storage callback
         * updates the local copy of the semantic value upon new semantic value
         * @param predicate
         * @param value
         */
        void updated_semantic(predicate_t predicate, value_t value) {
            for (groupsVectorIterator_t gvit = groupsVector_.begin(); gvit != groupsVector_.end(); ++gvit) {
                bool updated = false;
                for (typename groupValues_t::iterator gvalit = gvit->groupValues.begin(); gvalit != gvit->groupValues.end(); ++gvalit) {
                    if (gvalit->first == predicate) {
                        semantics_->aggregate(&gvalit->second, &value, predicate);
                        //                        debug().debug("Sval:%s-%s:%s", gvit->group.c_str(), predicate.c_str(), gvalit->second.c_str());
                        updated = true;
                        break;
                    }
                }
                //if new add it
                if (!updated) {
                    groupValuesEntry_t newgoupValueEntry;
                    newgoupValueEntry.first = predicate;
                    newgoupValueEntry.second = value;
                    gvit->groupValues.push_back(newgoupValueEntry);
                }

            }
        }

        groupsVector_t groupsVector_;
        neighborsVector_t neighborsVector_;
        int node_type_, lastid_;
        bool isGateway_;
        Semantics_t * semantics_;

        groupsVectorIterator_t lastNotified_;

        Radio * radio_;

        inline Radio & radio() {
            return *radio_;
        }
        Debug * debug_;

        inline Debug & debug() {
            return *debug_;
        }

    };
}
#endif //__GROUP_IT_H_
