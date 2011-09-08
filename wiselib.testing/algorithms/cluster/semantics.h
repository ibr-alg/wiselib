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

        typedef int value_t;
        typedef int semantic_id_t;

        typedef typename OsModel::block_data_t block_data_t;

        struct semantics {
            semantic_id_t semantic_id_;
            value_t semantic_value_;
        };

        enum semantics_types {
            BUILDING = 1,
            FLOOR = 2,
            SECTOR = 3,
            ROOM = 4,
            SIDE = 5,
            SCREEN = 10,
            LIGHT = 210,
            TEMP = 211,
            PIR = 212,
            MAX_SEMANTICS = 5
        };




        typedef struct semantics semantics_t;
        typedef wiselib::vector_static<OsModel, semantics_t, 2 * MAX_SEMANTICS > semantics_vector_t;
        typedef typename semantics_vector_t::iterator semantics_vector_iterator_t;

        struct group_entry {
            block_data_t * data_a;

            block_data_t * data() {
                return data_a;
            };

            size_t size_a;

            size_t size() {
                return size_a;
            }
            char buffer[30];

            char * c_str() {

                int bytes_written = 0;
                int mint;
                memcpy((void *)&mint, (void *)data_a, sizeof (int));
                bytes_written += sprintf(buffer + bytes_written, "%d", mint);
                buffer[bytes_written] = '\0';
                return buffer;
            }
        };

        typedef struct group_entry group_entry_t;
        typedef wiselib::vector_static<OsModel, group_entry_t, MAX_SEMANTICS> group_container_t;
        typedef wiselib::vector_static<OsModel, group_entry_t, MAX_SEMANTICS> value_container_t;

        Semantics() {
            semantics_vector_.clear();
        };

        ~Semantics() {
        };

        

        void set_semantic_value(semantic_id_t sema, value_t value) {
            if (!semantics_vector_.empty()) {
                for (semantics_vector_iterator_t si = semantics_vector_.begin(); si != semantics_vector_.end(); ++si) {
                    if (si->semantic_id_ == sema) {
                        si->semantic_value_ = value;
                        return;
                    }
                }
            }
            semantics_t newse;
            newse.semantic_id_ = sema;
            newse.semantic_value_ = value;
            semantics_vector_.push_back(newse);

        }
        
        group_container_t get_groups() {
            group_container_t my_group_container;
            my_group_container.clear();
            for (semantics_vector_iterator_t si = semantics_vector_.begin(); si != semantics_vector_.end(); ++si) {

                if (si->semantic_id_ < 200) {
                    group_entry_t ge;
                    ge.data_a = (block_data_t *) & si->semantic_id_;
                    ge.size_a = 2 * sizeof (semantic_id_t);
                    my_group_container.push_back(ge);
                }
            }
            return my_group_container;

        }

        bool has_group(block_data_t * group, size_t size) {
            semantic_id_t id;
            memcpy(&id, group, sizeof (int));
            value_t value;
            memcpy(&value, group + sizeof (int), sizeof (int));


            for (semantics_vector_iterator_t si = semantics_vector_.begin(); si != semantics_vector_.end(); ++si) {
                if ((si->semantic_id_ == id) && (si->semantic_value_ == value)) {
                    return true;
                }
            }
            return false;
        }

        value_container_t get_values(semantic_id_t predicate) {
            value_container_t my_value_container;
            my_value_container.clear();
            for (semantics_vector_iterator_t si = semantics_vector_.begin(); si != semantics_vector_.end(); ++si) {
                if (si->semantic_id_ == predicate) {
                    group_entry_t ge;
                    ge.data_a = (block_data_t *) & si->semantic_value_;
                    ge.size_a = sizeof (int);
                    my_value_container.push_back(ge);
                }
            }
            return my_value_container;
        }

        int cmp(group_entry_t a, group_entry_t b, semantic_id_t predicate) {
            int ia ;
            int ib ;
            memcpy(&ia,a.data_a,sizeof(int));
            memcpy(&ib,b.data_a,sizeof(int));
            switch (predicate) {
                default:
                    if (ia - ib > 0) {
                        return 1;
                    } else if (ia - ib < 0) {
                        return -1;
                    } else return 0;
            }
        }

        value_t aggregate(value_t a, value_t b, semantic_id_t predicate) {
            switch (predicate) {
                case PIR:
                    return (a + b) > 0?1:0;
                case LIGHT:
                case TEMP:
                    return (a + b)/2;
                default:
                    return a + b;
            }
        }
        
        
        //TODO : register_callback for when semantics change

    private:
        semantics_vector_t semantics_vector_;       
    };

}
