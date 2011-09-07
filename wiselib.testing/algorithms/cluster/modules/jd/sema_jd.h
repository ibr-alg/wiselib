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
        typedef typename Semantics_t::semantic_id_t semantic_id_t;
        typedef typename Semantics_t::value_t value_t;
        typedef typename Semantics_t::group_container_t group_container_t;
        typedef typename Semantics_t::value_container_t value_container_t;

        typedef JoinSemanticClusterMsg_t::semantics_t semantics_t;


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
            group_container_t mygroups = semantics_->get_groups();

            msg.set_sender(radio().id());
            msg.set_cluster_id(cluster_id_);
            msg.set_hops(hops_ + 1);

            for (typename group_container_t::iterator gi = mygroups.begin(); gi != mygroups.end(); ++gi) {
                //                debug_->debug("adding semantic size - %d : to add size %d", msg.length(), sizeof (size_t) + gi->size);
                msg.add_statement(gi->data, gi->size);
            }
            //                //            for (size_t count=0;count<total;count++){
            //                semantic_id_t * predicate = *gi;
            //                a[count].semantic_id_ = predicate;
            //                a[count++].value_ = semantics_->get_group_value(predicate);
            //
            //            }
            //            msg.add_statements(a, count);



            //                        char str[100];
            //                        int bytes_written = 0;
            //                        bytes_written += sprintf(str + bytes_written, "pl[");
            //                        for (int i = 0; i<msg.length(); i++) {
            //                            bytes_written += sprintf(str + bytes_written, "%x|", msg.buffer[i]);
            //                        }
            //                        str[bytes_written] = '\0';
            //                        debug_->debug("%s", str);

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
            return false;
            //            return semantics_->has_group(id, value);
        }

        /**
         * JOIN
         * respond to an JOIN message received
         * either join to a cluster or not
         */
        bool join(uint8_t * mess, size_t len) {

            //            debug().debug_payload(mess,len);

            //            bool joined_any = false;
            //            debug().debug("got a join");
            JoinSemanticClusterMsg_t msg;
            memcpy(&msg, mess, len);
            size_t count = msg.contained();
            //            debug().debug("Join on %d conditions from %x|%x ", count, msg.cluster_id(), msg.sender());
            //            debug().debug("Got a semantic cluster from %x|%d of %d semantics", mess->sender(), mess->hops(), count);

            for (size_t i = 0; i < count; i++) {
                //                debug().debug("checking from %d|%d ", a[i].semantic_id_, a[i].semantic_value_);
                size_t size_a = msg.get_statement_size(i);
                block_data_t * data_a = msg.get_statement_data(i);
                //                debug().debug("cond %d , size %d ,data1 %d", count, size_a, *data_a);
                if (!semantics_->has_group(data_a, size_a)) {
                    return false;
                }

            }



            
            if ((msg.cluster_id() >= radio().id()) || (msg.cluster_id() >= cluster_id_)) {
                return false;
            }
            
           
            hops_ = msg.hops();
            cluster_id_ = msg.cluster_id();
            joined_cluster(msg.cluster_id(), msg.hops(), msg.sender());
            return true;



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
