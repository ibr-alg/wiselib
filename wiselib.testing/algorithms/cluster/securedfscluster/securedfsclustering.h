//
// File:   dfsclustering.h
// Author: mpairakt
//
// Created on September 25, 2009, 4:04 PM
//

#ifndef _SECUREDFSCLUSTERING_H
#define	_SECUREDFSCLUSTERING_H

#include "util/delegates/delegate.hpp"
#include "securedfsclusteringmessage.h"
#include "algorithms/crypto/ecc.h"
#include <stack>
#include <cstdlib>
#include <iostream>

namespace wiselib {

    class ECGDH {
    public:
        static PubKey generate_random_ECpoint() {
		PubKey ret;
		ECC::p_clear(&(ret.W));
		ECC::gen_public_key(generate_random_ECfactor().s, &(ret.W));
		return ret;
	}
        static PrivKey generate_random_ECfactor() {
		uint8_t a[NUMWORDS];
		ECC::b_clear(a);
		uint8_t d=1, b=rand()%256;
		for (uint8_t i = NUMWORDS/2; i < NUMWORDS; i++)
		{
			d = (d*i) + (234 - b);
			a[i] = (uint8_t) d; //rand()%256;
		}
		ECC::b_mod(a, params.r, NUMWORDS/2);
		PrivKey ret;
		memcpy(ret.s, a, NUMWORDS);
		return ret;
	}
        static PubKey multiply(PrivKey k, PubKey P) {// return 0; } // Point * gen_shared_secret(uint8_t * a,Point * P0, Point * P1)
		PubKey ret;
		ECC::p_clear(&(ret.W));
		ECC::gen_shared_secret(k.s, &(P.W), &(ret.W));
		return ret;
	}
	static PubKey divide(PrivKey k, PubKey P) {
		PrivKey k_;
		ECC::b_clear(k_.s);
		ECC::f_inv(k.s, k_.s);
;
		return multiply(k_, P);
	}
	static PubKey encrypt(PubKey P) {
		PubKey ret, G;
		G.W=params.G;
		ECC::p_clear(&(ret.W));
		ECC::c_add(&(G.W), &(P.W), &(ret.W));
		return ret;
	}
	static PubKey decrypt(PubKey P) {
		PubKey ret, G;
		G.W=params.G;
		ECC::p_clear(&(ret.W));
		PubKey temp;
		ECC::p_clear(&(temp.W));
		ECC::f_add(G.W.x.val, G.W.y.val, temp.W.y.val);
		memcpy((void*) temp.W.x.val, (void*) G.W.x.val, NUMWORDS);
		ECC::c_add(&temp.W, &P.W, &ret.W);
		return ret;
	}
    };
   /** \brief Secured DFS Clustering.
    * 
    *  \ingroup clustering_concept
    *  \ingroup basic_algorithm_concept
    *  \ingroup clustering_algorithm
    * 
    */
    template<typename OsModel_P,
            typename Radio_P = typename OsModel_P::Radio,
	    typename Timer_P = typename OsModel_P::Timer>
	    class SecureDfsClustering {
        
    public:
        
        typedef int cluster_id_t;
        typedef int cluster_level_t;    //quite useless within current scheme, supported for compatibility issues
        typedef OsModel_P OsModel;
        typedef Radio_P Radio;
        typedef Timer_P Timer;
        
        typedef SecureDfsClustering<OsModel_P, Radio_P> self_t;
        
        typedef typename Radio::node_id_t node_id_t;
        typedef typename Radio::size_t size_t;
        typedef typename Radio::block_data_t block_data_t;

	typedef SecureDFSMessage<OsModel, Radio> Message;
        
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

	public:
	void enable();
	void disable();

        inline cluster_id_t cluster_id( cluster_level_t ) { return sid_; }
        inline cluster_level_t cluster_level() { return 0; }
	
	template<class T, void (T::*TMethod)(int)>
        inline int reg_changed_callback( T *obj_pnt ) {
            state_changed_callback_=cluster_delegate_t::from_method<T, TMethod>( obj_pnt );
        }
        inline void unreg_changed_callback( int idx ) {
            state_changed_callback_=cluster_delegate_t();
        }

	inline void set_cluster_head( bool cluster_head) { cluster_head_=cluster_head; }
	inline bool cluster_head() { return cluster_head_; }

	inline node_id_t parent() { return parent_; }

	inline int *group_key_x() {
		int *ret=new int[21];
		for(int i=0; i<21; i++) {
			ret[i]=group_key_.W.x.val[i+21];
		}
		return ret;
	}

	inline int *group_key_y() {
		int *ret=new int[21];
		for(int i=0; i<21; i++) {
			ret[i]=group_key_.W.y.val[i+21];
		}
		return ret;
	}

	///@name Construction / Destruction
        ///@{:137: err
        SecureDfsClustering():
	cluster_head_(false),
        sid_(-1),
	time_slice_(2001),
	received_ck_send(false)
        {};
        ~SecureDfsClustering() {};
        ///@}

	protected:
	void discover_neighbors();
	void timer_expired(void*);
        void receive(node_id_t from, size_t len, block_data_t* data);

	private:
        Radio& radio()
        { return *radio_; }
        
        Timer& timer()
        { return *timer_; }
        
        Radio * radio_;
        Timer * timer_;
      
	bool cluster_head_, received_ck_send;
        cluster_id_t sid_;
        int callback_id_;
        cluster_delegate_t state_changed_callback_;
	std::stack<node_id_t> neighbors_;
	node_id_t parent_;
	PrivKey private_key_;
	PubKey group_key_, parent_key_;
	int time_slice_;
    };

    template<typename OsModel_P,
            typename Radio_P,
            typename Timer_P>
    void
    SecureDfsClustering<OsModel_P, Radio_P, Timer_P>::
    enable() {
        ECC::init();
        radio().enable_radio();
	callback_id_ = radio().template reg_recv_callback<self_t, &self_t::receive>(this );
        if(cluster_head_) {
            //I am cluster_head, set SID_ to id and discover_neighbors
            sid_=parent_=radio().id();
	    private_key_=ECGDH::generate_random_ECfactor();
	    group_key_=ECGDH::generate_random_ECpoint();
            discover_neighbors();
		std::cout<<"Node "<<radio().id()<<" ( "<<sid_<<" ) generated group key : ";
		int *x=group_key_x();
		int *y=group_key_y();
		for(int i=0; i<7; i++) {
			printf("%x/%x ", x[i], y[i]);
		}
		std::cout<<std::endl;
        } else {
		private_key_=ECGDH::generate_random_ECfactor();
	}
    }

    template<typename OsModel_P,
            typename Radio_P,
            typename Timer_P>
    void
    SecureDfsClustering<OsModel_P, Radio_P, Timer_P>::
    discover_neighbors( void ) {
	Message omsg;
	omsg.set_msg_id(Message::NEIGHBOR_DISCOVERY);
        while(!neighbors_.empty()) neighbors_.pop();
        radio().send(radio().BROADCAST_ADDRESS, 1, (block_data_t*) &omsg);
        timer().template set_timer<self_t, &self_t::timer_expired>(time_slice_, this, (void*) 0);
    }

    template<typename OsModel_P,
            typename Radio_P,
            typename Timer_P>
    void
    SecureDfsClustering<OsModel_P, Radio_P, Timer_P>::
    timer_expired( void* data ) {
        if (neighbors_.empty()) {
            if(!cluster_head_) {
		Message msg;
		msg.set_msg_id(Message::RESUME);
		msg.set_sid(sid_);
		PubKey temp;
		ECC::p_clear(&temp.W);
		temp=ECGDH::encrypt(group_key_);
		msg.set_key(&temp);

                radio().send(parent_, msg.buffer_size(), (block_data_t*) &msg);
            } else {
		Message msg;
		msg.set_msg_id(Message::CK_SEND);
		msg.set_sid(sid_);
		PubKey temp;
		ECC::p_clear(&temp.W);
		temp=ECGDH::encrypt(group_key_);
		msg.set_key(&temp);
		received_ck_send=true;
                radio().send(radio().BROADCAST_ADDRESS, msg.buffer_size(),(block_data_t*) &msg);
	    }
        } else {
            node_id_t dest=neighbors_.top();
            neighbors_.pop();
	    Message msg;
	    msg.set_msg_id(Message::JOIN_REQUEST);
	    msg.set_sid(sid_);
	    msg.set_key(&group_key_);
            radio().send(dest, msg.buffer_size(), (block_data_t*) &msg);
        }

    }

    template<typename OsModel_P,
            typename Radio_P,
            typename Timer_P>
    void
    SecureDfsClustering<OsModel_P, Radio_P, Timer_P>::
    receive(node_id_t from, size_t len, block_data_t* data) {
        
        if ( from == radio().id() ) return;
        
	Message *msg=(Message*) data;
        if( msg->msg_id()==Message::NEIGHBOR_DISCOVERY ) {
            if(sid_==-1) {
		Message omsg;
		omsg.set_msg_id(Message::NEIGHBOR_REPLY );
                radio().send(from, 1, (block_data_t*) &omsg);
            }
        }
        
        else if( msg->msg_id() == Message::NEIGHBOR_REPLY ) {
            neighbors_.push(from);
        }
        
        else if( msg->msg_id() == Message::JOIN_REQUEST ) {
            if (sid_==-1) {
                parent_=from;
                sid_=msg->sid();
                state_changed_callback_(NODE_JOINED);
		memcpy(&parent_key_, msg->key(), 2*NUMWORDS);
                group_key_=ECGDH::multiply(private_key_, parent_key_);
                discover_neighbors();
		std::cout<<"Node "<<radio().id()<<" ( "<<sid_<<" ) (just joined) group key is: ";
		int *x=group_key_x();
		int *y=group_key_y();
		for(int i=0; i<7; i++) {
			printf("%x/%x ", x[i], y[i]);
		}
		std::cout<<std::endl;
            } else {
	        Message omsg;
		std::cout<<"sent join_deny"<<msg->sid()<<","<<sid_<<std::endl;
		omsg.set_msg_id(Message::JOIN_DENY);
                radio().send(from, 1,(block_data_t*) &omsg);
            }
	}
        
        else if( msg->msg_id() == Message::JOIN_DENY ) {
            discover_neighbors();
        }
        
        else if( msg->msg_id() == Message::RESUME ) {
	    group_key_=ECGDH::decrypt(*msg->key());
            discover_neighbors();
		std::cout<<"Node "<<radio().id()<<" ( "<<sid_<<" ) (just received resume) group key is: ";
		int *x=group_key_x();
		int *y=group_key_y();
		for(int i=0; i<7; i++) {
			printf("%x/%x ", x[i], y[i]);
		}
		std::cout<<std::endl;
        }

	else if( (msg->msg_id() == Message::CK_SEND) && (msg->sid()==sid_) ) {
	    group_key_=ECGDH::decrypt(*msg->key());
	    if(!received_ck_send) {
		    received_ck_send=true;
		    radio().send(radio().BROADCAST_ADDRESS, msg->buffer_size(),(block_data_t*) msg);
			std::cout<<"Node "<<radio().id()<<" ( "<<sid_<<" ) (just received ck_send) group key is: ";
			int *x=group_key_x();
			int *y=group_key_y();
			for(int i=0; i<7; i++) {
				printf("%x/%x ", x[i], y[i]);
			}
			std::cout<<std::endl;
	    }
	}

        
    }
}


#endif	/* _SECUREDFSCLUSTERING_H */

