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
#ifndef __UTIL_BASECLASSES_CLUSTERING_BASE_H__
#define __UTIL_BASECLASSES_CLUSTERING_BASE_H__

#include "util/delegates/delegate.hpp"
#include "util/pstl/vector_static.h"
#include "config.h"

namespace wiselib {

    /** \brief Base clustering class
     *  \ingroup modular_clustering_concept
     *
     *  Basic clustering class that provides helpful methods like registration of
     *  callbacks.
     */
    template <typename OsModel_P,
            int MAX_RECEIVERS = 5 >
            class ClusteringBase {
    public:

        typedef OsModel_P OsModel;
        typedef typename OsModel::Radio::node_id_t node_id_t;
        typedef delegate3<void, uint8_t, uint8_t, node_id_t> cluster_delegate_t;

        // --------------------------------------------------------------------
        typedef vector_static<OsModel, cluster_delegate_t, MAX_RECEIVERS> CallbackVector;
        typedef typename CallbackVector::iterator CallbackVectorIterator;
        // --------------------------------------------------------------------      

        enum ReturnValues {
            SUCCESS = OsModel::SUCCESS
        };
        // --------------------------------------------------------------------

        /**
         * used to register a callback to the clustering algorithm's notifications
         * @param obj_pnt
         * Object of the class registering the callback
         * @return
         * integer pointing to the callback vector entry
         */
        template<class T, void (T::*TMethod)(uint8_t, uint8_t, node_id_t) >
        int reg_state_changed_callback(T *obj_pnt) {
            if (callbacks_.empty())
                callbacks_.assign(MAX_RECEIVERS, cluster_delegate_t());

            for (unsigned int i = 0; i < callbacks_.size(); ++i) {
                if (callbacks_.at(i) == cluster_delegate_t()) {
                    callbacks_.at(i) = cluster_delegate_t::template from_method<T, TMethod > (obj_pnt);
                    return i;
                }
            }

            return -1;
        }
        // --------------------------------------------------------------------

        /**
         * used to unregister a callback to the clustering algorithm's notifications
         * @param idx
         * integer pointing to the callback vector entry
         * @return
         * SUCCESS
         */
        int unreg_state_changed_callback(int idx) {
            callbacks_.at(idx) = cluster_delegate_t();
            return SUCCESS;
        }
        // --------------------------------------------------------------------

        /**
         * 
         * @param event
         * the event type described in clustering_types.h EventIds
         * @param type
         * type of the message 
         * @param node
         * the destination of the message
         */
        void state_changed(uint8_t event, uint8_t type, node_id_t node) {
            for (CallbackVectorIterator
                it = callbacks_.begin();
                    it != callbacks_.end();
                    ++it) {
                if (*it != cluster_delegate_t())
                    (*it)(event, type, node);
            }
        }

    private:
        CallbackVector callbacks_;


    };

}
#endif
