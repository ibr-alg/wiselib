/*
 * File:   sema_chd.h
 * Author: amaxilat
 */

#ifndef __SEMA_CHD_H_
#define __SEMA_CHD_H_

#include "util//pstl/vector_static.h"

namespace wiselib {

    /**
     * Semantic cluster head decision module
     */
    template<typename OsModel_P, typename Radio_P, typename Semantics_P>
    class SemanticClusterHeadDecision {
    public:

        typedef OsModel_P OsModel;
        //TYPEDEFS
        typedef Radio_P Radio;
        //typedef typename OsModel::Radio Radio;
        typedef typename OsModel::Debug Debug;

        // data types
        typedef typename Radio::node_id_t node_id_t;
        typedef typename Radio::size_t size_t;
        typedef typename Radio::block_data_t block_data_t;
        typedef node_id_t cluster_id_t;
        typedef int cluster_level_t; //quite useless within current scheme, supported for compatibility issues


        typedef Semantics_P Semantics_t;
        typedef typename Semantics_t::semantic_id_t semantic_id_t;
        typedef typename Semantics_t::value_t value_t;
        typedef typename Semantics_t::group_container_t group_container_t;
        typedef typename Semantics_t::value_container_t value_container_t;
        typedef typename Semantics_t::group_entry_t group_entry_t;

        // delegate
        typedef delegate1<int, int*> chd_delegate_t;

        typedef delegate1<void, int > head_delegate_t;

        /*
         * Constructor
         * */
        SemanticClusterHeadDecision() :
        cluster_head_(false) {
            head_delegate_ = head_delegate_t();
        }

        /**
         * Destructor
         */
        ~SemanticClusterHeadDecision() {
        }

        /**
         * INIT
         * initializes the values of radio and debug
         */
        void init(Radio& radio, Debug& debug, Semantics_t &semantics) {
            radio_ = &radio;
            debug_ = &debug;
            semantics_ = &semantics;
            min_head_id_ = radio_->id();
        }


        /* GET functions */

        // Returns if Cluster Head

        inline bool is_cluster_head(void) {
            return cluster_head_;
        }

        /**
         * Reset
         * resets the module
         * initializes values
         */
        inline void reset() {
            cluster_head_ = false;
        }

        /*
         * CALCULATE_HEAD
         * defines if the node is a cluster head or not
         * if a cluster head return true
         * */
        inline bool calculate_head() {

            if ((min_head_id_ == radio_->id())) {
                cluster_head_ = true;
                became_head(1);
            } else {
                //                debug_->debug("CLPwaiting for head %x", min_head_id_);
            }
            return cluster_head_;
        }

        void receive(node_id_t from, size_t len, block_data_t * mess) {
            SemaAttributeMsg_t msg;
            //            debug_->debug("got the mess");
            memcpy(&msg, mess, len);
            //            debug_->debug("copied");
            size_t count = msg.contained();
            //            debug_->debug("contains %d", msg.contained());

            for (size_t i = 0; i < count; i++) {
                size_t size_a = msg.get_statement_size(i);
                block_data_t * data_a = msg.get_statement_data(i);
                if (!semantics_-> has_group(data_a, size_a)) {

                    return;
                }
            }

            if (min_head_id_ > msg.node_id()) {
                min_head_id_ = msg.node_id();
            }
        }

        SemaAttributeMsg_t get_attribute_payload() {
            //            debug_->debug("payload");
            SemaAttributeMsg_t msg;
            //            semantics_vector_.clear();
            group_container_t mygroups = semantics_->get_groups();

            for (typename group_container_t::iterator gi = mygroups.begin(); gi != mygroups.end(); ++gi) {
                //                debug_->debug("adding semantic size - %d : to add size %d", msg.length(), sizeof (size_t) + gi->size);
                msg.add_statement(gi->data(), gi->size());
            }
            msg.set_node_id(min_head_id_);

            //            debug_->debug("created array msgsize is %d contains %d", msg.length(), msg.contained());

            return msg;
        }

        template<class T, void (T::*TMethod)(int) >
        int reg_became_head_callback(T * obj_pnt) {
            head_delegate_ = head_delegate_t::template from_method<T, TMethod > (obj_pnt);
            return head_delegate_;
        }
        // --------------------------------------------------------------------

        int unreg_cluster_joined_callback(int idx) {
            head_delegate_ = head_delegate_t();
            return idx;
        }
        // --------------------------------------------------------------------

        void became_head(int val) {

            if (head_delegate_ != head_delegate_t()) {
                (head_delegate_) (val);
            }
        }

    private:

        bool cluster_head_; // if a cluster head        
        node_id_t min_head_id_;
        head_delegate_t head_delegate_;

        Semantics_t * semantics_;

        Radio * radio_;
        Debug * debug_;

    };

}

#endif
