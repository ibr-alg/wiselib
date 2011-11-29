/*
 * File:   se_query_routing.h
 * Author: amaxilat
 */

#ifndef __SE_QUERY_ROUTING_H_
#define __SE_QUERY_ROUTING_H_




#include "algorithms/cluster/clustering_types.h"
#include "util/pstl/vector_static.h"
#include "algorithms/routing/query/queryMsg.h"
#include "algorithms/routing/query/routesMsg.h"

#undef DEBUG

namespace wiselib {

    /**
     * Semantic cluster head decision module
     */
    template<typename OsModel_P, typename Radio_P, typename Timer_P, typename Semantics_P, typename ND_P>
    class SeQueryRouting {
    public:
        typedef OsModel_P OsModel;

        //TYPEDEFS
        typedef Radio_P Radio;
        typedef Timer_P Timer;
        typedef typename OsModel::Debug Debug;

        typedef Semantics_P Semantics_t;
        typedef typename Semantics_t::group_entry_t group_entry_t;

        typedef ND_P nd_t;

        typedef SeQueryRouting<OsModel, Radio, Timer, Semantics_t, nd_t> self_type;
        typedef self_type* self_pointer_t;

        typedef typename Radio::size_t size_t;
        typedef typename Radio::block_data_t block_data_t;
        typedef typename Radio::node_id_t node_id_t;
        typedef typename Radio::message_id_t message_id_t;

        typedef QueryMsg<OsModel, Radio> QueryMsg_t;
        typedef RoutesMsg<OsModel, Radio> RoutesMsg_t;

        struct entry {
            group_entry_t group;
            node_id_t node;
            bool updated;
            int hops;
        };

        typedef struct entry routing_entry_t;
        typedef wiselib::vector_static<OsModel, routing_entry_t, 20 > routing_table_t;
        typedef typename routing_table_t::iterator routing_table_iterator_t;


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
         * Constructor
         */
        SeQueryRouting();

        /**
         * Destructor
         */
        ~SeQueryRouting();
        // --------------------------------------------------------------------

        /**
         * 
         * @return 
         */
        int destruct();
        // --------------------------------------------------------------------

        /**
         * 
         * @return 
         */
        int enable_radio(void);
        // --------------------------------------------------------------------

        /**
         * 
         * @return 
         */
        int disable_radio(void);
        // --------------------------------------------------------------------

        /**
         * 
         * @param receiver
         * @param len
         * @param data
         * @return 
         */
        int send(node_id_t receiver, size_t len, block_data_t *data);
        // --------------------------------------------------------------------

        /**
         * 
         * @param from
         * @param len
         * @param data
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
         * Handler fro incoming routes
         * @param from origin of the message
         * @param mess pointer to the message
         */
        void handle_route(node_id_t from, RoutesMsg_t* mess);

        // --------------------------------------------------------------------

        /**
         * 
         * @return 
         */
        node_id_t id();
        // --------------------------------------------------------------------
        /**
         * 
         * @param data
         */
        void update(void * data);
        // --------------------------------------------------------------------

        /**
         * 
         * @param gi
         * @return 
         */
        node_id_t next_hop(group_entry_t gi) {
            for (routing_table_iterator_t it = routing_table_.begin(); it != routing_table_.end(); ++it) {
                debug().debug("routing_table_ %s", it->group.c_str());
                if (it->group == gi) {
                    //                    debug().debug("routing_table_ found %x", it->node);
                    return it->node;
                }
            }
            return Radio::NULL_NODE_ID;
        };
        // -----------------------------------------------------------------------

        /**
         * INIT
         * initializes the values of radio and debug
         */
        void init(Radio& radio, Timer& timer, Debug& debug, Semantics_t &semantics, nd_t &nd) {
            radio_ = &radio;
            timer_ = &timer;
            debug_ = &debug;
            semantics_ = &semantics;
            nd_ = &nd;
            routing_table_.clear();
        }

    private:
        int callback_id_;
        bool updated_;
        const static int interval = 10000;
        routing_table_iterator_t rit;

        Semantics_t * semantics_;
        nd_t * nd_;
        routing_table_t routing_table_;

        Radio * radio_; //radio module
        Timer * timer_; //timer module
        Debug * debug_; //debug module

        Radio & radio() {
            return *radio_;
        }

        Timer & timer() {
            return *timer_;
        }

        Debug & debug() {
            return *debug_;
        }

        nd_t & nd() {
            return *nd_;
        }

        Semantics_t & semantics() {
            return *semantics_;
        }
    };

    // -----------------------------------------------------------------------
    // -----------------------------------------------------------------------
    // -----------------------------------------------------------------------

    template<typename OsModel_P,
    typename Radio_P, typename Timer_P,
    typename Semantics_P, typename ND_P>
    SeQueryRouting<OsModel_P, Radio_P, Timer_P, Semantics_P, ND_P>::
    SeQueryRouting()
    : callback_id_(0),
    updated_(false) {
        rit = routing_table_.begin();
    }
    // -----------------------------------------------------------------------

    template<typename OsModel_P,
    typename Radio_P, typename Timer_P,
    typename Semantics_P, typename ND_P>
    SeQueryRouting<OsModel_P, Radio_P, Timer_P, Semantics_P, ND_P>::
    ~SeQueryRouting() {
#ifdef DEBUG
        debug().debug("SeQueryRouting:dtor");
#endif
    }
    // -----------------------------------------------------------------------

    template<typename OsModel_P,
    typename Radio_P, typename Timer_P,
    typename Semantics_P, typename ND_P>
    int
    SeQueryRouting<OsModel_P, Radio_P, Timer_P, Semantics_P, ND_P>::
    enable_radio(void) {
#ifdef DEBUG
        debug().debug("SeQueryRouting: Boot for %x", radio().id());
#endif

        radio().enable_radio();
        callback_id_ = radio().template reg_recv_callback<self_type, &self_type::receive > (this);
        timer().template set_timer<self_type, &self_type::update > (10000, this, 0);

        return SUCCESS;
    }
    // -----------------------------------------------------------------------

    template<typename OsModel_P,
    typename Radio_P, typename Timer_P,
    typename Semantics_P, typename ND_P>
    int
    SeQueryRouting<OsModel_P, Radio_P, Timer_P, Semantics_P, ND_P>::
    disable_radio(void) {
#ifdef DEBUG
        debug().debug("SeQueryRouting: Disable");
#endif
        radio().unreg_recv_callback(callback_id_);
        radio().disable();
        return SUCCESS;
    }

    // -----------------------------------------------------------------------

    template<typename OsModel_P,
    typename Radio_P, typename Timer_P,
    typename Semantics_P, typename ND_P>
    int
    SeQueryRouting<OsModel_P, Radio_P, Timer_P, Semantics_P, ND_P>::
    send(node_id_t destination, size_t len, block_data_t *data) {

        if (radio().id() != destination) {
            QueryMsg_t * mess = (QueryMsg_t *) data;

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
                    node_id_t next = next_hop(group);
                    if (next == Radio::NULL_NODE_ID) {
                        debug().debug("NoRoute");
                    } else {
                        if (next != radio().id()) {
#ifdef DEBUG
                            debug().debug("SeQueryRouting: send con: %d next : %x", mess->contained(), next);
#endif
                            radio().send(next, len, data);
                            debug().debug("RS;%x;%d;%x", radio().id(), mess->alg_id(), next);
                        } else {
                            //                            debug().debug("No new route needs to route to ? ");
                        }
                    }
                }
            }
            return SUCCESS;
        } else {
            return ERR_UNSPEC;
        }
    }

    // -----------------------------------------------------------------------

    template<typename OsModel_P, typename Radio_P, typename Timer_P, typename Semantics_P, typename ND_P>
    void SeQueryRouting<OsModel_P, Radio_P, Timer_P, Semantics_P, ND_P>::
    receive(node_id_t from, size_t len, block_data_t * msg) {
        if (!nd().is_neighbor_bidi(from)) return;
        if (msg[0] == ATTRIBUTE) {
            SemaGroupsMsg_t* mess = (SemaGroupsMsg_t*) msg;
            uint8_t group_count = mess->contained();
            //            debug_->debug("contains %d ,len : %d", group_count, len);

            for (uint8_t i = 0; i < group_count; i++) {
                bool found = false;
                group_entry_t gi = group_entry_t(mess->get_statement_data(i), mess->get_statement_size(i));

                if (!routing_table_.empty()) {
                    for (routing_table_iterator_t it = routing_table_.begin(); it != routing_table_.end(); ++it) {
                        if (it->group == gi) {
                            it->node = from;
                            it->hops = 1;
                            it->updated = false;
                            found = true;
                            break;
                        }
                    }
                }
                if (!found) {
                    routing_entry_t newentry;
                    newentry.group = group_entry_t(mess->get_statement_data(i), mess->get_statement_size(i));
                    newentry.node = from;
                    newentry.hops = 1;
                    newentry.updated = true;
                    routing_table_.push_back(newentry);
                    updated_ = true;
                }
            }
        } else if (msg[0] == QueryMsg_t::SEROUTING) {
            message_id_t type = msg[1];
            switch (type) {
                case QueryMsg_t::QUERY:
                {
                    handle_query(from, (QueryMsg_t*) msg);
                    break;
                }
                case RoutesMsg_t::ROUTES:
                {
                    handle_route(from, (RoutesMsg_t*) msg);
                    break;
                }
            }
        }
    }

    // -----------------------------------------------------------------------

    template<typename OsModel_P,
    typename Radio_P, typename Timer_P,
    typename Semantics_P, typename ND_P>
    void
    SeQueryRouting<OsModel_P, Radio_P, Timer_P, Semantics_P, ND_P>::
    handle_query(node_id_t from, QueryMsg_t * mess) {

#ifdef DEBUG
        debug().debug("SeQueryRouting: receive %x->%x con:%d", mess->sender(), mess->destination(), mess->contained());
#endif
        size_t contained = mess->contained();

        if (contained > 0) {
            bool canAnswer = true;
            for (size_t i = 0; i < contained; i++) {
                group_entry_t ge = group_entry_t(mess->get_statement_data(0), mess->get_statement_size(0));
                canAnswer = canAnswer && semantics_->has_group(ge);
            }

            if (canAnswer) {
                debug().debug("can Answer");
                //TODO: REPLY
            } else {
                //TODO: FORWARD
                group_entry_t group = group_entry_t(mess->get_statement_data(0), mess->get_statement_size(0));
                node_id_t next = next_hop(group);

                mess->set_hops(mess->hops() + 1);

                //                        debug().debug("SeQueryRouting: next : %x count %d", next, mess->hops());
                if (next != Radio::NULL_NODE_ID) {
                    radio().send(next, mess->length(), (block_data_t*) mess);
                    debug().debug("RS;%x;%d;%x", radio().id(), mess->alg_id(), next);
                } else {
                    debug().debug("NoRoute");
                }
            }
        }
    }

    // -----------------------------------------------------------------------

    template<typename OsModel_P,
    typename Radio_P, typename Timer_P,
    typename Semantics_P, typename ND_P>
    void
    SeQueryRouting<OsModel_P, Radio_P, Timer_P, Semantics_P, ND_P>::
    handle_route(node_id_t from, RoutesMsg_t * mess) {

        group_entry_t group = group_entry_t(mess->get_statement_data(), mess->get_statement_size());
        bool found = false;
        for (routing_table_iterator_t it = routing_table_.begin(); it != routing_table_.end(); ++it) {
            if (it->group == group) {
                found = true;
                if (it->hops > mess->hops()) {
                    it->hops = mess->hops();
                    it->node = from;
                    it->updated = true;
                }
                break;
            }
        }
        if (!found) {
            routing_entry_t newentry;
            newentry.group = group_entry_t(mess->get_statement_data(), mess->get_statement_size());
            newentry.node = from;
            newentry.updated = true;
            routing_table_.push_back(newentry);
        }
    }
    // -----------------------------------------------------------------------

    template<typename OsModel_P,
    typename Radio_P, typename Timer_P,
    typename Semantics_P, typename ND_P>
    void
    SeQueryRouting<OsModel_P, Radio_P, Timer_P, Semantics_P, ND_P>::
    update(void *data) {
        //broadcast routing table
        if (!routing_table_.empty()) {
            RoutesMsg_t msg;
            if (rit == routing_table_.end()) {
                rit = routing_table_.begin();
            }
            if (rit->updated) {
                msg.set_hops(rit->hops + 1);
                msg.add_route(rit->group.data(), rit->group.size());
                radio().send(0xffff, msg.length(), (block_data_t *) & msg);
                debug().debug("RS;%x;%d;%x", radio().id(), msg.alg_id(), 0xffff);
                rit->updated = false;
            }
            rit++;
            if (rit == routing_table_.end()) {
                rit = routing_table_.begin();
            }

        }
        //reset timer
        timer().template set_timer<self_type, &self_type::update > (2000, this, 0);
    }
}

#endif //__SE_QUERY_ROUTING_H_
