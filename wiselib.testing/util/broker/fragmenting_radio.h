/*
 * File:   fragmenting_radio.h
 * Author: maxpagel
 *
 * Created on 18. April 2012, 16:20
 */

#ifndef _FRAGMENTING_RADIO_H
#define	_FRAGMENTING_RADIO_H
#include "util/base_classes/radio_base.h"
#include "util/serialization/simple_types.h"
#include "fragment_message.h"
namespace wiselib
{

    template<typename OsModel_P,
            typename Radio_P = typename OsModel_P::Radio,
            typename Debug_P = typename OsModel_P::Debug,
            typename size_P = uint16_t>

            class FragmentingRadio : public RadioBase<OsModel_P, typename Radio_P::node_id_t,
             size_P, typename Radio_P::block_data_t>
    {
    public:
        typedef OsModel_P OsModel;
        typedef Radio_P Radio;

        typedef Debug_P Debug;
        typedef size_P  size_t;

        typedef FragmentingRadio<OsModel, Radio, Debug> self_type;
        typedef self_type* self_pointer_t;

        typedef typename Radio::node_id_t node_id_t;
        typedef typename Radio::block_data_t block_data_t;
        typedef typename Radio::message_id_t message_id_t;


        typedef FragmentMessage<OsModel, Radio> Message;

        // --------------------------------------------------------------------

        enum SpecialNodeIds
        {
            BROADCAST_ADDRESS = Radio_P::BROADCAST_ADDRESS, ///< All nodes in communication range
            NULL_NODE_ID = Radio_P::NULL_NODE_ID
            ///< Unknown/No node id
        };

        enum ErrorCodes
        {
            SUCCESS = OsModel::SUCCESS,
            ERR_UNSPEC = OsModel::ERR_UNSPEC,
            ERR_NETDOWN = OsModel::ERR_NETDOWN,
            ERR_HOSTUNREACH = OsModel::ERR_HOSTUNREACH
        };

        // --------------------------------------------------------------------

        enum Restrictions
        {
            MAX_MESSAGE_LENGTH = Radio_P::MAX_MESSAGE_LENGTH - Message::PAYLOAD_POS-20, ///< Maximal number of bytes in payload -20==Very Dirty Hack

        };
        // --------------------------------------------------------------------

        void init(typename Radio::self_pointer_t radio, typename Debug::self_pointer_t debug)
        {
            enabled_ = false;
            radio_ = radio;
            debug_ = debug;
        }

        int enable_radio()
        {
            radio_->enable_radio();
            debug_->debug("radio enabled");
            radio_->template reg_recv_callback<self_type, &self_type::receive_radio_message > (this);
            enabled_ = true;
            message_id_ = 1;//0 stands reserved for receiving no message curently
            return SUCCESS;
        }

        int disable_radio()
        {
            radio_->disable_radio();
            enabled_ = false;
            return SUCCESS;
        }

        int send(node_id_t receiver, size_t length, block_data_t *data)
        {
            int len = length;
            newMessage(0, 0, 0, length);
            size_t pos = 0;
            int i = 0;
            if(message_id_==255)
                message_id_ = 1;
            else
                message_id_++;
            
            while (len > 0)
            {
                size_t free_buffer_size = MAX_MESSAGE_LENGTH - buffer_.payload_length();
                size_t writable_bytes = 0;
                if (free_buffer_size >= len)
                    writable_bytes = len;
                else
                    writable_bytes = free_buffer_size;
//                debug_->debug("%d bytes available writable bytes %d free buffer size: %d, buffer size: %d payload size = %d\n",len,writable_bytes,free_buffer_size,buffer_.buffer_size(),buffer_.payload_length());
//                 debug_->debug("bytes to write:\n");
//                 for(int k = pos; k<pos + writable_bytes;k++)
//                 {
//                     debug_->debug("%d",data[k]);
//                 }
                bool success = buffer_.append(reinterpret_cast<block_data_t*> (data), pos, writable_bytes);
//                debug_->debug("sucess ? %d payload length:%d chunk %d\n",success,buffer_.payload_length(),i);

//                             for(int k = 0; k<buffer_.buffer_size();k++)
//                             {
//                                 debug_->debug("%d\n",reinterpret_cast<block_data_t*> (&buffer_)[k]);
//                             }
//                             debug_->debug("\n");
                if (!success)
                {
                    debug_->debug("error append failed. This should never happen\n");
                    //newMessage((buffer_.sequence_number() + 1) % 256, 1);
                    return ERR_UNSPEC;
                }
                pos += writable_bytes;
                len -= writable_bytes;

                radio_->send(BROADCAST_ADDRESS, buffer_.buffer_size(), reinterpret_cast<block_data_t*> (&buffer_));
                receive_radio_message(radio_->id(),buffer_.buffer_size(), reinterpret_cast<block_data_t*> (&buffer_));
//                for(int i =0; i<100;++i){
//                    debug_->debug("waiting \n");
//                }
                newMessage((buffer_.sequence_number() + 1) % 256, 1, pos, length);
                ++i;
            }

            newMessage((buffer_.sequence_number() + 1) % 256, 2, 0, length);
            radio_->send(BROADCAST_ADDRESS, buffer_.buffer_size(), reinterpret_cast<block_data_t*> (&buffer_));
            receive_radio_message(radio_->id(),buffer_.buffer_size(), reinterpret_cast<block_data_t*> (&buffer_));
            return SUCCESS;
        }

        node_id_t id()
        {
            return radio_->id();
        }

        void receive_radio_message(typename Radio::node_id_t source, typename Radio::size_t length,
                typename Radio::block_data_t *buf)
        {
            Message *msg = (Message*) buf;

//            debug_->debug("received message cmd %d from %d",msg->command_type(),source);
//            for(size_t i = 0;i<20;++i){
//                debug_->debug("%d ", buf[i]);
//            }
//            debug_->debug("received message");
            if (msg->command_type() == 114)
            {
//                debug_->debug("received message fragment with cmd: %d", msg->command());
                //debug_->debug("received message fragment payload len=%d from %x", msg->payload_length(), source);
                switch (msg->command())
                {
                    case 0: //new Message
                    {
//                        debug_->debug("fragment is new message with total size %d chunk length %d", msg->total_size(),msg->payload_length());
                        if (current_Message_) {
                            get_allocator().free_array(current_Message_);
						}
                        current_Message_ = get_allocator().template allocate_array<block_data_t > (msg->total_size());
                        message_id_reveived_ = msg->message_id();
                        if(msg->payload_length() > 0) {
                                memcpy(current_Message_.raw() + msg->index(), msg->payload(), msg->payload_length());
                                if(msg->payload_length() == msg->total_size()) {
                                           this->notify_receivers(source, msg->total_size(), current_Message_.raw());
                                }
                        }

                    }
                    break;
                    case 1:
                    {
                        if(message_id_reveived_ ==0 || message_id_reveived_ != msg->message_id()){                            
                            return;
                        }
//                        debug_->debug("Fragment , %d", msg->index());
						if(msg->payload_length() > 0) {
                        	memcpy(current_Message_.raw() + msg->index(), msg->payload(), msg->payload_length());
						}
                    }
                    break;
                    case 2:
                    {
                        if(message_id_reveived_ ==0 || message_id_reveived_ != msg->message_id()){                            
                            return;
                        }
//                        debug_->debug("fragment is end of message size %d",msg->total_size());
			if(msg->payload_length() > 0) {
                            memcpy(current_Message_.raw() + msg->index(), msg->payload(), msg->payload_length());
                        }
                        this->notify_receivers(source, msg->total_size(), current_Message_.raw());

//                        for(size_t i = 0;i<msg->total_size();++i){
//                            debug_->debug("%d ", current_Message_.raw()[i]);
//                        }
                    }
                    break;
                    default:
                    {
//                        debug_->debug("unknown command %d",msg->command());
                        return;
                    }
                    break;
                }
            }
        }

        void newMessage(uint8_t seqNr, uint8_t command, size_t index, size_t total_Size)
        {
            buffer_.set_command_type(114);
            buffer_.set_sequence_number(seqNr);
            buffer_.set_payload(0, 0);
            buffer_.set_command(command);
            buffer_.set_index(index);
            buffer_.set_total_size(total_Size);
            buffer_.set_message_id(message_id_);
//            debug_->debug("new message with command %d sizeof size_type = %d \n",buffer_.command(), sizeof(size_t));

        }


    private:
        bool enabled_;
        typename Radio::self_pointer_t radio_;
        typename Debug::self_pointer_t debug_;
        Message buffer_;
        size_t current_message_size_;
        typename Allocator::template array_pointer_t<block_data_t> current_Message_;
        uint8_t message_id_;
        uint8_t message_id_reveived_;


    };
}

#endif	/* _FRAGMENTING_RADIO_H */

