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

        //        typedef typename Semantics_t::semantics_vector_t semantics_vector_t;
        //        typedef typename Semantics_t::semantics_vector_iterator_t semantics_vector_iterator_t;


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

        /*
         * Destructor
         * */
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
            //            mygroups.clear();
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

            //            semantics_vector_.clear();
            //
            //            group_container_t mygroups = semantics_->get_groups();
            //            //            size_t total = mygroups.size();
            //            //            debug_->debug("total of %d groups", total);
            //            for (typename group_container_t::iterator gi = mygroups.begin(); gi != mygroups.end(); ++gi) {
            //                //            for (size_t count=0;count<total;count++){
            //                int predicate = *gi;
            //                debug_->debug("item %d", predicate);
            //                semantics_t a;                
            //                a.value_ = semantics_->get_group_value(predicate);
            //                a.semantic_id_ = predicate;
            //                //                //                
            //                //                value_container_t myvalues = semantics_->get_values(predicate);
            //                //                debug_->debug("values for %d semantic %d", predicate, myvalues.size());
            //                //                for (typename value_container_t::iterator vi = myvalues.begin(); vi != myvalues.end(); ++vi) {
            //                //                    debug_->debug("got a value of %d", *vi);
            //                //                }
            //                //                //                    //                    a[count].semantic_id_ = predicate;
            //                //                //                    //                    a[count++].value_ = 1;
            //                //                //                }
            //                //                }
            //                //              
            //                semantics_vector_.push_back(a);
            //            }            
            debug_->debug("initialized");

        }

        /*
         * CALCULATE_HEAD
         * defines if the node is a cluster head or not
         * if a cluster head return true
         * */
        inline bool calculate_head() {
            //
            //            for (typename semantics_vector_t::iterator si = semantics_vector_.begin(); si != semantics_vector_.end(); ++si) {
            //                debug_->debug("semantic is %d|%d  %x", si->semantic_id_, si->value_, si->node_id_);
            if ((min_head_id_ == radio_->id())) {
                cluster_head_ = true;
                became_head(1);
            } else {
//                debug_->debug("CLPwaiting for head %x", min_head_id_);
            }
            //            }
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

            //            debug_->debug("received a mess with %d semantics from %x", count, radio_->id());
            //            semantics_t a[count];
            //            msg.payload((uint8_t *) a);
            //
            //
            //            for (size_t i = 0; i < count; i++) {
            //                debug_->debug("semantic is %d|%d |from %x", a[i].semantic_id_, a[i].value_, msg.node_id());
            //                if (!semantics_->has_group(a[i].semantic_id_, a[i].value_)) {
            //                    return;
            //                }
            //                //                for (typename semantics_vector_t::iterator si = semantics_vector_.begin(); si != semantics_vector_.end(); ++si) {
            //                //                    
            //                //                }
            //            }

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
                msg.add_statement(gi->data, gi->size);
            }
            msg.set_node_id(min_head_id_);

            //            debug_->debug("created array msgsize is %d contains %d", msg.length(), msg.contained());

            //            char str[100];
            //            int bytes_written = 0;
            //            bytes_written += sprintf(str + bytes_written, "pl[");
            //            for (int i = 0; i<msg.length(); i++) {
            //                bytes_written += sprintf(str + bytes_written, "%x|", msg.buffer[i]);
            //            }
            //            str[bytes_written] = '\0';
            //            debug_->debug("%s", str);

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
        //        group_container_t mygroups;


        Radio * radio_;
        Debug * debug_;

    };

}

#endif
