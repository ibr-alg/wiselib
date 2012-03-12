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
 * File:   semantics.h
 * Author: amaxilat
 *
 */


#include "util/pstl/vector_static.h"
//#include "util/tuple_store/tuple_store.h"

/**
 * if uncommented AGGREGATE_VALUES calculates a mean of the current and last value
 * else the latest value is passed as the aggregated value
 */
#define AGGREGATE_VALUES

namespace wiselib {

    /**
     * Semantic cluster head decision module
     */
    template<typename OsModel_P,
            int MAX_RECEIVERS = 5 >
            class Semantics {
    public:

        typedef OsModel_P OsModel;
        typedef typename OsModel::block_data_t block_data_t;

        class data_t {
        public:

            data_t() {
                size_a = 0;
            };

            /**
             *
             * Create a new data_t with the given data
             *
             * @param dat
             *  pointer to the data to be added
             * @param size
             *  size of the data
             * @param allocator
             *  alloctator (for compatibility with TuppleStorage
             */
            data_t(block_data_t * dat, size_t size = sizeof (int), int allocator = 0) {
                memcpy(&data_a, dat, sizeof (int));
                size_a = size;
            };

            /**
             *
             * @return
             *  a pointer to the data of the object
             */
            block_data_t * data() {
                return (block_data_t*)&data_a;
            };

            /**
             *
             * @return
             *  the size of the data
             */
            size_t size() {
                return size_a;
            }

            /**
             *
             * @return
             *  a string description of the data
             */
            char * c_str() {
                if (size_a == sizeof (int)) {
                    int bytes_written = 0;
                    bytes_written += sprintf(buffer + bytes_written, "%d", data_a);
                    buffer[bytes_written] = '\0';
                }
                return buffer;
            }

            /**
             *
             * @param other
             * @return
             */
            bool operator==(data_t other) const {
                if (size_a != other.size()) {
                    return false;
                } else {
                    block_data_t * fdata = (block_data_t*) & data_a;
                    block_data_t * odata = other.data();
                    for (uint8_t i = 0; i < size_a; i++) {
                        if (fdata[i] != odata[i]) {
                            return false;
                        }
                    }
                    return true;
                }
            }
            int data_a;
            size_t size_a;
            char buffer[5];

        };
        // --------------------------------------------------------------------

        class group_entry_t {
        public:

            group_entry_t() {
                size_a = 0;
            };

            /**
             *
             * Create a new group_entry_t with the given data
             *
             * @param dat
             *  pointer to the data to be added
             * @param size
             *  size of the data
             * @param allocator
             *  alloctator (for compatibility with TuppleStorage
             */
            group_entry_t(block_data_t * dat, size_t size = 2 * sizeof (int), int allocator = 0) {
                memcpy(data_a, dat, 2 * sizeof (int));
                size_a = size;
            };

            /**
             *
             * @return
             *  a pointer to the data of the object
             */
            block_data_t * data() {
                return (block_data_t *) data_a;
            };

            /**
             *
             * @return
             *  the size of the data
             */
            size_t size() {
                return size_a;
            }

            /**
             *
             * @return
             *  a string description of the data
             */
            char * c_str() {
                if (size_a == sizeof (int) *2) {
                    int bytes_written = 0;
                    bytes_written += sprintf(buffer + bytes_written, "%d|%d", data_a[0], data_a[1]);
                    buffer[bytes_written] = '\0';
                }
                return buffer;
            }

            /**
             *
             * @param other
             * @return
             */
            bool operator==(group_entry_t other) const {
//                if (size_a != other.size()) {
//                    return false;
//                } else {
                    block_data_t * fdata = (block_data_t*) data_a;
                    block_data_t * odata = other.data();
                    for (uint8_t i = 0; i < size_a; i++) {
                        if (fdata[i] != odata[i]) {
                            return false;
                        }
                    }
                    return true;
//                }
            }
            int data_a[2];
            size_t size_a;
            char buffer[5];

        };
        // --------------------------------------------------------------------


        typedef data_t value_t;
        typedef data_t semantic_id_t;
        typedef semantic_id_t predicate_t;

        enum semantics_types {
            BUILDING = 1,
            FLOOR = 2,
            SECTOR = 3,
            INROOM = 4,
            SIDE = 5,
            SCREEN = 10,
            LIGHT = 210,
            TEMP = 211,
            PIR = 212,
            MAX_SEMANTICS = 10
        };

        typedef wiselib::vector_static<OsModel, group_entry_t, MAX_SEMANTICS> group_container_t;
        typedef wiselib::vector_static<OsModel, value_t, MAX_SEMANTICS> value_container_t;
        typedef wiselib::vector_static<OsModel, predicate_t, MAX_SEMANTICS> predicate_container_t;
        // --------------------------------------------------------------------

        Semantics() {

            semantics_vector_.clear();
        };

        ~Semantics() {
        };
        // --------------------------------------------------------------------

        /**
         *
         * Adds or updates the semantic storage contents
         * with an entry for the predicate - value pair
         *
         * @param predicate
         * the predicate to be added or updated
         * @param value
         * the value to be added or updated
         */
        void set_semantic_value(predicate_t predicate, value_t value) {
            int * sema_i = (int*) predicate.data();
            int * value_i = (int*) value.data();

            if (!semantics_vector_.empty()) {
                for (semantics_vector_iterator_t si = semantics_vector_.begin(); si != semantics_vector_.end(); ++si) {
                    if (si->first == *sema_i) {
                        si->second = *value_i;
                        semantic_updated(predicate, value);
                        return;
                    }
                }
            }
            semantics_t newse;
            newse.first = *sema_i;
            newse.second = *value_i;
            semantics_vector_.push_back(newse);


        }
        // --------------------------------------------------------------------

        /**
         *
         * Returns a vector containing all the predicates in the storage
         *
         * @return
         * the vector of all predicates
         */
        predicate_container_t get_predicates() {
            predicate_container_t my_predicate_container;
            my_predicate_container.clear();
            for (semantics_vector_iterator_t si = semantics_vector_.begin(); si != semantics_vector_.end(); ++si) {

                if (si->first > 200) {

                    predicate_t pred = predicate_t((block_data_t *) & si->first);
                    my_predicate_container.push_back(pred);
                }
            }
            return my_predicate_container;
        }
        // --------------------------------------------------------------------

        /**
         *
         * Returns a vector containing all the grouping predicates in the storage
         * as groups (predicate - value pairs)
         * grouping predicates are predicates less than 200
         *
         * @return
         * the vector of all groups
         */
        group_container_t get_groups() {
            group_container_t my_group_container;
            my_group_container.clear();
            for (semantics_vector_iterator_t si = semantics_vector_.begin(); si != semantics_vector_.end(); ++si) {

                if (si->first < 200) {
                    int groupValues[2];
                    groupValues[0] = si->first;
                    groupValues[1] = si->second;
                    group_entry_t ge = group_entry_t((block_data_t *) groupValues, 2 * sizeof (int));
                    my_group_container.push_back(ge);
                }
            }
            return my_group_container;
        }

        /**
         *
         * Checks if the given group_entry exists in the local semantic storage
         *
         * @param gi
         * the group entry to be checked
         * @return
         * true if the storage contains the group
         */
        bool has_group(group_entry_t gi) {
            int value;
            int id;
            memcpy(&id, gi.data(), sizeof (int));
            memcpy(&value, gi.data() + sizeof (int), sizeof (int));

            for (semantics_vector_iterator_t si = semantics_vector_.begin(); si != semantics_vector_.end(); ++si) {
                if ((si->first == id) && (si->second == value)) {

                    return true;
                }
            }
            return false;
        }
        // --------------------------------------------------------------------

        /**
         *
         * Returns a vector containing all the values in the storage
         * for a selected predicate
         *
         * @param predicate
         * the predicate for which the values should be returned
         * @return
         * a vector containing the values of the predicate
         */
        value_container_t get_values(semantic_id_t predicate) {
            value_container_t my_value_container;
            my_value_container.clear();
            int * predicate_i = (int*) predicate.data();
            for (semantics_vector_iterator_t si = semantics_vector_.begin(); si != semantics_vector_.end(); ++si) {
                if (si->first == *predicate_i) {

                    value_t ge = value_t((block_data_t *) & si->second, sizeof (int));
                    my_value_container.push_back(ge);
                }
            }
            return my_value_container;
        }
        // --------------------------------------------------------------------

        /**
         * 
         * Compares 2 value_t objects based on a selected predicate
         * 
         * @param first
         * the first value to check
         * @param second
         * the second value to check
         * @param predicate
         * the result of first > second as first - second
         */
        int cmp(value_t first, value_t second, predicate_t predicate) {
            int * ia = (int *) first.data_a;
            int * ib = (int *) second.data_a;

            return (*ia - *ib);

        }

        /**
         *
         * Aggregate oldvalue and newvalue and store data in oldvalue
         *
         * @param oldvalue
         * the old value to be updated
         * @param newvalue
         * the new value to be added to the old one
         * @param predicate
         * a predicate that can be used to aggregate in a different manner
         */
        void aggregate(value_t *oldvalue, value_t *newvalue, predicate_t predicate) {

#ifdef AGGREGATE_VALUES
            int * predicate_i = (int *) predicate.data();
            int * valuea = (int *) oldvalue->data();
            int * valueb = (int *) newvalue->data();

            int ans;
            switch (*predicate_i) {
                case PIR:
                    ans = (*valuea + *valueb) > 0 ? 1 : 0;
                    break;
                    //case LIGHT:
                    //case TEMP:
                default:
                    ans = (*valuea + *valueb * 2) / 3;
                    break;
            }
            if (*predicate_i < 200) {
                ans = *valuea;
            }
            memcpy(valuea, &ans, sizeof (int));
#else
            memcpy(oldvalue->data(), newvalue->data(), sizeof (int));
#endif
        }

        // --------------------------------------------------------------------

        /**
         *
         * For comptability issues with TuppleStorageAddaptor
         *
         * @return
         * return 1
         */
        int get_allocator() {
            return 1;
        }

        // --------------------------------------------------------------------

        /**
         *
         * @param obj_pnt
         * @return
         */
        template<class T, void (T::*TMethod)(predicate_t, value_t) >
        int reg_semantic_updated_callback(T *obj_pnt) {
            if (callbacks_.empty())
                callbacks_.assign(MAX_RECEIVERS, delegate_t());

            for (unsigned int i = 0; i < callbacks_.size(); ++i) {
                if (callbacks_.at(i) == delegate_t()) {
                    callbacks_.at(i) = delegate_t::template from_method<T, TMethod > (obj_pnt);
                    return i;
                }
            }

            return -1;
        }

        // --------------------------------------------------------------------

        /**
         *
         * @param idx
         * @return
         */
        int unreg_semantic_updated_callback(int idx) {
            callbacks_.at(idx) = delegate_t();
            return SUCCESS;
        }

    private:
        typedef wiselib::pair<int, int> semantics_t;
        typedef wiselib::vector_static<OsModel, semantics_t, MAX_SEMANTICS > semantics_vector_t;
        typedef typename semantics_vector_t::iterator semantics_vector_iterator_t;

        enum ReturnValues {
            SUCCESS = OsModel::SUCCESS
        };
        typedef delegate2<void, predicate_t, value_t> delegate_t;
        typedef vector_static<OsModel, delegate_t, MAX_RECEIVERS> CallbackVector;
        typedef typename CallbackVector::iterator CallbackVectorIterator;

        // --------------------------------------------------------------------

        /**
         *
         * Notifies listeners about the new predicate value pair
         *
         * @param predicate
         * @param value
         */
        void semantic_updated(predicate_t predicate, value_t value) {
            for (CallbackVectorIterator
                it = callbacks_.begin();
                    it != callbacks_.end();
                    ++it) {
                if (*it != delegate_t())
                    (*it)(predicate, value);
            }
        }
        // --------------------------------------------------------------------
        CallbackVector callbacks_;
        semantics_vector_t semantics_vector_;
    };

}
