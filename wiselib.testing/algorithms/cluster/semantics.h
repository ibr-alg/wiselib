/* 
 * File:   semantics.h
 * Author: amaxilat
 *
 */


#include "util/pstl/vector_static.h"

namespace wiselib {

    /**
     * Semantic cluster head decision module
     */
    template<typename OsModel_P, typename Radio_P>
    class Semantics {
    public:

        typedef OsModel_P OsModel;
        typedef Radio_P Radio;
        typedef typename Radio::node_id_t node_id_t;

        struct semantics {
            node_id_t node_id_;
            int semantic_id_;
            int semantic_value_;
            bool enabled_;
            int tot_value_; //only for cheads            
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
            PIR = 212
        };



        typedef struct semantics semantics_t;
        typedef wiselib::vector_static<OsModel, semantics_t, 10 > semantics_vector_t;
        typedef typename semantics_vector_t::iterator semantics_vector_iterator_t;

        Semantics() {
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
                        si->enabled_ = false;
                        return;
                    }
                }
            }
            semantics_t newse;
            newse.semantic_id_ = sema;
            newse.node_id_ = radio_->id();
            newse.semantic_value_ = value;
            newse.enabled_ = false;
            semantics_vector_.push_back(newse);

        }

        size_t enabled_semantics() {
            size_t count = 0;
            if (!semantics_vector_.empty()) {
                for (semantics_vector_iterator_t si = semantics_vector_.begin(); si != semantics_vector_.end(); ++si) {
                    if (si->enabled_ == true) {
                        count++;
                    }
                }
            }
            return count;
        }

        bool check_condition(int semantic) {
            //            debug_->debug("checking semantic %d", semantic);
            for (semantics_vector_iterator_t si = semantics_vector_.begin(); si != semantics_vector_.end(); ++si) {
                if (si->semantic_id_ == semantic) {
                    si->enabled_ = true;
                    return true;
                }
            }
            if (semantic > 200) return true;
            return false;
        }

        bool check_condition(int semantic, int value) {
            if (semantic > 200) return check_condition(semantic);
            //            debug_->debug("checking semantic value %d|%d", semantic, value);
            for (semantics_vector_iterator_t si = semantics_vector_.begin(); si != semantics_vector_.end(); ++si) {
                if ((si->semantic_id_ == semantic) && (si->semantic_value_ == value)) {
                    si->enabled_ = true;
                    return true;
                }
            }
            return false;
        }

        int semantic_value(int semantic) {
            for (semantics_vector_iterator_t si = semantics_vector_.begin(); si != semantics_vector_.end(); ++si) {
                if (si->semantic_id_ == semantic) {
                    return si->semantic_value_;
                }
            }
            return -1;
        }

        semantics_vector_t semantics_vector_;

    private:
        Radio* radio_;


    };

}
