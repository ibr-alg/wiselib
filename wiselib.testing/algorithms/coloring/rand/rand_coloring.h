#ifndef __ALGORITHMS_COLORING_RAND_H__
#define __ALGORITHMS_COLORING_RAND_H__

#include "algorithms/coloring/two_hops/two_hops_message_types.h"
#include "algorithms/routing/tora/tora_routing.h"
#include "internal_interface/routing_table/routing_table_static_array.h"
#include "algorithms/routing/tora/tora_routing_message.h"
// #include "internal_interface/coloring_table/color_table_map.h"
// #include "internal_interface/coloring_table/colors_sorted.h"
#include <string.h>
#include <string>
#include <stdlib.h>
#include <ctime>
#include <math.h>


#define DEBUG_RANDCOLORING

namespace wiselib {

#define DATA_STRUCTS

    struct rrep_c_change {
        int color_assigned;
        int color_removed;
        int hops;
        int node_id;
        int change_round;
    };

    struct rrep_fb {
        int color_foridden;
        int fb_round;
    };

    struct rrep_sf {
        int satisfaction;
        int node_id;
        int sf_round;
    };
   /**
    * \brief Random Coloring Algorithm
    *
    *  \ingroup coloring_concept
    *  \ingroup basic_algorithm_concept
    *  \ingroup coloring_algorithm
    *
    * A random coloring algorithm.
    */
    template<typename OsModel_P,
            typename Radio_P = typename OsModel_P::Radio,
            typename Debug_P = typename OsModel_P::Debug>
            class RandColoring {
    public:
        typedef OsModel_P OsModel;
        typedef Radio_P Radio;
        typedef Debug_P Debug;
        typedef typename OsModel_P::Timer Timer;
        typedef RandColoring<OsModel, Radio, Debug> self_type;
        typedef wiselib::StaticArrayRoutingTable<OsModel, Radio, 8, wiselib::ToraRoutingTableValue<OsModel, Radio> >
        ToraRoutingTable;
        typedef ToraRouting<OsModel, ToraRoutingTable, Radio, Debug> tora_routing_t;
        typedef ToraRoutingMessage<OsModel_P, Radio> tora_message;
        typedef typename Radio::node_id_t node_id_t;
        typedef typename Radio::size_t size_t;
        typedef typename Radio::block_data_t block_data_t;
        typedef typename Timer::millis_t millis_t;
        //Needed Message Types
        typedef TwoHopsMessage<OsModel, Radio> coloring_message;
        typedef StlMapColorTable<OsModel, Radio, node_id_t> ColorTable;
        typedef StlMapColorTable<OsModel, Radio, node_id_t> ForbiddenTable;
        typedef ColorsTable<OsModel, Radio> ColorsSorted;

        ///@name Construction / Destruction
        ///@{
        RandColoring();
        ~RandColoring();
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

        ///@name Methods called by Timer
        ///@{
        void resend_requests(void *userdata);
        ///@}

        ///@name Methods called by RadioModel
        ///@{
        void receive(node_id_t from, size_t len, block_data_t *data);
        ///@}

        ///@name Method that checks satisfaction with color
        ///@{
        int satisfied(int i);
        ///@}

        ///@name Method that makes a greed move
        ///@{
        void greed_move(int i);
        ///@}

        ///@name Method that tries a greed move
        ///@{
        void try_greed();
        ///@}

        ///@name Method that tries a fb move
        ///@{
        void try_fb();
        ///@}

        ///@name Method that makes a greed move
        ///@{
        void try_change();
        ///@}

        ///@make and send bootstraping message to the network
        ///@{
        void bootstrap_message();
        ///@}

        ///@make and send bootstraping message to the network
        ///@{
        void send_color_requests();
        ///@}

        void send_special_message(uint8_t* payload, uint source, uint8_t msg_id, uint destination, uint8_t routing_type, uint hops, uint16_t msg_id_num);

        ///@Get the number of neighboors and who are they if debug
        ///@{
        uint16_t get_neighboors();
        ///@}

        ///@Get the number of neighboors and who are they if debug
        ///@{
        uint16_t get_color_nodes();
        ///@}

        int fm(int arr[], int b, int n) {
            int f = b;
            int c;

            for (c = b + 1; c < n; c++)
                if (arr[c] > arr[f])
                    f = c;

            return f;
        }

        void isort(int arr[], int n) {
            int s, w;
            int sm;

            for (s = 0; s < n - 1; s++) {
                w = fm(arr, s, n);
                sm = arr[w];
                arr[w] = arr[s];
                arr[s] = sm;
            }
        }

        inline int get_alg_messages(){
            return alg_messages;
        }

        inline int get_color() {
            return color;
        }

        inline void set_color(int color_) {
            color = color_;
        }

        inline int get_network_size(){
            return net_size;
        }

        inline void set_network_size(int net_size_){
            net_size = net_size_;
        }

        inline uint16_t active_colors_size() {
            return color_numbers.get_active_colors();
        };

        inline uint16_t add_neighboor(uint neighboor_id, uint color_num) {
            neighboors_colors.insert(std::make_pair(neighboor_id, color_num));
        };

        inline bool is_in_messages(uint node_id, uint payload) {
            std::map<uint, uint>::iterator it = messages.find(node_id);
            if (it == messages.end()) {
                messages.insert(std::make_pair(node_id, payload));
                return false;
            } else {
                if (it->second == payload) {
                    return true;
                } else {
                    int msgRC = node_id * 100 + 21;
                    int msgRS = node_id * 100 + 22;
                    int msgRF = node_id * 100 + 23;
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
            //cout << "--------------------------" << endl;
            cout << "Eimai o :" << radio().id() << " me rec_count:" << rec_colors_count << " kai nodes_num:" << node_selection.size() << " kai fb_round:" << fb_round << " kai fb_ans:" << answerfb_count << " kai neigh:" << neighboors_colors.size()<<endl;
            if(neighboors_colors.size() != forbidden_colors.size()){
                for(std::map<uint,uint>::iterator it=neighboors_colors.begin();it != neighboors_colors.end();it++){
                    int exist = 0;
                    for(std::map<uint,uint>::iterator it21=forbidden_colors.begin();it21 != forbidden_colors.end();it21++){
                        if(it->first == it21->second) exist = 1;
                    }
                    if(!exist) cout<< radio().id() << ": Exw na dilwsw oti mou leipei tou:" << it->first << endl;
                }
            }
            //cout << "Eimai o :" << radio().id() << " me greed_round:" << greed_round << " kai rs_answer_count" << answerrs_count << endl;
            //cout << "Eimai o :" << radio().id() << " me change_round:" << change_round << " kai change_answer_count" << answerrc_count << endl;
        }
        inline uint16_t add_node_color(uint node_id, uint color_num) {
            color_nodes.insert(std::make_pair(node_id, color_num));
        };

        inline uint16_t add_forbidden_color(int color,int neigh) {
            forbidden_colors.insert(std::make_pair(color, neigh));
        };

        inline void change_node_color(uint node_id, uint color_num) {
            std::map<uint, uint>::iterator it = color_nodes.find(node_id);
            it->second = color_num;
        };

        inline void set_tora_routing(tora_routing_t* tora) {
            tora_routing = tora;
        };

        inline void set_diameter(int diam){
            diameter = diam;
        }

        inline void do_links(){
            for(int i=0;i<net_size;i++){
                int u = 0;
                send_special_message((uint8_t*)&u,radio().id(),0,i,1,0,0);
            }
        }

        int compare(const void * a, const void * b) {
            if (*(int*) a == *(int*) b) return 0;
            else if (*(int*) a < *(int*) b) return 1;
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

        tora_routing_t *tora_routing;
        uint16_t permission, answerfb_count, answerrs_count, answerrc_count, neigh_colors_count;
        uint8_t asked_color;
        ColorTable node_selection,color_nodes, neighboors_colors, messages;
        std::map<uint, uint>::iterator iter_color_nodes;
        ColorsSorted color_numbers;
        ForbiddenTable forbidden_colors;
        int color, m_color, temp_color, step, ncount, ncount_all, color_under_cons,rec_colors_count;
        int greed_round, change_round, fb_round, round_satisfaction,diameter,idle_rounds;
        int net_size,alg_messages;
    };
    // -----------------------------------------------------------------------
    // -----------Rand Coloring Constructor---------------------------------
    // -----------------------------------------------------------------------

    template<typename OsModel_P,
    typename Radio_P,
    typename Debug_P>
    RandColoring<OsModel_P, Radio_P, Debug_P>::
    RandColoring()
    : step(0),
    ncount(0),
    permission(1),
    temp_color(-1),
    ncount_all(0),
    color_under_cons(0),
    answerfb_count(0),
    answerrs_count(0),
    answerrc_count(0),
    fb_round(0),
    greed_round(1),
    change_round(2),
    idle_rounds(0),
    rec_colors_count(0),
    alg_messages(0){
    };
    // -----------Rand Coloring De-Constructor------------------------------

    template<typename OsModel_P,
    typename Radio_P,
    typename Debug_P>
    RandColoring<OsModel_P, Radio_P, Debug_P>::
    ~RandColoring() {

    };
    // -----------------------------------------------------------------------

    template<typename OsModel_P,
    typename Radio_P,
    typename Debug_P>
    void
    RandColoring<OsModel_P, Radio_P, Debug_P>::
    enable() {
        radio().enable_radio();
        radio().template reg_recv_callback<self_type, &self_type::receive > (this);
        timer().template set_timer<self_type, &self_type::timer_elapsed > (
                500, this, 0);
        color = (int) (radio().id()) + 1;
        color_numbers.init();
        color_numbers.insert(color);
        bootstrap_message();
        do_links();


    }
    // -----------------------------------------------------------------------

    template<typename OsModel_P,
    typename Radio_P,
    typename Debug_P>
    void
    RandColoring<OsModel_P, Radio_P, Debug_P>::
    disable(void) {
#ifdef DEBUG_RANDCOLORING
        debug().debug("RandColoring: Disable\n");
#endif
  //      cout << "Eimai o" << radio().id() << ". To teliko mou xrwma einai:" << get_color << endl;
    }
    // -----------------------------------------------------------------------

    template<typename OsModel_P,
    typename Radio_P,
    typename Debug_P>
    void
    RandColoring<OsModel_P, Radio_P, Debug_P>::
    timer_elapsed(void *userdata) {
        step = 1;
        ncount_all = get_neighboors();
        forbidden_colors.clear();
        temp_color = 0;
        permission = 1;
        color_under_cons = 0;
        m_color = 0;
        
        idle_rounds++;
        node_selection.clear();
        color_numbers.empty();
        send_color_requests();
        
    }

    // -----------------------------------------------------------------------

    template<typename OsModel_P,
    typename Radio_P,
    typename Debug_P>
    void
    RandColoring<OsModel_P, Radio_P, Debug_P>::
    receive(node_id_t from, size_t len, block_data_t *data) {
        if (from == radio().id())
            return;
        uint8_t msg_id = *data;
        coloring_message *message;
        tora_message *t_message;
        message = (coloring_message *) data;
        t_message = (tora_message *) data;

        if (msg_id == 194) {// If the messages are in tree_routing type get the color_message from payload
            message = (coloring_message *) t_message->payload();
            int non_tora_id = message->msg_id();
            int destination = message->destination();
            if (destination == radio().id()) {
                if (non_tora_id == REQ_COLOR) {
                    if (message->source() == 2) cout << "Egw o:" << radio().id() << "pira request apo ton 2" << endl;
                    send_special_message((uint8_t*) & color, radio().id(), REP_COLOR, message->source(), 1, 0, 0);
                } else if (non_tora_id == REP_COLOR) {
                    if ((node_selection.find(message->source()))->second == -1) {
                        uint color_rec = (uint) *((uint8_t*) message->payload());
                        if (radio().id() == 2) cout << "Egw o 2 pira apantisi apo ton:" << message->source() << endl;
                        node_selection[message->source()] = color_rec;
                        color_numbers.insert(color_rec);
                        rec_colors_count++;
                        if (rec_colors_count == node_selection.size()) {
                                cout << radio().id() << ":Ela re paidaki mou." << endl;
                            if (idle_rounds < (800 * diameter + 1)) {
                                rrep_fb fb;
                                fb.color_foridden = color;
                                fb.fb_round = fb_round;
                                send_special_message((uint8_t*) & fb, radio().id(), REP_FBID, -1, 0, 1, 0);
                            } else {
                                cout << "Baresa liksi: " << radio().id();
                            }
                            try_fb();
                            step = 1;
                        }
                    }
                }
            }
        } else {
            uint hops = message->hops();
            uint8_t* payload;
            uint pay_tmp = *((uint*) message->payload());
            payload = (uint8_t*) & pay_tmp;
            rrep_c_change rc = *((rrep_c_change*) message->payload());
            if (!is_in_messages(message->source(), message->id_num())) {

                if (msg_id == NODE_STARTED) {
                    message = (coloring_message *) data;
                    uint load = (uint) *((uint8_t*) message->payload());
                    std::map<uint, uint>::iterator it = neighboors_colors.find(load);
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

                    rrep_c_change rc = *((rrep_c_change*) message->payload());
                    if ((rc.node_id != radio().id()) && (hops == 1)) {
                        if (rc.change_round == change_round) {
                            answerrc_count++;
                            if ((rc.color_assigned != 0) && (rc.node_id != radio().id())) {
                                color_numbers.insert(rc.color_assigned);
                                color_numbers.remove(rc.color_removed);
                                idle_rounds = 0;
                            }
                            if ((answerrc_count == ncount_all) && (fb_round > 14))
                                ; //cout << "edw" << endl;
                            if (answerrc_count > (ncount_all)) {
                                int me = radio().id();
                                cout << "I am:" << me << " and it is Problem RECHANGE BIG TIME" << endl;
                            }
                            try_change();
                        } else {
                            cout << "Egw o::" << radio().id() << " PIRA AKURO CHANGE MINIMA ROUND" << rc.change_round << " cr:" << change_round << endl;
                        }
                    }
                } else if (msg_id == REP_SAT) {
                    if (hops == 1) {


                        rrep_sf sf = *((rrep_sf*) message->payload());
                        if (sf.sf_round == greed_round) {
                            answerrs_count++;
                            if (sf.node_id > radio().id()) {
                                if (sf.satisfaction == 0) {
                                    permission = 0;
                                }
                            }
                            if (answerrs_count > (ncount)) {
                                int me = radio().id();
                                cout << "I am:" << me << " and it is Problem REPSAT BIG TIME" << endl;
                            }
                            if (radio().id() == 1)
                                try_greed();
                            else
                                try_greed();
                        } else {
                            cout << "AKURO GREED MINIMA" << endl;
                        }
                    }
                } else if (msg_id == REP_FBID) {
                    if (hops == 1) {
                        rrep_fb fb = *((rrep_fb*) message->payload());
                        if (fb.fb_round == fb_round) {
                            answerfb_count++;

                            std::map<uint, uint>::iterator it = forbidden_colors.find(fb.color_foridden);
                            if (it == forbidden_colors.end()) {
                                add_forbidden_color(fb.color_foridden,message->source());
                            }
                            if (answerfb_count > (ncount)) {
                                int me = radio().id();
                                cout << "I am:" << me << " and it is Problem REPFBID BIG TIME" << endl;
                            }
                            try_fb();
                        } else {
                            cout << "Egw o::" << radio().id() << " PIRA AKURO FB MINIMA" << fb.fb_round << " cr:" << fb_round << endl;
                        }
                    }
                }
            } else {
                //cout<< "Egw o:"<< radio().id() << "to exw ksanaparei to minima me id:" <<(int)msg_id << " apo ton:" << (int)message->source() << "me id_num:" << message->id_num() <<  endl;
            }
        }

    }
    // -----------------------------------------------------------------------

    template<typename OsModel_P,
    typename Radio_P,
    typename Debug_P>
    void
    RandColoring<OsModel_P, Radio_P, Debug_P>::
    bootstrap_message() {
#ifdef DEBUG_RANDCOLORING
        //debug().debug("Node number %d sends bootstrap message\n", radio().id());
#endif
        int type;
        uint8_t payload = radio().id();
        type = NODE_STARTED;
        send_special_message(&payload, radio().id(), type, -1, 0, 1, 0);

    }

    // -----------------------------------------------------------------------

    template<typename OsModel_P,
    typename Radio_P,
    typename Debug_P>
    void
    RandColoring<OsModel_P, Radio_P, Debug_P>::
    send_special_message(uint8_t* payload, uint source, uint8_t msg_id, uint destination, uint8_t routing_type, uint hops, uint16_t msg_id_num) {
        coloring_message data;
        data.set_msg_id(msg_id);
        data.set_source(source);
        data.set_destination(destination);
        data.set_hops(hops);
        int id_nn = (int) source * 100 + msg_id;
        data.set_id_num(id_nn);
        if (routing_type == 0) {
            alg_messages++;
            uint8_t len;
            if (msg_id == REP_CHANGE) {
                len = sizeof (rrep_c_change);
                if (hops == 2) {
                    if(radio().id() == 41)
                    {
                    //rrep_c_change rc = *((rrep_c_change*) payload);
                    //cout<< "Estila to:" << rc.color_removed << " apo ton:" << rc.node_id << " me id:"<< data.id_num() << endl;
                    }
                }
            } else {
                len = sizeof (rrep_sf);
            }
            data.set_payload(len, (uint8_t*) payload);
            //std::cout << "------Egw o:" << (int) source << " estila minima ston: " << (int) destination << " me id:" << (int) msg_id << " sto hop:" << hops << "mesw toy:"<< radio().id() << "kai payload:" << (int) (*((uint8_t*) data.payload())) << "\n";
            radio().send(radio().BROADCAST_ADDRESS, data.buffer_size(), (uint8_t*) & data);
        } else if (routing_type == 1) {
            data.set_payload(sizeof(uint),payload);
            tora_routing->send(destination, data.buffer_size(), (block_data_t*) & data);
        }
    }


    // -----------------------------------------------------------------------

    template<typename OsModel_P,
    typename Radio_P,
    typename Debug_P>
    uint16_t
    RandColoring<OsModel_P, Radio_P, Debug_P>::
    get_neighboors() {
#ifdef DEBUG_RANDCOLORING
       // int i = 0;
        //debug().debug("%d:My neighboors are the following:\n", radio().id());
        //for (std::map<uint, uint>::iterator it = neighboors_colors.begin(); it != neighboors_colors.end(); it++) {
         //   std::cout << (int) it->first << "-";

        //    i = i + 1;
       // }

       // std::cout << "END!!!\n";
#endif
        return (uint16_t) neighboors_colors.size();
    }



    // -------------------------------
    // -----------------------------------------------------------------------

    template<typename OsModel_P,
    typename Radio_P,
    typename Debug_P>
    uint16_t
    RandColoring<OsModel_P, Radio_P, Debug_P>::
    get_color_nodes() {
#ifdef DEBUG_RANDDCOLORING
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
    int
    RandColoring<OsModel_P, Radio_P, Debug_P>::
    satisfied(int i) {
        int greed_satisfied = 1;
        for (int x = 1; x < color_numbers.get_size(); x++) {
            color_under_cons = color_numbers.get_color_from_position(x);
            if ((color_numbers.get_value_from_position(x) >= color_numbers.get_value_from_color(color)) && (color_under_cons != color)) {
                std::map<uint, uint>::iterator it = forbidden_colors.find(color_under_cons);
                if (it == forbidden_colors.end()) {
                    greed_satisfied = 0;
                    cout << "Egw o gamatos tupos::" << radio().id() << " mporw na paw apo xrwma::" << color << "se" << color_under_cons << " gia to round" << fb_round << endl;
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
    RandColoring<OsModel_P, Radio_P, Debug_P>::
    greed_move(int i) {
        cout << "Egw o gamatos tupos::" << radio().id() << " allaksa xrwma apo::" << color << "se" << i << " gia to round" << fb_round << endl;
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
    RandColoring<OsModel_P, Radio_P, Debug_P>::
    try_fb() {
        if (rec_colors_count == node_selection.size()) {
            if ((answerfb_count == ncount)) {
                step++;
                answerfb_count = 0;
                int sat = satisfied(radio().id());
                rrep_sf sf;
                sf.satisfaction = sat;
                sf.sf_round = greed_round;
                sf.node_id = radio().id();
                rec_colors_count = 0;
                send_special_message((uint8_t*) & sf, radio().id(), REP_SAT, -1, 0, 1, 0);
                fb_round++;
                try_greed();
            }
        }
    }

    // -----------------------------------------------------------------------

    template<typename OsModel_P,
    typename Radio_P,
    typename Debug_P>
    void
    RandColoring<OsModel_P, Radio_P, Debug_P>::
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
                rrep_c_change rc;
                rc.color_assigned = m_color;
                rc.color_removed = temp_color;
                rc.hops = 1;
                rc.node_id = (int) radio().id();
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
    RandColoring<OsModel_P, Radio_P, Debug_P>::
    try_change() {
        if (greed_round == change_round) {
            if ((answerrc_count == (ncount_all))) {
                answerrc_count = 0;
                change_round++;
                step = 0;
                cout << "Egw o:" << radio().id() << "-----Ksekinaw ton round------------" << change_round << endl;
                //if (change_round <= 38)
                    timer().template set_timer<self_type, &self_type::timer_elapsed > (
                        200, this, 0);
                //timer_elapsed(0);

            }

        }
    }

    // -----------------------------------------------------------------------

    template<typename OsModel_P,
    typename Radio_P,
    typename Debug_P>
    void
    RandColoring<OsModel_P, Radio_P, Debug_P>::
    send_color_requests() {
        srand((unsigned) time(0));
        int random_integer;
        int lowest = 0, highest = net_size-1;
        int range = (highest - lowest);
        int n_of_nodes = (int)log2(net_size);
        if(n_of_nodes==0) n_of_nodes = 1;
        int index = 0;
        int n_of_nodes_to_ask;
        if((net_size-neighboors_colors.size()) >= n_of_nodes)
            n_of_nodes_to_ask = n_of_nodes;
        else
            n_of_nodes_to_ask = net_size-neighboors_colors.size();
        while(index < n_of_nodes_to_ask)
        {
            random_integer = rand()%net_size;
            if((random_integer>49)||(random_integer<0)) cout << "OOOOXIIIIIIIIIII" << endl;
            if(neighboors_colors.find(random_integer) == neighboors_colors.end()){
                if(node_selection.find(random_integer) == node_selection.end()){
                    if(radio().id() != random_integer){
                    node_selection.insert(std::make_pair(random_integer,-1));
                        index++;
                        int payload = -1;
                        if(radio().id()==2) cout << "Egw o 2 estila request ston:" << random_integer << endl;
                        send_special_message((uint8_t*)&payload,radio().id(),REQ_COLOR,random_integer,1,0,0);
                    }
                }
            }
        }
        timer().template set_timer<self_type, &self_type::resend_requests > (
                500, this, 0);

    }

    // -----------------------------------------------------------------------

    template<typename OsModel_P,
    typename Radio_P,
    typename Debug_P>
    void
    RandColoring<OsModel_P, Radio_P, Debug_P>::
    resend_requests(void *userdata) {
        int existed = 0;
        for(std::map<uint,uint>::iterator it=node_selection.begin(); it!=node_selection.end();it++)
        {
            if(it->second == -1){
                existed = 1;
                int payload = -1;
                //if(radio().id()==2)
                    cout << radio().id() << " :Egw ksana-estila request ston:" << it->first << endl;
                    send_special_message((uint8_t*)&payload,radio().id(),REQ_COLOR,it->first,1,0,0);
            }
            if(existed){
               // timer().template set_timer<self_type, &self_type::resend_requests > (
               // 5000, this, 0);
                ;
            }
        }

    }
    // -----------------------------------------------------------------------

    // -----------------------------------------------------------------------

}
#endif


