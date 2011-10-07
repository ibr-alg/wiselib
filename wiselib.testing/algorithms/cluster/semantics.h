/* 
 * File:   semantics.h
 * Author: amaxilat
 *
 */


#include "util/pstl/vector_static.h"
//#include "util/tuple_store/tuple_store.h"

namespace wiselib {

    /**
     * Semantic cluster head decision module
     */
    template<typename OsModel_P>
    class Semantics {
    public:

        typedef OsModel_P OsModel;
        typedef typename OsModel::block_data_t block_data_t;

        class data_t {
        public:

            data_t() {
                size_a = 0;
            };

            data_t(block_data_t * dat) {
                data_a = (block_data_t*) dat;
                size_a = sizeof (int);
            };

            data_t(block_data_t * dat, size_t size) {
                data_a = (block_data_t*) dat;
                size_a = size;
            };

            data_t(block_data_t * dat, size_t size, int allocator) {
                data_a = (block_data_t*) dat;
                size_a = size;
            };

            block_data_t * data() {
                return data_a;
            };

            size_t size() {
                return size_a;
            }

            //            void set_data(int* data_p) {
            //                size_a = sizeof (int);
            //                data_a = (block_data_t*) data_p;
            //            }

            char * c_str() {
                if (size_a == sizeof (int)) {
                    int bytes_written = 0;
                    int mint;
                    memcpy((void *) &mint, (void *) data_a, sizeof (int));
                    bytes_written += sprintf(buffer + bytes_written, "%d", mint);
                    buffer[bytes_written] = '\0';
                } else if (size_a == sizeof (int) *2) {
                    int bytes_written = 0;
                    int mint;
                    memcpy((void *) &mint, (void *) data_a, sizeof (int));
                    bytes_written += sprintf(buffer + bytes_written, "%d", mint);
                    memcpy((void *) &mint, (void *) (data_a + sizeof (int)), sizeof (int));
                    bytes_written += sprintf(buffer + bytes_written, "|%d", mint);
                    buffer[bytes_written] = '\0';
                }
                return buffer;
            }

            bool operator==(data_t other) const {
                int *a = (int *) data_a;
                int *b = (int *) other.data();
                return *a == *b;
            }
            block_data_t * data_a;
            size_t size_a;
            char buffer[5];

        };


        typedef data_t value_t;
        typedef data_t semantic_id_t;
        typedef semantic_id_t predicate_t;

        //        struct semantics {
        //            int first;
        //            int second;
        //        };

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

        typedef wiselib::pair<int, int> semantics_t;
        typedef wiselib::vector_static<OsModel, semantics_t, MAX_SEMANTICS > semantics_vector_t;
        typedef typename semantics_vector_t::iterator semantics_vector_iterator_t;

        typedef data_t group_entry_t;

        //        typedef struct group_entry group_entry_t;
        typedef wiselib::vector_static<OsModel, group_entry_t, MAX_SEMANTICS> group_container_t;
        typedef wiselib::vector_static<OsModel, value_t, MAX_SEMANTICS> value_container_t;
        typedef wiselib::vector_static<OsModel, predicate_t, MAX_SEMANTICS> predicate_container_t;

        Semantics() {
            semantics_vector_.clear();
        };

        ~Semantics() {
        };

        void set_semantic_value(predicate_t sema, value_t value) {
            int * sema_i = (int*) sema.data();
            int * value_i = (int*) value.data();

            if (!semantics_vector_.empty()) {
                for (semantics_vector_iterator_t si = semantics_vector_.begin(); si != semantics_vector_.end(); ++si) {
                    if (si->first == *sema_i) {
                        si->second = *value_i;
                        return;
                    }
                }
            }
            semantics_t newse;
            newse.first = *sema_i;
            newse.second = *value_i;
            semantics_vector_.push_back(newse);

        }

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

        group_container_t get_groups() {
            group_container_t my_group_container;
            my_group_container.clear();
            for (semantics_vector_iterator_t si = semantics_vector_.begin(); si != semantics_vector_.end(); ++si) {

                if (si->first < 200) {
                    group_entry_t ge;
                    ge.data_a = (block_data_t *) & si->first;
                    ge.size_a = 2 * sizeof (int);
                    my_group_container.push_back(ge);
                }
            }
            return my_group_container;
        }

        bool has_group(group_entry_t gi) {
            int value;
            int id;
            memcpy(&id,gi.data(),sizeof(int));
            memcpy(&value,gi.data()+sizeof(int),sizeof(int));
            
            for (semantics_vector_iterator_t si = semantics_vector_.begin(); si != semantics_vector_.end(); ++si) {
                if ((si->first == id) && (si->second == value)) {
                    return true;
                }
            }
            return false;
        }

        value_container_t get_values(semantic_id_t predicate) {
            value_container_t my_value_container;
            my_value_container.clear();
            int * predicate_i = (int*) predicate.data();
            for (semantics_vector_iterator_t si = semantics_vector_.begin(); si != semantics_vector_.end(); ++si) {
                if (si->first == *predicate_i) {
                    group_entry_t ge;
                    ge.data_a = (block_data_t *) & si->second;
                    ge.size_a = sizeof (int);
                    my_value_container.push_back(ge);
                }
            }
            return my_value_container;
        }

        int cmp(value_t a, value_t b, predicate_t predicate) {
            int * ia = (int *) a.data_a;
            int * ib = (int *) b.data_a;

            return (*ia - *ib);

        }

        /**
         * Add value a and value b and store data in value a
         */
        void aggregate(value_t a, value_t b, predicate_t predicate) {
            int * predicate_i = (int *) predicate.data();
            int * valuea = (int *) a.data();
            int * valueb = (int *) b.data();

            int ans;
            switch (*predicate_i) {
                case PIR:
                    ans = (*valuea + *valueb) > 0 ? 1 : 0;
                    break;
                case LIGHT:
                case TEMP:
                    ans = (*valuea + *valueb) / 2;
                    break;
                default:
                    ans = *valuea + *valueb;
                    break;
            }
            if (*predicate_i < 200) {
                ans = *valuea;
            }
            memcpy(a.data(), &ans, sizeof (int));
        }

        int get_allocator() {
            return 1;
        }

        //TODO : register_callback for when semantics change

    private:
        semantics_vector_t semantics_vector_;
    };

}
