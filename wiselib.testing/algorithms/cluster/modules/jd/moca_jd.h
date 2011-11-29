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

    class MocaJoinDecision {
    public:

        //TYPEDEFS
        typedef OsModel_P OsModel;
        typedef Radio_P Radio;
        typedef typename OsModel::Debug Debug;


        // data types
        typedef typename Radio::node_id_t node_id_t;
        typedef typename Radio::block_data_t block_data_t;
        typedef node_id_t cluster_id_t;

        typedef delegate3<void, cluster_id_t, int, node_id_t> join_delegate_t;
        typedef wiselib::MapStaticVector<OsModel, cluster_id_t, int, 20 > clusters_joined_t;
        typedef wiselib::pair<cluster_id_t, int> clusters_joined_entry_t;

        // --------------------------------------------------------------------

        /*
         * Constructor
         */
        MocaJoinDecision() :
        maxhops_(0) {
            clusters_joined_.clear();
        };

        /*
         * Destructor
         */
        ~MocaJoinDecision() {
        };

        /*
         * INIT
         * initializes the values of radio and debug
         */
        void init(Radio& radio, Debug& debug) {
            radio_ = &radio;
            debug_ = &debug;
        };

        /* SET functions */

        void set_maxhops(int maxhops) {
            maxhops_ = maxhops;
        }

        void reset() {
            clusters_joined_.clear();
        }

        /*
         * JOIN
         * respond to an JOIN message received
         * either join to a cluster or not
         * */
        bool join(uint8_t *payload, uint8_t length) {

            bool joined_any = false;
            JoinMultipleClusterMsg_t *mess = (JoinMultipleClusterMsg_t *) payload;
            size_t total = mess->cluster_count() / sizeof (typename JoinMultipleClusterMsg_t::cluster_entry_t);

            typename JoinMultipleClusterMsg_t::cluster_entry_t cl_list[total];
            mess->clusters(cl_list);
#ifdef DEBUG_EXTRA
            debug().debug("got a message with %d cluster ids", mess.clusters(cl_list));
#endif
            if (total > 0) {
                for (size_t i = 0; i < total; i++) {
#ifdef DEBUG_EXTRA
                    debug().debug("Contains %x | %d ", cl_list[i].first, cl_list[i].second);
#endif
                    if (!clusters_joined_.contains(cl_list[i].first)) {
                        if (cl_list[i].second <= maxhops_) {
                            clusters_joined_entry_t cl_joined;
                            cl_joined.first = cl_list[i].first;
                            cl_joined.second = cl_list[i].second + 1;

                            clusters_joined_.insert(cl_joined);

                            //join the cluster
                            //return true
                            joined_any = true;
                            joined_cluster(cl_joined.first, cl_joined.second, mess->sender_id());
                        }
                    }
                }
            }
            return joined_any;
        }

        /**
          ENABLE
          enables the module
          initializes values
         */
        void enable() {
            maxhops_ = 0;
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
        int maxhops_; //hops from cluster head
        join_delegate_t join_delegate_;
        clusters_joined_t clusters_joined_;

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

