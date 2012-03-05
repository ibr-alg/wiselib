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
 * File:   attr_chd.h
 * Author: amaxilat
 */

#ifndef __ATTR_CHD_H_
#define __ATTR_CHD_H_

namespace wiselib {

/*
     * Attribute cluster head decision module
     */
    template<typename OsModel_P, typename Radio_P>
    class AtributeClusterHeadDecision {
    public:

        typedef OsModel_P OsModel;
        //TYPEDEFS
        typedef Radio_P Radio;
        //typedef typename OsModel::Radio Radio;
        typedef typename OsModel::Debug Debug;

        // data types
        typedef int cluster_id_t;
        typedef int cluster_level_t; //quite useless within current scheme, supported for compatibility issues
        typedef typename Radio::node_id_t node_id_t;
        typedef typename Radio::size_t size_t;
        typedef typename Radio::block_data_t block_data_t;

        // delegate
        typedef delegate1<int, int*> chd_delegate_t;

        /*
         * Constructor
         * */
        AtributeClusterHeadDecision() :
        cluster_head_(false), theta_(30) {
        }

        /*
         * Destructor
         * */
        ~AtributeClusterHeadDecision() {
        }

        /*
         * INIT
         * initializes the values of radio and debug
         */
        void init(Radio& radio, Debug& debug) {
            radio_ = &radio;
            debug_ = &debug;
        }

        /* SET functions */

        void set_attribute(int theta) {
            theta_ = theta;
            min_theta_ = theta;
        }

        /* GET functions */

        // Returns if Cluster Head

        inline bool is_cluster_head(void) {
            return cluster_head_;
        }

        /*
         * Reset
         * resets the module
         * initializes values
         * */
        inline void reset() {
            cluster_head_ = false;
            min_theta_ = theta_;
        }

        /*
         * CALCULATE_HEAD
         * defines if the node is a cluster head or not
         * if a cluster head return true
         * */
        inline bool calculate_head() {
            // check condition to be a cluster head
            if (theta_ == min_theta_) {
                cluster_head_ = true;
            } else {
                cluster_head_ = false;
            }
            return cluster_head_;
        }

        void receive(node_id_t from, size_t len, block_data_t * mess) {

            AttributeClusterMsg<OsModel, Radio> msg;
            memcpy(&msg, mess, len);
            node_id_t mes_theta = msg.attribute();

            if (mes_theta < min_theta_) {
                min_theta_ = mes_theta;
            }
        }

        AttributeClusterMsg<OsModel, Radio> get_attribute_payload() {
            AttributeClusterMsg<OsModel, Radio> msg;
            msg.set_attribute(min_theta_);
#ifdef DEBUG_PAYLOADS
            debug_->debug("Payload::%x::[%d|%x]::", radio_->id(), type, theta_);
#endif
            return msg;
        }

    private:

        bool cluster_head_; // if a cluster head
        node_id_t theta_; // clustering parameter
        // delegate for callbacks
        chd_delegate_t winner_callback_;
        chd_delegate_t sender_callback_;
        int min_theta_;

        Radio * radio_;
        Debug * debug_;

    };

}

#endif
