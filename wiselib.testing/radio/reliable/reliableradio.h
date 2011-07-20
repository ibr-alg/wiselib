/*
 * File:   reliableradio.h
 * Author: amaxilatis
 *
 * Created on July 27, 2010
 */

#ifndef _RELIABLERADIO_H
#define	_RELIABLERADIO_H

//#include "util/delegates/delegate.hpp"
#include "util/pstl/vector_static.h"
#include "util/pstl/pair.h"
#include "util/delegates/delegate.hpp"
#include "util/base_classes/radio_base.h"
#include "reliablemsg.h"

//#include "communication/reliablemsg.h"

    /*
     * DEBUG MESSAGES TEMPLATE
     * ReliableRadio::<task> [ type= ...] 
     *
     * */
//#define DEBUG_RELIABLERADIO
//#define EXTRA
#ifdef ISENSE_RADIO_ADDR_TYPE
    #define ISENSE_APP
#endif

#define MAX_PENDING 10	// Maximum number of pending for delivery messages
#define TIMEOUT 800		// Timeout for time to pass
#define MAX_CONNECTIONS 20
#define MAX_RECV 15

namespace wiselib {

/*
 * ReliableRadio Template
 * Uses OsModel, Radio and Timer
 * Degug is only for debugging
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
                       typename Radio_P::block_data_t>
{

public:
    // Type definitions
    typedef OsModel_P OsModel;

    typedef Radio_P Radio;
    typedef typename Radio::node_id_t node_id_t;
    typedef typename Radio::size_t size_t;
    typedef typename Radio::block_data_t block_data_t;

    typedef typename Radio::message_id_t message_id_t;
    typedef ReliableMsg< OsModel,Radio> ReliableMessage; // a Reliable message with some extra headers for delivery checks

    typedef delegate4<void, uint8_t, node_id_t, size_t, block_data_t*> event_notifier_delegate_t;

    struct information {
        uint16_t seq_no;
        node_id_t destination;
        int timestamp;
        int retries;
        event_notifier_delegate_t event_notifier_callback;
    }; // stores information about the status of a sent message

    typedef struct information information_t;
    typedef Debug_P Debug;
    typedef Timer_P Timer;

    // Vector Storing all messages sent but receive not confirmed yet
    typedef wiselib::vector_static<OsModel,
    pair<information, ReliableMessage>, MAX_PENDING> vector_t;

    typedef ReliableRadio<OsModel_P, Radio_P, Timer_P, Debug_P> self_t;


    // --------------------------------------------------------------------

    enum Events {
        MSG_DROPPED = 1,
        MSG_ACK_RCVD = 2
    };

    enum SpecialNodeIds {
        BROADCAST_ADDRESS = Radio::BROADCAST_ADDRESS, ///< All nodes in communication rnage
        NULL_NODE_ID = Radio::NULL_NODE_ID
        ///< Unknown/No node id
    };
    // --------------------------------------------------------------------

    enum Restrictions {

        MAX_MESSAGE_LENGTH = ReliableMessage::MAX_MESSAGE_LENGTH
        ///< Maximal number of bytes in payload
    };
    // --------------------------------------------------------------------

    void init (Radio& radio , Timer& timer , Debug& debug){
        radio_ = &radio;
        timer_ = &timer;
        debug_ = &debug;
    };

    ReliableRadio() {
        MAX_RETRIES = 5;

        acked_messages_=0;
    }
    ;

    ~ReliableRadio() {

    }
    ;




     // Enable the Radio

    void enable_radio() {
#ifdef DEBUG_RELIABLERADIO
       debug().debug("ReliableRadio::enable\n");
#endif

#ifdef ISENSE_APP
        debug().debug("Hello isense world\n");
#else
        debug().debug("running in shawn\n");
#endif

        //set the local os pointer
        //set_os(os_);

        //enable normal radio
        //Radio::enable(os());
        radio().enable_radio();

        radio().template reg_recv_callback<self_t,
                &self_t::receive > ( this);
        // initialize the sequense_numbers to 1 (0 means empty)
        sequence_numbers_ = 1;
        // initialize the Vector to empty ( seq_no=0 means empty)
        for (int i = 0; i < MAX_PENDING; i++) {
            pending_messages_[i].first.seq_no = 0;
            pending_messages_[i].first.event_notifier_callback = event_notifier_delegate_t();
        }
        for (int i=0;i < MAX_CONNECTIONS; i++){
            open_connections[i].conn_id=NULL_NODE_ID;
            open_connections[i].sequence_numbers_=1;
            open_connections[i].received_count=0;
        }
        // timestamps start from 0
        current_time_ = 0;
        // set timer to change time
        timer().template set_timer<self_t, &self_t::time_passes > (
                time_slice_, this, (void*) 0);
        // set timer to wake up realiable_deamon ( checks for undelivered messages and resends them )
        timer().template set_timer<self_t, &self_t::reliable_daemon > (
                sleep_time_, this, (void*) 0);

    }
    ;

    // --------------------------------------------------------------------

    void disable_radio() {
                radio().unreg_recv_callback(recv_callback_id_);
    };

    // Send a message the application has requested
//  template<class T, void (T::*TMethod)(uint8_t, node_id_t, size_t, block_data_t*)>
  //  void sendX( node_id_t id, uint8_t len, uint8_t *data,T *obj_pnt)
  template<class T, void (T::*TMethod)(uint8_t, node_id_t, size_t, block_data_t*)>
  void send_callback(node_id_t from, size_t len, block_data_t *data,T *obj_pnt )
  {

      int pending_id = 0;
      if ( (pending_id = send(from,len,data)) != -1) {
          pending_messages_[pending_id].first.event_notifier_callback = event_notifier_delegate_t::template from_method<T, TMethod>( obj_pnt );
      }

  };

    int send( node_id_t id, size_t len, block_data_t *data) {
        //set_os(os_);
        if (id == BROADCAST_ADDRESS) {
            // create a message to host the data
            ReliableMessage m ;
            m.set_msg_id(ReliableMessage::BROADCAST_MESSAGE);
            m.set_seq_number(0);
            m.set_payload(len, data);
            // get the real payload to send

#ifdef DEBUG_RELIABLERADIO
            debug().debug("ReliableRadio::send [ type= %d(broad) size= %d]\n", m.msg_id() , m.buffer_size() );
#endif

            // send a broadcast message
            radio().send(id, m.buffer_size(), (uint8_t * ) &m );
            return -1;
        }
            /*
             * ADD message to Vector
             * CREATE a special message containing the original
             * SEND through normal radio
             * WAIT for ack or reliability_daemon to resend it
             * */
        else {
            int pending_id = 0; // slot in the reliable vector available for new message
            pending_id = add_message(ReliableMessage::RELIABLE_MESSAGE, id, len, data); // add the message to the vector
            // If no position available inform APPLICATION
            // TODO: Inform application for error
            if (pending_id == -1) {
#ifdef DEBUG_RELIABLERADIO
                debug().debug( "ReliableRadio::error [ type= vector_full ]\n");
#endif
            }// Get message from the Vector and send it through normal radio
            else {

#ifdef DEBUG_RELIABLERADIO
#ifdef ISENSE_APP
                debug().debug("ReliableRadio::send [ type= %d(reliable) destination= %x seq_no= %d size= %d]", ReliableMessage::RELIABLE_MESSAGE, id, pending_messages_[pending_id].second.msg_id(), pending_messages_[pending_id].second.buffer_size());
#else
                debug().debug("ReliableRadio::send [ type= %d(reliable) destination= %d seq_no= %d size= %d]\n", ReliableMessage::RELIABLE_MESSAGE, id, pending_messages_[pending_id].second.msg_id(), pending_messages_[pending_id].second.buffer_size());
#endif
#endif
                // send message through normal radio
                radio().send(id, pending_messages_[pending_id].second.buffer_size(), (uint8_t *) & pending_messages_[pending_id].second);
            }

            return pending_id;
        }
    }
    ;



    // Adds a message to the Vector and initializes the information struct conserning it
    int add_message(int msg_id, node_id_t destination, uint8_t len, uint8_t * data) {
        int conn_id = check_connection(destination);
        int pos;
        // find a free slot in the pending messages Vector to store the message Else return -1
        if ((pos = find_postition()) != -1) {
            // Store information about the message
            pending_messages_[pos].first.seq_no = open_connections[conn_id].sequence_numbers_++;
            pending_messages_[pos].first.destination = destination;
            pending_messages_[pos].first.timestamp = current_time_;
            pending_messages_[pos].first.retries = 0;
            // Store the message and set its headers
            pending_messages_[pos].second.set_msg_id(msg_id);
            pending_messages_[pos].second.set_seq_number(open_connections[conn_id].sequence_numbers_ - 1);
            pending_messages_[pos].second.set_payload(len, data);
            if (sequence_numbers_ == 0) {
                sequence_numbers_++;
            }
            return pos;
        }
        return -1;
    }
    ;

    // --------------------------------------------------------------------


    // Increment time by 1

    void time_passes(void *) {
        current_time_++;
        //Debug::debug(os(),"Time is now %d\n",current_time_);
        timer().template set_timer<self_t, &self_t::time_passes > (
                time_slice_, this, (void*) 0);

    }
    ;

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
        for (int i = 0; i < MAX_PENDING; i++) { // search the message vector
            if (pending_messages_[i].first.seq_no != 0) { // only for valid entries : 0 means no message
                if (pending_messages_[i].first.timestamp < current_time_ - 2) { // 2 time units needed for message delivery
                    if (pending_messages_[i].first.retries < MAX_RETRIES) { // if max retries reached
                        // a message is to be resend
                        pending_messages_[i].first.timestamp = current_time_; // refresh its sent timestamp

                        size_t mess_size = pending_messages_[i].second.buffer_size();

                        mess_size = mess_size;

                        // resend the message
                        radio().send(
                                pending_messages_[i].first.destination,
                                pending_messages_[i].second.buffer_size(),
                                (uint8_t *) & pending_messages_[i].second);
                        // increase by 1 retries
                        pending_messages_[i].first.retries++;

#ifdef DEBUG_RELIABLERADIO
#ifdef ISENSE_APP
                        debug().debug("ReliableRadio::resend [ type= %d(reliable) destination= %x seq_no= %d size= %d]", ReliableMessage::RELIABLE_MESSAGE,
                                pending_messages_[i].first.destination,
                                pending_messages_[i].first.seq_no,
                                mess_size
                                );
#else
                        debug().debug("ReliableRadio::resend [ type= %d(reliable) destination= %d seq_no= %d size= %d]\n", ReliableMessage::RELIABLE_MESSAGE,
                                pending_messages_[i].first.destination,
                                pending_messages_[i].first.seq_no,
                                mess_size
                                );
#endif
#endif
                    } else { // if max retries reached abort sending

#ifdef DEBUG_RELIABLERADIO
#ifdef ISENSE_APP

                        debug().debug("ReliableRadio::abort [ type= %d(reliable) destination= %x seq_no= %d  max_retries]", ReliableMessage::RELIABLE_MESSAGE,
                                pending_messages_[i].first.destination,
                                pending_messages_[i].first.seq_no);
#else
                        debug().debug("ReliableRadio::abort [ type= %d(reliable) destination= %d seq_no= %d  max_retries]\n", ReliableMessage::RELIABLE_MESSAGE,
                                pending_messages_[i].first.destination,
                                pending_messages_[i].first.seq_no);
#endif
#endif
                        if ( pending_messages_[i].first.event_notifier_callback != event_notifier_delegate_t() )
                        {
                            pending_messages_[i].first.event_notifier_callback(MSG_DROPPED,
                                pending_messages_[i].first.destination,
                                pending_messages_[i].second.buffer_size(),
                                (uint8_t *) & pending_messages_[i].second );
                        }

                        // clear vector entry to be reused
                        pending_messages_[i].first.event_notifier_callback = event_notifier_delegate_t();
                        pending_messages_[i].first.seq_no = 0;
                    }
                    // a message was found
                    found = true;
                }
            }

        }

#ifdef DEBUG_RELIABLERADIO
        if (!found) {
            //                Debug::debug(os(), "ReliableRadio::info No pending messages\n");
        }
#endif
        // Reset the timer
        timer().template set_timer<self_t, &self_t::reliable_daemon > (
                sleep_time_, this, (void*) 0);
    }

    // find a free entry inside the pending messages vector

    int find_postition() {
        for (int i = 0; i < MAX_PENDING; i++) {
            if (pending_messages_[i].first.seq_no == 0) { // seq_no : 0 means no message
                return i;
            } else {
            }
        }
        return -1;
    }
    ;

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
    void receive(node_id_t from, size_t len, block_data_t* data) {


        // do not receive own messages
        if (radio().id() == from) {
            return;
        }
        // check for open connections with the sender
        int sd = check_connection(from);

        // if a connection can be opened
        if (sd != -1) {

            // check the message id
            if (data[0] == ReliableMessage::ACK_MESSAGE) { // if an ack message
                ReliableMessage recvack;
                memcpy(&recvack, data, len);
#ifdef DEBUG_RELIABLERADIO
#ifdef ISENSE_APP

                debug().debug("ReliableRadio::receive [ type= %d(ack) node= %x from= %x payload_size= %d ",
                        recvack.msg_id(),
                        radio().id(),
                        from,
                        recvack.payload_size());
#ifndef EXTRA
                debug().debug("]");
#endif

#else

                debug().debug("ReliableRadio::receive [ type= %d(ack) node= %d from= %d payload_size= %d ",
                        recvack.msg_id(),
                        radio().id(),
                        from,
                        recvack.payload_size());
#ifndef EXTRA
                debug().debug("]\n");
#endif
#endif
#endif

                uint8_t seq_nos[recvack.payload_size()];
                memcpy(seq_nos, recvack.payload(), recvack.payload_size());
                // get sequence from the message and remove the respective messages from the list
                for (size_t j = 0; j < recvack.payload_size(); j += 2) {
                    uint16_t seq_no_acked = seq_nos[j] + seq_nos[j + 1]*256;
#ifdef EXTRA
                    debug().debug("ReliableRadio::ack_msg_seq_no %d\n", seq_no_acked);
#endif
                    // search the whole vector
                    for (int i = 0; i < MAX_PENDING; i++) {
                        // if the sequence numbers match
                        if (pending_messages_[i].first.seq_no == seq_no_acked) {
                            // if the destinations match
                            if (pending_messages_[i].first.destination == from) {
                                // remove entry
#ifdef DEBUG_RELIABLERADIO
                                debug().debug("ReliableRadio::receive seq_no_acked=%d\n", seq_no_acked);
#endif
                                if ( pending_messages_[i].first.event_notifier_callback != event_notifier_delegate_t() )
                                {
                                    pending_messages_[i].first.event_notifier_callback(MSG_ACK_RCVD,
                                            pending_messages_[i].first.destination,
                                            pending_messages_[i].second.buffer_size(),
                                            (uint8_t *) & pending_messages_[i].second );
                                }

                                pending_messages_[i].first.event_notifier_callback = event_notifier_delegate_t();
                                pending_messages_[i].first.seq_no = 0;
                                acked_messages_++;
                            }
                        }
                    }
                }
#ifdef EXTRA
                debug().debug("]\n");
#endif
            } else if (data[0] == ReliableMessage::RELIABLE_MESSAGE) { // if a reliable message was received

                ReliableMessage recvm;

                memcpy(&recvm, data, len);

                // get sequence number from the message
                uint16_t curr_seq_no = recvm.seq_number();
                // check if the message exists in cache
                bool first_time = was_received(sd, curr_seq_no);
                // if it does not
                if (first_time) {
#ifdef DEBUG_RELIABLERADIO
#ifdef ISENSE_APP
                    debug().debug("ReliableRadio::receive [ type= %d(reliable) node= %x from= %x seq_no= %d ]",
                            data[0],
                            radio().id(),
                            from,
                            curr_seq_no);
#else
                    debug().debug("ReliableRadio::receive [ type= %d(reliable) node= %d from= %d seq_no= %d ]\n",
                            data[0],
                            radio().id(),
                            from,
                            curr_seq_no);
#endif
#endif
                    // extract the real payload from the message

                    block_data_t msg_striped[recvm.payload_size()];

                    memcpy(msg_striped, recvm.payload(), recvm.payload_size());

                    // forward the payload to the application
                    //message_received_callback_(from, recvm.payload_size(), msg_striped);
                    notify_receivers(from, recvm.payload_size(), msg_striped);


                    // create the new ack message to send

                    ReliableMessage ackm;

                    ackm.set_msg_id(ReliableMessage::ACK_MESSAGE);


                    ackm.set_seq_number(0);
                    set_ack_list(&ackm, sd);

                    // send the ack message
                    radio().send(from, ackm.buffer_size(), (uint8_t *) & ackm);
#ifdef DEBUG_RELIABLERADIO
#ifdef ISENSE_APP
                    debug().debug("ReliableRadio::send [ type= %d(ack) size= %d destination= %x ",
                            ackm.msg_id(),
                            ackm.buffer_size(),
                            from);
#else
                    debug().debug("ReliableRadio::send [ type= %d(ack) size= %d destination= %d ",
                            ackm.msg_id(),
                            ackm.buffer_size(),
                            from);
#endif
#endif
#ifdef EXTRA
                    block_data_t ack[ackm.payload_size()];
                    memcpy(ack,&ackm,ackm.payload_size());
                    for (int i = 0; i < ackm.payload_size(); i+=2) {
                        debug().debug(" %d", ack[i] + ack[i+1]);

                    }
#endif
#ifdef DEBUG_RELIABLERADIO
                    debug().debug("]\n");
#endif
                } else {// if the sequence number exists in cache then just send an ack message ( message was forwarded before to the application

                    // create the ack message
                    ReliableMessage ackm;

                    ackm.set_msg_id(ReliableMessage::ACK_MESSAGE);

                    //ackm.set_payload(open_connections[sd].received_count*2);

                    set_ack_list(&ackm, sd);
                    // send the ack message
                    radio().send(from, ackm.buffer_size(), (uint8_t *) & ackm);
                }
            } else if (data[0] == ReliableMessage::BROADCAST_MESSAGE) { // if a broadcast message was received forward to the application
#ifdef DEBUG_RELIABLERADIO
#ifdef ISENSE_APP
                debug().debug("ReliableRadio::receive [ type= %d(broad) from= %x ]", data[0], from);
#else
                debug().debug("ReliableRadio::receive [ type= %d(broad) from= %d ]\n", data[0], from);
#endif
#endif

                ReliableMessage recvm;

                memcpy(&recvm, data, len);

                // extract payload from the message received
                block_data_t msg_recv[recvm.payload_size()];
                memcpy(msg_recv, recvm.payload(), recvm.payload_size());

                // forward payload to the application
                //message_received_callback_(from, recvm.payload_size(), msg_recv);
                notify_receivers(from, recvm.payload_size(), msg_recv);

            } else {
#ifdef DEBUG_RELIABLERADIO
#ifdef ISENSE_APP
                debug().debug("ReliableRadio::error Unsupported message Received from %x", from);
#else
                debug().debug("ReliableRadio::error Unsupported message Received from %d\n", from);
#endif
#endif
            }
        }
    }
    ;

    void set_ack_list(ReliableMessage * m, int sd) {

        uint8_t seq_nos[open_connections[sd].received_count * 2];
        for (int i = 0; i < open_connections[sd].received_count; i++) {
            seq_nos[2 * i] = open_connections[sd].received[i] % 256;
            seq_nos[2 * i + 1] = open_connections[sd].received[i] / 256;

        }
        m->set_payload(open_connections[sd].received_count * 2, seq_nos);

    }

    // --------------------------------------------------------------------

    // Check if a connection with the sender was established before

    int check_connection(node_id_t sender) {
        int avail = -1;
        for (int i = 0; i < MAX_CONNECTIONS; i++) {

            if (open_connections[i].conn_id == sender) {
                return i;
            } else if (open_connections[i].conn_id == NULL_NODE_ID) {
                if (avail == -1) {
                    avail = i;
                }
            }
        }
        // if the first connection get a new entry from the table
        if (avail != -1) {
#ifdef DEBUG_RELIABLERADIO
#ifdef ISENSE_APP
            debug().debug("ReliableRadio::connection Opened Connection %x to node %x", radio().id(), sender);
#else
            debug().debug("ReliableRadio::connection Opened Connection %d to node %d\n", radio().id(), sender);
#endif
#endif
            open_connections[avail].conn_id = sender;
            open_connections[avail].max_received = 0;
            open_connections[avail].received_count = 0;
            for (int i = 0; i < MAX_RECV; i++) {
                open_connections[avail].received[i] = 0;
            }
        }
        // return the connection id
        return avail;
    };

    // --------------------------------------------------------------------

    // check sequence numbers cache to see if the message with this sequence number was previously received

    bool was_received(int sd, uint16_t seq_no) {
        bool first_time = true; // true if the first time received
        for (int i = 0; i < open_connections[sd].received_count; i++) {
            if (open_connections[sd].received[i] == seq_no) first_time = false;
        }
        // add to message cache
        if (open_connections[sd].received_count < MAX_RECV) {
            open_connections[sd].received_count++;
            open_connections[sd].received[open_connections[sd].received_count - 1] = seq_no;
        } else {
            // shift all messages to the past and add seq_number as the latest
            for (int i = 0; i < MAX_RECV - 1; i++) {
                open_connections[sd].received[i] = open_connections[sd].received[i + 1];
            }
            open_connections[sd].received[MAX_RECV - 1] = seq_no;
        }
#ifdef EXTRA
        debug().debug("RECEIVED[ ");
        for (int i = 0; i < MAX_RECV; i++) {
            debug().debug(" %d", open_connections[sd].received[i]);
        }
        debug().debug("]\n");
#endif

        return first_time;
    }
    ;

    void set_max_retries(int max_retries) {
        MAX_RETRIES = max_retries;
        debug().debug("ReliableRadio::set max_retries= %d\n", max_retries);
    };

    int max_retries() {
        return MAX_RETRIES;
    };

    int acked_messages() {
        return acked_messages_;
    };

    // --------------------------------------------------------------------
    // returns the node's id

    node_id_t id() {
        return radio().id();
    };
    // --------------------------------------------------------------------


    /*
            Os * os(){
                return os_;
            }
            void set_os( Os * os ){
                os_=os;
            };
     */
private:
    int recv_callback_id_; //
    //radio_delegate_t message_received_callback_;
    //Os * os_; // os pointer
    uint16_t sequence_numbers_; //
    uint16_t current_time_; //
    vector_t pending_messages_;
    static const int time_slice_ = 200; // time_passes delay
    static const int sleep_time_ = 1000; // reliable_daemon's sleep time

    struct connections {
        node_id_t conn_id;
        uint16_t received[MAX_RECV];
        int received_count;
        uint16_t max_received;
        uint16_t sequence_numbers_;
    } open_connections[MAX_CONNECTIONS];

    int MAX_RETRIES; // Maximum retries to deliver a message before abort


    //debuging
    int acked_messages_;

    Radio * radio_;
    Timer * timer_;
    Debug * debug_;

    Radio& radio() {
        return *radio_;
    }

    Timer& timer() {
        return *timer_;
    }

    Debug& debug() {
        return *debug_;
    }
};

}


#endif	/* _RELIABLERADIO_H */
