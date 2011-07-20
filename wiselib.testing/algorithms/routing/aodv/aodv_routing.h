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
#ifndef __ALGORITHMS_ROUTING_AODV_ROUTING_H__
#define __ALGORITHMS_ROUTING_AODV_ROUTING_H__

#include "algorithms/routing/aodv/aodv_routing_types.h"
#include "algorithms/routing/aodv/aodv_route_discovery_msg.h"
#include "algorithms/routing/aodv/aodv_routing_msg.h"
#include "util/base_classes/routing_base.h"
#include <string.h>
#include "config.h"
#include <map>
#include <list>

#undef DEBUG
//#define DEBUG

#define NET_DIAM 10
#define ROUTE_TIMEOUT 2 *NET_DIAM

#define ECHO_INTERVAL 2
#define ALLOWED_LOSS ECHO_INTERVAL * 2


#define RETRY_INTERVAL NET_DIAM * 2
#define MAX_RETRIES 1
using namespace std;
namespace wiselib {

    /** \brief DSR routing implementation of \ref routing_concept "Routing Concept"
     *  \ingroup routing_concept
     *  \ingroup radio_concept
     *  \ingroup basic_algorithm_concept
     *  \ingroup routing_algorithm
     *
     * DSR routing implementation of \ref routing_concept "Routing Concept" ...
     */
    template<typename OsModel_P,
            typename RoutingTable_P,
            typename Radio_P = typename OsModel_P::Radio,
            typename Debug_P = typename OsModel_P::Debug>
            class AODVRouting
            : public RoutingBase<OsModel_P, Radio_P> {
    public:
        typedef OsModel_P OsModel;
        typedef Radio_P Radio;
        typedef Debug_P Debug;

        typedef typename OsModel_P::Timer Timer;

        typedef RoutingTable_P RoutingTable;
        typedef typename RoutingTable::iterator RoutingTableIterator;
        typedef typename RoutingTable::mapped_type RoutingTableValue;
        typedef typename RoutingTable::value_type RoutingTableEntry;
        typedef typename RoutingTableValue::Path Path;

        enum {
            MAX_PATH_LENGTH = RoutingTableValue::MAX_PATH_LENGTH
        };

        typedef AODVRouting<OsModel, RoutingTable, Radio, Debug> self_type;

        typedef typename Radio::node_id_t node_id_t;
        typedef typename Radio::size_t size_t;
        typedef typename Radio::block_data_t block_data_t;

        typedef typename Timer::millis_t millis_t;

        //typedef DsrRouteDiscoveryMessage<OsModel, Radio, Path> RouteDiscoveryMessage;
        typedef AODVRouteDiscoveryMessage<OsModel, Radio, Path> RouteDiscoveryMessage;
        typedef map<uint8_t, RouteDiscoveryMessage> pending_msgs_t;

        //typedef DsrRoutingMessage<OsModel, Radio, Path> RoutingMessage;

        //typedef AODVRo
        // --------------------------------------------------------------------
        enum SpecialNodeIds {
           BROADCAST_ADDRESS = Radio_P::BROADCAST_ADDRESS, ///< All nodes in communication range
           NULL_NODE_ID      = Radio_P::NULL_NODE_ID      ///< Unknown/No node id
        };
        // --------------------------------------------------------------------
        enum Restrictions {
           MAX_MESSAGE_LENGTH = Radio_P::MAX_MESSAGE_LENGTH - RouteDiscoveryMessage::PAYLOAD_POS ///< Maximal number of bytes in payload
        };
        // --------------------------------------------------------------------
        ///@name Construction / Destruction
        ///@{
        AODVRouting();
        ~AODVRouting();
        ///@}

        ///@name Routing Control
        ///@{
        void enable_radio(void);
        void disable_radio(void);
        ///@}

        ///@name Methods called by Timer
        ///@{
        void timer_elapsed(void *userdata);
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

        void proc_rreq(node_id_t from, RouteDiscoveryMessage& message);
        void proc_rrep(node_id_t from, RouteDiscoveryMessage& message);
        void proc_err(node_id_t from, RouteDiscoveryMessage& message);
        void proc_data(node_id_t from, RouteDiscoveryMessage& message);
        bool route_exists(node_id_t destination);
        void check_pending_messages(uint16_t destination);
        void table_cleanup();
        void init_path_disc();
        void resend_rreq(uint16_t dest);


        void pend_dest_cleanup();

        void broadcaster();
        void neighbors_cleanup();

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

        uint16_t my_seq_nr_;
        uint16_t my_bcast_id_;
        map<uint16_t, uint16_t> seq_numbers_;

        pending_msgs_t pending_msgs_;

        struct rreq_info {
            uint8_t source;
            uint8_t bcast_id;
        };

        bool route_found;

        list<struct rreq_info> received_rreq_;
        typename list<struct rreq_info>::iterator rreq_iter_;

        typename pending_msgs_t::iterator pend_iter_;

        map<uint16_t, uint8_t> neighbors_;

        typedef typename map<uint16_t, uint8_t>::iterator neighbors_iter_;

        struct retry_info {
            uint8_t retries;
            uint8_t time;

        };

        map<uint16_t, struct retry_info> pend_dests_;
        typedef typename map<uint16_t, struct retry_info>::iterator pend_dests_iter_;


        short seconds;
        int rounds;

        RoutingTable routing_table_;
    };
    // -----------------------------------------------------------------------
    // -----------------------------------------------------------------------
    // ----------------------------------------------------------------------- 
    // -----------------------------------------------------------------------
    // -----------------------------------------------------------------------


    // ********************* LOCAL CONNECTIVITY  **************************
    template<typename OsModel_P,
    typename RoutingTable_P,
    typename Radio_P,
    typename Debug_P>
    void
    AODVRouting<OsModel_P, RoutingTable_P, Radio_P, Debug_P>::
    broadcaster(){
        RouteDiscoveryMessage hello_message(
        HELO, /*msg type*/
        0, /*bcast id*/
        0, /*hop count*/
        0, /*source seq*/
        0,/*destination seq*/
        radio().id(), /*source*/
        0, /*destination*/
        0/*next hop*/);

        radio().send(radio().BROADCAST_ADDRESS, hello_message.buffer_size(), (uint8_t*) & hello_message);

    }


    template<typename OsModel_P,
    typename RoutingTable_P,
    typename Radio_P,
    typename Debug_P>
    void
    AODVRouting<OsModel_P, RoutingTable_P, Radio_P, Debug_P>::
    neighbors_cleanup(){

        //debug().debug("  %i is cleaning up tables\n",radio().id() );
        neighbors_iter_ n_it;
        //neighbors_iter_ n_it;
        n_it = neighbors_.begin();
        //it =routing_table_.end();
	//entries = 0;
        while(n_it != neighbors_.end()){
	//entries++;
            // decrease entry's lifetime and check if is stale


            if(n_it->second == 0){
               //debug().debug(" %i no longer neighbor with %i \n",radio().id(), n_it->first);

               neighbors_.erase(n_it);

               if (n_it != neighbors_.end()){
                   break;
               }


	   }
		else{
		n_it->second -= 1;

		}
		n_it++;
        }
	//debug().debug(" %i has %i entries \n",radio().id(),entries);

  }




    template<typename OsModel_P,
    typename RoutingTable_P,
    typename Radio_P,
    typename Debug_P>
    AODVRouting<OsModel_P, RoutingTable_P, Radio_P, Debug_P>::
    AODVRouting()
    : my_seq_nr_(0),
    my_bcast_id_(0)
    {};


    // -----------------------------------------------------------------------

    template<typename OsModel_P,
    typename RoutingTable_P,
    typename Radio_P,
    typename Debug_P>
    AODVRouting<OsModel_P, RoutingTable_P, Radio_P, Debug_P>::
    ~AODVRouting() {
#ifdef DEBUG
        debug().debug("AODVRouting Destroyed\n");
#endif
    };
    // -----------------------------------------------------------------------

    template<typename OsModel_P,
    typename RoutingTable_P,
    typename Radio_P,
    typename Debug_P>
    void
    AODVRouting<OsModel_P, RoutingTable_P, Radio_P, Debug_P>::
    enable_radio(void) {
#ifdef DEBUG
        debug().debug("AODVRouting Boots for %i\n", radio().id());
#endif
        my_seq_nr_ = 0;
        my_bcast_id_ = 0;
        seconds = 0;

        rounds = 0;

        radio().enable_radio();
        radio().template reg_recv_callback<self_type, &self_type::receive > (this);

        timer().template set_timer<self_type, &self_type::timer_elapsed > (
                1000, this, 0);
    }


    // -----------------------------------------------------------------------

    template<typename OsModel_P,
    typename RoutingTable_P,
    typename Radio_P,
    typename Debug_P>
    void
    AODVRouting<OsModel_P, RoutingTable_P, Radio_P, Debug_P>::
    disable_radio(void) {
#ifdef DEBUG
        debug().debug("Called AODVRouting::disable\n");
#endif
    }


    // -----------------------------------------------------------------------

    template<typename OsModel_P,
    typename RoutingTable_P,
    typename Radio_P,
    typename Debug_P>
    void
    AODVRouting<OsModel_P, RoutingTable_P, Radio_P, Debug_P>::
    timer_elapsed(void *userdata) {

       //     debug().debug(" %i setting\n ",radio().id());
        seconds++;
       //     debug().debug(" %i resetting 1\n  %i",radio().id(), seconds);
        if( seconds == ECHO_INTERVAL ){
       //         debug().debug(" %i resetting2\n ",radio().id());
            broadcaster();
        //        debug().debug("  %iresetting3\n ",radio().id());
            seconds = 0;
        //        debug().debug(" %i resetting4\n ",radio().id());
        }
        // clean up routing table
      // debug().debug(" %i table clean up ",radio().id());
        table_cleanup();
      // debug().debug(" done\n ");
        // clean up neighbors
 //             debug().debug("%i neighbors  ",radio().id());
        neighbors_cleanup();
      // debug().debug(" done\n ");
        // clean up pending destinations
     //  debug().debug(" %i dest clean up ",radio().id());
       pend_dest_cleanup();
//              debug().debug(" done\n ");

        //re-setting timer
        timer().template set_timer<self_type, &self_type::timer_elapsed > (
                1000, this, 0);
//    debug().debug(" resetting\n ");
    }


    // -----------------------------------------------------------------------

    template<typename OsModel_P,
    typename RoutingTable_P,
    typename Radio_P,
    typename Debug_P>
    bool
    AODVRouting<OsModel_P, RoutingTable_P, Radio_P, Debug_P>::
    route_exists(node_id_t destination) {
        RoutingTableIterator it = routing_table_.find(destination);
        if (it != routing_table_.end()) {
            return true;
        } else
            return false;

    };


    // -----------------------------------------------------------------------

    template<typename OsModel_P,
    typename RoutingTable_P,
    typename Radio_P,
    typename Debug_P>
    void
    AODVRouting<OsModel_P, RoutingTable_P, Radio_P, Debug_P>::
    check_pending_messages(uint16_t destination) {
        pend_iter_ = pending_msgs_.find(destination);
        if (pend_iter_ != pending_msgs_.end()) {
//            debug().debug("%i has pending msg for %i \n", radio().id(), destination);

            uint16_t next_hop = routing_table_[destination].next_hop;
            radio().send(next_hop, pend_iter_->second.buffer_size(), (uint8_t*) & pend_iter_->second);
            
//            debug().debug("%i sent stored msg to %i.next hop[%i] \n", radio().id(), destination, next_hop);
          routing_table_[destination].lifetime = ROUTE_TIMEOUT;


            //remove saved msg
        } else {
//            debug().debug("%i no msg for %i \n", radio().id(), destination);

        }

    }

    template<typename OsModel_P,
    typename RoutingTable_P,
    typename Radio_P,
    typename Debug_P>
    void
    AODVRouting<OsModel_P, RoutingTable_P, Radio_P, Debug_P>::
    resend_rreq(uint16_t dest){
        if(pend_dests_[dest].retries > MAX_RETRIES){
             debug().debug(" %i was unable to find route for %i \n",radio().id(), dest );
             // TODO delete f

            pend_dests_.erase(dest);

            pending_msgs_.erase(dest);


        }
        else{
            pend_dests_[dest].retries+=1;
            pend_dests_[dest].time = RETRY_INTERVAL;



             RouteDiscoveryMessage message(
                    RREQ, /*msg type*/
                    ++my_bcast_id_, /*bcast id*/
                    0, /*hop count*/
                    ++my_seq_nr_, /*source seq*/
                    seq_numbers_[dest]/*destination seq*/,
                    radio().id(), /*source*/
                    dest, /*destination*/
                    0/*next hop*/);


            radio().send(radio().BROADCAST_ADDRESS, message.buffer_size(), (uint8_t*) & message);

            // add my own rreq to received rreq's
            struct rreq_info own_rreq;
            own_rreq.source = radio().id();
            own_rreq.bcast_id = my_bcast_id_;
            received_rreq_.push_back(own_rreq);

            debug().debug("%i REsent RREQ for %i \n", radio().id(), dest);

        }


    }


    template<typename OsModel_P,
    typename RoutingTable_P,
    typename Radio_P,
    typename Debug_P>
    void
    AODVRouting<OsModel_P, RoutingTable_P, Radio_P, Debug_P>::
    pend_dest_cleanup(){

        //debug().debug("  %i is cleaning up tables\n",radio().id() );
        pend_dests_iter_ p_it;
        //neighbors_iter_ n_it;
        p_it = pend_dests_.begin();
        //it =routing_table_.end();
	int entries = 0;
        while(p_it != pend_dests_.end()){

            if( p_it->second.time > 0){
                p_it->second.time -= 1;
            }

             //if(p_it->second.time == 0 && p_it->second.retries > 3)
            else if (p_it->second.time == 0)   {

                pend_dests_.erase(p_it);


                pending_msgs_.erase(p_it->first);

                if(p_it == pend_dests_.end()){
                    break;
                }

              //  resend_rreq(p_it->first);
                //debug().debug(" %i found no route for %i \n",radio().id(), p_it->first );
                // TODO also remove from pending messages
            }


            else if(p_it->second.time == 0 && p_it->second.retries <= MAX_RETRIES){
                resend_rreq(p_it->first);
                
            }

           // debug().debug(" %i : %i,%i \n",radio().id(), p_it->second.time ,p_it->second.retries );

        p_it++;
        }





    }



// -----------------------------------------------------------------------
    template<typename OsModel_P,
    typename RoutingTable_P,
    typename Radio_P,
    typename Debug_P>
    void
    AODVRouting<OsModel_P, RoutingTable_P, Radio_P, Debug_P>::
    table_cleanup(){
	int entries=0;
        //debug().debug("  %i is cleaning up tables\n",radio().id() );
        RoutingTableIterator it;
        it = routing_table_.begin();
        //it =routing_table_.end();
	entries = 0;
        while(it != routing_table_.end()){
	entries++;
            // decrease entry's lifetime and check if is stale

    
            if(it->second.lifetime == 0){
               //debug().debug(" %i delete's entry for %i lifetime:%i \n",radio().id(), it->second.destination,it->second.lifetime );
               routing_table_.erase(it);
               if (it != routing_table_.end()){
                   break;
               }
	   }
		else{
		it->second.lifetime -= 1;
		
		}
		it++;
        }
	//debug().debug(" %i has %i entries \n",radio().id(),entries);
    }


// -----------------------------------------------------------------------

    template<typename OsModel_P,
    typename RoutingTable_P,
    typename Radio_P,
    typename Debug_P>
    void
    AODVRouting<OsModel_P, RoutingTable_P, Radio_P, Debug_P>::
    send(node_id_t destination, size_t len, block_data_t *data) {
        // check for path
        if (route_exists(destination)) {

            // update ttl
            routing_table_[destination].lifetime = ROUTE_TIMEOUT;

            // get next hop
            uint8_t next_hop = routing_table_[destination].next_hop;
            debug().debug("%i found route for %i. next hop:[%i] %i hops away\n", radio().id(), destination, next_hop,routing_table_[destination].hop_cnt);

            RouteDiscoveryMessage encap_message(
                    DATA, /*msg type*/
                    0, /*bcast id*/
                    0, /*hop count*/
                    0, /*source seq*/
                    seq_numbers_[destination]/*destination seq*/,
                    radio().id(), /*source*/
                    destination, /*destination*/
                    0/*next hop*/);
            // TODO do actual encapsulation :)

            // forward data msg to next hop
            radio().send(next_hop, encap_message.buffer_size(), (uint8_t*) & encap_message);
            //update lifetime
            routing_table_[destination].lifetime = ROUTE_TIMEOUT;
            return;
        } else if (false/*destination is neighbor*/) {

            RouteDiscoveryMessage encap_message(
                    DATA, /*msg type*/
                    0, /*bcast id*/
                    0, /*hop count*/
                    0, /*source seq*/
                    seq_numbers_[destination]/*destination seq*/,
                    radio().id(), /*source*/
                    destination, /*destination*/
                    0/*next hop*/);
            // TODO do actual encapsulation :)

            // forward data msg to next hop
            radio().send(destination, encap_message.buffer_size(), (uint8_t*) & encap_message);
            ;
        }// init path disc
        else {
            debug().debug("%i Starting path discovery \n", radio().id());
            //block_data_t tmp[len];
            //memcpy(tmp, data, len);

            RouteDiscoveryMessage stored_message(
                    DATA, /*msg type*/
                    0, /*bcast id*/
                    0, /*hop count*/
                    0, /*source seq*/
                    seq_numbers_[destination]/*destination seq*/,
                    radio().id(), /*source*/
                    destination, /*destination*/
                    0/*next hop*/);
            //TODO add actual data and timeout to this message

            pending_msgs_[destination] = stored_message;
//            debug().debug("%i stored message %i to buffer \n", radio().id(), stored_message.msg_type());

            //TODO add new entry to pending_dest
            struct retry_info retry_entry;
            retry_entry.retries  = 1;
            retry_entry.time = RETRY_INTERVAL;
            pend_dests_[destination] = retry_entry;


            RouteDiscoveryMessage message(
                    RREQ, /*msg type*/
                    ++my_bcast_id_, /*bcast id*/
                    0, /*hop count*/
                    ++my_seq_nr_, /*source seq*/
                    seq_numbers_[destination]/*destination seq*/,
                    radio().id(), /*source*/
                    destination, /*destination*/
                    0/*next hop*/);


            radio().send(radio().BROADCAST_ADDRESS, message.buffer_size(), (uint8_t*) & message);

            // add my own rreq to received rreq's
            struct rreq_info own_rreq;
            own_rreq.source = radio().id();
            own_rreq.bcast_id = my_bcast_id_;
            received_rreq_.push_back(own_rreq);

            debug().debug("%i sent RREQ for %i \n", radio().id(), destination);

        }
    }

    //---------------------------------------------------------------------

    template<typename OsModel_P,
    typename RoutingTable_P,
    typename Radio_P,
    typename Debug_P>
    void
    AODVRouting<OsModel_P, RoutingTable_P, Radio_P, Debug_P>::
    proc_data(node_id_t from, RouteDiscoveryMessage& message) {
        debug().debug("%i received DATA message from %i \n", radio().id(), message.source());
        // msg for me
        if (message.destination() == radio().id()) {
            debug().debug("%i got his DATA message msg from %i \n", radio().id(), message.source());

        }            // destination is my neighbor
        else if (false) {
            ;

        }            // i have route to destination
        else if (route_exists(message.destination())) {
            debug().debug("%i forwards DATA message for %i. next hop [%i] %i hops away \n", radio().id(), message.destination(), routing_table_[message.destination()].next_hop, routing_table_[message.destination()].hop_cnt);
            //TODO update lifetime


            //forward message to next hop
            radio().send(routing_table_[message.destination()].next_hop, message.buffer_size(), (uint8_t*) & message);
            routing_table_[message.destination()].lifetime = ROUTE_TIMEOUT;
        } else {
            debug().debug("%i ERROR: no route for \n", radio().id(), message.destination());

        }

    }


    // -----------------------------------------------------------------------

    template<typename OsModel_P,
    typename RoutingTable_P,
    typename Radio_P,
    typename Debug_P>
    void
    AODVRouting<OsModel_P, RoutingTable_P, Radio_P, Debug_P>::
    proc_rreq(node_id_t from, RouteDiscoveryMessage& message) {
        //debug().debug("%i received RREQ from: %i. src:%i dst:%i bcast:%i hops:%i\n", radio().id(), from, message.source(), message.destination(), message.bcast_id(),message.hop_cnt());

        // drop any reduntant messages
        for (rreq_iter_ = received_rreq_.begin(); rreq_iter_ != received_rreq_.end(); ++rreq_iter_) {
            if ((*rreq_iter_).source == message.source()
                    && (*rreq_iter_).bcast_id == message.bcast_id()) {
                //cout << "\t\t\tDROPPED\n";
                return;
            }
        }

        // Continue with rreq processing
        // Record highest seq num for destination
        if (message.source_sequence_nr() - seq_numbers_[message.source()] > 0) {
            seq_numbers_[message.source()] = message.source_sequence_nr();
        }

        // add rreq to received rreq's
        struct rreq_info tmp_info;
        tmp_info.source = message.source();
        tmp_info.bcast_id = message.bcast_id();
        //tmp_info.lifetime = INT_MAX;
        received_rreq_.push_back(tmp_info);


        // add REVERSE path entry
        RoutingTableValue value;

        value.destination = message.source();
        value.next_hop = from;
        value.hop_cnt = message.hop_cnt() + 1;
        value.dest_seq = message.destination_sequence_nr();
        value.lifetime = ROUTE_TIMEOUT;
        routing_table_[message.source()] = value;

/*        debug().debug("%i added route entry %i:%i:%i:%i \n", radio().id(),
                routing_table_[message.source()].destination,
                routing_table_[message.source()].next_hop,
                routing_table_[message.source()].hop_cnt,
                routing_table_[message.source()].lifetime);*/

        // check if there are any pending messages for newly added entry
        check_pending_messages(message.source());

        //check if i have route for this destination
        RoutingTableIterator it = routing_table_.find(message.destination());
        if (it != routing_table_.end()) {
            debug().debug("%i found route for %i via %i, %i hops away\n", radio().id(), message.destination(), it->second.next_hop, it->second.hop_cnt);
            //debug().debug("%i route found.\n replying with RREP to %i \n", radio().id(), message.source());
            //send a route reply
            RouteDiscoveryMessage rrep_message(
                    RREP, /*msg type*/
                    0, /*bcast id*/
                    it->second.hop_cnt, /*hop cnt*/
                    seq_numbers_[message.source()], /* source seq nr*/
                    0/* dest seq nr*/,
                    radio().id(), /*source */
                    message.source(), /*destination*/
                    from/*next hop*/);
            radio().send(from, rrep_message.buffer_size(), (uint8_t*) & rrep_message);

            return;
        }

        // i am the destination
        if (message.destination() == radio().id()) {
            //debug().debug("%i replying with RREP to %i \n", radio().id(), message.source());
            // create a rrep message
            RouteDiscoveryMessage rrep_message(
                    RREP, /*msg type*/
                    0, /*bcast id*/
                    0, /*hop cnt*/
                    //TODO increase?
                    ++my_seq_nr_, /* source seq nr*/
                    seq_numbers_[message.source()]/* dest seq nr*/,
                    radio().id(), /*source */
                    message.source(), /*destination*/
                    from/*next hop*/);

            // send rrep
            radio().send(from, rrep_message.buffer_size(), (uint8_t*) & rrep_message);

        }            // no route. re-broadcast
        else {
            debug().debug("%i forwarding RREQ from %i\n", radio().id(), message.source());
            RouteDiscoveryMessage rreq_message(
                    RREQ, /*msg type*/
                    message.bcast_id(), /*bcast id*/
                    message.hop_cnt() + 1, /*hop cnt*/
                    my_seq_nr_, /* source seq nr*/
                    seq_numbers_[message.source()]/* dest seq nr*/,
                    message.source(), /*source */
                    message.destination(), /*destination*/
                    0/*bcast*/);
            radio().send(radio().BROADCAST_ADDRESS, message.buffer_size(), (uint8_t*) & rreq_message);

        }
        return;
    }


    // -----------------------------------------------------------------------

    template<typename OsModel_P,
    typename RoutingTable_P,
    typename Radio_P,
    typename Debug_P>
    void
    AODVRouting<OsModel_P, RoutingTable_P, Radio_P, Debug_P>::
    proc_rrep(node_id_t from, RouteDiscoveryMessage& message) {
        debug().debug("%i received RREP from %i via %i\n", radio().id(), message.source(), from, message.hop_cnt());

        // Record highest seq number
        if (message.source_sequence_nr() - seq_numbers_[message.source()] > 0) {
            seq_numbers_[message.source()] = message.source_sequence_nr();
        }

        // add to route entry only if this is the first entry OR rrep proposes a better path
        // or rrep is fresher
        if (!route_exists(message.source())
                || routing_table_[message.destination()].dest_seq < message.destination_sequence_nr()
                || (routing_table_[message.destination()].hop_cnt > message.hop_cnt()
                && routing_table_[message.destination()].dest_seq < message.destination_sequence_nr())) {

            RoutingTableValue value;

            value.destination = message.source();
            value.next_hop = from;
            value.hop_cnt = message.hop_cnt() + 1;
            value.dest_seq = seq_numbers_[message.destination()];
            value.lifetime = ROUTE_TIMEOUT;
            routing_table_[message.source()] = value;
/*            debug().debug("%i added route entry %i:%i:%i:%i \n", radio().id(),
                    routing_table_[message.source()].destination,
                    routing_table_[message.source()].next_hop,
                    routing_table_[message.source()].hop_cnt,
                    routing_table_[message.source()].lifetime);*/


            // check if there are any pending messages for this target
            check_pending_messages(message.source());

        }
        // this rrep is not for me
        if (message.destination() != radio().id()) {
            // unicast using reverse path
            RouteDiscoveryMessage rrep_message(
                    RREP, /*msg type*/
                    message.bcast_id(), /*bcast id*/
                    message.hop_cnt() + 1, /*hop cnt*/
                    my_seq_nr_, /* source seq nr*/
                    seq_numbers_[message.source()]/* dest seq nr*/,
                    message.source(), /*source */
                    message.destination(), /*destination*/
                    0/*bcast*/);

            radio().send(routing_table_[message.destination()].next_hop, rrep_message.buffer_size(), (uint8_t*) & rrep_message);
        }

        return;
    }


    // -----------------------------------------------------------------------

    template<typename OsModel_P,
    typename RoutingTable_P,
    typename Radio_P,
    typename Debug_P>
    void
    AODVRouting<OsModel_P, RoutingTable_P, Radio_P, Debug_P>::
    proc_err(node_id_t from, RouteDiscoveryMessage& message) {
        debug().debug("%i received ERROR from %i\n", radio().id(), from);
        return;
    }


    // -----------------------------------------------------------------------

    template<typename OsModel_P,
    typename RoutingTable_P,
    typename Radio_P,
    typename Debug_P>
    void
    AODVRouting<OsModel_P, RoutingTable_P, Radio_P, Debug_P>::
    receive(node_id_t from, size_t len, block_data_t *data) {
        if (from != radio().id()) {
            uint8_t msg_type = *data;
            //debug().debug("%i received msg %i \n", radio().id(), msg_type);
            switch (msg_type) {
                case HELO:
                {
                    // update sender's TTL
                    neighbors_[from] = ALLOWED_LOSS;
                    //debug().debug("%i received beacon from %i\n", radio().id(), from);
                    break;
                }

                case RREQ:
                {
                    RouteDiscoveryMessage *message = reinterpret_cast<RouteDiscoveryMessage*> (data);
                    proc_rreq(from, *message);
                    break;
                }
                case RREP:
                {
                    RouteDiscoveryMessage *message = reinterpret_cast<RouteDiscoveryMessage*> (data);
                    proc_rrep(from, *message);
                    break;
                }
                case ERR:
                {
                    //proc_err(from, *data);
                    break;
                }
                case DATA:
                {
                    RouteDiscoveryMessage *message = reinterpret_cast<RouteDiscoveryMessage*> (data);
                    proc_data(from, *message);
                    break;
                }
                default:
                {
                    debug().debug("%i received UNRECOGNIZED message type [%i]\n", radio().id(), msg_type);

                }


            }

        }

    }


}
#endif
