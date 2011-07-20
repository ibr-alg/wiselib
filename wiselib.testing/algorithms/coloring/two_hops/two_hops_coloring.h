#ifndef __ALGORITHMS_COLORING_TWO_HOPS_H__
#define __ALGORITHMS_COLORING_TWO_HOPS_H__

#include "algorithms/coloring/two_hops/two_hops_message_types.h"
//#include "algorithms/routing/tora/tora_routing.h"
#include "internal_interface/routing_table/routing_table_static_array.h"
//#include "internal_interface/coloring_table/color_table_map.h"
#include "internal_interface/coloring_table/colors_sorted.h"
#include "util/pstl/map_static_vector.h"
#include "util/pstl/pair.h"

#include <string.h>
//#include <string>
#define DEBUG_TWOHOPSCOLORING

namespace wiselib {

#define DATA_STRUCTS

    struct rep_c_change {
        uint32_t color_assigned;
        uint32_t color_removed;
        uint32_t hops;
        uint32_t node_id;
        uint32_t change_round;
    };

    struct rep_fb {
        uint32_t color_foridden;
        uint32_t fb_round;
    };

    struct rep_sf {
        uint32_t satisfaction;
        uint32_t node_id;
        uint32_t sf_round;
    };
   /**
    * \brief Two Hops Coloring Algorithm
    *
    *  \ingroup coloring_concept
    *  \ingroup basic_algorithm_concept
    *  \ingroup coloring_algorithm
    *
    * A two hops coloring algorithm.
    */
    template<typename OsModel_P,
            typename Radio_P = typename OsModel_P::Radio,
            typename Debug_P = typename OsModel_P::Debug>
            class TwoHopsColoring {
    public:
        typedef OsModel_P OsModel;
        typedef Radio_P Radio;
        typedef Debug_P Debug;
        typedef typename OsModel_P::Timer Timer;
        typedef TwoHopsColoring<OsModel, Radio, Debug> self_type;
//        typedef wiselib::StaticArrayRoutingTable<OsModel, Radio, 8, wiselib::ToraRoutingTableValue<OsModel, Radio> >
//        ToraRoutingTable;
     //   typedef ToraRouting<OsModel, ToraRoutingTable, Radio, Debug> tora_routing_t;
//        typedef ToraRoutingMessage<OsModel_P, Radio> tora_message;
        typedef typename Radio::node_id_t node_id_t;
        typedef typename Radio::size_t size_t;
        typedef typename Radio::block_data_t block_data_t;
        typedef typename Timer::millis_t millis_t;
        //Needed Message Types
        typedef TwoHopsMessage<OsModel, Radio> coloring_message;

//        typedef StlMapColorTable<OsModel, Radio, node_id_t> ColorTable;
        typedef MapStaticVector<OsModel , uint32_t, uint32_t, 50> ColorTable;
//        typedef StlMapColorTable<OsModel, Radio, node_id_t> ForbiddenTable;
        typedef MapStaticVector<OsModel , uint32_t, uint32_t, 100> ForbiddenTable;
        typedef ColorsTable<OsModel, Radio> ColorsSorted;

        typedef wiselib::pair<uint32_t, uint32_t> pair_t;

        typedef typename  wiselib::MapStaticVector<OsModel , uint32_t, uint32_t, 50>::iterator ColorTable_iterator;
        typedef typename  wiselib::MapStaticVector<OsModel , uint32_t, uint32_t, 100>::iterator ForbiddenTable_iterator;

        ///@name Construction / Destruction
        ///@{
        TwoHopsColoring();
        ~TwoHopsColoring();
        ///@}

        ///@name Routing Control
        ///@{
        void enable(void);
        void disable(void);
        ///@}

        ///@name Methods called by Timer
        ///@{
        void timer_elapsed(void *userdata);
        ///@}

        ///@name Methods called by RadioModel
        ///@{
        void receive(node_id_t from, size_t len, block_data_t *data);
        ///@}

        ///@name Method that checks satisfaction with color
        ///@{
        uint32_t satisfied(uint32_t i);
        ///@}

        ///@name Method that makes a greed move
        ///@{
        void greed_move(uint32_t i);
        ///@}

        ///@name Method that makes a greed move
        ///@{
        void try_greed();
        ///@}

        ///@name Method that makes a greed move
        ///@{
        void try_change();
        ///@}

        ///@make and send bootstraping message to the network
        ///@{
        void bootstrap_message();
        ///@}

        void send_special_message(uint8_t* payload, node_id_t source, uint8_t msg_id, node_id_t destination, uint8_t routing_type, uint hops, uint16_t msg_id_num);

        ///@Get the number of neighboors and who are they if debug
        ///@{
        uint16_t get_neighboors();
        ///@}

        ///@Get the number of neighboors and who are they if debug
        ///@{
        uint16_t get_color_nodes();
        ///@}

        uint32_t fm(uint32_t arr[], uint32_t b, uint32_t n) {
            uint32_t f = b;
            uint32_t c;

            for (c = b + 1; c < n; c++)
                if (arr[c] > arr[f])
                    f = c;

            return f;
        }

        void isort(uint32_t arr[], uint32_t n) {
            uint32_t s, w;
            uint32_t sm;

            for (s = 0; s < n - 1; s++) {
                w = fm(arr, s, n);
                sm = arr[w];
                arr[w] = arr[s];
                arr[s] = sm;
            }
        }

        inline uint32_t get_alg_messages(){
            return alg_messages;
        }

        inline uint32_t get_color() {
            return color;
        }

        inline void set_color(uint32_t color_) {
            color = color_;
        }

        inline uint16_t active_colors_size() {
            return color_numbers.get_active_colors();
        };

        inline void add_neighboor(uint neighboor_id, uint color_num) {
//            neighboors_colors.insert(  std::make_pair(neighboor_id, color_num));
            pair_t pp(neighboor_id,color_num);
            
            neighboors_colors.insert(  pp );
        };

        inline bool is_in_messages(uint node_id, uint payload) {
            ColorTable_iterator it = messages.find(node_id);
            
            if (it == messages.end()) {
                messages.insert( pair_t(node_id,payload) );
                return false;
            } else {
                if (it->second == payload) {
                    return true;
                } else {
                    uint32_t msgRC = node_id * 100 + 21;
                    uint32_t msgRS = node_id * 100 + 22;
                    uint32_t msgRF = node_id * 100 + 23;
                    if(((payload != msgRS) && (it->second == msgRF )) ||
                            ((payload != msgRC) && (it->second == msgRS )) ||
                            ((payload != msgRF) && (it->second == msgRC ))){
                        return true;
                    }else{
                        it->second = payload;
                        return false;
                    }
                }
            }
        };

        inline void print_metrics(){
            /*
             * Not Wiselib compatible: I/O streams
            cout << "--------------------------" << endl;
            cout << "Eimai o :" << radio().id() << " me fb_round:" << fb_round << " kai fb_answer_count" << answerfb_count << endl;
            cout << "Eimai o :" << radio().id() << " me greed_round:" << greed_round << " kai rs_answer_count" << answerrs_count << endl;
            cout << "Eimai o :" << radio().id() << " me change_round:" << change_round << " kai change_answer_count" << answerrc_count << endl;
             * */
            debug().debug("Node %i: fb_round=%d fb_answer_count=%d\n",radio().id(),fb_round,answerfb_count);
            debug().debug("Node %i: greed_round=%d rs_answer_count=%d\n",radio().id(),greed_round,answerrs_count);
            debug().debug("Node %i: change_round=%d change_answer_count=%d\n",radio().id(),change_round,answerrc_count);

        }

        inline void get_metrics(uint8_t *buf,uint8_t pos){
            memcpy(buf + pos, &fb_round, 4);
            memcpy(buf + pos + 4, &answerfb_count, 2);
            memcpy(buf + pos + 6, &greed_round, 4);
            memcpy(buf + pos + 10, &answerrs_count, 2);
            memcpy(buf + pos + 12, &change_round, 4);
            memcpy(buf + pos + 16, &answerrc_count, 2);
        }
        
        inline void add_node_color(uint node_id, uint color_num) {
            color_nodes.insert(pair_t(node_id, color_num));
        };

        inline uint16_t add_forbidden_color(uint32_t color) {
            forbidden_colors.insert(pair_t(color, 0));
            return 0;
        };

        inline void change_node_color(uint node_id, uint color_num) {
            ColorTable_iterator it = color_nodes.find(node_id);
            it->second = color_num;
        };

   //     inline void set_tora_routing(tora_routing_t* tora) {
    //        tora_routing = tora;
    //    };

        inline void set_diameter(uint32_t diam){
            diameter = diam;
        }

        uint32_t compare(const void * a, const void * b) {
            if (*(uint32_t*) a == *(uint32_t*) b) return 0;
            else if (*(uint32_t*) a < *(uint32_t*) b) return 1;
            else return -1;
        }
        
        void init( Radio& radio, Timer& timer, Debug& debug ) {
          radio_ = &radio;
          timer_ = &timer;
          debug_ = &debug;
        }
        
        void destruct() {
        }

    private:
      
        Radio& radio()
        { return *radio_; }
        
        Timer& timer()
        { return *timer_; }
        
        Debug& debug()
        { return *debug_; }
      
        Radio * radio_;
        Timer * timer_;
        Debug * debug_;
        
   //     tora_routing_t *tora_routing;
        uint32_t step, ncount, color, m_color, temp_color;
        uint16_t permission, neigh_colors_count;
        uint32_t ncount_all, color_under_cons;
        uint16_t answerfb_count,answerrs_count,answerrc_count;
        uint8_t asked_color;
        ColorTable color_nodes, neighboors_colors, messages;
//        std::map<uint, uint>::iterator iter_color_nodes;
        ColorsSorted color_numbers;
        ForbiddenTable forbidden_colors;
        uint32_t fb_round, greed_round, change_round, round_satisfaction,diameter,idle_rounds,alg_messages;
    };
    // -----------------------------------------------------------------------
    // -----------TwoHops Coloring Constructor---------------------------------
    // -----------------------------------------------------------------------

    template<typename OsModel_P,
    typename Radio_P,
    typename Debug_P>
    TwoHopsColoring<OsModel_P, Radio_P, Debug_P>::
    TwoHopsColoring()
    : step(0),
    ncount(0),
    temp_color(-1),
    permission(1),
    ncount_all(0),
    color_under_cons(0),
    answerfb_count(0),
    answerrs_count(0),
    answerrc_count(0),
    fb_round(0),
    greed_round(1),
    change_round(2),
    idle_rounds(0),
    alg_messages(0){
    };
    // -----------TwoHops Coloring De-Constructor------------------------------

    template<typename OsModel_P,
    typename Radio_P,
    typename Debug_P>
    TwoHopsColoring<OsModel_P, Radio_P, Debug_P>::
    ~TwoHopsColoring() {

    };
    // -----------------------------------------------------------------------

    template<typename OsModel_P,
    typename Radio_P,
    typename Debug_P>
    void
    TwoHopsColoring<OsModel_P, Radio_P, Debug_P>::
    enable() {
        radio().enable_radio();
        radio().template reg_recv_callback<self_type, &self_type::receive > (this);
        timer().template set_timer<self_type, &self_type::timer_elapsed > (
                500, this, 0);
        color = (uint32_t) (radio().id()) + 1;
        color_numbers.init();
        color_numbers.insert(color);
        bootstrap_message();


    }
    // -----------------------------------------------------------------------

    template<typename OsModel_P,
    typename Radio_P,
    typename Debug_P>
    void
    TwoHopsColoring<OsModel_P, Radio_P, Debug_P>::
    disable(void) {
#ifdef DEBUG_TWOHOPSCOLORING
        debug().debug("TwoHopsColoring: Disable\n");
#endif
//        cout << "Eimai o" << radio().id() << ". To teliko mou xrwma einai:" << get_color << endl;
    }
    // -----------------------------------------------------------------------

    template<typename OsModel_P,
    typename Radio_P,
    typename Debug_P>
    void
    TwoHopsColoring<OsModel_P, Radio_P, Debug_P>::
    timer_elapsed(void *userdata) {
        step = 1;
        ncount_all = get_neighboors();
        forbidden_colors.clear();
        temp_color = 0;
        permission = 1;
        color_under_cons = 0;
        m_color = 0;
        rep_fb fb;
        fb.color_foridden = color;
        fb.fb_round = fb_round;
        idle_rounds++;
        if(idle_rounds < (100*diameter+1)){
            send_special_message((uint8_t*) & fb, radio().id(), REP_FBID, -1, 0, 1, 0);
        }else{
            /*
             * Not Wiselib compatible: I/O streams
            cout<< "Baresa liksi: " << radio().id();
             * */
            debug().debug("Node %i: timer elapsed\n",radio().id());
        }
        step = 1;
    }

    // -----------------------------------------------------------------------

    template<typename OsModel_P,
    typename Radio_P,
    typename Debug_P>
    void
    TwoHopsColoring<OsModel_P, Radio_P, Debug_P>::
    receive(node_id_t from, size_t len, block_data_t *data) {
        if (from == radio().id())
            return;
        uint8_t msg_id = *data;
        coloring_message *message;
        message = (coloring_message *) data;
        uint hops = message->hops();
        uint8_t* payload;
        uint pay_tmp = *((uint*) message->payload());
        payload = (uint8_t*) & pay_tmp;
        if ((hops == 1)&&(diameter!=1)) {
            if (msg_id == REP_CHANGE) {
                    rep_c_change rc = *((rep_c_change*) message->payload());
                    send_special_message((uint8_t*) & rc, message->source(), msg_id, -1, 0, 2, 0);
                
            } else {
                send_special_message(payload, message->source(), msg_id, -1, 0, 2, 0);
            }
        }     
        rep_c_change rc = *((rep_c_change*) message->payload());
        if (!is_in_messages(message->source(), message->id_num())) {

            if (msg_id == NODE_STARTED) {
                message = (coloring_message *) data;
                uint load = (uint) *((uint8_t*) message->payload());
                
                ColorTable_iterator it = neighboors_colors.find(load);
                if (it == neighboors_colors.end()) {
                    if (load != radio().id()) {
                        add_neighboor(load, load + 1);
                        color_numbers.insert(load + 1);
                    }
                }
                if (hops == 1) {
                    ncount++;
                }
            } else if (msg_id == REP_CHANGE) {

                rep_c_change rc = *((rep_c_change*) message->payload());
                if (rc.node_id != radio().id()) {
                    if (rc.change_round == change_round) {
                        answerrc_count++;
                        if ((rc.color_assigned != 0) && (rc.node_id != radio().id())) {
                            color_numbers.insert(rc.color_assigned);
                            color_numbers.remove(rc.color_removed);
                            idle_rounds = 0;
                        }
                        if ((answerrc_count == ncount_all) && (fb_round > 14))
                            ;//cout << "edw" << endl;
                        if (answerrc_count > (ncount_all)) {
                            /*
                             * Not Wiselib compatible: I/O streams && 'int' date type
                            int me = radio().id();
                            cout << "I am:" << me << " and it is Problem RECHANGE BIG TIME" << endl;
                             */
                            debug().debug("Node %i: This should not have occurred (?REP_CHANGE?)\n",radio().id());
                        }
                        try_change();
                    }else{
                            /*
                             * Not Wiselib compatible: I/O streams
                             cout << "Egw o::" << radio().id() << " PIRA AKURO CHANGE MINIMA ROUND" <<  rc.change_round << " cr:" << change_round << endl;
                            */
                        debug().debug("Node %i: Received invalid REP message, dropping loudly\n",radio().id());
                    }
                }
            } else if (msg_id == REP_SAT) {
                if (hops == 1) {


                    rep_sf sf = *((rep_sf*) message->payload());
                    if (sf.sf_round == greed_round) {
                        answerrs_count++;
                        if (sf.node_id > radio().id()) {
                            if (sf.satisfaction == 0) {
                                permission = 0;
                            }
                        }
                        if (answerrs_count > (ncount)) {
                            /*
                             * Not Wiselib compatible: I/O streams
                            int me = radio().id();
                            cout << "I am:" << me << " and it is Problem REPSAT BIG TIME" << endl;
                             */
                            debug().debug("Node %i: This should not have occured (?REPSAT?)\n",radio().id());
                        }
                        if (radio().id() == 1)
                            try_greed();
                        else
                            try_greed();
                    } else {
                            /*
                             * Not Wiselib compatible: I/O streams
                            cout << "Egw o::" << radio().id() << " PIRA AKURO GREED MINIMA ROUND" <<  rc.change_round << " cr:" << change_round << endl;
                            */
                        debug().debug("Node %i: Received invalid GREED message, dropping loudly\n",radio().id());
                    }
                }
            } else if (msg_id == REP_FBID) {
                if (hops == 1) {
                    rep_fb fb = *((rep_fb*) message->payload());
                    if (fb.fb_round == fb_round) {
                        answerfb_count++;

                        ForbiddenTable_iterator it = forbidden_colors.find(fb.color_foridden);
                        if (it == forbidden_colors.end()) {
                            add_forbidden_color(fb.color_foridden);
                        }
                        if (answerfb_count > (ncount)) {
                            /*
                             * Not Wiselib compatible: I/O streams
                            int me = radio().id();
                            cout << "I am:" << me << " and it is Problem REPFBID BIG TIME" << endl;
                             */
                            debug().debug("Node %i: This should not have occured (?REPFBID?)\n",radio().id());
                        }
                        if ((answerfb_count == ncount)) {
                            step++;
                            answerfb_count = 0;
                            uint32_t sat = satisfied(radio().id());
                            rep_sf sf;
                            sf.satisfaction = sat;
                            sf.sf_round = greed_round;
                            sf.node_id = radio().id();
                            send_special_message((uint8_t*) & sf, radio().id(), REP_SAT, -1, 0, 1, 0);
                            fb_round++;
                            try_greed();
                        }
                    } else {
                        /*
                         * Not Wiselib compatible: I/O streams
                        cout << "Egw o::" << radio().id() << " PIRA AKURO FB MINIMA" <<  fb.fb_round << " cr:" << fb_round << endl;
                        */
                        debug().debug("Node %i: Received invalid FB message, dropping loudly\n",radio().id());
                    }
                }
            }
        } else {
            //cout<< "Egw o:"<< radio().id() << "to exw ksanaparei to minima me id:" <<(int)msg_id << " apo ton:" << (int)message->source() << "me id_num:" << message->id_num() <<  endl;
        }

    }
    // -----------------------------------------------------------------------

    template<typename OsModel_P,
    typename Radio_P,
    typename Debug_P>
    void
    TwoHopsColoring<OsModel_P, Radio_P, Debug_P>::
    bootstrap_message() {
#ifdef DEBUG_TWOHOPSCOLORING
        //debug().debug("Node number %d sends bootstrap message\n", radio().id());
#endif
        uint32_t type;
        uint8_t payload = radio().id();
        type = NODE_STARTED;
        send_special_message(&payload, radio().id(), type, -1, 0, 1, 0);

    }

    // -----------------------------------------------------------------------

    template<typename OsModel_P,
    typename Radio_P,
    typename Debug_P>
    void
    TwoHopsColoring<OsModel_P, Radio_P, Debug_P>::
    send_special_message(uint8_t* payload, node_id_t source, uint8_t msg_id, node_id_t destination, uint8_t routing_type, uint hops, uint16_t msg_id_num) {
        alg_messages++;
        coloring_message data;
        data.set_msg_id(msg_id);
        data.set_source(source);
        data.set_destination(destination);
        data.set_hops(hops);
        uint32_t id_nn = (uint32_t) source * 100 + msg_id;
        data.set_id_num(id_nn);
        if (routing_type == 0) {
            uint8_t len;
            if (msg_id == REP_CHANGE) {
                len = sizeof (rep_c_change);
                if (hops == 2) {
                    if(radio().id() == 41)
                    {
                    //rep_c_change rc = *((rep_c_change*) payload);
                    //cout<< "Estila to:" << rc.color_removed << " apo ton:" << rc.node_id << " me id:"<< data.id_num() << endl;
                    }
                }
            } else {
                len = sizeof (rep_sf);
            }
            data.set_payload(len, (uint8_t*) payload);
            //std::cout << "------Egw o:" << (int) source << " estila minima ston: " << (int) destination << " me id:" << (int) msg_id << " sto hop:" << hops << "mesw toy:"<< radio().id() << "kai payload:" << (int) (*((uint8_t*) data.payload())) << "\n";
            radio().send(radio().BROADCAST_ADDRESS, data.buffer_size(), (uint8_t*) & data);
        } else if (routing_type == 1) {
            data.set_payload((uint8_t)sizeof (ColorsSorted), (uint8_t*) payload);
         //   tora_routing->send(destination, data.buffer_size(), (block_data_t*) & data);
        }
    }


    // -----------------------------------------------------------------------

    template<typename OsModel_P,
    typename Radio_P,
    typename Debug_P>
    uint16_t
    TwoHopsColoring<OsModel_P, Radio_P, Debug_P>::
    get_neighboors() {
#ifdef DEBUG_TWOHOPSCOLORING
        uint32_t i = 0;
        //debug().debug("%d:My neighboors are the following:\n", radio().id());
        for (ColorTable_iterator it = neighboors_colors.begin(); it != neighboors_colors.end(); it++) {
            //std::cout << (int) it->first << "-";

            i = i + 1;
        }

        //std::cout << "END!!!\n";
#endif
        return (uint16_t) neighboors_colors.size();
    }



    // -------------------------------
    // -----------------------------------------------------------------------

    template<typename OsModel_P,
    typename Radio_P,
    typename Debug_P>
    uint16_t
    TwoHopsColoring<OsModel_P, Radio_P, Debug_P>::
    get_color_nodes() {
#ifdef DEBUG_TWOHOPSDCOLORING
        int i = 0;
        debug().debug("%d:The node-color diagram are the following:\n", radio().id());
        for (std::map<uint, uint>::iterator it = color_nodes.begin(); it != color_nodes.end(); it++) {
            std::cout << "Node:" << (int) it->first << "--Color:" << (int) it->second << "\n";
            i = i + 1;
        }
        //std::cout << '\n';
#endif
        return (uint16_t) color_nodes.size();
    }
    // -----------------------------------------------------------------------

    template<typename OsModel_P,
    typename Radio_P,
    typename Debug_P>
    uint32_t
    TwoHopsColoring<OsModel_P, Radio_P, Debug_P>::
    satisfied(uint32_t i) {
        uint32_t greed_satisfied = 1;
        for (uint32_t x = 1; x < color_numbers.get_size(); x++) {
            color_under_cons = color_numbers.get_color_from_position(x);
            if ((color_numbers.get_value_from_position(x) >= color_numbers.get_value_from_color(color)) && (color_under_cons != color)) {
                ForbiddenTable_iterator it = forbidden_colors.find(color_under_cons);
                if (it == forbidden_colors.end()) {
                    greed_satisfied = 0;
                    /*
                     * Not Wiselib compatible: I/O streams
                    cout << "Egw o gamatos tupos::" << radio().id() << " mporw na paw apo xrwma::" << color << "se" << color_under_cons << " gia to round" << fb_round << endl;
                    */
                    debug().debug("Node %i: I can change color %d for %d in round %d\n",radio().id(),color,color_under_cons,fb_round);
                    break;
                }
            } else {
                if (x == (color_numbers.get_size() - 1))
                    //cout << "Egw o gamatos tupos::" << radio().id() << "Eimai komple gia to round" << fb_round << endl;
                color_under_cons = -1;
            }
        }
        return greed_satisfied;
    }

    // -----------------------------------------------------------------------

    template<typename OsModel_P,
    typename Radio_P,
    typename Debug_P>
    void
    TwoHopsColoring<OsModel_P, Radio_P, Debug_P>::
    greed_move(uint32_t i) {
        /*
         * Not Wiselib compatible: I/O streams
        cout << "Egw o gamatos tupos::" << radio().id() << " mporw na paw apo xrwma::" << color << "se" << color_under_cons << " gia to round" << fb_round << endl;
        */
        debug().debug("Node %i: Changed color %d for %d in round %d\n",radio().id(),color,i,fb_round);
        
        color_numbers.remove(color);
        color = i;
        color_numbers.insert(color);
        idle_rounds = 0;
    }

    // -----------------------------------------------------------------------

    template<typename OsModel_P,
    typename Radio_P,
    typename Debug_P>
    void
    TwoHopsColoring<OsModel_P, Radio_P, Debug_P>::
    try_greed() {
        if (fb_round == greed_round) {
            if ((answerrs_count == ncount)) {
                step++;
                answerrs_count = 0;
                if ((permission) && (!satisfied(0))) {
                    temp_color = color;
                    greed_move(color_under_cons);
                    m_color = color;
                }
                rep_c_change rc;
                rc.color_assigned = m_color;
                rc.color_removed = temp_color;
                rc.hops = 1;
                rc.node_id = (uint32_t) radio().id();
                rc.change_round = change_round;
                send_special_message((uint8_t*) & rc, radio().id(), REP_CHANGE, -1, 0, 1, 0);
                greed_round++;
                try_change();
            }
        }
    }

    // -----------------------------------------------------------------------

    template<typename OsModel_P,
    typename Radio_P,
    typename Debug_P>
    void
    TwoHopsColoring<OsModel_P, Radio_P, Debug_P>::
    try_change() {
        if (greed_round == change_round) {
            if ((answerrc_count == (ncount_all))) {
                answerrc_count = 0;
                change_round++;
                step = 0;
                    timer().template set_timer<self_type, &self_type::timer_elapsed > (
                        200, this, 0);
                //timer_elapsed(0);
                
            }

        }
    }
//     // -----------------------------------------------------------------------

    // -----------------------------------------------------------------------

}
#endif


