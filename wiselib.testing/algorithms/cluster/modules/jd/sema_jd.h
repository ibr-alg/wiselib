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
    template<typename OsModel_P, typename Radio_P, typename Semantics_P >

    class SemanticJoinDecision {
    public:

        //TYPEDEFS
        typedef OsModel_P OsModel;
        typedef Radio_P Radio;
        typedef typename OsModel::Debug Debug;
        typedef Semantics_P Semantics_t;
        typedef typename Semantics_t::semantics_t semantics_t;
        typedef typename Semantics_t::semantics_vector_t semantics_vector_t;
        typedef typename Semantics_t::semantics_vector_iterator_t semantics_vector_iterator_t;

        // data types
        typedef typename Radio::node_id_t node_id_t;
        typedef typename Radio::block_data_t block_data_t;
        typedef node_id_t cluster_id_t;

        typedef delegate3<void, cluster_id_t, int, node_id_t> join_delegate_t;

        // --------------------------------------------------------------------

        /**
         * Constructor
         */
        SemanticJoinDecision() :
        cluster_id_(0xffff)
        , hops_(200) {
        };

        /**
         * Destructor
         */
        ~SemanticJoinDecision() {
        };

        /**
         * INIT
         * initializes the values of radio and debug
         */
        void init(Radio& radio, Debug& debug, Semantics_t &semantics) {
            radio_ = &radio;
            debug_ = &debug;
            semantics_ = &semantics;
        };

        /**
         * SET functions
         */

        void reset() {

        }

        void became_head() {
            hops_ = 0;
            cluster_id_ = radio().id();
        }

        JoinSemanticClusterMsg_t get_join_request_payload() {
            JoinSemanticClusterMsg_t msg;
            size_t count = 0;
            size_t total = semantics_->enabled_semantics();
            semantics_t a[total];
            msg.set_sender(radio().id());
            msg.set_cluster_id(cluster_id_);
            msg.set_hops(hops_ + 1);
            for (semantics_vector_iterator_t si = semantics_->semantics_vector_.begin(); si != semantics_->semantics_vector_.end(); ++si) {
                if (si->enabled_) {
                    a[count].semantic_id_ = si->semantic_id_;
                    a[count++].semantic_value_ = si->semantic_value_;
                }
            }
            msg.set_payload((uint8_t *) a, total * sizeof (semantics_t));
            return msg;
        }

        void drop_node(node_id_t node) {
            //            for (semantics_vector_iterator_t si = semantics_->semantics_vector_.begin(); si != semantics_->semantics_vector_.end(); ++si) {
            //                if ((*si).node_id_ == node) {
            //                    (*si).node_id_ = 0xffff;
            //                }
            //            }
        }

        bool has_semantic(int id, int value) {
            if (id > 200) {
                return true;
            }
            for (semantics_vector_iterator_t si = semantics_->semantics_vector_.begin(); si != semantics_->semantics_vector_.end(); ++si) {
                if (((*si).semantic_id_ == id) && ((*si).semantic_value_ == value)) {
                    //                    debug().debug("joining semantic cluster %d|%d", id, value);
                    return true;
                }
            }
            return false;
        }

        /**
         * JOIN
         * respond to an JOIN message received
         * either join to a cluster or not
         */
        bool join(uint8_t *payload, uint8_t length) {

            //            bool joined_any = false;
            JoinSemanticClusterMsg_t *mess = (JoinSemanticClusterMsg_t*) payload;
            size_t count = mess->contained() / sizeof (semantics_t);
            semantics_t a[count];
            mess->payload((uint8_t *) a);
            //            debug().debug("Got a semantic cluster from %x|%d of %d semantics", mess->sender(), mess->hops(), count);
            bool join = true;
            for (size_t i = 0; i < count; i++) {
                //                debug().debug("checking from %d|%d ", a[i].semantic_id_, a[i].semantic_value_);
                join = join && has_semantic(a[i].semantic_id_, a[i].semantic_value_);
            }
            if ((join) && (hops_ > mess->hops())) {
                hops_ = mess->hops();
                cluster_id_ = mess->cluster_id();
                joined_cluster(mess->cluster_id(), mess->hops(), mess->sender());
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
         * ENABLE
         * enables the module
         * initializes values
         */
        void enable() {

        };

        /**
         * DISABLE
         * disables this bfsclustering module
         * unregisters callbacks
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
        cluster_id_t cluster_id_;
        int hops_;
        Semantics_t * semantics_;

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
