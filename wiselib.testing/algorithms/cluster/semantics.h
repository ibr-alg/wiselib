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
    template<typename OsModel_P, typename Radio_P>
    class Semantics {
    public:

        typedef OsModel_P OsModel;
        typedef Radio_P Radio;


        typedef int value_t;
        typedef int semantic_id_t;
        typedef typename Radio::node_id_t node_id_t;
        typedef typename Radio::block_data_t block_data_t;

        struct semantics {
            node_id_t node_id_;
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
        };

        typedef struct group_entry group_entry_t;
        typedef wiselib::vector_static<OsModel, group_entry_t, MAX_SEMANTICS> group_container_t;



        typedef wiselib::vector_static<OsModel, value_t, MAX_SEMANTICS> value_container_t;

        Semantics() {
            semantics_vector_.clear();
        };

        ~Semantics() {
        };

        void init(Radio& radio) {
            radio_ = &radio;
            semantics_vector_.clear();
        }

        void set_semantic_value(int sema, int value) {
            if (!semantics_vector_.empty()) {
                for (semantics_vector_iterator_t si = semantics_vector_.begin(); si != semantics_vector_.end(); ++si) {
                    if (si->semantic_id_ == sema) {
                        si->semantic_value_ = value;
                        return;
                    }
                }
            }
        }

        void set_semantic(int sema, int value) {
            if (!semantics_vector_.empty()) {
                for (semantics_vector_iterator_t si = semantics_vector_.begin(); si != semantics_vector_.end(); ++si) {
                    if (si->semantic_id_ == sema) {
                        si->node_id_ = radio_->id();
                        si->semantic_value_ = value;
                        return;
                    }
                }
            }
            semantics_t newse;
            newse.semantic_id_ = sema;
            newse.node_id_ = radio_->id();
            newse.semantic_value_ = value;
            semantics_vector_.push_back(newse);

        }

        //        bool check_condition(int semantic) {
        //            //            debug_->debug("checking semantic %d", semantic);
        //            for (semantics_vector_iterator_t si = semantics_vector_.begin(); si != semantics_vector_.end(); ++si) {
        //                if (si->semantic_id_ == semantic) {
        //                    si->enabled_ = true;
        //                    return true;
        //                }
        //            }
        //            if (semantic > 200) return true;
        //            return false;
        //        }
        //
        //        bool check_condition(int semantic, int value) {
        //            if (semantic > 200) return check_condition(semantic);
        //            //            debug_->debug("checking semantic value %d|%d", semantic, value);
        //            for (semantics_vector_iterator_t si = semantics_vector_.begin(); si != semantics_vector_.end(); ++si) {
        //                if ((si->semantic_id_ == semantic) && (si->semantic_value_ == value)) {
        //                    si->enabled_ = true;
        //                    return true;
        //                }
        //            }
        //            return false;
        //        }

        //        int semantic_value(int semantic) {
        //            for (semantics_vector_iterator_t si = semantics_vector_.begin(); si != semantics_vector_.end(); ++si) {
        //                if (si->semantic_id_ == semantic) {
        //                    return si->semantic_value_;
        //                }
        //            }
        //            return -1;
        //        }

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

        void get_predicates() {

        }

        value_container_t get_values(semantic_id_t predicate) {
            value_container_t my_value_container;
            my_value_container.clear();
            for (semantics_vector_iterator_t si = semantics_vector_.begin(); si != semantics_vector_.end(); ++si) {
                //if (si->semantic_id_ == predicate) {
                my_value_container.push_back(si->semantic_value_);
                //}
            }
            return my_value_container;
        }

        int cmp(value_t a, value_t b, semantic_id_t predicate) {

            switch (predicate) {
                default:
                    if (a - b > 0) {
                        return 1;
                    } else if (a - b < 0) {
                        return -1;
                    } else return 0;
            }
        }

        value_t aggregate(value_t a, value_t b, semantic_id_t predicate) {
            switch (predicate) {
                case PIR:
                    return (a + b) > 0;
                case LIGHT:
                case TEMP:
                    return a + b;
                default:
                    return a + b;
            }
        }


        semantics_vector_t semantics_vector_;

    private:
        Radio* radio_;


    };

}
