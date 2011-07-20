/* 
 * File:   volumeradio.h
 * Author: amaxilatis
 *
 * Created on August 2, 2010, 1:15 PM
 */

#ifndef _VOLUMERADIO_H
#define	_VOLUMERADIO_H

//wiselib includes
#include "util/delegates/delegate.hpp"
#include "util/pstl/vector_static.h"
#include "util/pstl/pair.h"

#include "../radio_base.h"

// volume message type include
#include "volumemsg.h"

/*
 * DEBUG MESSAGES TEMPLATE
 * VolumeRadio::<task> [ type= ...]
 *
 * */
#define DEBUG_VOLUMERADIO

namespace wiselib {

    /*
     * ReliableRadio Template
     * Uses OsModel, Radio and Timer
     * Degug is only for debugging
     * ReliableRadio is used as a layer between normal radio
     * and application to make sure that messages will be
     * delivered besides any errors that may occur.
     * */
    template<typename OsModel_P, typename Radio_P, typename Timer_P,
    typename Debug_P>
    class VolumeRadio
    : public RadioBase<OsModel_P,Radio_P>{
    public:
        // Type definitions
        typedef OsModel_P OsModel;

        typedef Radio_P Radio;

        typedef Debug_P Debug;
        typedef Timer_P Timer;

        typedef typename Radio::node_id_t node_id_t;
        typedef typename Radio::size_t size_t;
        typedef typename Radio::block_data_t block_data_t;
        typedef typename Radio::message_id_t message_id_t;






        typedef VolumeRadio<OsModel_P, Radio_P, Timer_P, Debug_P> self_t;


        typedef VolumeMsg<OsModel, Radio > VolumeMessage_t;

        /*
         * struct that describes a message buffer
         * contains the sequence number
         * the source of the message
         * and data to show if receiving is complete
         */
        struct information {
            uint16_t seq_no;
            node_id_t source;
            uint8_t fragments_received;
            uint8_t total_fragments;
        };

        typedef struct information information_t;


        // radio callbacks delegate
        typedef delegate3<void, int, long, unsigned char*> radio_delegate_t;

        // --------------------------------------------------------------------

        enum SpecialNodeIds {
            BROADCAST_ADDRESS = Radio::BROADCAST_ADDRESS, ///< All nodes in communication rnage
            NULL_NODE_ID = Radio::NULL_NODE_ID
            ///< Unknown/No node id
        };
        // --------------------------------------------------------------------

        enum Restrictions {
            FRAGMENT_SIZE = VolumeMessage_t::FRAGMENT_SIZE,
            MAX_MESSAGE_LENGTH = VolumeMessage_t::MAX_MESSAGE_LENGTH
            ///< Maximal number of bytes in payload
        };

        // Vector Storing all messages received but not finished yet
        typedef wiselib::vector_static<OsModel,
        pair<information_t, uint8_t* >, MAX_PENDING> vector_t;

        // --------------------------------------------------------------------

        VolumeRadio() {
        }
        ;

        ~VolumeRadio() {
        }
        ;


        // Enable the Radio

        void enable_radio() {
#ifdef DEBUG_VOLUMERADIO
            debug().debug("VolumeRadio::enable\n");
#endif

            //set the local os pointer
            
            //enable normal radio
            radio().enable_radio();
            // register receive callback to normal radio
            recv_callback_id_ = radio().template reg_recv_callback<self_t,
                    &self_t::receive > ( this);

            // initialize the buffers for pending messages
            for (int i = 0; i < MAX_PENDING; i++) {
                pending_messages_[i].first.seq_no = 0; // seq_no ==  0 means unused
                pending_messages_[i].second = new uint8_t[MAX_MESSAGE_LENGTH]; // set the buffer size

            }

            // initialize the connections array
            for (int i = 0; i < MAX_CONNECTIONS_; i++) {
                open_connections[i].conn_id = -1; // -1 means connection unused
                open_connections[i].sequence_numbers_ = 1; // sequence numbers start from 1 ( 0 means unused see above)
            }

            // ??? only for broadcast messages ???
            seq_numbers_ = 1;




        }
        ;

        // --------------------------------------------------------------------

        
        // Send a message the application has requested

        void send( node_id_t id, uint16_t len, block_data_t *data) {

            block_data_t local_data[len];

            

            // check for an open connection to the destination
            int connection = check_connection(id);
            // get the next available sequence number 
            uint16_t seq_no = open_connections[connection].sequence_numbers_++;
            // if the next sequence number is 0 then set it as 1
            if (open_connections[connection].sequence_numbers_ == 0) open_connections[connection].sequence_numbers_++;

#ifdef DEBUG_VOLUMERADIO
            debug().debug( "VolumeRadio::send [node %d |to %d |len %d |seq_no %d ", radio().id(), id, len, seq_no);
#endif

            // create a new volume message
            VolumeMessage_t m = VolumeMessage_t();
            // initialize the message

            if (len>VolumeMessage_t::FRAGMENT_SIZE){
                m.set_msg_id(VolumeMessage_t::VOLUME_MESSAGE);
            }
            else{
                m.set_msg_id(VolumeMessage_t::SINGLE_MESSAGE);
            }

            m.set_seq_number(seq_no);
            
            uint8_t fragments = len/VolumeMessage_t::FRAGMENT_SIZE + 1;
#ifdef DEBUG_VOLUMERADIO
            debug().debug( "split_count= %d ]\n", fragments);
#endif
            m.set_fragments(fragments);
            // Send all fragments of the message
            uint8_t i;
            for (i = 0 ; i < (fragments-1); i++) {
                // create a temp buffer for the message
                m.set_fragment_id(i);
                m.set_payload(FRAGMENT_SIZE,local_data+(i*FRAGMENT_SIZE));

                // send the fragment
                radio().send(id, m.buffer_size(), (uint8_t * ) &m);
#ifdef DEBUG_VOLUMERADIO
                debug().debug( "VolumeRadio::send [%d |dest= %d |seq_no %d |fr_no %d |tot_fr %d |size= %d |...]\n",
                    m.msg_id(),
                    id,
                    m.seq_number(),
                    m.fragment_id(),
                    m.fragments(),
                    m.buffer_size()

                        );
#endif
                                                      
            }
            m.set_fragment_id(i);
            m.set_payload(len%FRAGMENT_SIZE,data+(i*FRAGMENT_SIZE));
            radio().send(id, (size_t)m.buffer_size(), (uint8_t * ) &m);
#ifdef DEBUG_VOLUMERADIO
                debug().debug( "VolumeRadio::send [%d |dest= %d |seq_no %d |fr_no %d |tot_fr %d |size= %d |...]\n",
                    m.msg_id(),
                    id,
                    m.seq_number(),
                    m.fragment_id(),
                    m.fragments(),
                    m.buffer_size()

                        );
#endif

           
            
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
        void receive(node_id_t from, size_t len, block_data_t* data_t) {
            // do not receive own messages
            if (radio().id() == from) {
                debug().debug( "message heard\n");
                return;
            }
            VolumeMessage_t mrecv;
            memcpy(&mrecv,data_t,len);
            
            if (mrecv.msg_id() == VolumeMessage_t::VOLUME_MESSAGE) {

#ifdef DEBUG_VOLUMERADIO
                debug().debug( "VolumeRadio::receive [%d |from %d |seq_no %d |fr_no %d |tot_fr %d |size %d |...]\n",
                        mrecv.msg_id(),
                        from,
                        mrecv.seq_number(),
                        mrecv.fragment_id(),
                        mrecv.fragments(),
                        len);
#endif

                // get all the information from the header
                node_id_t source = from;
                uint16_t seq_no = mrecv.seq_number();
                uint8_t fragment_num = mrecv.fragment_id();
                uint8_t total_frags = mrecv.fragments();


                // local buffer for payload fragment
                uint8_t t_buff[mrecv.payload_size()];

                memcpy(t_buff,mrecv.payload(),mrecv.payload_size());
                //t_buff = VolumeMessage_t::strip_message(data);

                // find the buffer dedicated to this message
                int fd = flow_id(seq_no, source, total_frags);

                if (fd != -1) {
#ifdef DEBUG_VOLUMERADIO
                    debug().debug( "VolumeRadio::receive pending_message %d frag_no %d\n", fd, fragment_num);
#endif

                    // add fragment to the buffer. if finished set the flag
                    bool finished = add_fragment_to_message(fd, fragment_num, t_buff);

                    // if finished forward to the application
                    if (finished) {
#ifdef DEBUG_VOLUMERADIO
                        debug().debug( "VolumeRadio::receive pending_message %d completed %d \n", fd , );
#endif
                        notify_receivers(from, (FRAGMENT_SIZE) * total_frags, pending_messages_[fd].second);
                    }


                } else {
#ifdef DEBUG_VOLUMERADIO
                    debug().debug( "VolumeRadio::receive vector is full\n");
#endif
                }


            } else if (mrecv.msg_id() == VolumeMessage_t::SINGLE_MESSAGE) {
                
#ifdef DEBUG_VOLUMERADIO
                debug().debug( "VolumeRadio::receive [%d |from %d |seq_no %d |fr_no %d |tot_fr %d |...]\n",
                        mrecv.msg_id(),
                        from,
                        mrecv.seq_number(),
                        mrecv.fragment_id(),
                        mrecv.fragments()
                        );
#endif
                // get the payload from the buffer
                uint8_t t_buff[mrecv.payload_size()];

                memcpy(t_buff,mrecv.payload(),mrecv.payload_size());
                //uint8_t * t_buff = new uint8_t[len - VolumeMessage_t::header_size()];
                //t_buff = VolumeMessage_t::strip_message(data);
                // forward message to the application
                
#ifdef DEBUG_VOLUMERADIO
                debug().debug( "VolumeRadio::message_received_callback\n");
#endif
                notify_receivers(from, mrecv.payload_size() , t_buff);
                

            }
             
        }
        ;

        // checks the vector of messages to see if the first fragment.
        // if not returns the id of the vector position
        // if yes allocates an available buffer and initializes data
        // if yes && buffers full returns -1

        int flow_id(uint16_t seq_no, node_id_t source, uint8_t total_frags) {
            int fd = -1;

            // search vector for message and a free slot
            for (int i = 0; i < MAX_PENDING; i++) {
                if ((seq_no == pending_messages_[i].first.seq_no) && (source == pending_messages_[i].first.source))
                    return i;
                if ((pending_messages_[i].first.seq_no == 0) && (fd == -1))
                    fd = i;
            }
            if (fd != -1) {
                // if does not exist but we have an empty buffer allocate it
                pending_messages_[fd].first.seq_no = seq_no;
                pending_messages_[fd].first.source = source;
                pending_messages_[fd].first.fragments_received = 0;
                pending_messages_[fd].first.total_fragments = total_frags;
            }
            // if not return -1
            return fd;

        };

        // add a fragment to the payload buffer and returns if finished

        bool add_fragment_to_message(int fd, uint8_t fragment_num, uint8_t * data) {
            // copy data to local memory
            memcpy(pending_messages_[fd].second + FRAGMENT_SIZE*fragment_num, data, FRAGMENT_SIZE);
            // inrease fragments received
            pending_messages_[fd].first.fragments_received++;
            // if the last fragment clear the buffer
            if (pending_messages_[fd].first.fragments_received ==
                    pending_messages_[fd].first.total_fragments) {
                pending_messages_[fd].first.seq_no = 0;
                // return message is complete
                return true;
            } else {
                // return message is incomplete
                return false;
            }

        };
        // --------------------------------------------------------------------

        // Check if a connection with the sender was established before

        int check_connection(node_id_t node) {
            int avail = -1;
            for (int i = 0; i < MAX_CONNECTIONS_; i++) {
                if (open_connections[i].conn_id == node) {
                    return i;
                } else if (open_connections[i].conn_id == -1) {
                    if (avail == -1) {
                        avail = i;
                    }
                }
            }
            // if the first connection get a new entry from the table
            if (avail != -1) {
#ifdef DEBUG_VOLUMERADIO
                debug().debug( "VolumeRadio::connection Opened Connection %d to node %d\n", radio().id(), node);
#endif
                open_connections[avail].conn_id = node;
                open_connections[avail].sequence_numbers_ = 1;

            } else {
#ifdef DEBUG_VOLUMERADIO
                debug().debug( "VolumeRadio::connection Out of connections node= %d\n", radio().id());
#endif
            }
            // return the connection id
            return avail;
        };

        // --------------------------------------------------------------------


        // returns the node's id

        node_id_t id() {
            return radio().id();
        }
        // --------------------------------------------------------------------


        /*
         initialize the module
         */
        void init(Radio& radio, Timer& timer, Debug& debug) {
            radio_ = &radio;
            timer_ = &timer;
            debug_ = &debug;
        };


    private:
        int recv_callback_id_; // callback for receive function
        radio_delegate_t message_received_callback_; // callback for application's receive function


        vector_t pending_messages_; // contains the buffers for the pending messages

        static const int MAX_CONNECTIONS_ = 40; // maximum available connections
        uint16_t seq_numbers_; // sequence number for messages

        struct connections {
            node_id_t conn_id;
            uint16_t sequence_numbers_;
        } open_connections[MAX_CONNECTIONS_];




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




#endif	/* _VOLUMERADIO_H */

