
#ifndef __ALGORITHMS_COLORING_PKJUDGED_COLORING_H__
#define __ALGORITHMS_COLORING_PKJUDGED_COLORING_H__

#include "algorithms/coloring/judged/judged_coloring.h"
#include "algorithms/coloring/judged/judged_coloring_message_types.h"
#include "algorithms/routing/tora/tora_routing.h"
#include "algorithms/routing/tree/tree_routing.h"
#include "internal_interface/routing_table/routing_table_static_array.h"
#include "internal_interface/coloring_table/color_table_map.h"
#include "internal_interface/coloring_table/colors_sorted.h"
#include <string.h>
#include <string>

#define DEBUG_PKJUDGEDCOLORING

namespace wiselib {
#ifndef DATA_STRUCTS
#define DATA_STRUCTS

    struct question {
        uint16_t who;
        uint16_t type;
        uint16_t answer;
        uint16_t answered;
        uint16_t position;
        uint16_t con_no;
        uint16_t curr_color_pos;
    };

    struct ask_color {
        int color;
        int priority;
    };
#endif
   /**
    * \brief PK Judged Coloring Algorithm
    *
    *  \ingroup coloring_concept
    *  \ingroup basic_algorithm_concept
    *  \ingroup coloring_algorithm
    *
    * A PK judged coloring algorithm.
    */
    template<typename OsModel_P,
            typename Radio_P = typename OsModel_P::Radio,
            typename Debug_P = typename OsModel_P::Debug>
            class PKJudgedColoring {
    public:
        typedef OsModel_P OsModel;
        typedef Radio_P Radio;
        typedef Debug_P Debug;

        typedef typename OsModel_P::Timer Timer;

        typedef PKJudgedColoring<OsModel, Radio, Debug> self_type;
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
        typedef TreeRoutingMessage<OsModel, Radio> tree_message;
        typedef TreeBroadcastMessage<OsModel, Radio> tree_broad_message;
        typedef JudgedColoringMessage<OsModel, Radio> coloring_message;
        typedef StlMapColorTable<OsModel, Radio, node_id_t> ColorTable;
        typedef ColorsTable<OsModel, Radio> ColorsSorted;

        ///@name Construction / Destruction
        ///@{
        PKJudgedColoring();
        ~PKJudgedColoring();
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

        ///@name Starts the game for the server
        ///@{
        void in_game(void *userdata);
        ///@}



        ///@name Routing Functionality
        ///@{
        void send(node_id_t receiver, size_t len, block_data_t *data);
        ///@}

        ///@name Methods called by RadioModel
        ///@{
        void receive(node_id_t from, size_t len, block_data_t *data);
        ///@}

        ///@make and send bootstraping message to the network
        ///@{
        void bootstrap_message();
        ///@}

        ///@make the judge known to everybody
        ///@{
        void broadcast_judge(uint8_t);
        ///@}


        void send_message(uint payload, uint source, uint8_t msg_id, uint destination, uint8_t routing_type);


        void send_special_message(uint8_t* payload, uint source, uint8_t msg_id, uint destination, uint8_t routing_type);

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

        inline void set_judge() {
            is_judge = 1;
            judge = radio().id();
        };

        inline int get_judge() {
            return judge;
        };

        inline int get_color() {
            return color;
        }

        inline int get_alg_messages(){
            return alg_messages;
        }
        
        inline void set_color(int color_) {
            color = color_;
        }

        inline void set_max_color(int max_color_) {
            max_color = max_color_;
        }

        inline uint16_t get_is_judge() {
            return is_judge;
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

        inline uint16_t active_colors_size() {
            color_numbers.print();
            return color_numbers.get_active_colors();
        };

        inline void set_judge_count(int jc) {
            judge_count = jc;
        };

        inline int get_appointed_judges() {
            return j_ass;
        };

        inline uint16_t add_neighboor(uint neighboor_id, uint color_num) {
            neighboors_colors.insert(std::make_pair(neighboor_id, color_num));
        };

        inline uint16_t add_node_color(uint node_id, uint color_num) {
            color_nodes.insert(std::make_pair(node_id, color_num));
            try_start();
        };

        inline void set_world_size(int nc) {
            node_count = nc;
        };

        inline int get_game_started() {
            return game_started;
        };

         inline void try_start() {
            if(is_judge && !game_started){
                if((c_ass == node_count - 1)&&(j_ass = judge_count - 1) ){
                    if(is_judge) this->isort(judges,20);
                    in_game(0);
                }
            }
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

        inline int get_ended_all() {
            return ended_all;
        };

        inline uint16_t is_terminated() {
            return end;
        };

        inline void change_node_color(uint node_id, uint color_num) {
            std::map<uint, uint>::iterator it = color_nodes.find(node_id);
            it->second = color_num;
        };

        inline void change_neigh_color(uint node_id, uint color_num) {
            std::map<uint, uint>::iterator it = neighboors_colors.find(node_id);
            if(it != neighboors_colors.end())
            it->second = color_num;
        };

        inline void set_tree_routing(tree_route* tree) {
            tree_routing = tree;
        };

        inline void set_tora_routing(tora_routing_t* tora) {
            tora_routing = tora;
        };

        int compare(const void * a, const void * b) {
            if (*(int*) a == *(int*) b) return 0;
            else if (*(int*) a < *(int*) b) return 1;
            else return -1;
        }

        inline void init_question() {
            last_question.answered = 0;
            last_question.position = 1;
            last_question.answer = color_numbers.get_color_from_position(last_question.position);
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
        
        int judge_suspended, baton, last_herd_count,checked_nodes,round;
        int judges[20];
        question last_question;
        tree_route *tree_routing;
        tora_routing_t *tora_routing;
        uint16_t permitted_count, answer_count, neigh_colors_count;
        uint8_t is_judge;
        uint8_t asked_color;
        int judge;
        int max_color;
        ColorTable neighboors_colors;
        ColorTable color_nodes,c_ass_help;
        std::map<uint, uint>::iterator iter_color_nodes;
        ColorsSorted color_numbers;
        int color, color_under_cons, game_started,end,node_count,judge_count,c_ass,j_ass;
        ask_color q_color;
        int exeption,alg_messages,ended_all;
    };
    // -----------------------------------------------------------------------
    // -----------Judged Coloring Constructor---------------------------------
    // -----------------------------------------------------------------------

    template<typename OsModel_P,
    typename Radio_P,
    typename Debug_P>
    PKJudgedColoring<OsModel_P, Radio_P, Debug_P>::
    PKJudgedColoring()
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
    baton(0),
    last_herd_count(0),
    checked_nodes(1),
    round(0),
    exeption(0),
    alg_messages(0),
    end(0),
    node_count(0),
            judge_count(0),
    ended_all(0),
    c_ass(0),
    j_ass(0){
    };
    // -----------Judged Coloring De-Constructor------------------------------

    template<typename OsModel_P,
    typename Radio_P,
    typename Debug_P>
    PKJudgedColoring<OsModel_P, Radio_P, Debug_P>::
    ~PKJudgedColoring() {

    };
    // -----------------------------------------------------------------------

    template<typename OsModel_P,
    typename Radio_P,
    typename Debug_P>
    void
    PKJudgedColoring<OsModel_P, Radio_P, Debug_P>::
    enable() {

        for (int i = 0; i < 20; i++) judges[i] = -1;
        radio().enable_radio();
        radio().template reg_recv_callback<self_type, &self_type::receive > (this);

        if (is_judge) {
            color_numbers.init();
            timer().template set_timer<self_type, &self_type::timer_elapsed > (
                    501, this, 0);
        } 
        bootstrap_message();



    }
    // -------------------------------------tiemr----------------------------------

    template<typename OsModel_P,
    typename Radio_P,
    typename Debug_P>
    void
    PKJudgedColoring<OsModel_P, Radio_P, Debug_P>::
    disable(void) {
#ifdef DEBUG_PKJUDGEDCOLORING
        debug().debug("PKJudgedColoring: Disable\n");
#endif
    }
    // -----------------------------------------------------------------------

    template<typename OsModel_P,
    typename Radio_P,
    typename Debug_P>
    void
    PKJudgedColoring<OsModel_P, Radio_P, Debug_P>::
    timer_elapsed(void *userdata) {
        if (color == -1 && !is_judge && (asked_color < 1)) {
            //timer().template set_timer<self_type, &self_type::timer_elapsed > (
            //       50, this, 0);
            asked_color++;
            //send_message(radio().id(), radio().id(), REQUEST_COLOR, judge, 1);
           // cout << "I:" << (int) radio().id() << "sent color request to:" << judge << endl;
        } else if (is_judge && asked_color < 1) {
            asked_color++;
            color_numbers.insert(color);
            add_node_color(radio().id(), color);
            iter_color_nodes = color_nodes.begin();
            last_question.who = radio().id();
            last_question.con_no = 0;
            int i;
            this->isort(judges, 20);
            if(radio().id()==judges[0])
                round = round + 1;
            try_start();
#ifdef DEBUG_PKJUDGEDCOLORING
            debug().debug("Node %d Judge Started his game\n", radio().id());
#endif



        }
    }
    // -----------------------------------------------------------------------

    template<typename OsModel_P,
    typename Radio_P,
    typename Debug_P>
    void
    PKJudgedColoring<OsModel_P, Radio_P, Debug_P>::
    in_game(void *userdata) {
        get_neighboors();
        // timer().template set_timer<self_type, &self_type::in_game > (
        //         50, this, 0);
        if(radio().id() == 7)
            int uio = 1;
        game_started = 1;
        if ((last_question.con_no < (color_nodes.size())) || (exeption)) {
            if (last_question.who == radio().id()) {
                get_color_nodes();
                color_numbers.print();
                if (!last_question.who)iter_color_nodes;
                if(round && baton) iter_color_nodes = color_nodes.begin();
                //cout << "Problem:" << color_nodes.size() <<endl;
                if (iter_color_nodes->first == radio().id()) {
                    if (++iter_color_nodes == color_nodes.end())
                        iter_color_nodes = color_nodes.begin();
                }
                last_question.who = iter_color_nodes->first;

                last_question.curr_color_pos = color_numbers.get_position_from_color(color_nodes.find(last_question.who)->second);
                last_question.con_no = 1;
                init_question();
                last_question.type = REASSIGN_COLOR;
            } else if (last_question.answered) {
                if (last_question.type == REASSIGN_COLOR) {
                    if (last_question.answer == 0) {
                        // std::cout << "O kombos:" << (int)last_question.who << "Apantise" << "\n";

                        last_question.answered = 0;
                        last_question.position++;
                        last_question.answer = color_numbers.get_color_from_position(last_question.position);
                        if (((last_question.position == color_numbers.get_size()) ||
                                (color_numbers.get_value_from_position(last_question.curr_color_pos) > color_numbers.get_value_from_position(last_question.position)))
                                && ((round==0) || (radio().id() == judges[0]))) {
                            last_question.con_no++;
                            //std::cout << radio().id() << "----CON NOOOOOOOOOOOO:" << last_question.con_no << "\n";
                            if (++iter_color_nodes == color_nodes.end())
                                iter_color_nodes = color_nodes.begin();
                            if (iter_color_nodes->first == radio().id()) {
                                if (++iter_color_nodes == color_nodes.end())
                                    iter_color_nodes = color_nodes.begin();
                            }
                            last_question.who = iter_color_nodes->first;
                            last_question.curr_color_pos = color_numbers.get_position_from_color(color_nodes.find(last_question.who)->second);
                            init_question();
                            //if(last_question.con_no==99) cout << "After Print" << endl;

                        } else if((last_question.position >= color_numbers.get_size()
                                || (color_numbers.get_value_from_position(last_question.curr_color_pos) < color_numbers.get_value_from_position(last_question.position)))
                                &&((round == 1) && (radio().id() != judges[0]))){
                             last_question.answered = 0;
                             last_question.position++;
                             last_question.answer = ++max_color;
                        }

                    } else if ((last_question.answer == color_numbers.get_color_from_position(last_question.position)) || ( ((round) && (radio().id() != judges[0]) ) && (last_question.answer == max_color)  )) {
                        //std::cout << "O kombos:" << (int)last_question.who << "Apantise" << "\n";
                        //std::cout << "tou arese to xrwma "<< (int)last_question.answer  <<" omws \n";
                        last_question.con_no = 1;
                        color_numbers.insert(last_question.answer);
                        color_numbers.remove(color_nodes.find(last_question.who)->second);
                        if(round && (radio().id() != judges[0])) {
                            ///color_nodes.erase(iter_color_nodes);

                            change_node_color(last_question.who, last_question.answer);
                            change_neigh_color(last_question.who, last_question.answer);
                            if ((++iter_color_nodes) == color_nodes.end()){
                                last_question.con_no = color_nodes.size();
                                exeption=0;
                            }
                        } else {
                            change_node_color(last_question.who, last_question.answer);
                            if ((++iter_color_nodes) == color_nodes.end())
                                iter_color_nodes = color_nodes.begin();
                            if (iter_color_nodes->first == radio().id()) {
                                if (++iter_color_nodes == color_nodes.end())
                                    iter_color_nodes = color_nodes.begin();
                            }
                        }

                        if(last_question.con_no != color_nodes.size()){
                        last_question.who = iter_color_nodes->first;
                        last_question.curr_color_pos = color_numbers.get_position_from_color(color_nodes.find(last_question.who)->second);
                        init_question();
                        }else{
                            in_game(0);
                        }
                        //color_numbers.print();
                    }

                }
            }
            if (!last_question.answered)
                send_message((uint16_t) last_question.answer, radio().id(), REASSIGN_COLOR, (uint16_t) last_question.who, 1);
        } else if (((last_question.con_no) == color_nodes.size()) && (judge_suspended == 0)) {
            send_message(-1, radio().id(), ASK_COLOR, -1, 0);
            judge_suspended = 1;
        } else if ((judge_suspended == 1) || (judge_suspended == 4)) {
            int i = 0, ok = 1;
            if(judge_suspended != 4){
            while (((i < color_numbers.get_size()) && ((color_numbers.get_value_from_color(color) <= color_numbers.get_value_from_position(i + 1)))) || ((round) && (radio().id()!=judges[0]))) {
                if(radio().id()==15)
                        cout << endl;
                int color_under_cons;
                if(round){
                    int ioio = radio().id();
                    
                }
                if(i < color_numbers.get_size())
                    color_under_cons = color_numbers.get_color_from_position(i + 1);
                else
                    color_under_cons = ++max_color;
                for (std::map<uint, uint>::iterator it = neighboors_colors.begin(); it != neighboors_colors.end(); it++) {
                    int neigh_id = (int) it->first;
                    int neigh_color = neighboors_colors.find(neigh_id)->second;
                    if ((neigh_color == color_under_cons) || (color == color_under_cons))
                        ok = 0;
                }
                if (ok) {
                    color_numbers.remove(color);
                    color = color_under_cons;
                    color_numbers.insert(color_under_cons);
                    color_nodes.find((uint) radio().id())->second = color;
                    if(!round)last_question.con_no = 1;
                    last_question.who = radio().id();
                    if(round)
                        judge_suspended = 4;
                    else
                        judge_suspended = 0;
                    timer().template set_timer<self_type, &self_type::in_game > (
                            10, this, 0);
                    ok = 1000;
                }
                if (ok != 1000)
                    ok = 1;
                else {
                    break;
                }
                i++;
            }
            }else{
                ok = 1;
            }

            if (ok != 1000) {
                cout<< "Helloooooooooooooooooooooooo:" << radio().id() << endl;

                if (round == 1) {
                    int tmp = 0;
                    this->isort(judges,20);
                    int b;
                    for (int i = 0; i < 20; i++) {
                        int a = radio().id();
                        b = judges[i];
                        if ((judges[i] < radio().id()) && (judges[i] != -1)) {
                            send_special_message((uint8_t*) & color_numbers, radio().id(), TAKE_TURN, judges[i], 1);
                            b = judges[i];
                            tmp=i;
                            break;
                        }
                    }
                    if (tmp==0 && baton){
                        end = 1;
                        ended_all = 1;
                        coloring_message data;
                        data.set_msg_id(ALL_END);
                        radio().send(radio().BROADCAST_ADDRESS, data.buffer_size(), (uint8_t*) & data);
                 }
                    cout << "Egw o server:" << radio().id() << " edwsa thn sktali ston:" << b << endl;
                    round = 0;
                }
                if (baton) {
                    for (std::map<uint, uint>::iterator it = color_nodes.begin(); it != color_nodes.end(); it++) {
                        if (radio().id() != it->first)
                            send_message(-1, radio().id(), FINAL_CHECK,it->first,  1);
                    }
                    last_herd_count = color_nodes.size();
                    color_nodes.clear();
                    baton = 0;
                }

            }
        }
        if ((color_nodes.size() == 1) && ((round) || (baton) || (radio().id() == judges[0]))) {
            int b;
            int tmp = 0;
            this->isort(judges,20);
            for (int i = 0; i < 20; i++) {
                int a = radio().id();
                
                if ((judges[i] < radio().id()) && (judges[i] != -1)) {
                    b = judges[i];
                    send_special_message((uint8_t*) & color_numbers, radio().id(), TAKE_TURN, judges[i], 1);
                    tmp =i;
                    break;
                }
            }
            if (tmp==0 && baton){
                        end = 1;
                        ended_all = 1;
                        coloring_message data;
                        data.set_msg_id(ALL_END);
                        radio().send(, radio().BROADCAST_ADDRESS, data.buffer_size(), (uint8_t*) & data);
                 }
            color = 1000 * radio().id();
            cout << "Egw o server:" << radio().id() << " edwsa thn sktali ston:" << b << endl;
            if(radio().id() == judges[0]) round = 0;
            baton = 0;

        }

    }
    // -----------------------------------------------------------------------

    template<typename OsModel_P,
    typename Radio_P,
    typename Debug_P>
    void
    PKJudgedColoring<OsModel_P, Radio_P, Debug_P>::
    send(node_id_t destination, size_t len, block_data_t *data) {

    }
    // -----------------------------------------------------------------------

    template<typename OsModel_P,
    typename Radio_P,
    typename Debug_P>
    void
    PKJudgedColoring<OsModel_P, Radio_P, Debug_P>::
    receive(node_id_t from, size_t len, block_data_t *data) {
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
           // std::cout << " Egw o:" << destination << " elaba minima apo ton: " << source << " me id:" << msg_id2 << "kai payload:" << payload << "\n";
            
        if(message->source() == radio().id())
            return;

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
        } else if (msg_id == ASK_COLOR) {
            send_message(color, radio().id(), REPLY_COLOR, message->source(), 0);
            //cout << "Elaba ASK_COLOR apo ton:" << message->source() << endl;
        }  else if (msg_id == ALL_END) {
            if(!ended_all){
                ended_all = 1;
                coloring_message data;
                data.set_msg_id(ALL_END);
                radio().send(radio().BROADCAST_ADDRESS, data.buffer_size(), (uint8_t*) & data);
            }
        }else if ((msg_id == REPLY_COLOR) && (message->destination() == radio().id())) {
            payload = *((uint*) message->payload());
            std::map<uint, uint>::iterator it = neighboors_colors.find((int) message->source());
            if (it == neighboors_colors.end())
                neighboors_colors.insert(std::make_pair(message->source(), (uint) payload));
            else it->second = payload;
            int neigh_no = get_neighboors();
            if (neigh_colors_count != neigh_no) {
                neigh_colors_count++;
            }
            if (neigh_colors_count == neigh_no) {
                neigh_colors_count = 0;
                if (is_judge) {
                    in_game(0);
                } else {
                    int ok = 1;
                    for (std::map<uint, uint>::iterator it = neighboors_colors.begin(); it != neighboors_colors.end(); it++) {
                        if (it->second == color) {
                            send_message(color, radio().id(),CHECK_NOT_OK, judge, 1);
                            cout << "O kombos:" << radio().id() << " dilwse problima ston:" << judge << endl;
                            ok = 0;
                            break;
                        }                    
                    }
                    if(ok) {
                        send_message(color, radio().id(), CHECK_OK, judge, 1);
                    }
                }
            }
        } else if (is_judge) {
            if (msg_id == 194) {// If the messages are in tree_routing type get the color_message from payload
                message = (coloring_message *) t_message->payload();
                int tmp = message->msg_id();
                int payload = *((uint*) message->payload());
                if ((tmp < 10) && (tmp > 0) && (message->destination() == radio().id()))
                    tmp = tmp - 1;
                payload = payload;

            }
            int payload = 0;
            if ((message->msg_id() == TAKE_TURN) && (message->destination() == radio().id())) {
                cout << "Judge:" << (int) radio().id() << " started his game!" << endl;
                baton = 1;
                ColorsSorted cn_temp = *((ColorsSorted*) message->payload());
                for (int i = 0; i < (cn_temp.get_size() - 1); i++) {
                    int color_tmp = cn_temp.get_color_from_position(i + 1);
                    int count_tmp = cn_temp.get_value_from_color(color_tmp);
                    for (int j = 0; j < count_tmp; j++) {
                        color_numbers.insert(color_tmp);
                    }
                }     
                if (judge_suspended > 0) {
                    last_question.con_no = 1;
                    if((c_ass == node_count - 1)&&(j_ass = judge_count - 1) ){
                        if(is_judge) this->isort(judges,20);
                            in_game(0);
                    }
                    judge_suspended == 0;
                }else{
                    if((c_ass == node_count - 1)&&(j_ass = judge_count - 1) ){
                        if(is_judge) this->isort(judges,20);
                        in_game(0);
                    }
                }
                color_numbers.print();

            }
            if (message->msg_id() != ASK_PERMISSION) {
                payload = *((uint*) message->payload());
            } else {
                q_color = *((ask_color*) message->payload());
            }
            if (((message->msg_id() == COLOR_ASSIGNED) || (message->msg_id() == COLOR_NOT_ASSIGNED)) && (message->source() == last_question.who) && (message->destination() == radio().id())) {
                if ((payload > 0) && (message->msg_id() == COLOR_ASSIGNED)) {
                    std::map<uint, uint>::iterator it = color_nodes.find((int) message->source());
                    
                    if (it == color_nodes.end()) {
                        color_nodes.insert(std::make_pair(message->source(), (uint) payload));
                        color_numbers.insert((uint) payload);

                    } else {

                        if ((it->second != payload) && ((payload == color_numbers.get_color_from_position(last_question.position)) || (round))) {
                            last_question.answered = 1;
                            last_question.answer = (uint) payload;
                            in_game(0);

                        }
                    }
                } else if (message->msg_id() == COLOR_NOT_ASSIGNED) {
                    if (((uint) message->source() == (int) last_question.who) && ((uint) payload == (int) last_question.answer)) {
                        last_question.answered = 1;
                        last_question.answer = 0;
                        in_game(0);
                    }
                }
            }else if((message->msg_id() == COLOR_ASSIGNED) && (message->destination() == radio().id())){
                if(c_ass_help.find(message->source()) == c_ass_help.end()){
                    c_ass_help.insert(std::make_pair(message->source(),(uint)-1));
                    c_ass++;
                    if(radio().id() == 8)
                    cout << "AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAa" << c_ass << ":" << node_count << endl;
                     try_start();
                }          
            } else if (message->msg_id() == CHECK_NOT_OK && radio().id()==message->destination()) {
                add_node_color(message->source(), (uint) payload);
                checked_nodes++;
                if (checked_nodes == last_herd_count) {
                    if(color_nodes.size() == 1){
                        exeption = 1;
                        iter_color_nodes = color_nodes.begin();
                    }
                    init_question();
                    last_question.who = radio().id();
                    last_question.con_no = 1;
                    iter_color_nodes = color_nodes.begin();
                    round++;
                    in_game(0);
                }
            } else if (message->msg_id() == CHECK_OK && radio().id()==message->destination()) {
                checked_nodes++;
                if (checked_nodes == last_herd_count) {
                    if(color_nodes.size() == 1){
                        exeption = 1;
                        iter_color_nodes = color_nodes.begin();
                    }
                    init_question();
                    last_question.who = radio().id();
                    last_question.con_no = 1;
                    iter_color_nodes = color_nodes.begin();
                    round++;
                    in_game(0);
                }
            } else if ((message->msg_id() == REQUEST_COLOR) && (message->destination() == radio().id())) {
                int routing_type;
                if (msg_id == 194)
                    routing_type = 1;
                else routing_type = 0;

                cout << "Received color request from:" << message->source() << endl;
                std::map<uint, uint>::iterator it = color_nodes.find((uint) (message->source()));
                if (it == color_nodes.end()) {
                    add_node_color(message->source(), max_color);
                    color_numbers.insert(max_color);
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

        } else if ((!is_judge) || (judge_suspended == 1)) {
            if (msg_id == 194) // If the messages are in tree_routing type get the color_message from payload
                message = (coloring_message *) t_message->payload();
            int payload = 0;
            int tmp = message->msg_id();
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
                    //std::cout<< " Ase katw re to xrwma:"<< color << " " << ((int)(*(message->payload()))) <<" ilithie:" << message->source() << " giati to lew egw" << radio().id() <<"\n";
                } else {
                    send_message((uint) q_color.color, radio().id(), GIVE_PERMISSION, message->source(), 0);
                }
            } else if (((message->msg_id() == GIVE_PERMISSION) || (message->msg_id() == DECL_PERMISSION)) && message->destination() == radio().id() && color_under_cons != 0) {
                if (payload == color_under_cons) {
                    answer_count++;
                    if (message->msg_id() == GIVE_PERMISSION) {
                        permitted_count++;
                    } else {
                        //std::cout << " Den to thelw re to xrwma:" << color_under_cons << " blaka!!!!" << message->source() << "\n";
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
                                last_question.answer = color;
                                judge_suspended = 3;
                                in_game(0);
                            }
                            color_under_cons = 0;
                        }
                        answer_count = 0;
                    }
                }
            } else if (message->msg_id() == REASSIGN_COLOR && message->destination() == radio().id()) {
            } else if ((message->msg_id()) == FINAL_CHECK && (message->destination() == radio().id())){
                send_message(-1, radio().id(), ASK_COLOR, -1, 0);
                //cout << " MY FINAL CHECK IS NIGH" << endl;
            }
        }

    }
    // -----------------------------------------------------------------------

    template<typename OsModel_P,
    typename Radio_P,
    typename Debug_P>
    void
    PKJudgedColoring<OsModel_P, Radio_P, Debug_P>::
    bootstrap_message() {
#ifdef DEBUG_PKJUDGEDCOLORING
        //debug().debug("Node number %d sends bootstrap message\n", radio().id());
#endif
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
    PKJudgedColoring<OsModel_P, Radio_P, Debug_P>::
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

            if (judge == -1) {

                judge = judge_id;
                if (!is_judge) {
                    send_message(radio().id(), radio().id(), REQUEST_COLOR, judge_id, 1);
                }


            } else if (is_judge) {
                cout << "Sta stila ola ki emeina ston aso" << endl;
                send_message(radio().id(), radio().id(), -1, judge_id, 1);
            }
            
#ifdef DEBUG_PKJUDGEDCOLORING
            debug().debug("Node number %d acknowledges %d as Judge!\n", radio().id(), judge_id);
#endif
            send_message(judge_id, radio().id(), BROAD_JUDGE, -1, 0);



        }

    }
    // -----------------------------------------------------------------------

    template<typename OsModel_P,
    typename Radio_P,
    typename Debug_P>
    void
    PKJudgedColoring<OsModel_P, Radio_P, Debug_P>::
    send_message(uint payload, uint source, uint8_t msg_id, uint destination, uint8_t routing_type) {
        coloring_message data;
        data.set_msg_id(msg_id);
        data.set_source(source);
        data.set_destination(destination);
        data.set_payload(sizeof (uint), (uint8_t*) & payload);
        if (routing_type == 0) {
            alg_messages++;
            radio().send(radio().BROADCAST_ADDRESS, data.buffer_size(), (uint8_t*) & data);
            //std::cout << " Egw o:" <<(int) source <<" estila minima ston: " <<(int) destination << " me id:"<<(int)msg_id << "kai payload:" << (int)*((uint*)data.payload())<< "\n";
        } else if (routing_type == 1) {
           std::cout << " Egw o:" << (int) source << " estila minima ston: " << (int) destination << " me id:" << (int) msg_id << "kai payload:" << (int) *((uint*) data.payload()) << "\n";
            tora_routing->send(destination, data.buffer_size(), (block_data_t*) & data);
            // tree_routing->send(destination, data.buffer_size(), (block_data_t*) & data);
        }
    }

    // -----------------------------------------------------------------------

    template<typename OsModel_P,
    typename Radio_P,
    typename Debug_P>
    void
    PKJudgedColoring<OsModel_P, Radio_P, Debug_P>::
    send_special_message(uint8_t* payload, uint source, uint8_t msg_id, uint destination, uint8_t routing_type) {
        coloring_message data;
        data.set_msg_id(msg_id);
        data.set_source(source);
        data.set_destination(destination);

        if (routing_type == 0) {
            alg_messages++;
            data.set_payload(sizeof (ask_color), (uint8_t*) payload);
            //std::cout << "------Egw o:" << (int) source << " estila minima ston: " << (int) destination << " me id:" << (int) msg_id << "kai payload:" << (int) (*((ask_color*) data.payload())).color << "\n";
            radio().send(radio().BROADCAST_ADDRESS, data.buffer_size(), (uint8_t*) & data);
        } else if (routing_type == 1) {
            data.set_payload((uint8_t) sizeof (ColorsSorted), (uint8_t*) payload);
            tora_routing->send(destination, data.buffer_size(), (block_data_t*) & data);
        }
    }


    // -----------------------------------------------------------------------

    template<typename OsModel_P,
    typename Radio_P,
    typename Debug_P>
    uint16_t
    PKJudgedColoring<OsModel_P, Radio_P, Debug_P>::
    get_neighboors() {
#ifdef DEBUG_PKJUDGEDCOLORING
        int i = 0;
        // debug().debug("%d:My neighboors are the following:\n", radio().id());
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
    PKJudgedColoring<OsModel_P, Radio_P, Debug_P>::
    get_color_nodes() {
#ifdef DEBUG_PKJUDGEDCOLORING
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

    // -----------------------------------------------------------------------

}
#endif


