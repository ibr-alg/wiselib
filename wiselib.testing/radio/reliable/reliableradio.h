/*
 * File:   reliableradio.h
 * Author: amaxilatis
 *
 * Created on July 27, 2010
 */

#ifndef _RELIABLERADIO_H
#define	_RELIABLERADIO_H

#include "util/pstl/vector_static.h"
#include "util/pstl/map_static_vector.h"
#include "util/pstl/pair.h"
#include "util/delegates/delegate.hpp"
#include "util/base_classes/radio_base.h"
#include "reliablemsg.h"

//#include "communication/reliablemsg.h"

/*
 * DEBUG MESSAGES TEMPLATE
 * RR;<task> [ type= ...]
 *
 * */
//#define DEBUG_RELIABLERADIO
//#define EXTRA

#define MAX_PENDING 10	// Maximum number of pending for delivery messages
#define MAX_CONNECTIONS 20
#define MAX_RECV 15

namespace wiselib {

    /*
     * ReliableRadio
     * Uses OsModel, Radio and Timer
     * Debug is only for debugging
     * ReliableRadio is used as a layer between normal radio
     * and application to make sure that messages will be
     * delivered besides any errors that may occur.
     * */
    template<typename OsModel_P,
    typename Radio_P,
    typename Timer_P,
    typename Debug_P>
    class ReliableRadio
    : public RadioBase<OsModel_P,
    typename Radio_P::node_id_t,
    typename Radio_P::size_t,
    typename Radio_P::block_data_t> {
    public:
        // Type definitions
        typedef OsModel_P OsModel;

        typedef Radio_P Radio;
        typedef typename Radio::node_id_t node_id_t;
        typedef typename Radio::size_t size_t;
        typedef typename Radio::block_data_t block_data_t;
        typedef typename Radio::message_id_t message_id_t;

        typedef ReliableMsg< OsModel, Radio> ReliableMessage_t; // a Reliable message with some extra headers for delivery checks

        typedef delegate4<void, uint8_t, node_id_t, size_t, block_data_t*> event_notifier_delegate_t;

        //contains information about the messages pending delivery
        struct pending_messages_entry {
            uint16_t seq_no;
            node_id_t destination;
            int timestamp;
            int retries;
            event_notifier_delegate_t event_notifier_callback;
            ReliableMessage_t msg;
        }; // stores information about the status of a sent message

        typedef struct pending_messages_entry pending_messages_entry_t;
        typedef Debug_P Debug;
        typedef Timer_P Timer;

        // Vector Storing all messages sent but receive not confirmed yet
        typedef wiselib::vector_static<OsModel, pending_messages_entry_t, MAX_PENDING> pending_messages_vector_t;

        typedef ReliableRadio<OsModel_P, Radio_P, Timer_P, Debug_P> self_t;

        typedef uint16_t seqNo_t;


        // --------------------------------------------------------------------

        enum Events {
            MSG_DROPPED = 1,
            MSG_ACK_RCVD = 2
        };

        enum SpecialNodeIds {
            BROADCAST_ADDRESS = Radio::BROADCAST_ADDRESS, //< All nodes in communication rnage
            NULL_NODE_ID = Radio::NULL_NODE_ID //< Unknown/No node id
        };
        // --------------------------------------------------------------------

        enum Restrictions {
            MAX_MESSAGE_LENGTH = ReliableMessage_t::MAX_MESSAGE_LENGTH            //< Maximal number of bytes in payload
        };
        // --------------------------------------------------------------------

        void init(Radio& radio, Timer& timer, Debug& debug) {
            radio_ = &radio;
            timer_ = &timer;
            debug_ = &debug;


            // initialize the Vector to empty ( seq_no=0 means empty)
            pending_messages_.clear();
            open_connections.clear();
        };
        // --------------------------------------------------------------------

        ReliableRadio():
        max_retries_(5){
        }
        // --------------------------------------------------------------------

        ~ReliableRadio() {
        }
        // --------------------------------------------------------------------

        void enable_radio() {
#ifdef DEBUG_RELIABLERADIO
            debug().debug("RR;enable");
#endif
            radio().enable_radio();

            radio().template reg_recv_callback<self_t, &self_t::receive > (this);
           
            // timestamps start from 0
            current_time_ = 0;
            // set timer to change time
            timer().template set_timer<self_t, &self_t::time_passes > (
                    time_slice_, this, (void*) 0);
            // set timer to wake up realiable_deamon ( checks for undelivered messages and resends them )
            timer().template set_timer<self_t, &self_t::reliable_daemon > (
                    sleep_time_, this, (void*) 0);
        }

        // --------------------------------------------------------------------

        void disable_radio() {
            radio().unreg_recv_callback(recv_callback_id_);
        };

        int send(node_id_t id, size_t len, block_data_t *data) {
            
            if (id == BROADCAST_ADDRESS) {
                // create a message to host the data
                ReliableMessage_t m;
                m.set_msg_id(ReliableMessage_t::BROADCAST_MESSAGE);
                m.set_seq_number(0);
                m.set_payload(len, data);
                // get the real payload to send

#ifdef DEBUG_RELIABLERADIO
                debug().debug("RR;send [ type= %d(broad) size= %d]\n", m.msg_id(), m.buffer_size());
#endif

                // send a broadcast message
                radio().send(id, m.buffer_size(), (uint8_t *) & m);
                return -1;
            }/*
             * ADD message to Vector
             * CREATE a special message containing the original
             * SEND through normal radio
             * WAIT for ack or reliability_daemon to resend it
             * */
            else {

                if (pending_messages_.size() != pending_messages_.max_size()) {

                    seqNo_t next_seq_no = next_seq(id);

                    debug().debug("RR;next_seq_no=%d", next_seq_no);

                    if (next_seq_no != 0) {
                        pending_messages_entry_t newmessage;

                        newmessage.seq_no = next_seq_no;
                        newmessage.destination = id;
                        newmessage.timestamp = current_time_;
                        newmessage.retries = 0;
                        // Store the message and set its headers
                        newmessage.msg.set_msg_id(ReliableMessage_t::RELIABLE_MESSAGE);
                        newmessage.msg.set_seq_number(next_seq_no);
                        newmessage.msg.set_payload(len, data);

                        pending_messages_.push_back(newmessage);
                        debug().debug("RR;added pending message");


#ifdef DEBUG_RELIABLERADIO
                        debug().debug("RR;send;type=%d;dest=%x;seq_no=%d;size=%d;", ReliableMessage_t::RELIABLE_MESSAGE, id, newmessage.msg.seq_number(), newmessage.msg.buffer_size());
#endif
                        radio().send(id, newmessage.msg.buffer_size(), (uint8_t *) & newmessage.msg);
                    }
                } else {
#ifdef DEBUG_RELIABLERADIO
                    debug().debug("RR;error;vector_full");
#endif
                }// Get message from the Vector and send it through normal radio

            }
            return 1;
        }

        // --------------------------------------------------------------------

        // Increment time by 1

        void time_passes(void *) {
            current_time_++;
            //Debug::debug(os(),"Time is now %d\n",current_time_);
            timer().template set_timer<self_t, &self_t::time_passes > (
                    time_slice_, this, (void*) 0);
        }

        // --------------------------------------------------------------------

        /*
         * Reliable daemon
         * Check to see if any messages are sent
         * but not acked so to send them again.
         *
         *      check by timestamps
         *          if time for ack receive has passed
         *      and retries
         *          if maximum retries reached abort
         *
         * Reset timer for daemon
         */

        void reliable_daemon(void *) {
            bool found = false; // true if a message is to be resent
            for (typename pending_messages_vector_t::iterator it = pending_messages_.begin(); it != pending_messages_.end(); ++it) {
                if (it->seq_no != 0) { // only for valid entries : 0 means no message
                    if (it->timestamp < current_time_ - 2) { // 2 time units needed for message delivery
                        if (it->retries < max_retries_) { // if max retries reached
                            // a message is to be resend
                            it->timestamp = current_time_; // refresh its sent timestamp

                            size_t mess_size = it->msg.buffer_size();

                            mess_size = mess_size;

                            // resend the message
                            radio().send(
                                    it->destination,
                                    it->msg.buffer_size(),
                                    (uint8_t *) & it->msg);
                            // increase by 1 retries
                            it->retries++;

#ifdef DEBUG_RELIABLERADIO
                            debug().debug("RR;resend;RELIABLE_MESSAGE;%x;%d;%d", it->destination, it->seq_no, it->msg.buffer_size());
#endif
                        } else { // if max retries reached abort sending

#ifdef DEBUG_RELIABLERADIO
#ifdef SHAWN

                            debug().debug("RR;abort [ type= %d(reliable) destination= %x seq_no= %d  max_retries]", ReliableMessage::RELIABLE_MESSAGE,
                                    pending_messages_[i].first.destination,
                                    pending_messages_[i].first.seq_no);
#else
                            debug().debug("RR;abort;RELIABLE_MESSAGE;%d;%d;max_retries", it->destination, it->seq_no);
#endif
#endif
                            if (it->event_notifier_callback != event_notifier_delegate_t()) {
                                it->event_notifier_callback(MSG_DROPPED,
                                        it->destination,
                                        it->msg.buffer_size(),
                                        (uint8_t *) & it->msg);
                            }
                            pending_messages_.erase(it);

                            //                            // clear vector entry to be reused
                            //                            it->event_notifier_callback = event_notifier_delegate_t();
                            //                            it->seq_no = 0;
                            it--;
                        }
                        // a message was found
                        found = true;
                    }
                }
            }

#ifdef DEBUG_RELIABLERADIO
            if (!found) {
                //                Debug::debug(os(), "RR;info No pending messages\n");
            }
#endif
            // Reset the timer
            timer().template set_timer<self_t, &self_t::reliable_daemon > (
                    sleep_time_, this, (void*) 0);
        }

        // find a free entry inside the pending messages vector

        //        int find_postition() {
        //            for (int i = 0; i < MAX_PENDING; i++) {
        //                if (pending_messages_[i].first.seq_no == 0) { // seq_no : 0 means no message
        //                    return i;
        //                } else {
        //                }
        //            }
        //            return -1;
        //        }
        //        ;

        // --------------------------------------------------------------------

        /*
         * Callback from the Radio module
         *
         * when a new message is received check its type:
         * - Ack_Message : mark as received all containing sequence numbers
         * - Broadcast message : send to the application
         * - ReliableMessage : send ack , and forward message to the application
         *
         *
         */
        void receive(node_id_t from, size_t len, block_data_t * data) {

#ifdef SHAWN
            // do not receive own messages
            if (radio().id() == from) {
                return;
            }
#endif
            // if a connection can be opened
            if (check_connection(from)) {

                // check the message id
                if (data[0] == ReliableMessage_t::ACK_MESSAGE) { // if an ack message                    
                    handle_ack_message((ReliableMessage_t *) data, from);
                } else if (data[0] == ReliableMessage_t::RELIABLE_MESSAGE) {
                    handle_reliable_message((ReliableMessage_t *) data, from);
                } else if (data[0] == ReliableMessage_t::BROADCAST_MESSAGE) { // if a broadcast message was received forward to the application
#ifdef DEBUG_RELIABLERADIO
                    debug().debug("RR;receive;BROADCAST_MESSAGE;%x", from);
#endif

                    ReliableMessage_t * recvm = (ReliableMessage_t *) data;

                    // extract payload from the message received
                    block_data_t msg_recv[recvm->payload_size()];
                    memcpy(msg_recv, recvm->payload(), recvm->payload_size());

                    // forward payload to the application
                    //message_received_callback_(from, recvm.payload_size(), msg_recv);
                    notify_receivers(from, recvm->payload_size(), msg_recv);

                } else {
#ifdef DEBUG_RELIABLERADIO
                    debug().debug("RR;error Unsupported message Received from %x", from);
#endif
                }
            }
        }

        void handle_ack_message(ReliableMessage_t * ackmess, node_id_t from) {
#ifdef DEBUG_RELIABLERADIO
            debug().debug("RR;receive;ACK_MESSAGE;%d;%d", from, ackmess->payload_size());
#endif

            uint8_t seq_nos[ackmess->payload_size()];
            memcpy(seq_nos, ackmess->payload(), ackmess->payload_size());
            // get sequence from the message and remove the respective messages from the list
            for (size_t j = 0; j < ackmess->payload_size(); j += 2) {
                uint16_t seq_no_acked = seq_nos[j] + seq_nos[j + 1]*256;
                // search the whole vector
                for (typename pending_messages_vector_t::iterator it = pending_messages_.begin(); it != pending_messages_.end(); ++it) {
                    // if the sequence numbers match
                    if (it->seq_no == seq_no_acked) {
                        // if the destinations match
                        if (it->destination == from) {
                            // remove entry
#ifdef DEBUG_RELIABLERADIO
                            debug().debug("RR;receive seq_no_acked=%d", seq_no_acked);
#endif
                            if (it->event_notifier_callback != event_notifier_delegate_t()) {
                                it->event_notifier_callback(MSG_ACK_RCVD,
                                        it->destination,
                                        it->msg.buffer_size(),
                                        (uint8_t *) & it->msg);
                            }

                            pending_messages_.erase(it);
                            it--;
                        }
                    }
                }
            }
        }

        void handle_reliable_message(ReliableMessage_t * relmess, node_id_t from) {
            // get sequence number from the message
            uint16_t curr_seq_no = relmess->seq_number();
            // check if the message exists in cache
            bool first_time = was_received(from, curr_seq_no);

#ifdef DEBUG_RELIABLERADIO
            debug().debug("RR;receive;RELIABLE_MESSAGE;%x;%d;%d", from, curr_seq_no, first_time);
#endif
            if (first_time) {

                // extract the real payload from the message
                block_data_t msg_striped[relmess->payload_size()];
                memcpy(msg_striped, relmess->payload(), relmess->payload_size());

                // forward the payload to the application
                notify_receivers(from, relmess->payload_size(), msg_striped);


                // create the new ack message to send
                ReliableMessage_t ackm;
                ackm.set_msg_id(ReliableMessage_t::ACK_MESSAGE);
                ackm.set_seq_number(0);
                set_ack_list(&ackm, from);
                // send the ack message
                radio().send(from, ackm.buffer_size(), (uint8_t *) & ackm);
#ifdef DEBUG_RELIABLERADIO
                debug().debug("RR;send;ACK_MESSAGE;%x;%d", from, ackm.buffer_size());
#endif
#ifdef EXTRA
                block_data_t ack[ackm.payload_size()];
                memcpy(ack, &ackm, ackm.payload_size());
                for (int i = 0; i < ackm.payload_size(); i += 2) {
                    debug().debug(" %d", ack[i] + ack[i + 1]);

                }
#endif
            } else {// if the sequence number exists in cache then just send an ack message ( message was forwarded before to the application
                // create the new ack message to send
                ReliableMessage_t ackm;
                ackm.set_msg_id(ReliableMessage_t::ACK_MESSAGE);
                ackm.set_seq_number(0);
                set_ack_list(&ackm, from);
                // send the ack message
                radio().send(from, ackm.buffer_size(), (uint8_t *) & ackm);
#ifdef DEBUG_RELIABLERADIO
                debug().debug("RR;resend;ACK_MESSAGE;%x;%d", from, ackm.buffer_size());
#endif
#ifdef EXTRA
                block_data_t ack[ackm.payload_size()];
                memcpy(ack, &ackm, ackm.payload_size());
                for (int i = 0; i < ackm.payload_size(); i += 2) {
                    debug().debug(" %d", ack[i] + ack[i + 1]);

                }
#endif
            }
        }

        void set_ack_list(ReliableMessage_t * m, node_id_t sender) {

            uint8_t seq_nos[open_connections[sender].received_seqs_.size()];
            int i = 0;
            for (received_seqs_iterator_t it = open_connections[sender].received_seqs_.begin(); it != open_connections[sender].received_seqs_.end(); ++it) {
                seq_nos[2 * i] = *it % 256;
                seq_nos[2 * i + 1] = *it / 256;
                i++;
            }
            m->set_payload(open_connections[sender].received_seqs_.size() * 2, seq_nos);

        }

        // --------------------------------------------------------------------

        //        // Check if a connection with the sender was established before
        //

        bool check_connection(node_id_t sender) {
            bool exists = (open_connections.contains(sender));
            if (!exists) {
                return add_connection(sender);
            } else {
                return exists;
            }
        }

        seqNo_t next_seq(node_id_t destination) {
            if (open_connections.contains(destination)) {
                return open_connections[destination].sequence_numbers_++;
            } else {
                newconn_t newconnection;
                newconnection.first = destination;
                newconnection.second.received_seqs_.clear();
                newconnection.second.sequence_numbers_ = 2;
                open_connections.push_back(newconnection);
                return 1;
            }
            return 0;
        }

        // --------------------------------------------------------------------

        bool add_connection(node_id_t node) {
            if (open_connections.contains(node)) {
                return false;
            } else {
                newconn_t newconnection;
                newconnection.first = node;
                newconnection.second.received_seqs_.clear();
                newconnection.second.sequence_numbers_ = 1;
                open_connections.push_back(newconnection);
            }
            return true;
        }


        // check sequence numbers cache to see if the message with this sequence number was previously received

        bool was_received(node_id_t sd, seqNo_t seq_no) {
            bool first_time = true; // true if the first time received
            if (open_connections.contains(sd)) {
                for (received_seqs_iterator_t it = open_connections[sd].received_seqs_.begin(); it != open_connections[sd].received_seqs_.end(); ++it) {
                    if (*it == seq_no) {
                        first_time = false;
                        break;
                    }
                }
                if (first_time) {
                    //if reached the max seq numbers delete the oldes
                    if (open_connections[sd].received_seqs_.size() == open_connections[sd].received_seqs_.max_size()) {
                        //                        debug().debug("exeeded max list size");
                        open_connections[sd].received_seqs_.erase(open_connections[sd].received_seqs_.begin());
                    }
                    //                    debug().debug("add %d to existing seq nos", seq_no);
                    open_connections[sd].received_seqs_.push_back(seq_no);
                }
                return first_time;
            }
            return first_time;
        }

        void set_max_retries(int max_retries) {
            max_retries_ = max_retries;
            //            debug().debug("RR;set max_retries= %d\n", max_retries);
        };

        int max_retries() {
            return max_retries_;
        };
        // --------------------------------------------------------------------
        // returns the node's id

        node_id_t id() {
            return radio().id();
        };
        // --------------------------------------------------------------------

    private:
        int recv_callback_id_;
        uint16_t current_time_; //
        pending_messages_vector_t pending_messages_;
        static const int time_slice_ = 200; // time_passes delay
        static const int sleep_time_ = 1000; // reliable_daemon's sleep time

        typedef typename wiselib::vector_static<OsModel, seqNo_t, MAX_RECV> received_seqs_vector_t;
        typedef typename received_seqs_vector_t::iterator received_seqs_iterator_t;

        struct connections {
            received_seqs_vector_t received_seqs_;
            uint16_t sequence_numbers_;
        };
        typedef struct connections connection_entry_t;
        typedef wiselib::pair<node_id_t, connection_entry_t> newconn_t;

        typedef typename wiselib::MapStaticVector<OsModel, node_id_t, connection_entry_t, MAX_CONNECTIONS> open_connections_t;
        open_connections_t open_connections;

        int max_retries_; // Maximum retries to deliver a message before abort

        Radio * radio_;
        Timer * timer_;
        Debug * debug_;

        Radio & radio() {
            return *radio_;
        }

        Timer & timer() {
            return *timer_;
        }

        Debug & debug() {
            return *debug_;
        }
    };

}


#endif	/* _RELIABLERADIO_H */
