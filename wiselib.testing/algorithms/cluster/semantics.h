

namespace wiselib {

    struct semantics {
        wiselib::OSMODEL::Radio::node_id_t node_id_;
        int semantic_id_;
        int semantic_value_;
        //        int semantic_hops_;
        //bool cluster_head_;
        bool enabled_;
    };
    typedef struct semantics semantics_t;

    enum semantics_types {
        BUILDING = 1,
        FLOOR = 2,
        SECTOR = 3,
        ROOM = 4,
        SCREEN = 10

    };

}
