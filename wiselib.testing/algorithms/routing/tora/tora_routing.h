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
#ifndef __ALGORITHMS_ROUTING_TORA_ROUTING_H__
#define __ALGORITHMS_ROUTING_TORA_ROUTING_H__

#include "util/base_classes/routing_base.h"
#include "algorithms/routing/tora/tora_routing_types.h"
#include "algorithms/routing/tora/tora_routing_message.h"
#include "algorithms/routing/tora/tora_broadcast_message.h"
#include <string.h>
#include <map>
#include <limits.h>

#undef DEBUG
#define DEBUG
#define UN 0
#define DN 1
#define UP 2

#define ECHO_INTERVAL 2
#define ALLOWED_LOSS ECHO_INTERVAL * 2

#define NET_DIAM 30
#define TORA_TIMEOUT NET_DIAM * 2

using namespace std;
namespace wiselib {

    /** \brief Tora routing algorithm.
     * 
     *  \ingroup routing_concept
     *  \ingroup radio_concept
     *  \ingroup basic_algorithm_concept
     *  \ingroup routing_algorithm
     * 
     */
    template<typename OsModel_P,
            typename RoutingTable_P,
            typename Radio_P = typename OsModel_P::Radio,
            //             typename Timer_P = typename OsModel_P::Timer,
            typename Debug_P = typename OsModel_P::Debug>
            class ToraRouting
            : public RoutingBase<OsModel_P, Radio_P> {
    public:
        typedef OsModel_P OsModel;
        typedef Radio_P Radio;
        //       typedef Timer_P Timer;
        typedef Debug_P Debug;

        typedef RoutingTable_P RoutingTable;
        typedef typename RoutingTable::iterator RoutingTableIterator;
        typedef typename RoutingTable::value_type RoutingTableValue;
        typedef typename RoutingTable::mapped_type RoutingTableEntry;
        typedef ToraRouting<OsModel, RoutingTable, Radio, Debug> self_type;
        typedef typename OsModel_P::Timer Timer;

        typedef typename Radio::node_id_t node_id_t;
        typedef typename Radio::size_t size_t;
        typedef typename Radio::block_data_t block_data_t;

        typedef typename Timer::millis_t millis_t;
        typedef ToraRoutingMessage<OsModel, Radio> RoutingMessage;
        typedef ToraBroadcastMessage<OsModel, Radio, RoutingTableValue, 8 > BroadcastMessage;

        typedef map<int, RoutingMessage*> pend_msg_t;

        typedef map<int, height> NeighborsHeight;
        typedef std::map<int, bool> route_required_flag;
        typedef std::map<int, int> link_state;
        // --------------------------------------------------------------------
        enum SpecialNodeIds {
           BROADCAST_ADDRESS = Radio_P::BROADCAST_ADDRESS, ///< All nodes in communication range
           NULL_NODE_ID      = Radio_P::NULL_NODE_ID      ///< Unknown/No node id
        };
        // --------------------------------------------------------------------
        enum Restrictions {
           MAX_MESSAGE_LENGTH = Radio_P::MAX_MESSAGE_LENGTH - RoutingMessage::PAYLOAD_POS  ///< Maximal number of bytes in payload
        };
        // --------------------------------------------------------------------
        ///@name Construction / Destruction
        ///@{
        ToraRouting();
        ~ToraRouting();
        ///@}

        ///@name Routing Control
        ///@{
        void enable_radio(void);
        void disable_radio(void);
        ///@}

        ///@name Radio Concept
        ///@{
        /**
         */
        void send( node_id_t receiver, size_t len, block_data_t *data );
        /**
         */
        void receive( node_id_t from, size_t len, block_data_t *data );
        /**
         */
        typename Radio::node_id_t id()
        {
           return radio_->id();
        };
        ///@}

        ///@name Methods called by Timer
        ///@{
        void timer_elapsed(void *userdata);
        ///@}

        //Receive and process a BDC message
        void recvBDC(node_id_t);

        //Receive and process a QRY message
        void recvQRY(node_id_t, int);

        //Receive and process an UPD message
        void recvUPD(node_id_t, RoutingMessage*);

        //Receive and process a CLR message
        void recvCLR(node_id_t, RoutingMessage*);

        //Receive and process a DATA message
        void recvDATA(node_id_t, RoutingMessage*);

        //Clear Height for all destination after TIMEOUT period
        void clearDest();

        /**
         * Debug functions
         */
        //Print Neighbors
        inline void printNeighbors();
        
        //Print height 
        inline void printHeight(int);

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

        //Map: Destination ID, Route reruest flag
        route_required_flag RR;

        millis_t startup_time_;
        millis_t work_period_;

        //Map: Destination ID, timestamp
        map<int, int> destinations;

        //Pending messages
        pend_msg_t pend_msgs;
        
        //Node Height per destination
        map<int, height> H;

        //Neighbors Height per destination
        map<int, NeighborsHeight> NH;

        //Link state per destination
        map<int, link_state> LS;

        //Map with neigbors id
        map<int, int> neighbors;

        //Downstream links
        int DNlinks;

        //Map: Neighbor ID, Heisht Struct
        NeighborsHeight::iterator NHIter;

        //Iterator for neighbors map
        map<int, int>::iterator neighborsIter;

        //Iterator for destinations map
        map<int, int>::iterator destinationsIter;

        //Iterator for Pending Messages
        typedef typename pend_msg_t::iterator pend_msg_iter;

        //Link state iterator
        link_state::iterator LSIter;

        //RR iterator
        route_required_flag::iterator RRIter;

        //Determine whether a Neighbor Height was set
        bool NH_exists(int, int);

        //Determine whether a Neighbor exists
        bool neighbor_exists(int);

        //Determine whether a destination exist
        bool destination_exists(int);

        //Determine whether a RR flag for a specific destination was set
        bool RR_flag_is_set(int);

        //Count downstream links for a specific destination
        int count_DNs(int);

        //Updates link state map for a specific destination
        void updateLS(int);

        //Determine whether all reference levels for a destination are equal
        bool ref_lvls_eq(int);

        //Returns the node id with the minimun reference level for a specific destination
        int find_min_ref_lvl(int);

        //Determine whether all r on Neighbors Height are equal with 1
        bool r_eq_one(int);

        //Determine whether originator ID is equal with the node ID for a specific destination
        bool oid_eq_id(int);

        //Empties the buffer and sends stored messages
        void empty_buffer(int);

        //Determine whether the Height for a specific destination is Zero
        bool heightIsZero(height tmpHeight) {
            return (tmpHeight.time == 0 && tmpHeight.originator_id == 0
                    && tmpHeight.r == 0 && tmpHeight.delta == 0);
        }

        //Determine whether the Height for a specific destination is Null
        bool heightIsNull(height tmpHeight) {
            return (tmpHeight.time == SHRT_MIN && tmpHeight.originator_id == SHRT_MIN
                    && tmpHeight.r == SHRT_MIN && tmpHeight.delta == SHRT_MIN);
        }
        // ----------------------------------------------------------------------
        //                          Private ECHO functions
        // ----------------------------------------------------------------------

        void decrease();
        
        void clean_Dead_Neighbors();

        void broadcaster();

        int timer_cnt;
    };
    // -----------------------------------------------------------------------
    // -----------------------------------------------------------------------
    // -----------------------------------------------------------------------

    template<typename OsModel_P,
    typename RoutingTable_P,
    typename Radio_P,
    typename Debug_P>
    ToraRouting<OsModel_P, RoutingTable_P, Radio_P, Debug_P>::
    ToraRouting()
    : startup_time_(1000),
    work_period_(5000),
    timer_cnt(0){
    };
    // -----------------------------------------------------------------------

    template<typename OsModel_P,
    typename RoutingTable_P,
    typename Radio_P,
    typename Debug_P>
    ToraRouting<OsModel_P, RoutingTable_P, Radio_P, Debug_P>::
    ~ToraRouting() {
#ifdef DEBUG
        debug().debug("Tora Routing Destroyed\n");
#endif
    };
    // -----------------------------------------------------------------------

    template<typename OsModel_P,
    typename RoutingTable_P,
    typename Radio_P,
    typename Debug_P>
    void
    ToraRouting<OsModel_P, RoutingTable_P, Radio_P, Debug_P>::
    enable_radio(void) {
#ifdef DEBUG
        debug().debug("ToraRouting Boots for %i\n", radio().id());
#endif
        broadcaster();
        radio().enable_radio();
        radio().template reg_recv_callback<self_type, &self_type::receive > (this);
        timer().template set_timer<self_type, &self_type::timer_elapsed > (startup_time_, this, 0);
        
    }
    // -----------------------------------------------------------------------

    template<typename OsModel_P,
    typename RoutingTable_P,
    typename Radio_P,
    typename Debug_P>
    void
    ToraRouting<OsModel_P, RoutingTable_P, Radio_P, Debug_P>::
    disable_radio(void) {
#ifdef DEBUG
        debug().debug("Called ToraRouting::disable\n");
#endif
    }
    // -----------------------------------------------------------------------

    template<typename OsModel_P,
    typename RoutingTable_P,
    typename Radio_P,
    typename Debug_P>
    void
    ToraRouting<OsModel_P, RoutingTable_P, Radio_P, Debug_P>::
    send(node_id_t destination, size_t len, block_data_t * data) {


        if (neighbor_exists(destination)){
            cout << "\tDestination  " << destination << " is a neighbor " << endl;
        }else{
            cout << "\tNo route from " << radio().id() << " to " << destination << " -- Initializing TORA " << endl;
            RoutingMessage QRYmsg;
            QRYmsg.set_msg_id(QRY);
            QRYmsg.set_destination(destination);
            radio().send(radio().BROADCAST_ADDRESS, QRYmsg.buffer_size(), (uint8_t*) & QRYmsg);
            RR[destination] = true;
            pend_msgs[destination] ;

        }

    }
    // ----------------------------------------------------------------------
    //                          Private ECHO functions
    // ----------------------------------------------------------------------
    template<typename OsModel_P,
    typename RoutingTable_P,
    typename Radio_P,
    typename Debug_P>
    void
    ToraRouting<OsModel_P, RoutingTable_P, Radio_P, Debug_P>::recvBDC(node_id_t from) {
#ifdef DEBUG
      //  debug().debug("BCD Rcvd at: %i  from: %i\n", radio().id(), from);
#endif
        //Add new neighbor
        neighbors[from] = ALLOWED_LOSS;
    }
    
    // -----------------------------------------------------------------------
    template<typename OsModel_P,
    typename RoutingTable_P,
    typename Radio_P,
    typename Debug_P>
    void
    ToraRouting<OsModel_P, RoutingTable_P, Radio_P, Debug_P>::broadcaster(){
        
        //Decrease neighbors timestamp
        decrease();
        /*cout<< "Node :\t" << owner().id() << " Neighbors : ";
        map<int,int>::iterator iterator;
        for (iterator = neighbors.begin(); iterator != neighbors.end(); iterator++) {
            cout << "("<<iterator->first << " , " << iterator->second << ") " ;
        }
        cout << endl;*/

        if(timer_cnt % ECHO_INTERVAL == 0){
            BroadcastMessage message;
            message.set_msg_id(ToraBroadcastMsgId);
            radio().send(radio().BROADCAST_ADDRESS, message.buffer_size(), (uint8_t*) & message);
        }
        
        if(timer_cnt == ALLOWED_LOSS){
            clean_Dead_Neighbors();
            timer_cnt = 0;
         }

         timer_cnt++;

    }

// ----------------------------------------------------------------------
    template<typename OsModel_P,
    typename RoutingTable_P,
    typename Radio_P,
    typename Debug_P>
    void
    ToraRouting<OsModel_P, RoutingTable_P, Radio_P, Debug_P>::decrease(){
        map<int,int>::iterator iterator;
        for (iterator = neighbors.begin(); iterator != neighbors.end(); iterator++) {
            --iterator->second;
        }
    }

// ----------------------------------------------------------------------
    template<typename OsModel_P,
    typename RoutingTable_P,
    typename Radio_P,
    typename Debug_P>
    void
    ToraRouting<OsModel_P, RoutingTable_P, Radio_P, Debug_P>::clean_Dead_Neighbors(){
        map<int,int>::iterator iterator;
        int tmp_id;
        int tmp_s;
        for (iterator = neighbors.begin(); iterator != neighbors.end() ; iterator++) {
            tmp_id = iterator->first;
            tmp_s = iterator->second;
            
            if(tmp_s <= 0){

               map<int, NeighborsHeight>::iterator NHiter;
                for(NHiter = NH.begin(); NHiter != NH.end(); NHiter++){
                    if(NH_exists( tmp_id,NHiter->first)){
                        NH[NHiter->first].erase(tmp_id);
                        LS[NHiter->first].erase(tmp_id);
                        
                        cout << radio().id()<<" : "<< " Delete DEAD neighbor " << tmp_id  << "-- "<<iterator->second << " destination " << NHiter->first << endl;
                        updateLS(NHiter->first);



                           if (count_DNs(NHiter->first) == 0 && radio().id() != NHiter->first ){
                                cout << radio().id() << " : DnLinks = " <<  count_DNs(NHiter->first) << " for destination : "<< NHiter->first<<endl;

                                //Propagate new ref lvl
                                //Propagates the new Height
                                int msg_dest = NHiter->first;
                                H[msg_dest].time = H[msg_dest].time + 2;
                                H[msg_dest].originator_id = radio().id();
                                H[msg_dest].r = 0;
                                H[msg_dest].delta = 0;
                                H[msg_dest].ID = radio().id();
                                updateLS(msg_dest);

                                RoutingMessage UPDmsg;
                                UPDmsg.set_msg_id(UPD);
                                UPDmsg.set_destination(msg_dest);
                                UPDmsg.set_height(H[msg_dest]);
                                radio().send(radio().BROADCAST_ADDRESS, UPDmsg.buffer_size(), (uint8_t*) & UPDmsg);
                                cout<<"\tBroadcating an UPD message"<<endl;
                            }
                            neighbors.erase(tmp_id);
                    }
                }


            }

        }
    }


// ----------------------------------------------------------------------
//                          Private Tora functions
// ----------------------------------------------------------------------
    

    template<typename OsModel_P,
    typename RoutingTable_P,
    typename Radio_P,
    typename Debug_P>
    void
    ToraRouting<OsModel_P, RoutingTable_P, Radio_P, Debug_P>::
    timer_elapsed(void* userdata) {


        //Excecutes the Broadcaster
        broadcaster();
        
        //Clear Destinations table
        clearDest();
        
        
        timer().template set_timer<self_type, &self_type::timer_elapsed > (startup_time_, this, 0);

    }
    // -----------------------------------------------------------------------

    template<typename OsModel_P,
    typename RoutingTable_P,
    typename Radio_P,
    typename Debug_P>
    void
    ToraRouting<OsModel_P, RoutingTable_P, Radio_P, Debug_P>::
    receive(node_id_t from, size_t len, block_data_t *data) {

        if (from == radio().id())
            return;

        uint8_t msg_id = *data;
        switch (msg_id){
            case ToraBroadcastMsgId:
            {
                recvBDC(from);
                break;
            }
            case QRY:
            {
                RoutingMessage *message = reinterpret_cast<RoutingMessage*> (data);
                recvQRY(from, message->destination());
                break;
            }
            case UPD:
            {
                RoutingMessage *message = reinterpret_cast<RoutingMessage*> (data);
                recvUPD(from, message);
                break;
            }
            case CLR:
            {
                RoutingMessage *message = reinterpret_cast<RoutingMessage*> (data);
                recvCLR(from, message);
                break;
            }
            case DATA:
            {
                RoutingMessage *message = reinterpret_cast<RoutingMessage*> (data);
                recvDATA(from, message);                
                break;
            }
        }
    }
    
    // -----------------------------------------------------------------------
    template<typename OsModel_P,
    typename RoutingTable_P,
    typename Radio_P,
    typename Debug_P>
    void
    ToraRouting<OsModel_P, RoutingTable_P, Radio_P, Debug_P>::recvDATA(node_id_t from, RoutingMessage* hmsg) {
        if(hmsg->next_node() ==  radio().id()){
            cout<< radio().id() << " : Received a DATA message from "<< from << " with destination "<< hmsg->destination()<<endl;
            if(hmsg->destination() == radio().id()){
                cout<<"\tData message delivered to destination"<< endl;
            }else{
                //Check if message destination is a neighbor
                if(neighbor_exists(hmsg->destination())){
                    RoutingMessage DATAmsg;
                    DATAmsg.set_msg_id(DATA);
                    DATAmsg.set_next_node(hmsg->destination());
                    DATAmsg.set_destination(hmsg->destination());
                    radio().send(radio().BROADCAST_ADDRESS, DATAmsg.buffer_size(), (uint8_t*) & DATAmsg);
                    cout<<"\tForwarding a Data message to destination "<< hmsg->destination()<<endl;
                }else{
                    int tmpH = SHRT_MAX;
                    int next_node = SHRT_MAX;
                    for (LSIter = LS[hmsg->destination()].begin(); LSIter != LS[hmsg->destination()].end(); LSIter++) {
                        if (LSIter->second == DN){
                            if(NH[hmsg->destination()][LSIter->first].delta > 0 && NH[hmsg->destination()][LSIter->first].delta < tmpH){
                                tmpH = NH[hmsg->destination()][LSIter->first].delta;
                                next_node = LSIter->first;
                            }else if (NH[hmsg->destination()][LSIter->first].delta < 0){
                                next_node = LSIter->first;
                            }
                        }
                    }
                    RoutingMessage DATAmsg;
                    DATAmsg.set_msg_id(DATA);
                    DATAmsg.set_next_node(next_node);
                    DATAmsg.set_destination(hmsg->destination());
                    radio().send(radio().BROADCAST_ADDRESS, DATAmsg.buffer_size(), (uint8_t*) & DATAmsg);
                    cout<<"\tForwarding a Data message with destination "<< hmsg->destination() << " to "<< next_node <<endl;

                }

            }
            
        }

     }
     // -----------------------------------------------------------------------

    template<typename OsModel_P,
    typename RoutingTable_P,
    typename Radio_P,
    typename Debug_P>
    void
    ToraRouting<OsModel_P, RoutingTable_P, Radio_P, Debug_P>::recvCLR(node_id_t from, RoutingMessage* hmsg) {
        int msg_dest = hmsg->destination();
        int msg_sour = from;

        cout<< radio().id() << " Received a CLR message from "<< msg_sour <<endl;

        //Update Neighbors Height
        NH[msg_dest][msg_sour].time = hmsg->get_height().time;
        NH[msg_dest][msg_sour].originator_id = hmsg->get_height().originator_id;
        NH[msg_dest][msg_sour].r = hmsg->get_height().r;
        NH[msg_dest][msg_sour].delta = hmsg->get_height().delta;

        if(!heightIsNull(H[msg_dest])){

            if(!neighbor_exists(msg_dest)){
                //Update Height
                H[msg_dest].time = SHRT_MIN;
                H[msg_dest].originator_id = SHRT_MIN;
                H[msg_dest].r = SHRT_MIN;
                H[msg_dest].delta = SHRT_MIN;
            }else{
                H[msg_dest].time = 0;
                H[msg_dest].originator_id = 0;
                H[msg_dest].r = 0;
                H[msg_dest].delta = 0;
            }


            updateLS(msg_dest);

            RoutingMessage CLRmsg;
            CLRmsg.set_msg_id(CLR);
            CLRmsg.set_destination(msg_dest);
            CLRmsg.set_height(H[msg_dest]);
            radio().send(radio().BROADCAST_ADDRESS, CLRmsg.buffer_size(), (uint8_t*) & CLRmsg);
     
            cout<<"\tRebroadcasting the CLR message"<<endl;
        }
    }
    // -----------------------------------------------------------------------

    template<typename OsModel_P,
    typename RoutingTable_P,
    typename Radio_P,
    typename Debug_P>
    void
    ToraRouting<OsModel_P, RoutingTable_P, Radio_P, Debug_P>::recvUPD(node_id_t from, RoutingMessage* hmsg) {
        int msg_dest = hmsg->destination();
        int msg_sour = from;

        destinations[msg_dest] = TORA_TIMEOUT;

        cout << radio().id() << " : Received UPD from " << msg_sour << " : ("
                << hmsg->get_height().time << "," << hmsg->get_height().originator_id << ","
                << hmsg->get_height().r << "," << hmsg->get_height().delta << ","
                << hmsg->get_height().ID << ")" <<endl;

        //Update neighbors Height
        NH[msg_dest][msg_sour].time = hmsg->get_height().time;
        NH[msg_dest][msg_sour].originator_id = hmsg->get_height().originator_id;
        NH[msg_dest][msg_sour].r = hmsg->get_height().r;
        NH[msg_dest][msg_sour].delta = hmsg->get_height().delta;
        NH[msg_dest][msg_sour].ID = hmsg->get_height().ID;

        //Update Height only if flag is set
        
        if (((heightIsNull(H[msg_dest]) && RR[msg_dest] == true) ) ||
               (!heightIsNull(H[msg_dest]) && (hmsg->get_height().delta +1 ) < H[msg_dest].delta)) {

            H[msg_dest].time = hmsg->get_height().time;
            H[msg_dest].originator_id = hmsg->get_height().originator_id;
            H[msg_dest].r = hmsg->get_height().r;
            H[msg_dest].delta = hmsg->get_height().delta + 1;

            RR[msg_dest] = false;
            updateLS(msg_dest);

            RoutingMessage UPDmsg;
            UPDmsg.set_msg_id(UPD);
            UPDmsg.set_destination(msg_dest);
            UPDmsg.set_height(H[msg_dest]);
            radio().send(radio().BROADCAST_ADDRESS, UPDmsg.buffer_size(), (uint8_t*) & UPDmsg);
            cout<< "\tBroadcasting an UPD message"<<endl;



        }//Update LS
        else {

            updateLS(msg_dest);
            DNlinks = count_DNs(msg_dest);            

            //Link failure. No downstream links
            if (DNlinks == 0 && msg_dest!=radio().id()) {

                //Propagate Case
             if (!ref_lvls_eq(msg_dest)) {
                      cout<< "\t --Propagate case 2"<<endl;

                    //Find the reference level of its highest neighbor and selects a height
                    //which is lower than all neighbors with that reference level
                    int neighID = find_min_ref_lvl(msg_dest);

                    //Update height
                    H[msg_dest].time = NH[msg_dest][neighID].time  ;
                    H[msg_dest].originator_id = NH[msg_dest][neighID].originator_id;
                    H[msg_dest].r = NH[msg_dest][neighID].r;
                    H[msg_dest].delta = NH[msg_dest][neighID].delta - 1;

                    //Propagates the new Height
                    RoutingMessage UPDmsg;
                    UPDmsg.set_msg_id(UPD);
                    UPDmsg.set_destination(msg_dest);
                    UPDmsg.set_height(H[msg_dest]);
                    radio().send(radio().BROADCAST_ADDRESS, UPDmsg.buffer_size(), (uint8_t*) & UPDmsg);
                    cout<<"\tBroadcasting an UPD message"<<endl;
                    updateLS(msg_dest);
                //cout<< count_DNs(msg_dest) <<endl;
                }
                else{
                    //Reflect case with r = 0
                    if(!r_eq_one(msg_dest)){
                        //Update height
                        cout<< "\t --Reflect case 3"<<endl;
                        H[msg_dest].time = NH[msg_dest][msg_sour].time ;
                        H[msg_dest].originator_id = NH[msg_dest][msg_sour].originator_id;
                        H[msg_dest].r = 1;
                        H[msg_dest].delta = 0;

                        //Reflects back a higher sub-level
                        RoutingMessage UPDmsg;
                        UPDmsg.set_msg_id(UPD);
                        UPDmsg.set_destination(msg_dest);
                        UPDmsg.set_height(H[msg_dest]);
                        radio().send(radio().BROADCAST_ADDRESS, UPDmsg.buffer_size(), (uint8_t*) & UPDmsg);
                        cout<<"\tBroadcasting an UPD message"<<endl;
                        updateLS(msg_dest);
                        //cout<< count_DNs(msg_dest) <<endl;

                    }//r = 1
                    else{
                        //Detect case with originator_id == owner().id()
                        if (oid_eq_id(msg_dest)) {
                           H[msg_dest].time = SHRT_MIN;
                            H[msg_dest].originator_id = SHRT_MIN;
                            H[msg_dest].r = SHRT_MIN;
                            H[msg_dest].delta = SHRT_MIN;

                            RoutingMessage CLRmsg;
                            CLRmsg.set_msg_id(CLR);
                            CLRmsg.set_destination(msg_dest);
                            CLRmsg.set_height(H[msg_dest]);
                            radio().send(radio().BROADCAST_ADDRESS, CLRmsg.buffer_size(), (uint8_t*) & CLRmsg);
                            cout<<"\tBroadcasting a CLR message"<<endl;

                            updateLS(msg_dest);


                        }//Generate case with originator_id != owner().id()
                        else {
                            cout << "\t --Generate Case 5" << endl;
                            //Update Height
                            H[msg_dest].time = NH[msg_dest][msg_sour].time;
                            H[msg_dest].originator_id = NH[msg_dest][msg_sour].originator_id;
                            H[msg_dest].r = 0;
                            H[msg_dest].delta = 0;

                            //Propagates the new Height
                            RoutingMessage UPDmsg;
                            UPDmsg.set_msg_id(UPD);
                            UPDmsg.set_destination(msg_dest);
                            UPDmsg.set_height(H[msg_dest]);
                            radio().send(radio().BROADCAST_ADDRESS, UPDmsg.buffer_size(), (uint8_t*) & UPDmsg);
                            cout<<"\tBroadcasting an UPD message"<<endl;

                            updateLS(msg_dest);
                        }
                    }

                }

            }

        }
        if (count_DNs(msg_dest) > 0) {
            //Sends pending messages for the specific destination
            empty_buffer(msg_dest);
       }
       
    }
    // -----------------------------------------------------------------------

    template<typename OsModel_P,
    typename RoutingTable_P,
    typename Radio_P,
    typename Debug_P>
    void
    ToraRouting<OsModel_P, RoutingTable_P, Radio_P, Debug_P>::printHeight(int dest) {
    cout<< radio().id()<<" HEIGHT "<< " : ("<< H[dest].time << "," << H[dest].originator_id << ","
                << H[dest].r << "," << H[dest].delta << ","
                << H[dest].ID << ")" <<endl;
    }
    // -----------------------------------------------------------------------

    template<typename OsModel_P,
    typename RoutingTable_P,
    typename Radio_P,
    typename Debug_P>
    void
    ToraRouting<OsModel_P, RoutingTable_P, Radio_P, Debug_P>::printNeighbors() {
        map<int, int>::iterator iterator;
#ifdef DEBUG
        debug().debug(" %i neigbors: ", radio().id());
#endif
        for (iterator = neighbors.begin(); iterator != neighbors.end(); iterator++) {
#ifdef DEBUG
            debug().debug("%i ", iterator->first);
#endif
        }
#ifdef DEBUG
        debug().debug("\n");
#endif
    }
    // -----------------------------------------------------------------------

    template<typename OsModel_P,
    typename RoutingTable_P,
    typename Radio_P,
    typename Debug_P>
    void
    ToraRouting<OsModel_P, RoutingTable_P, Radio_P, Debug_P>::recvQRY(node_id_t from, int destination) {
        int msg_dest = destination;
        int msg_sour = from;
        //Add new destination and initialize Height
           if (!destination_exists(msg_dest)) {

               destinations[msg_dest] = TORA_TIMEOUT;
               H[msg_dest].time = SHRT_MIN;
               H[msg_dest].originator_id = SHRT_MIN;
               H[msg_dest].r = SHRT_MIN;
               H[msg_dest].delta = SHRT_MIN ;
               H[msg_dest].ID =  radio().id();
            }

           //Initialize neighbors HEIGHT/destination
           for (neighborsIter = neighbors.begin(); neighborsIter != neighbors.end(); neighborsIter++) {
               if (!NH_exists(neighborsIter->first, msg_dest)) {
                   height tmp;
                   tmp.time = SHRT_MIN;
                   tmp.originator_id = SHRT_MIN;
                   tmp.r = SHRT_MIN;
                   tmp.delta = SHRT_MIN;
                   tmp.ID = neighborsIter->first;
                   NH[msg_dest][neighborsIter->first] = tmp;
                   LS[msg_dest][neighborsIter->first] = UN;
               }
           }

           //If destination node is neighbor set Height to Zero
           if (NH_exists(msg_dest, msg_dest)) {
               NH[msg_dest][msg_sour].time = 0;
               NH[msg_dest][msg_sour].originator_id = 0;
               NH[msg_dest][msg_sour].r = 0;
               NH[msg_dest][msg_sour].delta = 0;
               NH[msg_dest][msg_sour].ID = msg_sour;

           }

           //Update links
           if (NH_exists(msg_sour, msg_dest)) {
               if (H[msg_dest].delta == SHRT_MIN) {
                   if (NH[msg_dest][msg_sour].delta != SHRT_MIN) {
                       LS[msg_dest][msg_sour] = DN;
                   } else {
                       LS[msg_dest][msg_sour] = UN;
                   }
               } else {
                   if (NH[msg_dest][msg_sour].delta == SHRT_MIN) {
                       LS[msg_dest][msg_sour] = UN;
                   } else {
                       if (NH[msg_dest][msg_sour].delta > H[msg_dest].delta) {
                           LS[msg_dest][msg_sour] = UP;
                       } else if (NH[msg_dest][msg_sour].delta < H[msg_dest].delta) {
                           LS[msg_dest][msg_sour] = DN;
                       }
                   }
               }
           }

           DNlinks = count_DNs(msg_dest);

           cout << radio().id() << " : Received QRY - From: " << msg_sour << " Destination: " << msg_dest << " "<<DNlinks << endl;;
        //There are no downstream links
        if (DNlinks == 0) {
            //Checks if the RR flag is set
            if (!RR_flag_is_set(msg_dest)) {
                RR[msg_dest] = false;
            }
            if (RR[msg_dest] == false && H[msg_dest].delta == SHRT_MIN) {
                RR[msg_dest] = true;
                RoutingMessage QRYmsg;
                QRYmsg.set_msg_id(QRY);
                QRYmsg.set_destination(msg_dest);
                radio().send(radio().BROADCAST_ADDRESS, QRYmsg.buffer_size(), (uint8_t*) & QRYmsg);
                cout<<"\tRebroadcasting the QRY message"<<endl;
                destinations[msg_dest] = TORA_TIMEOUT;
            } else {
                //Discards the package
                cout << "\tQRY message discarded" << endl;
            }

        }//There is at least one downstream link
        else {
            destinations[msg_dest] = TORA_TIMEOUT;
            //if height is NULL
            if (heightIsNull(H[msg_dest])) {

                //finds the minimum height of its not-NULL neighbor
                int neighID = SHRT_MAX;
                int neighDelta = SHRT_MAX;
                for (NHIter = NH[msg_dest].begin(); NHIter != NH[msg_dest].end(); NHIter++) {
                    if (NHIter->second.delta != SHRT_MIN && NHIter->second.delta < neighDelta) {
                        neighID = NHIter->first;
                        neighDelta = NHIter->second.delta;
                    }
                }
                H[msg_dest].time = NH[msg_dest][neighID].time;
                H[msg_dest].originator_id = NH[msg_dest][neighID].originator_id;
                H[msg_dest].r = NH[msg_dest][neighID].r;
                H[msg_dest].delta = NH[msg_dest][neighID].delta + 1;

                cout << "\tDestination "
                        << NH[msg_dest][msg_dest].ID << " found " << endl;

                RoutingMessage UPDmsg;
                UPDmsg.set_msg_id(UPD);
                UPDmsg.set_destination(msg_dest);
                UPDmsg.set_height(H[msg_dest]);
                radio().send(radio().BROADCAST_ADDRESS, UPDmsg.buffer_size(), (uint8_t*) & UPDmsg);
                cout << "\tBroadcasting an UPD message " << endl;

            } else {
                cout << "\t***FIX THIS!!!" << endl;


            }
        }


    }
    // ----------------------------------------------------------------------

    template<typename OsModel_P,
    typename RoutingTable_P,
    typename Radio_P,
    typename Debug_P>
    bool
    ToraRouting<OsModel_P, RoutingTable_P, Radio_P, Debug_P>::NH_exists(int node_id, int msg_dest) {
        NHIter = NH[msg_dest].find(NeighborsHeight::key_type(node_id));
        if (NHIter != NH[msg_dest].end()) {
            return true;
        }
        return false;
    }

    // ----------------------------------------------------------------------

    template<typename OsModel_P,
    typename RoutingTable_P,
    typename Radio_P,
    typename Debug_P>
    bool
    ToraRouting<OsModel_P, RoutingTable_P, Radio_P, Debug_P>::neighbor_exists(int node_id) {
        neighborsIter = neighbors.find(map<int, int>::key_type(node_id));
        if (neighborsIter != neighbors.end()) {
            return true;
        }
        return false;
    }

    // ----------------------------------------------------------------------

    template<typename OsModel_P,
    typename RoutingTable_P,
    typename Radio_P,
    typename Debug_P>
    bool
    ToraRouting<OsModel_P, RoutingTable_P, Radio_P, Debug_P>::destination_exists(int node_id) {
        destinationsIter = destinations.find(node_id);
        if (destinationsIter != destinations.end()) {
            return true;
        }
        return false;
    }

    // ----------------------------------------------------------------------

    template<typename OsModel_P,
    typename RoutingTable_P,
    typename Radio_P,
    typename Debug_P>
    bool
    ToraRouting<OsModel_P, RoutingTable_P, Radio_P, Debug_P>::RR_flag_is_set(int node_id) {
        RRIter = RR.find(node_id);
        if (RRIter != RR.end()) {
            return true;
        }
        return false;
    }

    // ----------------------------------------------------------------------

    template<typename OsModel_P,
    typename RoutingTable_P,
    typename Radio_P,
    typename Debug_P>
    int
    ToraRouting<OsModel_P, RoutingTable_P, Radio_P, Debug_P>::count_DNs(int node_id) {
        int links = 0;
        for (LSIter = LS[node_id].begin(); LSIter != LS[node_id].end(); LSIter++) {
            if (LSIter->second == DN) {
                links++;
            }
        }
        return links;
    }

    // ----------------------------------------------------------------------

    template<typename OsModel_P,
    typename RoutingTable_P,
    typename Radio_P,
    typename Debug_P>
    void
    ToraRouting<OsModel_P, RoutingTable_P, Radio_P, Debug_P>::
    updateLS(int msg_dest) {
        int msg_sour;
        for (neighborsIter = neighbors.begin(); neighborsIter != neighbors.end(); neighborsIter++) {
            if ((neighborsIter->first == msg_dest) && (NH_exists(msg_dest, msg_dest))) {
                NH[msg_dest][msg_dest].time = 0;
                NH[msg_dest][msg_dest].originator_id = 0;
                NH[msg_dest][msg_dest].r = 0;
                NH[msg_dest][msg_dest].delta = 0;
                NH[msg_dest][msg_dest].ID = msg_dest;
                LS[msg_dest][msg_dest] = DN;
                cout<< "\tDN to "<< msg_dest << " --Destination : " <<  msg_dest <<endl;
            }
            else if (NH_exists(neighborsIter->first, msg_dest)) {
                msg_sour = neighborsIter->first;
                if (heightIsNull(NH[msg_dest][msg_sour]) && !heightIsNull(NH[msg_dest][msg_sour])) {
                    LS[msg_dest][msg_sour] = DN;
                     cout<< "\tDN to " << msg_sour << " --Destination : " <<  msg_dest <<endl;
                } else if (heightIsNull(NH[msg_dest][msg_sour]) && heightIsNull(NH[msg_dest][msg_sour])) {
                    LS[msg_dest][msg_sour] = UN;
                     cout<< "\tUP to " << msg_sour << " --Destination : " <<  msg_dest <<endl;
                } else {
                    if (NH[msg_dest][msg_sour].time == H[msg_dest].time) {
                        if (NH[msg_dest][msg_sour].originator_id == H[msg_dest].originator_id) {
                            if (NH[msg_dest][msg_sour].r == H[msg_dest].r) {
                                if (NH[msg_dest][msg_sour].delta >= H[msg_dest].delta) {
                                    LS[msg_dest][msg_sour] = UP;
                                    cout<< "\tUP to " << msg_sour << " --Destination : " <<  msg_dest <<endl;
                                } else if (NH[msg_dest][msg_sour].delta < H[msg_dest].delta) {
                                    LS[msg_dest][msg_sour] = DN;
                                    cout<< "\tDN to " << msg_sour << " --Destination : " <<  msg_dest <<endl;
                                }
                            } else {
                                if (NH[msg_dest][msg_sour].r >= H[msg_dest].r) {
                                        cout<< "\tUP to " << msg_sour << " --Destination : " <<  msg_dest <<endl;
                                        LS[msg_dest][msg_sour] = UP;
                                } else if (NH[msg_dest][msg_sour].r < H[msg_dest].r) {
                                    LS[msg_dest][msg_sour] = DN;
                                    cout<< "\tDN to " << msg_sour << " --Destination : " <<  msg_dest <<endl;
                                }
                            }
                        } else {
                            if (NH[msg_dest][msg_sour].originator_id > H[msg_dest].originator_id) {
                                    cout<< "\tUP to " << msg_sour << " --Destination : " <<  msg_dest <<endl;
                                    LS[msg_dest][msg_sour] = UP;
                            } else if (NH[msg_dest][msg_sour].originator_id < H[msg_dest].originator_id) {
                                LS[msg_dest][msg_sour] = DN;
                                cout<< "\tDN to " << msg_sour << " --Destination : " <<  msg_dest <<endl;
                            }
                        }
                    } else {
                        if (NH[msg_dest][msg_sour].time > H[msg_dest].time) {
                                cout<< "\tUP to " << msg_sour << " --Destination : " <<  msg_dest <<endl;
                                LS[msg_dest][msg_sour] = UP;
                        } else if (NH[msg_dest][msg_sour].time < H[msg_dest].time) {
                            LS[msg_dest][msg_sour] = DN;
                            cout<< "\tDN to " << msg_sour << " --Destination : " <<  msg_dest <<endl;
                        }
                    }
                }
            }
        }

    }
    // -----------------------------------------------------------------------

    template<typename OsModel_P,
    typename RoutingTable_P,
    typename Radio_P,
    typename Debug_P>
    bool
    ToraRouting<OsModel_P, RoutingTable_P, Radio_P, Debug_P>::
    ref_lvls_eq(int msg_dest) {
        height tmp;
        tmp.time = SHRT_MIN;
        tmp.originator_id = SHRT_MIN;
        tmp.r = SHRT_MIN;
        for (neighborsIter = neighbors.begin(); neighborsIter != neighbors.end(); neighborsIter++) {
            if (NH_exists(neighborsIter->first, msg_dest)) {
                if (tmp.time == SHRT_MIN) {
                    tmp.time = NH[msg_dest][neighborsIter->first].time;
                    tmp.originator_id = NH[msg_dest][neighborsIter->first].originator_id;
                    tmp.r = NH[msg_dest][neighborsIter->first].r;
                } else {
                    if (tmp.time != NH[msg_dest][neighborsIter->first].time
                            || tmp.originator_id != NH[msg_dest][neighborsIter->first].originator_id
                            || tmp.r != NH[msg_dest][neighborsIter->first].r) {
                        return false;
                    }
                }

            }
        }
        return true;

    }
    // ----------------------------------------------------------------------

    template<typename OsModel_P,
    typename RoutingTable_P,
    typename Radio_P,
    typename Debug_P>
    int
    ToraRouting<OsModel_P, RoutingTable_P, Radio_P, Debug_P>::
    find_min_ref_lvl(int msg_dest) {
        int tmp_time = SHRT_MIN;
        int neighID = SHRT_MIN;
        int tmp_d = SHRT_MAX;
        int tmp_oid = SHRT_MIN;
        int tmp_r = SHRT_MIN;
        for (NHIter = NH[msg_dest].begin(); NHIter != NH[msg_dest].end(); NHIter++) {
            if (NHIter->second.time != SHRT_MIN && NHIter->second.time > tmp_time) {
                neighID = NHIter->first;
                tmp_time = NHIter->second.time;
            }
        }
        for (NHIter = NH[msg_dest].begin(); NHIter != NH[msg_dest].end(); NHIter++) {
            if (NHIter->second.time == tmp_time && NHIter->second.originator_id > tmp_oid) {
                neighID = NHIter->first;
                tmp_oid = NHIter->second.originator_id;
            }
        }
        for (NHIter = NH[msg_dest].begin(); NHIter != NH[msg_dest].end(); NHIter++) {
            if (NHIter->second.time == tmp_time && NHIter->second.originator_id == tmp_oid && NHIter->second.r > tmp_r) {
                neighID = NHIter->first;
                tmp_r = NHIter->second.r;
            }
        }

        for (NHIter = NH[msg_dest].begin(); NHIter != NH[msg_dest].end(); NHIter++) {
            if (NHIter->second.time == tmp_time && NHIter->second.originator_id == tmp_oid && NHIter->second.r == tmp_r && NHIter->second.delta < tmp_d) {
                neighID = NHIter->first;

            }
        }
        return neighID;

    }
    // ----------------------------------------------------------------------

    template<typename OsModel_P,
    typename RoutingTable_P,
    typename Radio_P,
    typename Debug_P>
    bool
    ToraRouting<OsModel_P, RoutingTable_P, Radio_P, Debug_P>::
    r_eq_one(int msg_dest) {
        for (NHIter = NH[msg_dest].begin(); NHIter != NH[msg_dest].end(); NHIter++) {
            if (NHIter->second.r != 1) {
                return false;
            }
        }
        return true;
    }
    // ----------------------------------------------------------------------

    template<typename OsModel_P,
    typename RoutingTable_P,
    typename Radio_P,
    typename Debug_P>
    bool
    ToraRouting<OsModel_P, RoutingTable_P, Radio_P, Debug_P>::
    oid_eq_id(int msg_dest) {
        for (neighborsIter = neighbors.begin(); neighborsIter != neighbors.end(); neighborsIter++) {
            if (NH[msg_dest][neighborsIter->first].originator_id != radio().id()) {
                return false;
            }
        }

        return true;
    }
    // ----------------------------------------------------------------------

    template<typename OsModel_P,
    typename RoutingTable_P,
    typename Radio_P,
    typename Debug_P>
    void
    ToraRouting<OsModel_P, RoutingTable_P, Radio_P, Debug_P>::
    empty_buffer(int dest) {
        pend_msg_iter iter = pend_msgs.find(dest);
        if (iter != pend_msgs.end()) {
            int tmpH = SHRT_MAX;
            int next_node = SHRT_MAX;
             for (LSIter = LS[dest].begin(); LSIter != LS[dest].end(); LSIter++) {
                if (LSIter->second == DN){
                    if(NH[dest][LSIter->first].delta > 0 && NH[dest][LSIter->first].delta < tmpH){
                        tmpH = NH[dest][LSIter->first].delta;
                        next_node = LSIter->first;
                    }else if (NH[dest][LSIter->first].delta < 0){
                        next_node = LSIter->first;
                    }
                }
            }
            RoutingMessage DATAmsg;
            DATAmsg.set_msg_id(DATA);
            DATAmsg.set_next_node(next_node);
            DATAmsg.set_destination(dest);
            radio().send(radio().BROADCAST_ADDRESS, DATAmsg.buffer_size(), (uint8_t*) & DATAmsg);
            cout<< "\t\tSending a Data message with destination "<< dest << " to "<< next_node <<endl;
        }
         pend_msgs.erase(dest);
    }
    // ----------------------------------------------------------------------
    template<typename OsModel_P,
    typename RoutingTable_P,
    typename Radio_P,
    typename Debug_P>
    void
    ToraRouting<OsModel_P, RoutingTable_P, Radio_P, Debug_P>::
    clearDest() {
        for (destinationsIter = destinations.begin(); destinationsIter != destinations.end(); destinationsIter++) {
            destinationsIter->second--;
            if (destinationsIter->second <= 0) {
                NH.erase(destinationsIter->first);
                LS.erase(destinationsIter->first);
                RR.erase(destinationsIter->first);
                cout << radio().id() << " : Delete destination " << destinationsIter->first << endl;
                pend_msg_iter iter = pend_msgs.find(destinationsIter->first);
                if (iter != pend_msgs.end()) {
                    cout <<"\t Message to destination: " << destinationsIter->first << " could not be delivered... NO ROUTE found" << endl;
                    pend_msgs.erase(destinationsIter->first);
                }
                destinations.erase(destinationsIter->first);
            }
        }

    }
}
#endif
