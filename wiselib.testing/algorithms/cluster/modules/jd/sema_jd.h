#ifndef _MOCA_JD_H
#define	_MOCA_JD_H



#include "util/delegates/delegate.hpp"
#include "util/pstl/vector_static.h"
#include "util/pstl/map_static_vector.h"
#include "algorithms/cluster/clustering_types.h"

namespace wiselib {

    /**
     * \ingroup jd_concept
     * 
     * Moca join decision module.
     */
    template<typename OsModel_P, typename Radio_P>

    class SemanticJoinDecision {
    public:

        //TYPEDEFS
        typedef OsModel_P OsModel;
        typedef Radio_P Radio;
        typedef typename OsModel::Debug Debug;


        // data types
        typedef typename Radio::node_id_t node_id_t;
        typedef typename Radio::block_data_t block_data_t;
        typedef int cluster_id_t;

        typedef delegate3<void, cluster_id_t, int, node_id_t> join_delegate_t;


        typedef wiselib::vector_static<OsModel, semantics_t, 10 > semantics_vector_t;

        // --------------------------------------------------------------------

        /*
         * Constructor
         */
        SemanticJoinDecision() : hops_(200) {
            semantics_vector_.clear();

        };

        /*
         * Destructor
         */
        ~SemanticJoinDecision() {
        };

        /*
         * INIT
         * initializes the values of radio and debug
         */
        void init(Radio& radio, Debug& debug) {
            radio_ = &radio;
            debug_ = &debug;
            semantics_vector_.clear();
        };

        /* SET functions */

        void reset() {

        }

        void set_semantic(cluster_id_t sema, int value) {
            for (typename semantics_vector_t::iterator si = semantics_vector_.begin(); si != semantics_vector_.end(); ++si) {
                if (si->semantic_id_ == sema) {
                    si->node_id_ = radio_->id();
                    si->semantic_value_ = value;

                    si->cluster_head_ = false;
                    si->enabled_ = false;
                    return;
                }
            }
            semantics_t newse;
            newse.semantic_id_ = sema;
            newse.node_id_ = radio_->id();
            newse.cluster_head_ = false;
            newse.semantic_value_ = value;
            //            newse.semantic_hops_ = 200;
            newse.enabled_ = false;
            semantics_vector_.push_back(newse);
        }

        bool check_condition(int semantic) {
            for (typename semantics_vector_t::iterator si = semantics_vector_.begin(); si != semantics_vector_.end(); ++si) {
                if (si->semantic_id_ == semantic) {
                    si->enabled_ = true;
                    return true;
                }
            }
            return false;
        }

        bool check_condition(int semantic, int value) {
            for (typename semantics_vector_t::iterator si = semantics_vector_.begin(); si != semantics_vector_.end(); ++si) {
                if ((si->semantic_id_ == semantic) && (si->semantic_value_ == value)) {
                    si->enabled_ = true;
                    return true;
                }
            }
            return false;
        }

        void set_head(int semantic) {
            for (typename semantics_vector_t::iterator si = semantics_vector_.begin(); si != semantics_vector_.end(); ++si) {
                if (si->semantic_id_ == semantic) {
                    hops_ = 0;
                    si->cluster_head_ = true;
                    return;
                }
            }
        }

        size_t enabled_semantics() {
            size_t count = 0;
            for (typename semantics_vector_t::iterator si = semantics_vector_.begin(); si != semantics_vector_.end(); ++si) {
                if (si->enabled_ == true) {
                    count++;
                }
            }
            return count;
        }

        JoinSemanticClusterMsg_t get_join_request_payload() {
            JoinSemanticClusterMsg_t msg;

            size_t count = 0;
            size_t total = enabled_semantics();
            semantics_t a[total];

            msg.set_sender(radio().id());
            msg.set_hops(hops_ + 1);
            for (typename semantics_vector_t::iterator si = semantics_vector_.begin(); si != semantics_vector_.end(); ++si) {
                if (si->enabled_) {
                    a[count].semantic_id_ = si->semantic_id_;
                    a[count++].semantic_value_ = si->semantic_value_;
                }
            }

            msg.set_payload((uint8_t *) a, total * sizeof (semantics_t));
            return msg;
        }


        //
        //        void set_head() {
        //            for (typename semantics_vector_t::iterator svit = semantics_vector_.begin(); svit != semantics_vector_.end(); ++svit) {
        //                (*svit).semantic_hops_ = 0;
        //            }
        //        }

        void drop_node(node_id_t node) {
            for (typename semantics_vector_t::iterator svit = semantics_vector_.begin(); svit != semantics_vector_.end(); ++svit) {
                if ((*svit).node_id_ == node) {
                    (*svit).node_id_ = 0xffff;
                    //                    (*svit).semantic_hops_ = 200;
                }
            }
        }

        bool has_semantic(int id, int value) {
            for (typename semantics_vector_t::iterator svit = semantics_vector_.begin(); svit != semantics_vector_.end(); ++svit) {
                if (((*svit).semantic_id_ == id) && ((*svit).semantic_value_ == value)) {
                    debug().debug("joining semantic cluster %d|%d", id, value);
                    return true;
                }
            }
            return false;

        }

        /*
         * JOIN
         * respond to an JOIN message received
         * either join to a cluster or not
         * */
        bool join(uint8_t *payload, uint8_t length) {
            //
            //            bool joined_any = false;
            JoinSemanticClusterMsg_t *mess = (JoinSemanticClusterMsg_t*) payload;
            size_t count = mess->contained() / sizeof (semantics_t);
            semantics_t a[count];
            mess->payload((uint8_t *) a);
            debug().debug("Got a semantic cluster from %x|%d of %d semantics", mess->sender(), mess->hops(), count);
            bool join = true;
            for (size_t i = 0; i < count; i++) {
                debug().debug("checking from %d|%d ", a[i].semantic_id_, a[i].semantic_value_);
                join = join && has_semantic(a[i].semantic_id_, a[i].semantic_value_);
            }
            if ((join) && (hops_ > mess->hops())) {
                hops_ = mess->hops();
                joined_cluster(mess->sender(), mess->hops(), mess->sender());
                return true;
            }
            return false;

            //
            //            typename JoinMultipleClusterMsg<OsModel, Radio>::cluster_entry_t cl_list[10];
            //            size_t count = mess->clusters(cl_list);
            //#ifdef DEBUG_EXTRA
            //            debug().debug("got a message with %d cluster ids", mess.clusters(cl_list));
            //#endif
            //            if (count > 0) {
            //                for (size_t i = 0; i < count; i++) {
            //#ifdef DEBUG_EXTRA
            //                    debug().debug("Contains %x | %d ", cl_list[i].first, cl_list[i].second);
            //#endif
            //                    if (!clusters_joined_.contains(cl_list[i].first)) {
            //                        if (cl_list[i].second <= maxhops_) {
            //                            clusters_joined_entry_t cl_joined;
            //                            cl_joined.first = cl_list[i].first;
            //                            cl_joined.second = cl_list[i].second + 1;
            //
            //                            clusters_joined_.insert(cl_joined);
            //
            //                            //join the cluster
            //                            //return true
            //                            joined_any = true;
            //                            joined_cluster(cl_joined.first, cl_joined.second, mess->sender_id());
            //                        }
            //                    }
            //                }
            //            }

        }

        /**
          ENABLE
          enables the module
          initializes values
         */
        void enable() {

        };

        /**
          DISABLE
          disables this bfsclustering module
          unregisters callbacks
         */
        void disable() {
        };

        template<class T, void (T::*TMethod)(cluster_id_t, int, node_id_t) >
        int reg_cluster_joined_callback(T *obj_pnt) {
            join_delegate_ = join_delegate_t::template from_method<T, TMethod > (obj_pnt);
            return join_delegate_;
        }
        // --------------------------------------------------------------------

        int unreg_cluster_joined_callback(int idx) {
            join_delegate_ = join_delegate_t();
            return idx;
        }
        // --------------------------------------------------------------------

        void joined_cluster(cluster_id_t cluster, int hops, node_id_t parent) {

            if (join_delegate_ != join_delegate_t()) {
                (join_delegate_) (cluster, hops, parent);
            }
        }
    private:
        join_delegate_t join_delegate_;
        semantics_vector_t semantics_vector_;
        int hops_;

        Radio * radio_; //radio module
        Debug * debug_; //debug module

        Radio& radio() {
            return *radio_;
        }

        Debug& debug() {
            return *debug_;
        }
    };
}


#endif

