/*
 * File:   cluster_radio.h
 * Author: Amaxilatis
 *
 * Created on March 11 2011
 */

#ifndef __CLUSTER_RADIO__
#define __CLUSTER_RADIO__

#include "algorithms/cluster/clustering_types.h"
#include "util/base_classes/radio_base.h"

#undef DEBUG
// Uncomment to enable Debug
#define DEBUG
#ifdef DEBUG
//#define DEBUG_CLUSTERRADIO
//#define DEBUG_CLUSTERRADIO_EXTRA
//#define DEBUG_CLUSTERRADIO_RECEIVED
//#define DEBUG_CLUSTERRADIO_PAYLOADS
#endif


namespace wiselib {

    template<typename OsModel_P, typename Radio_P, typename ND_P, typename Clustering_P>
    class ClusterRadio
    : public RadioBase < OsModel_P,
    typename Radio_P::node_id_t,
    typename Radio_P::size_t,
    typename Radio_P::block_data_t> {
    public:
        //TYPEDEFS
        typedef OsModel_P OsModel;
        // os modules
        typedef Radio_P Radio;
        typedef typename OsModel::Timer Timer;
        typedef typename OsModel::Debug Debug;
        typedef Clustering_P Clustering;
        typedef ND_P ND;

        // self type
        typedef ClusterRadio<OsModel_P, Radio_P, ND_P, Clustering_P> self_type;

        // data types
        typedef typename Radio::node_id_t node_id_t;
        typedef node_id_t cluster_id_t;
        typedef typename Radio::size_t size_t;
        typedef typename Radio::block_data_t block_data_t;
        typedef uint16_t cluster_level_t; //quite useless within current scheme, supported for compatibility issues

        struct RTentry {
            cluster_id_t dest_cluster_;
            size_t distance;
            node_id_t link;
            bool exists_;
        };

        enum SpecialNodeIds {
            BROADCAST_ADDRESS = Radio::BROADCAST_ADDRESS, ///< All nodes in communication rnage
            NULL_NODE_ID = Radio::NULL_NODE_ID
            ///< Unknown/No node id
        };

        typedef RTentry RTentry_t;

        //typedef wiselib::vector_static<OsModel, RTentry_t, 7 > ClusterRoutingTable;
        //typedef typename ClusterRoutingTable::iterator ClusterRoutingTable_iterator_t;

        struct ND_payload {
            cluster_id_t cluster;
            size_t dist;
            node_id_t link;
        };

        typedef ND_payload ND_payload_t;

        enum event_ids {
            ADD_ROUTE = 1,
            REMOVE_ROUTE = 2
        };

        typedef delegate4<void, uint8_t, node_id_t, cluster_id_t, node_id_t> notify_delegate_t;
#define MAX_EVENT_RECEIVERS 2
        typedef vector_static<OsModel, notify_delegate_t, MAX_EVENT_RECEIVERS> NotifyCallbackVector;
        typedef typename NotifyCallbackVector::iterator NotifyCallbackVectorIterator;


        //        typedef wiselib::vector_static<OsModel, node_id_t, 30 > vector_t;

        /*
         * Constructor
         * */
        ClusterRadio() :
        enabled_(false)//CDOWN, enable_debug_(false)
        {
        }

        /*
         * Destructor
         * */
        ~ClusterRadio() {
        }

        /*
         * INIT
         * initializes the values of radio timer and debug
         */
        void init(Radio& radiot, Timer& timert, Debug& debugt, ND& nd, Clustering& clusteringt) {
            radio_ = &radiot;
            timer_ = &timert;
            debug_ = &debugt;
            clustering_ = &clusteringt;
            clustering_->template reg_state_changed_callback<self_type, &self_type::reset > (this);
            clustering_->template reg_notify_cradio<self_type, &self_type::notify_cradio > (this);

            nd_ = &nd;

            uint8_t flags = ND::NEW_PAYLOAD_BIDI | ND::DROPPED_NB | ND::LOST_NB_BIDI;
//CDOWN            nd_->template reg_event_callback<self_type, &self_type::nd_callback > (CLRADIO, flags, this);
            nd_->register_payload_space((uint8_t) CLRADIO);

            notify_callbacks_.clear();


            for (size_t i = 0; i < 10; i++) {
                rt_[i].dest_cluster_ = 0xffff;
            }

        }

        /* set functions */

        void reg_debug_callback() {
//            reg_event_callback<self_type, &self_type::debug_callback > (this);
//            enable_debug_ = true;
        }


        // --------------------------------------------------------------------

        template<class T, void(T::*TMethod)(uint8_t, node_id_t, cluster_id_t, node_id_t) >
        uint8_t reg_event_callback(T *obj_pnt) {

            if (notify_callbacks_.empty())
                notify_callbacks_.assign(MAX_EVENT_RECEIVERS, notify_delegate_t());

            for (NotifyCallbackVectorIterator
                    it = notify_callbacks_.begin();
                    it != notify_callbacks_.end();
                    it++) {
                if ((*it) == notify_delegate_t()) {
                    (*it) = notify_delegate_t::template from_method<T, TMethod > (obj_pnt);
                    return 0;
                }
            }

/*            for (unsigned int i = 0; i < notify_callbacks_.size(); ++i) {
                if (notify_callbacks_.at(i) == notify_delegate_t()) {
                    notify_callbacks_.at(i) = notify_delegate_t::template from_method<T, TMethod > (obj_pnt);
                    return i;
                }
            }
*/
            return -1;
        }
        // --------------------------------------------------------------------

        int unreg_event_callback(int idx) {
            notify_callbacks_.at(idx) = notify_delegate_t();
            return idx;
        }
        // --------------------------------------------------------------------

        void notify_listeners(uint8_t event, node_id_t from, cluster_id_t cluster, node_id_t link) {
            for (NotifyCallbackVectorIterator it = notify_callbacks_.begin();
                    it != notify_callbacks_.end(); ++it) {
                if (*it != notify_delegate_t()) (*it)(event, from, cluster, link);

            }
//            debug_callback(event, from, cluster, link);
        }

        void present_neighbors(void) {
		if (!enabled_) return;
            //for (ClusterRoutingTable_iterator_t it = rt_.begin(); it != rt_.end(); it++) {
            for (size_t i = 0; i < 10; i++) {
                if (rt_[i].dest_cluster_ != 0xffff) {
                    //debug().debug("%x -> %x -> %x in %d exits:%d", radio().id(), rt_[i].link, rt_[i].dest_cluster_, rt_[i].distance, rt_[i].exists_);
                }
            }
        }

        /*
         * Enable
         * enables the mbfsclustering module
         * enable chd it and jd modules
         * initializes values
         * registers callbacks
         * calls find head to start clustering
         * */
        inline void enable() {

            if (enabled_)
                return;
            //set as enabled
            enabled_ = true;
            gateway_connections_ = 0;

            for (size_t i = 0; i < 10; i++) {
                rt_[i].dest_cluster_ = 0xffff;
            }
#ifdef DEBUG_CLUSTERRADIO
            debug().debug("CLradio::Enable::%x::", radio().id());
#ifdef SHAWN
            debug().debug("\n");
#endif
#endif
            // receive receive callback
            receive_callback_id_ = radio().template reg_recv_callback<self_type, &self_type::receive > (this);

            timer().template set_timer<self_type, &self_type::clean_lists > (
                    100 * 1000, this, (void *) 0);
        }

        int connections() {
		if (!enabled_)return 0;
            size_t len = 0;
            for (size_t i = 0; i < 10; i++) {
                if (rt_[i].dest_cluster_ != 0xffff) {
                    len++;
                }
            }
            return len;
        }

        void clean_lists(void * a) {
//            present_neighbors();
            for (size_t i = 0; i < 10; i++) {
                if (rt_[i].exists_ == false) {
                    if (rt_[i].dest_cluster_ != 0xffff) {
                        notify_listeners(REMOVE_ROUTE, radio().id(), rt_[i].dest_cluster_, rt_[i].link);
                        //debug().debug("CLradio::Node%x::RemoveRoute::to%x::through%x::%d::", radio().id(), rt_[i].dest_cluster_, rt_[i].link, connections());
                        rt_[i].dest_cluster_ = 0xffff;
                    }
                }
                rt_[i].exists_ = false;
            }
            timer().template set_timer<self_type, &self_type::clean_lists > (
                    9000, this, (void *) 0);
        }

        void send(cluster_id_t destination, size_t len, uint8_t * msg) {
		if (!enabled_) return ;

            if (destination == Radio::BROADCAST_ADDRESS) {


//                debug().debug("CLradio::Send::from%x::to%x::", radio().id(), destination);
#ifdef DEBUG_CLUSTERRADIO
                debug().debug("CLradio::Send::from%x::to%x::", radio().id(), destination);
#ifdef SHAWN
                debug().debug("\n");
#endif
#endif

                //for (ClusterRoutingTable_iterator_t it = rt_.begin(); it != rt_.end(); it++) {
                for (size_t i = 0; i < 10; i++) {
                    if (rt_[i].dest_cluster_ != 0xffff && rt_[i].dest_cluster_ != radio().NULL_NODE_ID) {
                        send(rt_[i].dest_cluster_, len, msg);
                    }
                }

            } else {

//                debug().debug("CLradio::Send::from%x::to%x::", radio().id(), destination);
#ifdef DEBUG_CLUSTERRADIO
                debug().debug("CLradio::Send::from%x::to%x::", radio().id(), destination);
#ifdef SHAWN
                debug().debug("\n");
#endif
#endif


                node_id_t next_link = Radio::NULL_NODE_ID;

                //for (ClusterRoutingTable_iterator_t it = rt_.begin(); it != rt_.end(); it++) {
                for (size_t i = 0; i < 10; i++) {
                    if (rt_[i].dest_cluster_ != 0xffff) {
                        if (rt_[i].dest_cluster_ == destination) {
                            next_link = rt_[i].link;
                            break;
                        }
                    }
                }

                if (next_link != Radio::NULL_NODE_ID) {

                    ClusterRadioMsg<OsModel, Radio> mess;
                    mess.set_source(clustering().cluster_id());
                    mess.set_destination(destination);
                    mess.set_payload(msg, len);
                    radio().send(next_link, mess.length(), (uint8_t*) & mess);

//                    debug().debug("CLradio::Route2KNOWN::from%x::to%x::through%x::len%d::", radio().id(), destination, next_link, len);
#ifdef DEBUG_CLUSTERRADIO
                    debug().debug("CLradio::Route2KNOWN::from%x::to%x::through%x::len%d::", radio().id(), destination, next_link, len);
#ifdef SHAWN
                    debug().debug("\n");
#endif
#else
//                    if (enable_debug_) {
                        debug().debug("CLRS;%x;%d;%x;%x", radio().id(), mess.msg_id(), next_link,msg[0+3]);
//                    }
#endif

                } else {


//                    debug().debug("CLradio::UnknownRoute::from%x::to%x::len%x::", radio().id(), destination, len);
#ifdef DEBUG_CLUSTERRADIO
                    debug().debug("CLradio::UnknownRoute::from%x::to%x::len%x::", radio().id(), destination, len);
#ifdef SHAWN
                    debug().debug("\n");
#endif
#endif
                }
            }
        }

        /*
         * Disable
         * disables the bfsclustering module
         * unregisters callbacks
         * */
        inline void disable(void) {
            // Unregister the callback
            radio().unreg_recv_callback(receive_callback_id_);
            enabled_ = false;
        }

    protected:

        void nd_callback(uint8_t event, node_id_t link, uint8_t len, uint8_t * data) {
            if (ND::NEW_PAYLOAD_BIDI == event) {
            }
        }

        void notify_cradio(uint8_t event, cluster_id_t cluster, node_id_t link) {
		if (!enabled_) return;
            if (event == RESUME) {
                if (!routeExists(cluster)) {
                    //debug().debug("Got a new link %x<->%x cluster %x", radio().id(), link, cluster);
                    if (addRoute(cluster, link, 1)) {
                        //if (clustering().is_gateway()) {
                        notify_listeners(ADD_ROUTE, radio().id(), cluster, link);
//			debug().debug("CLRC;%x;%x",radio().id(),clustering().parent());

                        // }
                    }
                }
                //debug().debug("Reporting link!%x -> %x", radio().id(), cluster);
                uint8_t add[2 + sizeof (cluster_id_t) + 1];
                add[0] = 90;
                add[1] = 90;
                add[1 + sizeof (cluster_id_t)] = 1;
                cluster_id_t toadd = cluster;
                memcpy(add + 2, &toadd, sizeof (cluster_id_t));
                radio().send(clustering().parent(), 2 + sizeof (cluster_id_t) + 1, add);
            }
        }

        bool routeExists(cluster_id_t cluster) {
		if (!enabled_) return false;
            for (size_t i = 0; i < 10; i++) {
                if (rt_[i].dest_cluster_ == cluster) {
                    rt_[i].exists_ = true;
//			debug().debug("CLRC;%x;%x",radio().id(),clustering().parent());
                    return true;
                }
            }
            return false;
        }

        bool addRoute(cluster_id_t cluster, node_id_t link, uint8_t dista) {
		if (!enabled_) return false;
            if (cluster == clustering().cluster_id()) {
                //debug().debug("Same Cluster %x -> %x -> %x", radio().id(), link, cluster);
                return false;
            }

            for (size_t i = 0; i < 10; i++) {
                //if routeExists
                if (rt_[i].dest_cluster_ == cluster) {
                    if (rt_[i].distance <= dista) {
                        rt_[i].exists_ = true;

                        return false;
                    } else {
                        rt_[i].link = link;
                        rt_[i].distance = dista;
                        rt_[i].exists_ = true;
                        return true;
                    }
                }
            }

            for (size_t i = 0; i < 10; i++) {
                if (rt_[i].dest_cluster_ == 0xffff) {
                    rt_[i].dest_cluster_ = cluster;
                    rt_[i].distance = dista;
                    rt_[i].link = link;
                    rt_[i].exists_ = true;
                    return true;
                }
            }
            return false;
        }

        void send2head(node_id_t destination, size_t len, uint8_t * msg) {
		if (!enabled_) return false;

            ClusterRadioMsg<OsModel, Radio> mess;
            mess.set_destination(destination);
            mess.set_payload(msg, len);
#ifdef DEBUG_CLUSTERRADIO
            debug().debug("CLradio::Sending::from%x::to%x::for%x::len%d::", radio().id(), clustering().parent(), mess.destination(), mess.length());
#ifdef SHAWN
            debug().debug("\n");
#endif
#endif
            radio().send(clustering().parent(), mess.length(), (uint8_t*) & mess);
		debug().debug("CLRS;%x;%d;%x",radio().id(),mess.msg_id(),clustering().parent());
        }

        /*
         * RECEIVE
         * respond to the new messages received
         * callback from the radio
         * */
        void receive(node_id_t from, size_t len, block_data_t * data) {

            if (radio().id() == from) return;

//CDOWN            uint8_t recvm[len];
//CDOWN            memcpy(recvm, data, len);
            uint8_t *recvm = data;

//CDOWN            uint8_t type = *recvm;

            if (*recvm == CLRADIO_MSG) {
//CDOWN
               ClusterRadioMsg<OsModel, Radio> *mess;
               mess = (ClusterRadioMsg<OsModel, Radio> *)recvm;
//CDOWN        memcpy(&mess, recvm, len);
#ifdef DEBUG_CLUSTERRADIO_RECEIVED
                debug().debug("CLradio::Received::node%x::type%d::from%x::to%x::", radio().id(), mess->msg_id(), from, mess->destination());
#ifdef SHAWN
                debug().debug("\n");
#endif
#endif
                if (clustering().cluster_id() == mess->destination()) {
                    if (radio().id() == mess->destination()) {
                        uint8_t payload[mess->payload_size()];
                        mess->get_payload(payload);
                        notify_receivers(mess->source(), mess->payload_size(), payload);

#ifdef DEBUG_CLUSTERRADIO_RECEIVED
                        debug().debug("CLradio::Absorb::node%x::from%x::", radio().id(), mess->source());
#ifdef SHAWN
                        debug().debug("\n");
#endif
#else
                        //                        if (enable_debug_) {
                        //                            debug().debug("CLradio::Absorb::node%x::from%x::", radio().id(), mess->source());
                        //                        }
#endif

                    } else {
#ifdef DEBUG_CLUSTERRADIO_RECEIVED
                        debug().debug("CLradio::Route2Head::node%x::to%x::forw::", radio().id(), clustering().parent());
#ifdef SHAWN
                        debug().debug("\n");
#endif
#else
//                        if (enable_debug_) {
                            debug().debug("CLRS;%x;%d;%x;%x", radio().id(), CLRADIO_MSG, clustering().parent(),recvm[ClusterRadioMsg<OsModel, Radio>::PAYLOAD+3]);
//                        }
#endif

                        radio().send(clustering().parent(), len, recvm);
                    }
                } else {
                    node_id_t next_link = Radio::NULL_NODE_ID;

                    //for (ClusterRoutingTable_iterator_t it = rt_.begin(); it != rt_.end(); it++) {
                    for (size_t i = 0; i < 10; i++) {
                        if (rt_[i].dest_cluster_ != 0xffff) {
                            if (rt_[i].dest_cluster_ == mess->destination()) {
                                next_link = rt_[i].link;
                                break;
                            }
                        }
                    }

                    //drop a circling message
                    if (mess->source() == radio().id()) return;

                    if (next_link != Radio::NULL_NODE_ID) {
#ifdef DEBUG_CLUSTERRADIO

                        debug().debug("CLradio::SendNextLink::node%x::next%x::forw::", radio().id(), next_link);
#ifdef SHAWN
                        debug().debug("\n");
#endif
#else
//                        if (enable_debug_) {
                            debug().debug("CLRS;%x;%d;%x;%x", radio().id(), CLRADIO_MSG, next_link,recvm[ClusterRadioMsg<OsModel, Radio>::PAYLOAD+3]);
//                        }
#endif
                        radio().send(next_link, len, recvm);
                    } else {
#ifdef DEBUG_CLUSTERRADIO

                        debug().debug("CLradio::UnknownNextLink::node%x::dest%x::forw::", radio().id(), mess->destination());
#ifdef SHAWN
                        debug().debug("\n");
#endif
#endif
                    }

                }
            } else if (*recvm == 90) {
                if (recvm[1] == 90) {
                    cluster_id_t cluster;
                    uint8_t dist;
                    memcpy(&cluster, recvm + 2, sizeof (cluster_id_t));
                    dist = recvm[1 + sizeof (cluster_id_t)];
                    if (addRoute(cluster, from, dist + 1)) {
                        notify_listeners(ADD_ROUTE, radio().id(), cluster, from);
//			debug().debug("CLRC;%x;%x",radio().id(),clustering().parent());
                        //debug().debug("CLradio::Node%x::NewRoute::to%x::through%x::cl::%d::", radio().id(), cluster, from, connections());
                    }
                    if (clustering().parent() != radio().id()) {
                        //debug().debug("Reporting link!%x -> %x", radio().id(), cluster);
                        uint8_t add[2 + sizeof (cluster_id_t) + 1];
                        add[0] = 90;
                        add[1] = 90;
                        add[1 + sizeof (cluster_id_t)] = dist + 1;
                        cluster_id_t toadd = cluster;
                        memcpy(add + 2, &toadd, sizeof (cluster_id_t));
                        radio().send(clustering().parent(), 2 + sizeof (cluster_id_t) + 1, add);
			//debug().debug("CLRC;%x;%x",radio().id(),clustering().parent());

                    }
                } else if (recvm[1] == 91) {
                    cluster_id_t cluster;
                    memcpy(&cluster, recvm + 2, sizeof (cluster_id_t));
                    if (routeExists(cluster)) {
                        for (size_t i = 0; i < 10; i++) {
                            if (rt_[i].dest_cluster_ == cluster) {
                                rt_[i].dest_cluster_ = 0xffff;
                            }
                        }
                        radio().send(0xffff, len, recvm);
			debug().debug("CLRC;%x;%x",radio().id(),0xffff);

                    }
                }
            }
        }

        void reset(int event) {
            //debug().debug("Reseting Cradio %x", radio().id());
            if ((event == CLUSTER_HEAD_CHANGED) || (event == NODE_JOINED)) {
                //for (ClusterRoutingTable_iterator_t it = rt_.begin(); it != rt_.end(); it++) {
                for (size_t i = 0; i < 10; i++) {
                    if (rt_[i].dest_cluster_ != 0xffff) {
                        if (clustering().is_gateway()) {
                            //notify_listeners(REMOVE_ROUTE, radio().id(), rt_[i].dest_cluster_, rt_[i].link);
                        }
                        //debug().debug("CLradio::Node%x::RemoveRoute::to%x::through%x::", radio().id(), rt_[i].dest_cluster_, rt_[i].link);

#ifdef DEBUG_CLUSTERRADIO

                        debug().debug("CLradio::Node%x::RemoveRoute::to%x::through%x::", radio().id(), rt_[i].dest_cluster_, rt_[i].link);
#ifdef SHAWN
                        debug().debug("\n");
#endif

#endif

                    }
                }

                for (int i = 0; i < 10; i++)
                    rt_[i].dest_cluster_ = 0xffff;

                gateway_connections_ = 0;
                uint8_t payload;
                nd().set_payload((uint8_t) CLRADIO, (uint8_t*) & payload, 0);
            }
        }


    private:
        int receive_callback_id_; // receive message callback

        bool enabled_;
//CDOWN        bool enable_debug_;
        size_t gateway_connections_;


        NotifyCallbackVector notify_callbacks_;


        Radio * radio_; // radio module
        Timer * timer_; // timer module
        Debug * debug_; // debug module

        //ClusterRoutingTable rt_;
        RTentry_t rt_[10];

        Clustering * clustering_;
        ND * nd_;

        Radio & radio() {

            return *radio_;
        }

        Timer & timer() {

            return *timer_;
        }

        Debug & debug() {
#ifdef SHAWN
            debug_->debug("\n");
#endif
            return *debug_;
        }

        Clustering & clustering() {

            return *clustering_;
        }

        ND & nd() {
            return *nd_;
        }

    };
}
#endif
