//
// File:   hdlclustering.h
// Author: mpairakt
//
// Created on September 23, 2009, 12:32 PM
//

#ifndef _HDL_CLUSTERING_H
#define	_HDL_CLUSTERING_H

#include "util/delegates/delegate.hpp"

namespace wiselib {
    /**
     * \ingroup clustering_concept
     *  \ingroup basic_algorithm_concept
     * \ingroup clustering_algorithm
     * 
     * HDL clustering algorithm.
     */    
    template<typename OsModel_P,
            typename Radio_P = typename OsModel_P::Radio>
    class HdlClustering {
        
    public:
        
        typedef int cluster_id_t;
        typedef int cluster_level_t;    //quite useless within current scheme, supported for compatibility issues
        typedef OsModel_P OsModel;
        typedef Radio_P Radio;
        
        typedef HdlClustering<OsModel_P, Radio_P> self_t;
        
        typedef typename Radio::node_id_t node_id_t;
        typedef typename Radio::size_t size_t;
        typedef typename Radio::block_data_t block_data_t;
        
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
        
        inline void enable( void );
        inline void disable( void );
        
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
        
        inline void receive(node_id_t from, size_t len, block_data_t* data);
        
        inline void set_sink( bool sink) { sink_=sink; }
        
        ///@name Construction / Destruction
        ///@{
        HdlClustering() { HDL_=-1; };
        ~HdlClustering() {};
        ///@}
        
    private:
        Radio& radio()
        { return *radio_; }
        
        Radio * radio_;
        
        bool sink_;
        cluster_id_t HDL_;
        int callback_id_;
        cluster_delegate_t state_changed_callback_;
        
    };
    
    
    template<typename OsModel_P,
            typename Radio_P>
    void
    HdlClustering<OsModel_P, Radio_P>::
    enable( void ) {
        radio().enable_radio();
        callback_id_ = radio().template reg_recv_callback<self_t,
                &self_t::receive>( this );
        if(sink_) {
            //I am sink, bcast first message with HDL_=0
            typename radio().block_data_t m_hdl=HDL_=0;
            radio().send(radio().BROADCAST_ADDRESS, 1, &m_hdl);
        }
    }
    
    template<typename OsModel_P,
            typename Radio_P>
    void
    HdlClustering<OsModel_P, Radio_P>::
    disable( void ) {
        radio().unreg_recv_callback( callback_id_ );
        radio().disable();
    }
    
    template<typename OsModel_P,
            typename Radio_P>
    void
    HdlClustering<OsModel_P, Radio_P>::
    receive(node_id_t from, size_t len, block_data_t* data) {
        
        if ( from == radio().id( os() ) ) return;
        
        if((HDL_==-1)||(HDL_<*data-1)) {
            HDL_=*data+1;
            typename radio().block_data_t m_hdl=HDL_;
            radio().send(Radio::BROADCAST_ADDRESS, 1, &m_hdl);
            state_changed_callback_(NODE_JOINED);
            
        }
    }
}


#endif	/* _HDL_CLUSTERING_H */
