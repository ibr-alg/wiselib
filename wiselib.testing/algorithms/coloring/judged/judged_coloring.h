
#ifndef __ALGORITHMS_COLORING_JUDGED_COLORING_H__
#define __ALGORITHMS_COLORING_JUDGED_COLORING_H__

#include "algorithms/coloring/judged/judged_coloring_message_types.h"
#include "algorithms/routing/tree/tree_routing.h"
#include "algorithms/routing/tora/tora_routing.h"
#include "internal_interface/routing_table/routing_table_static_array.h"
#include "internal_interface/coloring_table/color_table_map.h"
#include "internal_interface/coloring_table/colors_sorted.h"
#include <string.h>
#include <string>
#define DEBUG_JUDGEDCOLORING

namespace wiselib {
#ifndef DATA_STRUCTS
    #define DATA_STRUCTS
    struct question {
        uint16_t who; // Poion rwtisame
        uint16_t type;
        uint16_t answer; // I apantisi.
        uint16_t answered; // An dothike apantis 0 h 1. Elegxetai alla itan gia tuxon dipla minimata
        uint16_t position; // thesi sto map color_nodes tou teleutaiou node pou rwtisame
        uint16_t con_no; // Arithmos sinexomenwn kombwn pou den allaksan xrwma.
        uint16_t curr_color_pos; // Position ston pinaka color_scores tou teleutaiou xrwmatos pou steilame
    };

    struct ask_color { // Domi poy stelnoume gia pollaplous dikastes
        int color;
        int priority;
    };
#endif
   /**
    * \brief udged Coloring Algorithm
    *
    *  \ingroup coloring_concept
    *  \ingroup basic_algorithm_concept
    *  \ingroup coloring_algorithm
    *
    * A judged coloring algorithm.
    */
    template<typename OsModel_P,
            typename Radio_P = typename OsModel_P::Radio,
            typename Debug_P = typename OsModel_P::Debug>
            class JudgedColoring {
    public:
        typedef OsModel_P OsModel;
        typedef Radio_P Radio;
        typedef Debug_P Debug;

        typedef typename OsModel_P::Timer Timer;

        typedef JudgedColoring<OsModel, Radio, Debug> self_type;
        typedef TreeRouting<OsModel, Timer, Radio, Debug> tree_route;

        typedef wiselib::StaticArrayRoutingTable<OsModel, Radio, 8, wiselib::ToraRoutingTableValue<OsModel, Radio> >
        ToraRoutingTable;
        typedef ToraRouting<OsModel, ToraRoutingTable, Radio, Debug> tora_routing_t;
        typedef ToraRoutingMessage<OsModel_P, Radio> tora_message;
        typedef typename Radio::node_id_t node_id_t;
        typedef typename Radio::size_t size_t;
        typedef typename Radio::block_data_t block_data_t;

        typedef typename Timer::millis_t millis_t;
        //Needed Message Types
        typedef JudgedColoringMessage<OsModel, Radio> coloring_message;
        typedef StlMapColorTable<OsModel, Radio, node_id_t> ColorTable;
        typedef ColorsTable<OsModel, Radio> ColorsSorted;

        ///@name Construction / Destruction
        ///@{
        JudgedColoring();
        ~JudgedColoring();
        ///@}

        ///@name 
        ///@{
        void enable(void);
        void disable(void);
        ///@}

        ///@name Methods called by Timer
        ///@{
        void timer_elapsed(void *userdata);
        ///@}

        ///@name Starts the game for the server
        ///@{
        void in_game(void *userdata);
        ///@}


        ///@name Basiki send den xrisimopoieitai
        ///@{
        void send(node_id_t receiver, size_t len, block_data_t *data);
        ///@}

        ///@name Sinartisi lipsis minimatwn
        ///@{
        void receive(node_id_t from, size_t len, block_data_t *data);
        ///@}

        ///@name Sinartisi lipsis minimatwn
        ///@{
        void judge_receive(coloring_message* message,uint8_t msg_id);
        ///@}

        ///@name Sinartisi lipsis minimatwn
        ///@{
        void node_receive(coloring_message* message,uint8_t msg_id);
        ///@}

        ///@make and send bootstraping message to the network
        ///@{
        void bootstrap_message();
        ///@}

        ///@make the judge known to everybody
        ///@{
        void broadcast_judge(uint8_t);
        ///@}


        //// Duo send mia gia aplo payload kai i deuteri gia structs
        void send_message(uint payload, uint source, uint8_t msg_id, uint destination, uint8_t routing_type);
        void send_special_message(uint8_t* payload, uint source, uint8_t msg_id, uint destination, uint8_t routing_type);

        ///@Get the number of neighboors and who are they if debug
        ///@{
        uint16_t get_neighboors();
        ///@}

        ///@O pinakas me xrwmata kai kombous, eepistrefei to megethos tou kai poioi einai
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

        inline void set_judge() {
            is_judge = 1;
            judge = radio().id();
        };

        inline int get_judge() {
            return judge;
        };

        inline int get_appointed_judges() {
            return j_ass;
        };

        inline int get_alg_messages(){
            return alg_messages;
        }

        inline int get_game_started() {
            return game_started;
        }

        inline int get_color() {
            return color;
        }

        inline int get_ended_all() {
            return ended_all;
        }

        inline int get_ended_centralized() {
            return ended_centralized;
        }

        inline void set_color(int color_) {
            color = color_;
        }

        inline void set_world_size(int nc) {
            node_count = nc;
        }

        inline void set_judge_count(int jc) {
            judge_count = jc;
        }

        inline void set_max_color(int max_color_) {
            max_color = max_color_;
        }

        inline uint16_t get_is_judge() {
            return is_judge;
        };

        inline uint16_t is_terminated() {
            return end;
        };

        inline uint16_t has_min_priority() {
            int has = 1;
            for (int i = 1; i < 20; i++) {
                if (judges[i] == -1)break;
                else if (judges[i] < radio().id())
                    has = 0;
            }
            return has;
        };

        inline uint16_t has_max_priority() {
            int has = 1;
            for (int i = 1; i < 20; i++) {
                if (judges[i] == -1)break;
                else if (judges[i] > radio().id())
                    has = 0;
            }
            return has;
        };

        inline uint16_t active_colors_size() {
            return color_scores.get_active_colors();
        };

        inline uint16_t add_neighboor(uint neighboor_id, uint color_num) {
            neighboors_colors.insert(std::make_pair(neighboor_id, color_num));
        };

        inline uint16_t add_node_color(uint node_id, uint color_num) {
            color_nodes.insert(std::make_pair(node_id, color_num));            
        };

        inline void try_start() {
            if(is_judge && !game_started){
                if((c_ass == node_count - 1)&&(j_ass = judge_count - 1) && (has_max_priority())){
                    
                    coloring_message data;
                    data.set_msg_id(CENT_END);
                    radio().send(radio().BROADCAST_ADDRESS, data.buffer_size(), (uint8_t*) & data);
                     if(is_judge) this->isort(judges,20);
                    in_game(0);
                }
                if(c_ass == node_count -1)
                    ended_centralized = 1;
            }
        };

        inline void change_node_color(uint node_id, uint color_num) {
            std::map<uint, uint>::iterator it = color_nodes.find(node_id);
            it->second = color_num;
        };

        inline void set_tora_routing(tora_routing_t* tora) {
            tora_routing = tora;
        };

        int compare(const void * a, const void * b){
            if(*(int*)a==*(int*)b) return 0;
            else if (*(int*)a < *(int*)b) return 1;
            else return -1;
        }

        inline void init_question() {
            lq.answered = 0;
            lq.position = 1;
            lq.answer = color_scores.get_color_from_position(lq.position);
        };
        
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
        
        int judge_suspended;
        int judges[20];
        question lq;
        tora_routing_t *tora_routing;
        uint16_t permitted_count, answer_count, neigh_colors_count;
        uint8_t is_judge;
        uint8_t asked_color;
        int judge;
        int max_color;
        ColorTable neighboors_colors;
        ColorTable color_nodes;
        std::map<uint, uint>::iterator iter_color_nodes;
        ColorsSorted color_scores;
        int color, color_under_cons, game_started,alg_messages,end,node_count,judge_count,c_ass,j_ass,ended_centralized,ended_all;
        ask_color q_color;
    };
    // -----------------------------------------------------------------------
    // -----------Judged Coloring Constructor---------------------------------
    // -----------------------------------------------------------------------

    template<typename OsModel_P,
    typename Radio_P,
    typename Debug_P>
    JudgedColoring<OsModel_P, Radio_P, Debug_P>::
    JudgedColoring()
    : is_judge(0),
    judge(-1),
    color(-1),
    max_color(2),
    asked_color(0),
    permitted_count(0),
    answer_count(0),
    game_started(0),
    judge_suspended(0),
    neigh_colors_count(0),
    alg_messages(0),
    end(0),
    node_count(0),
    c_ass(0),
    j_ass(0),
    judge_count(0),
    ended_centralized(0),
    ended_all(0) {
    };
    // -----------Judged Coloring De-Constructor------------------------------

    template<typename OsModel_P,
    typename Radio_P,
    typename Debug_P>
    JudgedColoring<OsModel_P, Radio_P, Debug_P>::
    ~JudgedColoring() {

    };
    // -----------------------------------------------------------------------

    template<typename OsModel_P,
    typename Radio_P,
    typename Debug_P>
    void
    JudgedColoring<OsModel_P, Radio_P, Debug_P>::
    enable() {

        for (int i = 0; i < 20; i++) judges[i] = -1;
        radio().enable_radio();
        radio().template reg_recv_callback<self_type, &self_type::receive > (this);

        if (is_judge) {
            color_scores.init();
            timer().template set_timer<self_type, &self_type::timer_elapsed > (
                    501, this, 0);
        } 
        bootstrap_message();
    }
    // -----------------------------------------------------------------------

    template<typename OsModel_P,
    typename Radio_P,
    typename Debug_P>
    void
    JudgedColoring<OsModel_P, Radio_P, Debug_P>::
    disable(void) {
#ifdef DEBUG_JUDGEDCOLORING
        debug().debug("JudgedColoring: Disable\n");
#endif
       
    }
    // -----------------------------------------------------------------------

    template<typename OsModel_P,
    typename Radio_P,
    typename Debug_P>
    void
    JudgedColoring<OsModel_P, Radio_P, Debug_P>::
    timer_elapsed(void *userdata) {
        if (color == -1 && !is_judge && (asked_color < 1)) {
            timer().template set_timer<self_type, &self_type::timer_elapsed > (
                    0, this, 0);
            asked_color++;
            //send_message(radio().id(), radio().id(), REQUEST_COLOR, judge, 1);
        } else if (is_judge && asked_color < 1) {
            asked_color++;
            color_scores.insert(color);
            add_node_color(radio().id(), color);
            iter_color_nodes = color_nodes.begin();
            lq.who = radio().id();
            lq.con_no = 0;
            int i;
            this->isort(judges,20);      
        }
    }
    // -----------------------------------------------------------------------

    template<typename OsModel_P,
    typename Radio_P,
    typename Debug_P>
    void
    JudgedColoring<OsModel_P, Radio_P, Debug_P>::
    in_game(void *userdata) {
        get_neighboors();

        game_started = 1;
        if (lq.con_no < (color_nodes.size())) {
            if (lq.who == radio().id()) { // I prwti fora pou mpainei einai o idios
                cout<< "This is the start..."<< endl;
                if (!lq.who)iter_color_nodes;
                if (iter_color_nodes->first == radio().id()) {
                    if (++iter_color_nodes == color_nodes.end())
                        iter_color_nodes = color_nodes.begin();
                }
                lq.who = iter_color_nodes->first;
                lq.curr_color_pos = color_scores.get_position_from_color(color_nodes.find(lq.who)->second);
                lq.con_no = 1;
                init_question();
                lq.type = REASSIGN_COLOR;
            } else if (lq.answered) {
                if (lq.type == REASSIGN_COLOR) {
                    if (lq.answer == 0) { // An i apantisti einai 0 den peirame xrwma
                        lq.answered = 0;
                        lq.position++;
                        lq.answer = color_scores.get_color_from_position(lq.position);
                        if ((lq.position == color_scores.get_size()) || (color_scores.get_value_from_position(lq.curr_color_pos) > color_scores.get_value_from_position(lq.position))) {
                            lq.con_no++;
                            if (++iter_color_nodes == color_nodes.end())
                                iter_color_nodes = color_nodes.begin();
                            if (iter_color_nodes->first == radio().id()) {
                                if (++iter_color_nodes == color_nodes.end())
                                    iter_color_nodes = color_nodes.begin();
                            }
                            lq.who = iter_color_nodes->first;
                            lq.curr_color_pos = color_scores.get_position_from_color(color_nodes.find(lq.who)->second);
                            init_question();
                        }
                    } else if (lq.answer == color_scores.get_color_from_position(lq.position)) { // An i apantisi einai to xrwma pou stilame
                        lq.con_no = 1;
                        color_scores.insert(lq.answer);
                        color_scores.remove(color_nodes.find(lq.who)->second);
                        change_node_color(lq.who, lq.answer);
                        if ((++iter_color_nodes) == color_nodes.end())
                            iter_color_nodes = color_nodes.begin();
                        if (iter_color_nodes->first == radio().id()) {
                            if (++iter_color_nodes == color_nodes.end())
                                iter_color_nodes = color_nodes.begin();
                        }
                        lq.who = iter_color_nodes->first;
                        lq.curr_color_pos = color_scores.get_position_from_color(color_nodes.find(lq.who)->second);
                        init_question();
                    }
                }
            }

            if (!lq.answered){ // mpikame tuxaia min steileis tipota ( prwti ekdosi i sinartisi etrexe me timer )
                send_message((uint16_t) lq.answer, radio().id(), REASSIGN_COLOR, (uint16_t) lq.who, 1);
                cout<< "I am node = " << radio().id() << " Sending to " << lq.who  <<endl;
            }
        } else if (((lq.con_no) == color_nodes.size())&&(judge_suspended == 0)) { // An exoume con_no == color_nodes.size() thn prwti fora
            send_message(-1,radio().id(),ASK_COLOR,-1,0);                      // Rwta tous geitones an exeis kalo xrwma
            judge_suspended = 1;
        } else if (judge_suspended == 1){ // Molis sou apantisoun elegkse to diko sou kai allakse en anagki
            int i = 0,ok = 1;
           while (( i < color_scores.get_size() ) && (color_scores.get_value_from_color(color) <= color_scores.get_value_from_position(i+1))) {
               int color_under_cons = color_scores.get_color_from_position(i+1);
                for (std::map<uint, uint>::iterator it = neighboors_colors.begin(); it != neighboors_colors.end(); it++) {
                    int neigh_id = (int)it->first;
                    int neigh_color = neighboors_colors.find(neigh_id)->second;
                    if((neigh_color == color_under_cons ) || (color == color_under_cons ))
                        ok = 0;
                }
                if(ok){
                    color_scores.remove(color);
                    color = color_under_cons;
                    color_scores.insert(color_under_cons);
                    color_nodes.find((uint)radio().id())->second = color;
                    lq.con_no = 1; // An allakseis trekse kai pali olo ton algorithmo
                    lq.who = radio().id();
                    judge_suspended = 0;
                    timer().template set_timer<self_type, &self_type::in_game > (
                    10, this, 0);
                    ok=1000;
                }
                if(ok!=1000)
                ok=1;
                else{
                    break;
                }
                i++;
            }
            if (ok != 1000) { // An den allakses tote teleiwse i seira sou steile ston epomeno an uparxei
                cout << "This is the end..." << endl;

                int tmp = 0;
                this->isort(judges,20);    
                for (int i = 0; i < 20; i++) {
                    int a = radio().id();
                    int b = judges[i];
                    if ((judges[i] < radio().id()) && (judges[i] != -1)) {
                        send_special_message((uint8_t*) & color_scores, radio().id(), TAKE_TURN, judges[i], 1);
                        tmp = i;
                        break;
                    }
                   
                }
                 if (tmp==0){
                        end = 1;
                        ended_all = 1;
                        coloring_message data;
                        data.set_msg_id(ALL_END);
                        radio().send(radio().BROADCAST_ADDRESS, data.buffer_size(), (uint8_t*) & data);
                 }
            }
        }

        if(color_nodes.size() == 1){ // An exeis mono ton eauto sou dwse tin skutali
            int tmp = 0;
            this->isort(judges,20); 
            for(int i=0;i<20;i++){
                int a = radio().id();
                int b = judges[i];
                if((judges[i] < radio().id()) && (judges[i]!=-1)){
                    send_special_message((uint8_t*)&color_scores,radio().id(),TAKE_TURN,judges[i],1);
                    tmp = i;
                    break;
                }
            }
           if (tmp==0){
                        end = 1;
                        ended_all = 1;
                        coloring_message data;
                        data.set_msg_id(ALL_END);
                        radio().send(radio().BROADCAST_ADDRESS, data.buffer_size(), (uint8_t*) & data);
           }
        }

    }
    // -----------------------------------------------------------------------

    template<typename OsModel_P,
    typename Radio_P,
    typename Debug_P>
    void
    JudgedColoring<OsModel_P, Radio_P, Debug_P>::
    send(node_id_t destination, size_t len, block_data_t *data) {

    }
    // -----------------------------------------------------------------------

    template<typename OsModel_P,
    typename Radio_P,
    typename Debug_P>
    void
    JudgedColoring<OsModel_P, Radio_P, Debug_P>::
    receive(node_id_t from, size_t len, block_data_t *data) {
        /*if (from == radio().id())
            return;

        uint8_t msg_id = *data;
        uint source, payload, msg_id2;
        coloring_message *message;
        tora_message *t_message;
        message = (coloring_message *) data;
        t_message = (tora_message *) data;

        
        if (((msg_id == 194) || ((msg_id > 2) && (msg_id < 10)))) {
            if (msg_id == 194)
                message = (coloring_message *) t_message->payload();
        }
          
         */

        if (from == radio().id())
            return;

        uint8_t msg_id = *data;
        uint destination, source, payload, msg_id2;
        coloring_message *message;
        //tree_message *t_message;
        tora_message *t_message;
        message = (coloring_message *) data;
        t_message = (tora_message *) data;

        if (((msg_id == 194) || ((msg_id > 2) && (msg_id < 10)))) {
            if (msg_id == 194)
                message = (coloring_message *) t_message->payload();
            destination = message->destination();
            if ((radio().id() == destination)) {
                source = message->source();
                if (message->msg_id() != ASK_PERMISSION) {
                    payload = *((uint*) message->payload());
                } else {
                    payload = *((uint*) (message->payload()));
                }
                msg_id2 = message->msg_id();
            }
        }

        if ((radio().id() == destination) && (0 < msg_id2 < 11) )
            std::cout << " Egw o:" << destination << " elaba minima apo ton: " << source << " me id:" << msg_id2 << "kai payload:" << payload << "\n";

        if (msg_id == JUDGE_START) {
            message = (coloring_message *) data;
            add_neighboor(message->source(), 0);
            broadcast_judge(message->source());
        } else if (msg_id == NODE_START) {
            message = (coloring_message *) data;
            add_neighboor(message->source(), 0);
        } else if (msg_id == BROAD_JUDGE) {
            message = (coloring_message *) data;
            broadcast_judge(*((uint*) message->payload()));
        } else if (msg_id == ASK_COLOR){
            send_message(color,radio().id(),REPLY_COLOR,message->source(),0);
        } else if(msg_id == CENT_END) {
            if(!ended_centralized){
                ended_centralized = 1;
                coloring_message data;
                data.set_msg_id(CENT_END);
                radio().send(radio().BROADCAST_ADDRESS, data.buffer_size(), (uint8_t*) & data);
            }
        } else if (msg_id == ALL_END) {
            if(!ended_all){
                ended_all = 1;
                coloring_message data;
                data.set_msg_id(ALL_END);
                radio().send(radio().BROADCAST_ADDRESS, data.buffer_size(), (uint8_t*) & data);
            }
        }else if ((msg_id == REPLY_COLOR) && (message->destination() == radio().id())){
            payload = *((uint*) message->payload());
            std::map<uint, uint>::iterator it = neighboors_colors.find((int) message->source());
            if (it == neighboors_colors.end())
                neighboors_colors.insert(std::make_pair(message->source(), (uint) payload));
            else it->second = payload;
            int neigh_no = get_neighboors();
            if(neigh_colors_count != neigh_no){
                neigh_colors_count++;
            } 
            if(neigh_colors_count == neigh_no){
                neigh_colors_count = 0;
                if(is_judge) in_game(0);
            }
        }else if (is_judge) {
            if (msg_id == 194) {// If the messages are in tree_routing type get the color_message from payload
                message = (coloring_message *) t_message->payload();
            }
            judge_receive(message,msg_id);

        } else if ((!is_judge) || (judge_suspended==1)) {
            if (msg_id == 194) // If the messages are in tree_routing type get the color_message from payload
                message = (coloring_message *) t_message->payload();
            node_receive(message,msg_id);
        }

    }
    // -----------------------------------------------------------------------

    template<typename OsModel_P,
    typename Radio_P,
    typename Debug_P>
    void
    JudgedColoring<OsModel_P, Radio_P, Debug_P>::
    bootstrap_message() {
        int type;
        if (this->is_judge) {
            type = JUDGE_START;
            judges[0] = radio().id();
        } else if (!this->is_judge) {
            type = NODE_START;
        }
        send_message(-1, radio().id(), type, -1, 0);

    }
    // -----------------------------------------------------------------------

    template<typename OsModel_P,
    typename Radio_P,
    typename Debug_P>
    void
    JudgedColoring<OsModel_P, Radio_P, Debug_P>::
    broadcast_judge(uint8_t judge_id) {
        int got_it_before = 0, i;
        for (i = 0; i < 20; i++) {
            if (judges[i] == judge_id) {
                got_it_before = 1;
                break;
            } else if (judges[i] == -1) {
                judges[i] = judge_id;
                j_ass++;
                if(is_judge) try_start();
                break;
            }
        }

        if (got_it_before == 0) {
            cout << "Egw o " << radio().id() << " acknowledge judge:" << judge_id << endl;
            if (judge == -1) {

                judge = judge_id;
                if (!is_judge) {
                    send_message(radio().id(), radio().id(), REQUEST_COLOR, judge_id, 1);

                }


            } else if (is_judge) {
                send_message(radio().id(), radio().id(), -1, judge_id, 1);
            }

            send_message(judge_id, radio().id(), BROAD_JUDGE, -1, 0);
        }

    }
    // -----------------------------------------------------------------------

    template<typename OsModel_P,
    typename Radio_P,
    typename Debug_P>
    void
    JudgedColoring<OsModel_P, Radio_P, Debug_P>::
    send_message(uint payload, uint source, uint8_t msg_id, uint destination, uint8_t routing_type) {
        coloring_message data;
        data.set_msg_id(msg_id);
        data.set_source(source);
        data.set_destination(destination);
        data.set_payload(sizeof (uint), (uint8_t*) & payload);
        if (routing_type == 0) {
            alg_messages++;
            radio().send(radio().BROADCAST_ADDRESS, data.buffer_size(), (uint8_t*) & data);
        } else if (routing_type == 1) {
            std::cout << " Egw o:" << (int) source << " estila minima ston: " << (int) destination << " me id:" << (int) msg_id << "kai payload:" << (int) *((uint*) data.payload()) << "\n";
            tora_routing->send(destination, data.buffer_size(), (block_data_t*) & data);
        }
    }

    // -----------------------------------------------------------------------

    template<typename OsModel_P,
    typename Radio_P,
    typename Debug_P>
    void
    JudgedColoring<OsModel_P, Radio_P, Debug_P>::
    send_special_message(uint8_t* payload, uint source, uint8_t msg_id, uint destination, uint8_t routing_type) {
        coloring_message data;
        data.set_msg_id(msg_id);
        data.set_source(source);
        data.set_destination(destination);

        if (routing_type == 0) {
            alg_messages++;
            data.set_payload(sizeof (ask_color), (uint8_t*) payload);
            radio().send(radio().BROADCAST_ADDRESS, data.buffer_size(), (uint8_t*) & data);
        } else if (routing_type == 1) {
            std::cout << " Egw o:" << (int) source << " estila minima ston: " << (int) destination << " me id:" << (int) msg_id << "kai payload:" << (int) *((uint*) data.payload()) << "\n";
            data.set_payload((uint8_t) sizeof (ColorsSorted), (uint8_t*) payload);
            tora_routing->send(destination, data.buffer_size(), (block_data_t*) & data);
        }
    }


    // -----------------------------------------------------------------------

    template<typename OsModel_P,
    typename Radio_P,
    typename Debug_P>
    uint16_t
    JudgedColoring<OsModel_P, Radio_P, Debug_P>::
    get_neighboors() {
#ifdef DEBUG_JUDGEDCOLORING
        int i = 0;
        //debug().debug("%d:My neighboors are the following:\n", radio().id());
        for (std::map<uint, uint>::iterator it = neighboors_colors.begin(); it != neighboors_colors.end(); it++) {
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
    JudgedColoring<OsModel_P, Radio_P, Debug_P>::
    get_color_nodes() {
#ifdef DEBUG_JUDGEDCOLORING
        int i = 0;
        //debug().debug("%d:The node-color diagram are the following:\n", radio().id());
        for (std::map<uint, uint>::iterator it = color_nodes.begin(); it != color_nodes.end(); it++) {
            //std::cout << "Node:" << (int) it->first << "--Color:" << (int) it->second << "\n";
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
    void
    JudgedColoring<OsModel_P, Radio_P, Debug_P>::
    node_receive(coloring_message* message,uint8_t msg_id) {
        int payload = 0;
        int tmp = message->msg_id();
        if((message->source() == judge) && (message->destination() == radio().id()))
        cout << "I " << radio().id() << " received message from judge:" << endl;
        if (message->msg_id() != ASK_PERMISSION) {
            payload = *((uint*) message->payload());
        } else {
            q_color = *((ask_color*) message->payload());
        }
        if (message->msg_id() == ASSIGN_COLOR && message->destination() == radio().id() && (payload != color)) {
            color = (int) payload;
            if (msg_id == ASSIGN_COLOR) {
                send_message(color, (uint16_t) radio().id(), COLOR_ASSIGNED, judge, 0);
            } else {
                send_message(color, (uint16_t) radio().id(), COLOR_ASSIGNED, judge, 1);
            }
        } else if (message->msg_id() == REASSIGN_COLOR && message->destination() == radio().id() && (answer_count < 1)) {

            if ((payload != color) && (payload != color_under_cons)) {
                answer_count = 0;
                permitted_count = 0;
                q_color.color = payload;
                q_color.priority = judge;
                send_special_message((uint8_t*) & q_color, radio().id(), ASK_PERMISSION, -1, 0);
                color_under_cons = payload;

            } else {
                send_message(payload, radio().id(), COLOR_NOT_ASSIGNED, judge, 1);
            }
        } else if ((message->msg_id() == ASK_PERMISSION)) {
            if ((q_color.color == color) && (q_color.priority <= judge)) {
                send_message((uint) q_color.color, radio().id(), DECL_PERMISSION, message->source(), 0);

            } else {
                send_message((uint) q_color.color, radio().id(), GIVE_PERMISSION, message->source(), 0);
            }
        } else if (((message->msg_id() == GIVE_PERMISSION) || (message->msg_id() == DECL_PERMISSION)) && message->destination() == radio().id() && color_under_cons != 0) {
            if (payload == color_under_cons) {
                answer_count++;
                if (message->msg_id() == GIVE_PERMISSION) {
                    permitted_count++;
                }
                if (answer_count == neighboors_colors.size()) {
                    if (answer_count == permitted_count) {
                        color = color_under_cons;
                        color_under_cons = 0;
                        if (!is_judge) {
                            send_message(color, radio().id(), COLOR_ASSIGNED, judge, 1);
                        } else {
                            in_game(0);
                        }
                    } else {
                        if (!is_judge) {
                            send_message(color_under_cons, radio().id(), COLOR_NOT_ASSIGNED, judge, 1);
                        } else {
                            lq.answer = color;
                            judge_suspended = 3;
                            in_game(0);
                        }
                        color_under_cons = 0;
                    }
                    answer_count = 0;
                }
            }
        } else if (message->msg_id() == REASSIGN_COLOR && message->destination() == radio().id()) {
        }

    }

    // -----------------------------------------------------------------------

    template<typename OsModel_P,
    typename Radio_P,
    typename Debug_P>
    void
    JudgedColoring<OsModel_P, Radio_P, Debug_P>::
    judge_receive(coloring_message* message,uint8_t msg_id) {
        int payload = 0;
        if ((message->msg_id() == TAKE_TURN) && (message->destination() == radio().id())) {
            ColorsSorted cn_temp = *((ColorsSorted*) message->payload());
            for (int i = 0; i < cn_temp.get_size(); i++) {
                int color_tmp = cn_temp.get_color_from_position(i + 1);
                int count_tmp = cn_temp.get_value_from_color(color_tmp);
                for (int j = 0; j < count_tmp; j++) {
                    color_scores.insert(color_tmp);
                }
            }
        }
        if (message->msg_id() != ASK_PERMISSION) {
            payload = *((uint*) message->payload());
        } else {
            q_color = *((ask_color*) message->payload());
        }
        if ((message->msg_id() == TAKE_TURN) && (message->destination() == radio().id())) {
            in_game(0);
        } else if (((message->msg_id() == COLOR_ASSIGNED) || (message->msg_id() == COLOR_NOT_ASSIGNED)) && (message->source() == lq.who) && (message->destination() == radio().id())) {
            if ((payload > 0) && (message->msg_id() == COLOR_ASSIGNED)) {
                std::map<uint, uint>::iterator it = color_nodes.find((int) message->source());
                if (it == color_nodes.end()) {
                    color_nodes.insert(std::make_pair(message->source(), (uint) payload));
                    color_scores.insert((uint) payload);

                } else {

                    if ((it->second != payload) && (payload == color_scores.get_color_from_position(lq.position))) {
                        lq.answered = 1;
                        lq.answer = (uint) payload;
                        in_game(0);

                    }
                }
            } else if (message->msg_id() == COLOR_NOT_ASSIGNED) {
                if (((uint) message->source() == (int) lq.who) && ((uint) payload == (int) lq.answer)) {
                    lq.answered = 1;
                    lq.answer = 0;
                    in_game(0);
                }
            }
        }else if((message->msg_id() == COLOR_ASSIGNED) && (message->destination() == radio().id())){
            c_ass++;
            try_start();
        } else if ((message->msg_id() == REQUEST_COLOR) && (message->destination() == radio().id())) {
            int routing_type;
            if (msg_id == 194)
                routing_type = 1;
            else routing_type = 0;

            std::map<uint, uint>::iterator it = color_nodes.find((uint) (message->source()));
            if (it == color_nodes.end()) {
                add_node_color(message->source(), max_color);
                color_scores.insert(max_color);
                send_message(max_color, radio().id(), ASSIGN_COLOR, (uint16_t) (message->source()), routing_type);
                max_color++;

            } else {
                send_message((uint) it->second, radio().id(), ASSIGN_COLOR, (uint16_t) (message->source()), routing_type);
            }
        } else if (message->msg_id() == ASK_PERMISSION) {
            uint permission = 1;
            if ((q_color.color == color) && (q_color.priority <= radio().id()))
                permission = 0;
            if (permission) {
                send_message((uint) q_color.color, radio().id(), GIVE_PERMISSION, message->source(), 0);
            } else {
                send_message((uint) q_color.color, radio().id(), DECL_PERMISSION, message->source(), 0);
            }
        }

    }

    // -----------------------------------------------------------------------

    // -----------------------------------------------------------------------

}
#endif


