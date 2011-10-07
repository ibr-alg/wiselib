/*
 * File:   sema_it.h
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
#ifdef ANSWERING
        typedef typename Semantics_t::predicate_container_t predicate_container_t;
#endif
        typedef typename Semantics_t::group_entry_t group_entry_t;

        //        struct semantic_head {
        //            semantic_id_t semantic_id_;
        //            value_t semantic_value_;
        //        };
        //

        struct semantic_head_item {
            block_data_t data[20];
            predicate_t predicate;
            value_t value;
        };
        typedef semantic_head_item semantic_head_item_t;
        typedef wiselib::pair<semantic_head_item_t, semantic_head_item_t> semantic_head_entry_t;
        typedef wiselib::vector_static<OsModel, semantic_head_entry_t, 10 > semantic_head_vector_t;

        typedef GroupIterator<OsModel_P, Radio_P, Semantics_P> self_t;

        // data types
        typedef int cluster_id_t;
        typedef typename Radio::node_id_t node_id_t;
        typedef typename Radio::block_data_t block_data_t;
        //
        //        typedef wiselib::vector_static<OsModel, node_id_t, 20 > vector_t;
        //        typedef wiselib::pair<node_id_t, cluster_id_t> gateway_entry_t;
        //        //        typedef wiselib::MapStaticVector<OsModel, node_id_t, cluster_id_t, 20 > gateway_vector_t;
        //        typedef wiselib::vector_static<OsModel, wiselib::pair<node_id_t, node_id_t>, 20 > tree_childs_t;

        struct groups_joined_entry {
            int group_id_;
            block_data_t data[30];
            size_t size_;
            node_id_t parent_;
        };
        typedef struct groups_joined_entry groups_joined_entry_t;
        typedef wiselib::vector_static<OsModel, groups_joined_entry_t, 10 > groups_vector_t;

        /*
         * Constructor
         */
        GroupIterator() :
        node_type_(UNCLUSTERED),
        lastid_(2),
        is_gateway_(false) {
            groups_vector_.clear();
        }

        /*
         * Destructor
         * */
        ~GroupIterator() {
        }

        /*
         * INIT
         * initializes the values of radio timer and debug
         */
        void init(Radio& radio, Timer& timer, Debug& debug, Semantics_t &semantics) {
            radio_ = &radio;
            timer_ = &timer;
            debug_ = &debug;
            is_gateway_ = false;
            semantics_ = &semantics;
        }

        /**
         * SET functions : node_type_
         */
        inline void set_node_type(int node_type) {
            node_type_ = node_type;
        }

        /**
         * GET functions : parent_
         */
        inline node_id_t parent(int parent_id) {
            return 0xffff;
        }

        /**
         * GET functions : node_type_
         */
        inline int node_type() {
            return node_type_;
        }

        /**
         * Enable
         */
        void reset(void) {
            node_type_ = UNCLUSTERED;
            is_gateway_ = false;
            groups_vector_.clear();
            group_container_t mygroups = semantics_->get_groups();

            for (typename group_container_t::iterator gi = mygroups.begin(); gi != mygroups.end(); ++gi) {
                //                debug_->debug("adding semantic size - %d : to add size %d", msg.length(), sizeof (size_t) + gi->size());
                add_group(*gi, radio().id());
            }
        }

        bool is_gateway() {
            return is_gateway_;
        }

        /*
         * Drops the node_id from
         * both lists of cluster
         * and non_cluster neighbors
         */
        inline void drop_node(node_id_t node) {
            //            cluster_neighbors_.erase(node);
            //            non_cluster_neighbors_.erase(node);
        }

        //return the number of nodes known

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


        //TODO: IMPLEMENT CLEAN-UP
        //TODO: TEST CLEAN-UP for stability

        void cleanup() {
            //                for (typename vector_t::iterator it = cluster_neighbors_.begin(); it
            //                                != cluster_neighbors_.end(); ++it) {
            //                            if ((*it).second == false) {
            //                                cluster_neighbors_.erase(it);
            //                            } else {
            //                                (*it).second = false;
            //                            }
            //                        }
        }

        void childs(node_id_t *list) {
            //            for (size_t i = 0; i < cluster_neighbors_.size(); i++) {
            //                list[i] = cluster_neighbors_.at(i).first;
            //            }

        }

        //get the non cluster neighbors in a list

        //        inline int get_outer_nodes(node_id_t* position) {
        //                        return non_cluster_neighbors_.size();
        //        }

        SemaResumeMsg_t get_resume_payload() {
            SemaResumeMsg_t msg;
            msg.set_node_id(radio().id());
#ifdef ANSWERING
            predicate_container_t my_predicates = semantics_->get_predicates();

            for (typename predicate_container_t::iterator it = my_predicates.begin(); it != my_predicates.end(); ++it) {
                value_container_t myvalues = semantics_->get_values(*it);
                for (typename value_container_t::iterator gi = myvalues.begin(); gi != myvalues.end(); ++gi) {
                    msg.add_predicate(it->data(), it->size(), gi->data(), gi->size());
                }
            }
#endif
            return msg;
        }

        void eat_resume(size_t len, uint8_t * data) {
            SemaResumeMsg_t * msg = (SemaResumeMsg_t *) data;

            node_id_t sender = msg->node_id();

            size_t count = msg->contained();
            for (size_t i = 0; i < count; i++) {
#ifdef ANSWERING

                predicate_t predicate = predicate_t(msg->get_predicate_data(i), msg->get_predicate_size(i), semantics_->get_allocator());
                value_t value = value_t(msg->get_value_data(i), msg->get_value_size(i), semantics_->get_allocator());
                add_semantic_value(predicate, value);
                //                debug().debug("Received a resume with %d|%d statement from %x", predicate, value, sender);

#endif
            }
            node_joined(sender);
        }

        void add_semantic_value(predicate_t predicate, value_t value) {//NEDDDSS TOO RREEE TTHHIIINNKKK
            if (!semantic_head_vector_.empty()) {
                for (typename semantic_head_vector_t::iterator it = semantic_head_vector_.begin();
                        it != semantic_head_vector_.end(); ++it) {
                    if (semantics_->cmp(predicate, it->first.predicate, predicate) == 0) {
                        semantics_->aggregate(it->second.value, value, predicate);
                    }
                }
            }
            semantic_head_entry_t newentry;
            newentry.first.predicate = predicate;
            newentry.second.value = value;
            semantic_head_vector_.push_back(newentry);
        }

        void became_head() {

#ifdef ANSWERING
            predicate_container_t my_predicates = semantics_->get_predicates();

            for (typename predicate_container_t::iterator it = my_predicates.begin(); it != my_predicates.end(); ++it) {
                //                debug().debug("Predicate %s", it->c_str());
                value_container_t myvalues = semantics_->get_values(*it);
                for (typename value_container_t::iterator gi = myvalues.begin(); gi != myvalues.end(); ++gi) {
                    add_semantic_value(*it, *gi);
                }
            }
#endif
            //#ifdef INTEGER
            //            int predicate = 210;
            //            predicate_t pred = predicate_t(&predicate);
            //#else
            //            ///    predicate_t pred = predicate_t("temp");
            //#endif
            //
            //
            //
            //            //            value_container_t myvalues = semantics_->get_values(pred);
            //            //            for (typename value_container_t::iterator gi = myvalues.begin(); gi != myvalues.end(); ++gi) {
            //            //
            //            //                add_semantic_value(pred, *gi);
            //            //            }
            //
            //#ifdef INTEGER
            //            predicate = 211;
            //
            //#else
            //#endif
            //
            //            //            myvalues = semantics_->get_values(pred);
            //            //            for (typename value_container_t::iterator gi = myvalues.begin(); gi != myvalues.end(); ++gi) {
            //            //                add_semantic_value(pred, *gi);
            //            //            }
            //


        }

        int add_group(group_entry_t gi, node_id_t parent) {
            if (!groups_vector_.empty()) {
                for (typename groups_vector_t::iterator it = groups_vector_.begin(); it != groups_vector_.end(); ++it) {
                    //if of the same size maybe the same
                    if (gi.size() == it->size_) {
                        bool same = true;
                        //byte to byte comparisson
                        for (size_t i = 0; i < it->size_; i++) {
                            if (it->data[i] != *(gi.data() + i)) {
                                same = false;
                                break;
                            }
                        }

                        if (same) {
                            if (it->parent_ < parent) {
                                it->parent_ = parent;
                                return it->group_id_;
                            }
                            return -1;
                        }
                    }
                }
            }
            groups_joined_entry_t newgroup;
            memcpy(newgroup.data, gi.data(), gi.size());
            newgroup.size_ = gi.size();
            newgroup.group_id_ = lastid_;
            newgroup.parent_ = parent;
            lastid_ = lastid_ % 100;
            groups_vector_.push_back(newgroup);
            return lastid_++;
        }

        size_t get_group_count() {
            return groups_vector_.size();
        }

        group_entry_t get_value_for_predicate(predicate_t id) {
            int val = -1;
            group_entry_t a;
            a.size_a = 0;
            a.data_a = (block_data_t*) & val;

            for (typename semantic_head_vector_t::iterator it = semantic_head_vector_.begin();
                    it != semantic_head_vector_.end(); ++it) {

                if (semantics_->cmp(it->first.predicate, id, it->first.predicate) == 0) {//not completely correct
                    return it->second.value;
                }
            }


            return a;
        }

        /* SHOW all the known nodes */
        void present_neighbors() {
            //            if (!groups_vector_.empty()) {
            //                char buffer[1024];
            //                int bytes_written = 0;
            //                bytes_written += sprintf(buffer + bytes_written, "Groups(%x)", radio().id());
            //
            //                for (typename groups_vector_t::iterator it = groups_vector_.begin(); it != groups_vector_.end(); ++it) {
            //                    predicate_t pred = predicate_t(it->data, it->size_);
            //                    bytes_written += sprintf(buffer + bytes_written, "%d-%x-%s|", it->group_id_, it->parent_, pred.c_str());
            //                }
            //
            //                //            for (typename vector_t::iterator cni = cluster_neighbors_.begin(); cni != cluster_neighbors_.end(); ++cni) {
            //
            //                //            }
            //                buffer[bytes_written] = '\0';
            //                debug_->debug("%s", buffer);
            //            }
        }

        semantic_head_vector_t semantic_head_vector_;

    private:
        groups_vector_t groups_vector_;
        int node_type_, lastid_;
        bool is_gateway_;
        Semantics_t * semantics_;

        Radio * radio_;

        inline Radio& radio() {
            return *radio_;
        }
        Timer * timer_;

        inline Timer& timer() {
            return *timer_;
        }
        Debug * debug_;

        inline Debug& debug() {
            return *debug_;
        }

    };
}
#endif //__FRONTS_IT_H_
