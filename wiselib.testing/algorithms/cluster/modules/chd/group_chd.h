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
