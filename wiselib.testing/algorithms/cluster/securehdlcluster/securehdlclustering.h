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

#ifndef __ALGORITHMS_SECURE_HDL_CLUSTERING_H_
#define __ALGORITHMS_SECURE_HDL_CLUSTERING_H_

#include "securehdlclusteringtypes.h"
#include "securehdlclusteringmessage.h"
#include "algorithms/crypto/sha1.h"

#include <iostream>

namespace wiselib {
    
    class HKC {
    public:
        static bool equal(const HKCelement_t& a, const HKCelement_t& b) {
            for (int i = 0; i < HKC_ELEMENT_SIZE; i++) {
                if(a.A[i]!=b.A[i]) return false;
            }
            return true;
        }
        static HKCelement_t zero_element() {
            HKCelement_t ret;
            for (int i = 0; i < HKC_ELEMENT_SIZE; i++) ret.A[i]=0;
            return ret;
        }
        static HKCelement_t secure_one_way_key_function(const HKCelement_t& input) {
          HKCelement_t ret;
	  //perform sHA1 on the input
	  SHA1Context sha;
	  SHA1::SHA1Reset(&sha);    
	  SHA1::SHA1Update(&sha, input.A, 20 );
	  SHA1::SHA1Digest(&sha, ret.A);  
	  return ret;
        }
        static bool check_authenticity(const HKCelement_t& input, const HKCelement_t& my_element, uint8_t times) {
            HKCelement_t temp=input;
            for(int i=0; i<times; i++) temp=secure_one_way_key_function(temp);
	    std::cout<<"comparing ";
	    for(int i=0; i<20; i++) printf("%2x", temp.A[i]);
	    std::cout<<" with ";
	    for(int i=0; i<20; i++) printf("%2x", my_element.A[i]);
	    std::cout<<std::endl;
            if(equal(temp, my_element)) return true;
            else return false;
        }
        
        // --------------- initial key generation --------------------------------------------
        
        static HKCelement_t random_HKC_element() {
            return zero_element();
        }
        static HKCelement_t* generate_HKC(HKCelement_t& input, uint8_t size) {
            HKCelement_t* ret=new HKCelement_t[size];
	    //place input (the last key of chain) at the end of the chain
	    HKCelement_t temp;
	    for (int i = 0; i < HKC_ELEMENT_SIZE; i++) temp.A[i]=input.A[i];
	    for (int j = 0; j < HKC_ELEMENT_SIZE; j++) ret[size-1].A[j]=temp.A[j];
	    
	    //generate rest of the key chain and store it on ret
            for (int k = size-2; k >= 0; k--) {
                temp=secure_one_way_key_function(temp);
		for (int l = 0; l < HKC_ELEMENT_SIZE; l++) ret[k].A[l]=temp.A[l];
            }
            return ret;
        } 
    };

   /** \brief Secured HDL Clustering.
    * 
    *  \ingroup clustering_concept
    *  \ingroup basic_algorithm_concept
    *  \ingroup clustering_algorithm
    * 
    */    
    template<typename OsModel_P,
            typename Radio_P = typename OsModel_P::Radio,
            typename Timer_P = typename OsModel_P::Timer>
    class SecureHdlClustering {
        
    public:
        
        typedef int cluster_id_t;
        typedef uint8_t cluster_level_t;    //quite useless within current scheme, supported for compatibility issues
        typedef OsModel_P OsModel;
        typedef Radio_P Radio;
        typedef Timer_P Timer;
          
        typedef SecureHdlClustering<OsModel_P, Radio_P, Timer_P> self_t;
        
        typedef typename Radio::node_id_t node_id_t;
        typedef typename Radio::size_t size_t;
        typedef typename Radio::block_data_t block_data_t;
        
        typedef SecureHdlMessage<OsModel, Radio> SHdlMessage;
        
        typedef delegate1<void, int> cluster_delegate_t;
        
        //quite useless within current scheme, supported for compatibility issues
        enum EventIds {
            CLUSTER_HEAD_CHANGED = 0,
            NODE_JOINED          = 1,
            NODE_LEFT            = 2
        };
        
        //quite useless within current scheme, supported for compatibility issues
        enum ClusterIds {
            UNKNOWN_CLUSTER_HEAD = 0
        };
        
        void enable( void );
        void disable( void );
        
        //same as original get_hdl()
        inline cluster_id_t cluster_id( cluster_level_t ) { return HDL_; }
        
        //quite useless within current scheme, supported for compatibility issues
        inline cluster_level_t cluster_level() { return 0; }
        
        template<class T, void (T::*TMethod)(int)>
        inline int reg_changed_callback( T *obj_pnt ) {
            state_changed_callback_=cluster_delegate_t::from_method<T, TMethod>( obj_pnt );
        }
        inline void unreg_changed_callback( int idx ) {
            state_changed_callback_=cluster_delegate_t();
        }
        
        inline void set_sink( bool sink) { sink_ = sink; }
        
        inline void set_chain(HKCelement_t *input, uint8_t size) {
            if(!sink_) return;
            chain_=new HKCelement_t[size];
            chain_size=size;
            memcpy(chain_, input, size*sizeof(HKCelement_t));
        }
        inline void set_chain_element(const HKCelement_t& input) {
            k0_=input;
        }
        
        ///@name Construction / Destruction
        ///@{
        SecureHdlClustering():
        sink_(false),
        HDL_(-1),
        flag(true)
        {};
        ~SecureHdlClustering() {};
        ///@}
        
    protected:
        void receive(node_id_t from, size_t len, block_data_t* data);
        void timer_expired(void*);
        
    private:
        Radio& radio()
        { return *radio_; }
        
        Timer& timer()
        { return *timer_; }
      
        Radio * radio_;
        Timer * timer_;
      
        bool sink_, flag;
        cluster_id_t HDL_;
        int callback_id_;
        cluster_delegate_t state_changed_callback_;
        HKCelement_t *chain_, k0_;
        int chain_size, current_layer_, time_slice_, time_step;
        
    };
    
    template<typename OsModel_P,
            typename Radio_P,
            typename Timer_P>
    void
    SecureHdlClustering<OsModel_P, Radio_P, Timer_P>::
    enable( void ) {
        time_slice_=time_step=2000; //1334
        Radio_P::enable_radio();
        callback_id_ = Radio_P::template reg_recv_callback<self_t,
                &self_t::receive>(this );
        current_layer_=1;
        if(sink_) {
            //I am sink, bcast first message with HDL=0
            HDL_=0;
            SHdlMessage msg;
            msg.set_current_layer(current_layer_);
            msg.set_key(&chain_[current_layer_]);
	    std::cout<<"Sink signs with : ";
	    for(int i=0;i<20;i++) printf("%2x", msg.key()->A[i]);
	    std::cout<<std::endl;
            radio().send(radio().BROADCAST_ADDRESS, msg.buffer_size(), (block_data_t*) &msg);
        } else {
            HDL_=-1;
        }
        timer().template set_timer<self_t,
                &self_t::timer_expired>(time_slice_, this, (void*) 0);
    }
    
    template<typename OsModel_P,
            typename Radio_P,
            typename Timer_P>
    void
    SecureHdlClustering<OsModel_P, Radio_P, Timer_P>::
    disable( void ) {
        Radio_P::unreg_recv_callback(callback_id_ );
        Radio_P::disable();
    }
    
    template<typename OsModel_P,
            typename Radio_P,
            typename Timer_P>
    void
    SecureHdlClustering<OsModel_P, Radio_P, Timer_P>::
    receive(node_id_t from, size_t len, block_data_t* data) {
        
        if ( from == radio().id(  ) ) return;
        
        SHdlMessage *msg=(SHdlMessage*) data;
        if(HDL_==-1) {
	  std::cout<<"Node "<<radio().id()<<" gets ";
	  for(int i=0;i<20;i++) printf("%2x", msg->key()->A[i]);
	  std::cout<<std::endl;
	  putchar('\n');
            if((current_layer_==msg->current_layer()) && (HKC::check_authenticity(*(msg->key()), k0_, current_layer_))) {
                HDL_=msg->current_layer();
                state_changed_callback_(NODE_JOINED);
            }
        } else if((flag) && (msg->current_layer()>HDL_)) {
            flag=false;
            Radio_P::send(Radio_P::BROADCAST_ADDRESS, msg->buffer_size(), data);
        }
        
    }
    
    template<typename OsModel_P,
            typename Radio_P,
            typename Timer_P>
    void
    SecureHdlClustering<OsModel_P, Radio_P, Timer_P>::
    timer_expired(void *data) {
        current_layer_++;
        time_slice_+=time_step;
        flag=true;
        if(sink_){
            if(current_layer_>=chain_size) return;
            SHdlMessage msg;
            msg.set_current_layer(current_layer_);
            msg.set_key(&chain_[current_layer_]);
	    std::cout<<"Sink signs with : ";
	    for(int i=0;i<20;i++) printf("%2x", msg.key()->A[i]);
	    std::cout<<std::endl;
            radio().send(radio().BROADCAST_ADDRESS, msg.buffer_size(), (block_data_t*) &msg);
        }
        timer().template set_timer<self_t,
                &self_t::timer_expired>(time_slice_, this, (void*) 0);
    }

}

#endif
