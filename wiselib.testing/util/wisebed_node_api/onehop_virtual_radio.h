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
#ifndef CONNECTOR_ISENSE_VIRTUALRADIOMODEL_H
#define CONNECTOR_ISENSE_VIRTUALRADIOMODEL_H

#include "external_interface/isense/isense_types.h"
#include "util/delegates/delegate.hpp"
#include <isense/os.h>
#include <isense/radio.h>
#include <isense/dispatcher.h>
#include <util/pstl/vector_static.h>
#include <util/pstl/pair.h>

//to gateway:
#define FRW_UNICAST_MSG 200
#define FRW_BCAST_MSG 201

#define VRADIO_MSG_HEADER 4 // 2(for the vlink type msg) + 2 (and for the source vneigbor)

namespace wiselib {
    // -----------------------------------------------------------------------
    // -----------------------------------------------------------------------
    // -----------------------------------------------------------------------
        static uint16_t GATEWAY_ADDRESS; ///Unkown until the right msh is received
        static uint16_t nTxPacket; ///Unkown until the right msh is received
        static uint16_t nRxPacketT1; ///Unkown until the right msh is received
        static uint16_t nRxPacketT2; ///Unkown until the right msh is received
        static uint16_t nRxPacketT3; ///Unkown until the right msh is received
        static uint16_t nDropPackets; ///Unkown until the right msh is received

        static uint8_t payload[116];
        static uint16_t vneighbors[8];
        static uint8_t vneighbors_idx;
        

    /** \brief iSense Implementation of \ref radio_concept "Radio Concept"
     *  \ingroup radio_concept
     *
     * iSense implementation of the \ref radio_concept "Radio concept" ...
     */
    template<typename OsModel_P,
             typename Radio_P = typename OsModel_P::Radio>
    class VirtualRadioModel {
    public:
        typedef OsModel_P OsModel;
        typedef typename OsModel::Os Os;
        typedef Radio_P Radio;
        typedef VirtualRadioModel <OsModel, Radio> self_type;

        typedef typename Radio::node_id_t node_id_t;
        typedef typename Radio::size_t size_t;
        typedef typename Radio::block_data_t block_data_t;
        typedef radio_delegate_t vradio_delegate_t;

        typedef vector_static<OsModel, vradio_delegate_t, 10> receivers_t;
        typedef typename receivers_t::iterator rcv_iterator;

        typedef vector_static<OsModel, node_id_t, 10> virtual_links;
        typedef typename virtual_links::iterator iterator;

        typedef vector_static<OsModel, node_id_t, 10> deactivated_links;
        typedef typename deactivated_links::iterator dl_iterator;


        static virtual_links vlinks;
        static deactivated_links dlinks;
        static receivers_t receivers;

        enum ControlMessageTypes {
            SET_GW     = 100, //set as the master gateway the sender
            SET_VLINK  = 101, //set a new vlink
            USET_VLINK = 102, //remove a vlink
            SET_DLINK  = 103, //deactivate a link
            USET_DLINK = 104  //activate a link
        };
        // --------------------------------------------------------------------
        enum SpecialNodeIds {
            BROADCAST_ADDRESS = Radio::BROADCAST_ADDRESS, ///< All nodes in communication range
            NULL_NODE_ID      = Radio::NULL_NODE_ID ///< Unknown/No node id
        };
        // --------------------------------------------------------------------

        enum Restrictions {
            // TODO: MAX_MESSAGE_LENGTH = Radio::MAX_MESSAGE_LENGTH - x;
            MAX_MESSAGE_LENGTH = 115 ///< Maximal number of bytes in payload
                                     //128 - 1(MSG_TYPE)-1(LEN)-2(S_ADDR)-2(D_ADDR)
                                     //-1(VLINK_MSG_TYPE)
        };
        // --------------------------------------------------------------------

        static inline uint16_t
        getGW(void)
        {
            return GATEWAY_ADDRESS;
        }

        static inline char *
        dumpStats(void)
        {
            char dump[140];
            sprintf(dump,"nTxPacket=%d nRxPacketT1=%d nRxPacketT2=%d nRxPacketT3=%d dp=%d",
                    nTxPacket,
                    nRxPacketT1,
                    nRxPacketT2,
                    nRxPacketT3,
                    nDropPackets);
            return dump;
        }


        static inline int
        findvneighbor(uint16_t addr)
        {
            for(int i=0;i<vneighbors_idx;i++)
                if(vneighbors[i]==addr)
                    return i;

            return -1;
        }

        static inline void
        addvneighbor(uint16_t addr)
        {
            vneighbors[vneighbors_idx++]=addr;
        }

        static inline void
        removevneighbor(uint16_t addr)
        {
            for(int i=0;i<vneighbors_idx;i++)
                if(vneighbors[i]==addr)
                {
                    if(i!=(vneighbors_idx-1))
                        for(int z=i;z<(vneighbors_idx-1);z++)
                            vneighbors[z] = vneighbors[z+1];
                    vneighbors_idx--;
                }
        }

        static inline void
        send(Os *os, node_id_t id, size_t len, block_data_t *data) {

        nTxPacket++;
//       sendToGw(os,100,id,len,data );
            if(GATEWAY_ADDRESS!=0 && vneighbors_idx!=0)
            {
               if(id == BROADCAST_ADDRESS)
                   sendToGw(os,FRW_BCAST_MSG,id,len,data);
               else
               {
                    for(int i=0;i<vneighbors_idx;i++)
                    {
                        if(vneighbors[i]==id)
                        {
                           sendToGw(os,FRW_UNICAST_MSG,id,len,data);
                           return;
                        }
                    }
               }
            }
            
            Radio::send(os, id, len, data);
            //         os->radio().send( id, len, data, 0, 0 );
        };
       // --------------------------------------------------------------------

        static inline void
        sendToGw(Os *os,uint8_t vlink_msg_type, node_id_t id, size_t len, block_data_t *data) {
                    nRxPacketT2++;

            payload[0] = 100;
            payload[1] = vlink_msg_type;
            memcpy(payload+2,&id,sizeof(node_id_t));
            memcpy(payload+4,data,len);     //FIX: check for msg len <113
            
            Radio::send(os, GATEWAY_ADDRESS, len+VRADIO_MSG_HEADER, payload);
        }

        // --------------------------------------------------------------------
        static inline void enable(Os *os) {

//            vlinks.push_back( 0 );
            nTxPacket = 0; ///Unkown until the right msh is received
            nRxPacketT1 = 0; ///Unkown until the right msh is received
            nRxPacketT2 = 0; ///Unkown until the right msh is received
            nRxPacketT3 = 0; ///Unkown until the right msh is received
            nDropPackets = 0;


            Radio:: template reg_recv_callback <&self_type::vlink_rcv > (os);
            Radio::enable(os);
            GATEWAY_ADDRESS=0;
            vneighbors_idx=0;
            //          Radio::template reg_recv_callback<self_type, &self_type::receive>( os(), this );
        }

        static inline void vlink_rcv(node_id_t id, size_t len, block_data_t* data) {

            nRxPacketT1++;

                if (data[0] == 100) {
                    uint16_t v_dest = (uint16_t)(data[2]) << 8;
                    v_dest = v_dest + (data[3]);
                   // GATEWAY_ADDRESS = 0x9958;
                    
                switch(data[1]) {
                    case SET_GW:

                    GATEWAY_ADDRESS = id; //TODO: if the node is listening >1 gateways
                    //choose the one that has better LQI/RSSI
                        break;
                    case SET_VLINK:
                        addvneighbor(v_dest);
                        break;

                    case USET_VLINK:
                        removevneighbor(v_dest);
                        break;

                    case SET_DLINK:

                        dlinks.push_back(v_dest);
                        break;

                    case FRW_UNICAST_MSG:
                        for(rcv_iterator it= receivers.begin();it!=receivers.end();it++)
                            (*it)(v_dest,len-VRADIO_MSG_HEADER,data+VRADIO_MSG_HEADER);//add destination ??
                        break;

                    case FRW_BCAST_MSG:
                        for(rcv_iterator it= receivers.begin();it!=receivers.end();it++)
                            (*it)(v_dest,len-VRADIO_MSG_HEADER,data+VRADIO_MSG_HEADER);//add destination ??
                        break;
                        
                    case USET_DLINK:
                        nRxPacketT3++;

                        for(dl_iterator it= dlinks.begin();it!=dlinks.end();it++)
                            if((*it) == v_dest)
                                (*it) = 0;
//                                dlinks.insert(it,0);
                        //TODO: add proper remove
                        break;

                    default:
                        ;
                }

            }
            else
            {

                for(dl_iterator it= dlinks.begin();it!=dlinks.end();it++)
                    if((*it) == id)
                    {
                        nDropPackets++;
                        return;
                    }
                for(rcv_iterator it= receivers.begin();it!=receivers.end();it++)
                    (*it)(id,len,data);
            }
        }

        // --------------------------------------------------------------------

        static inline void disable(Os *os) {
            Radio::disable(os());
        }

        // --------------------------------------------------------------------

        static inline node_id_t id(Os *os) {
            
            return Radio::id(os);
        }

        // --------------------------------------------------------------------

        template<class T, void (T::*TMethod)(node_id_t, size_t, block_data_t*)>
        static inline int reg_recv_callback(Os *os, T *obj_pnt)
        {
            receivers.push_back( vradio_delegate_t::from_method<T, TMethod>( obj_pnt ) );
            // TODO: return real idx!
            return 0;
        };
        // --------------------------------------------------------------------

        static inline void unreg_recv_callback(Os *os, int idx) {
			 //Radio::unreg_recv_callback(os, idx);
            //         os->dispatcher().remove_receiver( &isense_radio_callbacks[idx] );
        };


    };
}

#endif
