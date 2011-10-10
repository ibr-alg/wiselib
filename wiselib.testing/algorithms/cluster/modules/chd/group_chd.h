/*
 * File:   group_chd.h
 * Author: amaxilat
 */

#ifndef __GROUP_CHD_H_
#define __GROUP_CHD_H_

namespace wiselib {

    /**
     * Semantic cluster head decision module
     */
    template<typename OsModel_P, typename Radio_P, typename Semantics_P>
    class NothingClusterHeadDecision {
    public:
        typedef OsModel_P OsModel;
        //TYPEDEFS
        typedef Radio_P Radio;
        //typedef typename OsModel::Radio Radio;
        typedef typename OsModel::Debug Debug;

        typedef Semantics_P Semantics_t;

        /**
         * Constructor
         */
        NothingClusterHeadDecision() {
        }

        /**
         * Destructor
         */
        ~NothingClusterHeadDecision() {
        }

        /**
         * INIT
         * initializes the values of radio and debug
         */
        void init(Radio& radio, Debug& debug, Semantics_t &semantics) {
        }

        inline bool is_cluster_head(void) {
            return false;
        }

        /**
         * Reset
         * resets the module
         * initializes values
         */
        inline void reset() {
        }

        /*
         * CALCULATE_HEAD
         * defines if the node is a cluster head or not
         * if a cluster head return true
         * */
        inline bool calculate_head() {
            return false;
        }



    private:
    };

}

#endif
