/*
 * File:   se_query_flooding.h
 * Author: amaxilat
 */

#ifndef __SE_QUERY_FLOODING_H_
#define __SE_QUERY_FLOODING_H_

#include "util/base_classes/routing_base.h"
#include "queryMsg.h"

#undef DEBUG

//#define DEBUG
#define DEBUG_SQ

#include "algorithms/routing/flooding/flooding_algorithm.h"

namespace wiselib {

    /**
     * Sends queries for gathering data from SEs
     */
    template<typename OsModel_P, typename Radio_P, typename Timer_P, typename Semantics_P, typename ND_P>
    class SeQueryFlooding : RoutingBase<OsModel_P, Radio_P> {
    public:
        typedef OsModel_P OsModel;

        //TYPEDEFS
        typedef Radio_P Radio;
        typedef Timer_P Timer;
        typedef typename OsModel::Debug Debug;

        typedef Semantics_P Semantics_t;
        typedef typename Semantics_t::group_entry_t group_entry_t;

        typedef ND_P nd_t;

        typedef SeQueryFlooding<OsModel, Radio, Timer, Semantics_t, nd_t> self_type;
        typedef self_type* self_pointer_t;

        typedef typename Radio::size_t size_t;
        typedef typename Radio::block_data_t block_data_t;
        typedef typename Radio::node_id_t node_id_t;
        typedef typename Radio::message_id_t message_id_t;

        typedef wiselib::StaticArrayRoutingTable<OsModel, Radio, 20 > FloodingStaticMap;
        typedef wiselib::FloodingAlgorithm<OsModel, FloodingStaticMap, Radio, Debug> routing_t;

        typedef QueryMsg<OsModel, Radio> QueryMsg_t;


        // --------------------------------------------------------------------

        enum ErrorCodes {
            SUCCESS = OsModel::SUCCESS,
            ERR_UNSPEC = OsModel::ERR_UNSPEC,
            ERR_NETDOWN = OsModel::ERR_NETDOWN
        };
        // --------------------------------------------------------------------

        enum SpecialNodeIds {
            BROADCAST_ADDRESS = Radio_P::BROADCAST_ADDRESS, ///< All nodes in communication range
            NULL_NODE_ID = Radio_P::NULL_NODE_ID ///< Unknown/No node id
        };
        // --------------------------------------------------------------------

        enum Restrictions {
            MAX_MESSAGE_LENGTH = Radio_P::MAX_MESSAGE_LENGTH
        };
        // --------------------------------------------------------------------

        /**
         * Constructor.
         */
        SeQueryFlooding();

        /**
         * Destructor.
         */
        ~SeQueryFlooding();
        // --------------------------------------------------------------------

        /**
         * Destroy object.
         * @return 
         */
        int destruct();
        // --------------------------------------------------------------------

        /**
         * Enable.
         * @return SUCCESS if enabled
         */
        int enable_radio(void);
        // --------------------------------------------------------------------

        /**
         * Disable.
         * @return SUCCESS if disabled
         */
        int disable_radio(void);
        // --------------------------------------------------------------------

        /**
         * Send a Query to the network.
         * @param receiver destination of the message
         * @param len length of the message
         * @param data the message
         * @return SUCCESS if the message was sent
         */
        int send(node_id_t receiver, size_t len, block_data_t *data);
        // --------------------------------------------------------------------

        /**
         * Receive a message from the Radio.
         * @param from the source of the message
         * @param len the length of the message
         * @param data the message
         */
        void receive(node_id_t from, size_t len, block_data_t * msg);

        // --------------------------------------------------------------------

        /**
         * Handler for incoming queries
         * @param from origin of the message
         * @param mess pointer to the message
         */
        void handle_query(node_id_t from, QueryMsg_t* mess);

        // --------------------------------------------------------------------

        /**
         * Handler fro incoming replies
         * @param from origin of the message
         * @param mess pointer to the message
         */
        void handle_reply(node_id_t from, QueryMsg_t* mess);

        // --------------------------------------------------------------------

        /**
         * The id of the device.
         * @return id provided by the radio
         */
        node_id_t id();
        // --------------------------------------------------------------------

        /**
         * Initializes the algorithm.
         * @param radio the radio interface
         * @param debug the debug interface
         * @param semantics a semantics storage backend
         * @param nd an nd algorithm
         */
        void init(Radio& radio, Timer& timer, Debug& debug, Semantics_t &semantics, nd_t &nd) {
            radio_ = &radio;
            debug_ = &debug;
            semantics_ = &semantics;
            nd_ = &nd;
        }

    private:

        int callback_id_;

        routing_t routing_;

        Radio * radio_; //radio module
        Debug * debug_; //debug module
        Semantics_t * semantics_;
        nd_t * nd_;

        Radio & radio() {
            return *radio_;
        }

        Debug & debug() {
            return *debug_;
        }

        Semantics_t & semantics() {
            return *semantics_;
        }

        nd_t & nd() {
            return *nd_;
        }

        routing_t & routing() {
            return routing_;
        }
    };

    // -----------------------------------------------------------------------
    // -----------------------------------------------------------------------
    // -----------------------------------------------------------------------

    template<typename OsModel_P,
    typename Radio_P, typename Timer_P,
    typename Semantics_P, typename ND_P>
    SeQueryFlooding<OsModel_P, Radio_P, Timer_P, Semantics_P, ND_P>::
    SeQueryFlooding()
    : callback_id_(0) {
    }
    // -----------------------------------------------------------------------

    template<typename OsModel_P,
    typename Radio_P, typename Timer_P,
    typename Semantics_P, typename ND_P>
    SeQueryFlooding<OsModel_P, Radio_P, Timer_P, Semantics_P, ND_P>::
    ~SeQueryFlooding() {
#ifdef DEBUG
        debug().debug("SeQueryFlooding:dtor");
#endif
    }
    // -----------------------------------------------------------------------

    template<typename OsModel_P,
    typename Radio_P, typename Timer_P,
    typename Semantics_P, typename ND_P>
    int
    SeQueryFlooding<OsModel_P, Radio_P, Timer_P, Semantics_P, ND_P>::
    enable_radio(void) {
#ifdef DEBUG
        debug().debug("SeQueryFlooding: Boot for %x", radio().id());
#endif

        routing().init(*radio_, *debug_);
        routing().enable_radio();
        callback_id_ = routing().template reg_recv_callback<self_type, &self_type::receive > (this);

        return SUCCESS;
    }
    // -----------------------------------------------------------------------

    template<typename OsModel_P,
    typename Radio_P, typename Timer_P,
    typename Semantics_P, typename ND_P>
    int
    SeQueryFlooding<OsModel_P, Radio_P, Timer_P, Semantics_P, ND_P>::
    disable_radio(void) {
#ifdef DEBUG
        debug().debug("SeQueryRouting: Disable");
#endif
        routing().unreg_recv_callback(callback_id_);
        routing().disable();
        return SUCCESS;
    }

    // -----------------------------------------------------------------------

    template<typename OsModel_P,
    typename Radio_P, typename Timer_P,
    typename Semantics_P, typename ND_P>
    int
    SeQueryFlooding<OsModel_P, Radio_P, Timer_P, Semantics_P, ND_P>::
    send(node_id_t destination, size_t len, block_data_t *data) {
        if (radio().id() != destination) {
            QueryMsg_t* mess = (QueryMsg_t*) data;

            size_t contained = mess->contained();

            if (contained > 0) {
                bool canAnswer = true;
                group_entry_t group;
                for (size_t i = 0; i < contained; i++) {
                    group = group_entry_t(mess->get_statement_data(0), mess->get_statement_size(0));
                    canAnswer = canAnswer && semantics().has_group(group);
                }

                if (canAnswer) {
                    debug().debug("SR;%x;%s", mess->sender(), group.c_str());
                } else {
                    routing().send(Radio::BROADCAST_ADDRESS, len, data);
                }
            }
        }

        return SUCCESS;
    }
    // -----------------------------------------------------------------------

    template<typename OsModel_P,
    typename Radio_P, typename Timer_P,
    typename Semantics_P, typename ND_P>
    void
    SeQueryFlooding<OsModel_P, Radio_P, Timer_P, Semantics_P, ND_P>::
    receive(node_id_t from, size_t len, block_data_t * msg) {
        message_id_t type = msg[1];
        if (msg[0] == QueryMsg_t::SEROUTING) {
            if (type == QueryMsg_t::QUERY) {
                handle_query(from, (QueryMsg_t*) msg);
            } else if (type == QueryMsg_t::REPLY) {
                handle_reply(from, (QueryMsg_t*) msg);
            }
        }
    }

    // -----------------------------------------------------------------------

    template<typename OsModel_P,
    typename Radio_P, typename Timer_P,
    typename Semantics_P, typename ND_P>
    void
    SeQueryFlooding<OsModel_P, Radio_P, Timer_P, Semantics_P, ND_P>::
    handle_query(node_id_t from, QueryMsg_t * mess) {
#ifdef DEBUG
        debug().debug("SeQueryRouting: receive %x->%x con:%d", mess->sender(), mess->destination(), mess->contained());
#endif
        size_t contained = mess->contained();

        if (contained > 0) {
            bool canAnswer = true;
            group_entry_t group;
            for (size_t i = 0; i < contained; i++) {
                group = group_entry_t(mess->get_statement_data(0), mess->get_statement_size(0));
                canAnswer = canAnswer && semantics().has_group(group);
            }

            if (canAnswer) {
                debug().debug("SeQueryRouting: can Answer");
                //TODO: REPLY
                QueryMsg_t reply;
                reply.set_msg_id(QueryMsg_t::REPLY);
                reply.set_destination(mess->sender());
                reply.set_sender(radio().id());
                reply.add_statement(group.data(), group.size());
                routing().send(Radio::BROADCAST_ADDRESS, reply.length(), (block_data_t *) & reply);
            }
        }
    }

    // -----------------------------------------------------------------------

    template<typename OsModel_P,
    typename Radio_P, typename Timer_P,
    typename Semantics_P, typename ND_P>
    void
    SeQueryFlooding<OsModel_P, Radio_P, Timer_P, Semantics_P, ND_P>::
    handle_reply(node_id_t from, QueryMsg_t * mess) {
        if (mess->destination() == radio().id()) {
            size_t contained = mess->contained();
            if (contained > 0) {
                group_entry_t group = group_entry_t(mess->get_statement_data(0), mess->get_statement_size(0));
                debug().debug("SR;%x;%s", mess->sender(), group.c_str());
            }

        }
    }

}

#endif //__SE_QUERY_ROUTING_H_
