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
#ifndef __ALGORITHMS_ROUTING_OLSR_ROUTING_H__
#define __ALGORITHMS_ROUTING_OLSR_ROUTING_H__

#include "routing_base.h"
#include "olsr_routing_types.h"
#include "olsr_routing_msg.h"
#include "olsr_broadcast_hello_msg.h"
#include "olsr_broadcast_tc_msg.h"
#include <string.h>
#include <time.h>
#include <cstdlib>
#include <math.h>
#include <vector>
#include <map>
#include <set>
#include <iterator>

//#define DEBUG_OLSRROUTING

/**********************************************************************************/
//                          Useful Macros                                          /
/**********************************************************************************/

#ifndef NULL
#define NULL 0
#endif

#ifndef MIN
#define	MIN(a, b) (((a) < (b)) ? (a) : (b))
#endif

#ifndef MAX
#define	MAX(a, b) (((a) > (b)) ? (a) : (b))
#endif

/********** Intervals **********/
#define OLSR_HELLO_INTERVAL		2
#define OLSR_TC_INTERVAL		5
#define OLSR_REFRESH_INTERVAL	2
#define OLSR_NEIGHB_HOLD_TIME	3*OLSR_REFRESH_INTERVAL
#define OLSR_TOP_HOLD_TIME		3*OLSR_TC_INTERVAL
#define OLSR_DUP_HOLD_TIME		30

#define OLSR_UNSPEC_LINK	0
#define OLSR_ASYM_LINK		1
#define OLSR_SYM_LINK		2
#define OLSR_LOST_LINK		3

#define OLSR_NOT_NEIGH		0
#define OLSR_SYM_NEIGH		1
#define OLSR_MPR_NEIGH		2

#define OLSR_WILL_NEVER		0
#define OLSR_WILL_LOW		1
#define OLSR_WILL_DEFAULT	3
#define OLSR_WILL_HIGH		6
#define OLSR_WILL_ALWAYS	7

#define OLSR_MAX_HELLOS			12							// Maximum number of hello_msg per Hello message (4 link types * 3 neighbor types)
#define OLSR_MAX_ADDRS			64							// Maximum number of Neighbor_Addr per hello_msg

#define OLSR_MSG_HDR_SIZE		12							// Size (in bytes) of message header
#define OLSR_HELLO_HDR_SIZE		1							// Size (in bytes) of hello header
#define OLSR_HELLO_MSG_HDR_SIZE	3							// Size (in bytes) of hello_msg header
#define OLSR_TC_HDR_SIZE		2							// Size (in bytes) of tc header

#define OLSR_MAX_SEQ_NUM	65535
#define OLSR_STATUS_NOT_SYM	0								// Used to set status of an OLSR_nb_tuple as "not symmetric".
#define OLSR_STATUS_SYM		1								// Used to set status of an OLSR_nb_tuple as "symmetric".

#define OLSR_C 0.0625										// Scaling factor used in RFC 3626.

/****************************************************************************************/
/****************************************************************************************/

namespace wiselib
{

   /** \brief Olsr routing implementation of \ref routing_concept "Routing Concept" in group routing_concept
    * 	Olsr routing implementation of \ref routing_concept "Routing Concept" 
    * 
    *  \ingroup routing_concept
    *  \ingroup radio_concept
    *  \ingroup basic_algorithm_concept
    *  \ingroup routing_algorithm
    */
   template<typename OsModel_P,
            typename RoutingTable_P,
            typename Clock_P,
            typename Radio_P = typename OsModel_P::Radio,
            typename Debug_P = typename OsModel_P::Debug>
   class OlsrRouting
      : public RoutingBase<OsModel_P, Radio_P>
   {
   public:
      typedef OsModel_P OsModel;
	  typedef RoutingTable_P RoutingTable;
      typedef Radio_P Radio;
      typedef Debug_P Debug;
      typedef Clock_P Clock;

      typedef typename OsModel_P::Timer Timer;

      typedef typename RoutingTable::iterator RoutingTableIterator;
      typedef typename RoutingTable::value_type RoutingTableValue;
      typedef typename RoutingTable::mapped_type RoutingTableEntry;

      typedef OlsrRouting<OsModel, RoutingTable, Clock, Radio, Debug> self_type;

      typedef typename OsModel::Os Os;

      typedef typename Radio::node_id_t node_id_t;
      typedef typename Radio::size_t size_t;
      typedef typename Radio::block_data_t block_data_t;
      typedef typename Clock::time_t time_t;

      typedef typename Timer::millis_t millis_t;

      typedef OlsrRoutingMessage<OsModel, Radio> RoutingMessage;
      typedef OlsrBroadcastHelloMessage<OsModel, Radio, RoutingTableValue> 	BroadcastHelloMessage;
      typedef OlsrBroadcastTcMessage<OsModel, Radio, RoutingTableValue> BroadcastTcMessage;

      OlsrRouting();
      ~OlsrRouting();

      //@name Routing Control
      void enable( void );
      void disable( void );

      //@name Routing Functionality
      void send( node_id_t receiver, size_t len, block_data_t *data );

      inline void set_os( Os* os )
      { os_ = os; };

      inline Os* os()
      { return os_; };

      /****************************************************************************************/
	  //                          Data Structure in OLSR                                       /
      /****************************************************************************************/

       typedef struct OLSR_rt_entry {					// A Routing table entry

				node_id_t			dest_addr_;			// Address of the destination node.
				node_id_t			next_addr_;			// Address of the next hop.
				uint32_t			dist_;				// Distance in hops to the destination.

				inline node_id_t&	dest_addr()			{ return dest_addr_; }
				inline node_id_t&	next_addr()			{ return next_addr_; }
				inline uint32_t&	dist()				{ return dist_; }

        } OLSR_rt_entry;


       typedef struct OLSR_link_tuple {					// A Link Tuple.

            	node_id_t			local_node_addr_;	// Address of the local node.
            	node_id_t			nb_node_addr_;		// Address of the neighbor node.
            	double				sym_time_;			// The link is considered bidirectional until this time.
            	double				asym_time_;			// The link is considered unidirectional until this time.
            	double				lost_time_;			// The link is considered lost until this time (used for link layer notification).
            	double				time_;				// Time at which this tuple expires and must be removed.

            	inline node_id_t&	local_node_addr()	{ return local_node_addr_; }
            	inline node_id_t&	nb_node_addr()		{ return nb_node_addr_; }
            	inline double&		sym_time()			{ return sym_time_; }
            	inline double&		asym_time()			{ return asym_time_; }
            	inline double&		lost_time()			{ return lost_time_; }
            	inline double&		time()				{ return time_; }

            } OLSR_link_tuple;


        typedef struct OLSR_nb_tuple {      			// A Neighbor Tuple.

            	node_id_t 			nb_node_addr_;		// Address of the neighbor node.
            	uint8_t 			status_;			// Neighbor Type and Link Type at the four less significant digits.
            	uint8_t 			willingness_;		// A value between 0 and 7 specifying the node's willingness to carry traffic on behalf of other nodes.

            	inline node_id_t&	nb_node_addr()		{ return nb_node_addr_; }
            	inline uint8_t&		status()			{ return status_; }
            	inline uint8_t&		willingness()		{ return willingness_; }

            } OLSR_nb_tuple;


        typedef struct OLSR_nb2hop_tuple {     	 		// A 2-hop Tuple.

            	node_id_t			nb_node_addr_;		// Address of the neighbor node.
            	node_id_t			nb2hop_addr_;		// Address of a 2-hop neighbor with a symmetric link to nb_node_addr.
            	double				time_;				// Time at which this tuple expires and must be removed.

            	inline node_id_t&	nb_node_addr()		{ return nb_node_addr_; }
            	inline node_id_t&	nb2hop_addr()		{ return nb2hop_addr_; }
            	inline double&		time()				{ return time_; }

         } OLSR_nb2hop_tuple;


        typedef struct OLSR_mprsel_tuple {      		// An MPR-Selector Tuple.

            	node_id_t			node_addr_;			// Address of a node which have selected this node as a MPR.
            	double				time_;				// Time at which this tuple expires and must be removed.

            	inline node_id_t&	node_addr()			{ return node_addr_; }
            	inline double&		time()				{ return time_; }

         } OLSR_mprsel_tuple;


        typedef struct OLSR_dup_tuple {					// A Duplicate Tuple.

            	node_id_t			addr_;				// Originator address of the message.
            	uint16_t			seq_num_;			// Message sequence number.
            	bool				retransmitted_;		// Indicates whether the message has been retransmitted or not.
            	double				time_;				// Time at which this tuple expires and must be removed.

            	inline node_id_t&	addr()				{ return addr_; }
            	inline uint16_t&	seq_num()			{ return seq_num_; }
            	inline bool&		retransmitted()		{ return retransmitted_; }
            	inline double&		time()				{ return time_; }

         } OLSR_dup_tuple;


         typedef struct OLSR_topology_tuple { 			// A Topology Tuple.

            	node_id_t			dest_addr_;			// Address of the destination.
            	node_id_t			last_addr_;			// Address of a node which is a neighbor of the destination.
            	uint16_t			seq_;				// Sequence number.
            	double				time_;				// Time at which this tuple expires and must be removed.

            	inline node_id_t&	dest_addr()			{ return dest_addr_; }
            	inline node_id_t&	last_addr()			{ return last_addr_; }
            	inline uint16_t&	seq()				{ return seq_; }
            	inline double&		time()				{ return time_; }

          } OLSR_topology_tuple;

          typedef std::vector<OLSR_link_tuple*>			linkset_t;
          typedef std::vector<OLSR_nb_tuple*>			nbset_t;
          typedef std::vector<OLSR_nb2hop_tuple*>		nb2hopset_t;
          typedef std::set<node_id_t>					mprset_t;
          typedef std::vector<OLSR_mprsel_tuple*>		mprselset_t;
          typedef std::vector<OLSR_topology_tuple*>		topologyset_t;
          typedef std::vector<OLSR_dup_tuple*>			dupset_t;

          linkset_t		linkset_;
          nbset_t		nbset_;
          nb2hopset_t	nb2hopset_;
          mprset_t		mprset_;
          mprselset_t	mprselset_;
          topologyset_t	topologyset_;
          dupset_t		dupset_;


            /**********************************************************************/
            //                        Messages in OLSR                             /
            /**********************************************************************/
          typedef struct OLSR_hello_msg { 						// OLSR_hello_msg (inside OLSR_hello).

      		uint8_t				link_code_;
      		uint16_t			link_msg_size_;
      		node_id_t			nb_addrs_[OLSR_MAX_ADDRS];
      		int					nb_addrs_count_;

      		inline uint8_t&		link_code()						{ return link_code_; }
      		inline uint16_t&	link_msg_size()					{ return link_msg_size_; }
      		inline node_id_t&	neighbor_addr_list(int i)		{ return nb_addrs_[i]; }
      		inline int 			neighbor_addr_count()			{ return nb_addrs_count_;}

      		inline uint32_t 	size() 							// size of each OLSR_hello_msg
      		{
      			return OLSR_HELLO_MSG_HDR_SIZE + nb_addrs_count_ * 4;
      		}

      		inline uint32_t size_pnt()
			{
      			uint32_t a = OLSR_HELLO_MSG_HDR_SIZE+nb_addrs_count_*4;

		      	return a;
		    }

          } OLSR_hello_msg;


          typedef struct OLSR_hello {										// OLSR_hello
        	uint8_t 				htime_;
    		uint8_t					willingness_;
    		OLSR_hello_msg			hello_body_[OLSR_MAX_HELLOS];
    		int						hello_msg_count_;

    		inline uint8_t&			htime()						{ return htime_;       }
    		inline uint8_t&			willingness()				{ return willingness_; }
    		inline OLSR_hello_msg&	hello_msg(int i)			{ return hello_body_[i]; }
    		inline int				hello_msg_count()			{ return hello_msg_count_;}

    		inline uint32_t size()											// size of OLSR_hello ( including all OLSR_hello_msg )
    		{
    			uint32_t sz = OLSR_HELLO_HDR_SIZE;

    			for (int i = 0; i < hello_msg_count_; i++)
    				sz += hello_msg(i).size();

    			return sz;
    		}

          } OLSR_hello;



      	   typedef struct OLSR_tc {									// OLSR_tc

      	    uint16_t			ansn_;
      	    node_id_t			adv_nb_addrs_[OLSR_MAX_ADDRS];
      	    int					adv_nb_addrs_count_;

      	    inline	uint16_t&	ansn()							{ return ansn_; }
      	    inline	node_id_t&	adv_neighbor_addr_list(int i)	{ return adv_nb_addrs_[i]; }
      	    inline int			adv_neighbor_addrs_count()		{ return adv_nb_addrs_count_;}

      	    inline	uint32_t 	size()								// size of OLSR_tc
				{
					return OLSR_TC_HDR_SIZE + adv_nb_addrs_count_*4;
				}

      	    } OLSR_tc;


            typedef struct OLSR_msg {								// OLSR_msg

            	uint8_t				msg_id_;						// Message type.
            	uint8_t				vtime_;							// Validity time.
            	uint16_t			msg_size_;						// Message size (in bytes).
            	node_id_t			orig_addr_;						// Address of the node which generated this message.
            	uint8_t				ttl_;							// Time to live (in hops).
            	uint8_t				hop_count_;						// Number of hops which the message has taken.
            	uint16_t			msg_seq_num_;					// Message sequence number.

            	union {	OLSR_hello	hello_;
						OLSR_tc		tc_;
      				} msg_body_;									// Message body.

      		   inline	uint8_t&	msg_id()						{ return msg_id_; }
      		   inline	uint8_t&	vtime()							{ return vtime_; }
      		   inline	uint16_t&	msg_size()						{ return msg_size_; }
      		   inline	node_id_t&	originator_addr()				{ return orig_addr_; }
      		   inline	uint8_t&	ttl()							{ return ttl_; }
      		   inline	uint8_t&	hop_count()						{ return hop_count_; }
      		   inline	uint16_t&	msg_seq_num()					{ return msg_seq_num_; }

               inline	OLSR_hello&	hello()							{ return msg_body_.hello_; }
               inline	OLSR_tc&	tc()							{ return msg_body_.tc_; }

               inline  uint32_t size()								// size of OLSR_msg
      			{
      				uint32_t sz = OLSR_MSG_HDR_SIZE;
      				if (msg_id() == HELLO)
      					sz += hello().size();
      				else if (msg_id() == TC)
      					sz += tc().size();
      				return sz;
      			}

            } OLSR_msg;


            /**********************************************************************/
            //                OLSR function & Node states' manipulation            /
            /**********************************************************************/
	        inline	linkset_t&			linkset()		{ return linkset_; }
	        inline	nbset_t&			nbset()			{ return nbset_; }
	        inline	nb2hopset_t&		nb2hopset()		{ return nb2hopset_; }
	        inline	mprset_t&			mprset()		{ return mprset_; }
	        inline	mprselset_t&		mprselset()		{ return mprselset_; }
	        inline	topologyset_t&		topologyset()	{ return topologyset_; }
	        inline	dupset_t&			dupset()		{ return dupset_; }

	        void						nb_loss(OLSR_link_tuple* tuple);						// Case of Neighbor loss

            void 						routing_table_computation();      						// Compute the Routing Table
			int 						degree(OLSR_nb_tuple*);	    		     				// This function is used for calculating the MPR Set
			bool 						route_exists(node_id_t destination);					// Check whether there is an entry in the local routing table for the destination in the received message

            void 						mpr_computation(); 										// Neighbor Detection & Calculation of the MPR nodes
	        bool						find_mpr_addr(node_id_t);								// based on the 1-hop & 2-hop neighbor information
	        void						insert_mpr_addr(node_id_t);
	        void						clear_mprset();

            void 						process_hello(BroadcastHelloMessage&, node_id_t);		// HELLO processing
            void 						process_tc(BroadcastTcMessage&, node_id_t);				// TC processing
            void 						process_data(RoutingMessage&, node_id_t);      			// DATA processing
            void						forward_hello(node_id_t, BroadcastHelloMessage&, OLSR_dup_tuple*); // HELLO forwarding
            void						forward_tc(node_id_t, BroadcastTcMessage&, OLSR_dup_tuple*);	   // TC forwarding

			void 						broadcast_hello();										// Hello generation & broadcast
			void 						broadcast_tc();											// TC generation & broadcast

            void 						link_sensing(BroadcastHelloMessage&, node_id_t);
            void 						populate_nbset(BroadcastHelloMessage&);					// Updates the Neighbor Set according to the information contained in a new received HELLO message
            void 						populate_nb2hopset(BroadcastHelloMessage&);				// Neighbor Detection & Updating of the 2-hop_Neighbor_Tuple based on the update of the Link_Tuple
            void 						populate_mprselset(BroadcastHelloMessage&);				// Neighbor Detection & Calculation of the MPR Selector set for the MPR nodes based on the 1-hop & 2-hop neighbor information

            void 						add_dup_tuple(OLSR_dup_tuple*);
            void 						rm_dup_tuple(OLSR_dup_tuple*);
        	OLSR_dup_tuple* 			find_dup_tuple(node_id_t, uint16_t);
        	void						erase_dup_tuple(OLSR_dup_tuple*);
        	void						insert_dup_tuple(OLSR_dup_tuple*);

            void 						add_link_tuple(OLSR_link_tuple*, uint8_t);				// Link Set manipulation
            void 						rm_link_tuple(OLSR_link_tuple*);
            void 						updated_link_tuple(OLSR_link_tuple*); 					// Update a Link_Tuple, also update the corresponding neighbor tuple if it is needed
	        OLSR_link_tuple*			find_link_tuple(node_id_t);
	        OLSR_link_tuple*			find_sym_link_tuple(node_id_t, double);
	        void						erase_link_tuple(OLSR_link_tuple*);
	        void						insert_link_tuple(OLSR_link_tuple*);

            void 						add_nb_tuple(OLSR_nb_tuple*);
            void 						rm_nb_tuple(OLSR_nb_tuple*);
	        OLSR_nb_tuple*				find_nb_tuple(node_id_t);
	        OLSR_nb_tuple*				find_nb_tuple(node_id_t, uint8_t);
	        OLSR_nb_tuple*				find_sym_nb_tuple(node_id_t);
	        void						erase_nb_tuple(OLSR_nb_tuple*);
	        void						insert_nb_tuple(OLSR_nb_tuple*);

            void 						add_nb2hop_tuple(OLSR_nb2hop_tuple*);
            void 						rm_nb2hop_tuple(OLSR_nb2hop_tuple*);
	        OLSR_nb2hop_tuple*			find_nb2hop_tuple(node_id_t, node_id_t);
	        void						erase_nb2hop_tuple(OLSR_nb2hop_tuple*);
	        void						erase_nb2hop_tuples(node_id_t);
	        void						erase_nb2hop_tuples(node_id_t, node_id_t);
	        void						insert_nb2hop_tuple(OLSR_nb2hop_tuple*);

            void 						add_mprsel_tuple(OLSR_mprsel_tuple*);
            void 						rm_mprsel_tuple(OLSR_mprsel_tuple*);
	        OLSR_mprsel_tuple*			find_mprsel_tuple(node_id_t);
	        void						erase_mprsel_tuple(OLSR_mprsel_tuple*);					// Called by rm_mprsel_tuple()
	        void						erase_mprsel_tuples(node_id_t);
	        void						insert_mprsel_tuple(OLSR_mprsel_tuple*);				// Called by add_mprsel_tuple()

            void 						add_topology_tuple(OLSR_topology_tuple*);
            void 						rm_topology_tuple(OLSR_topology_tuple*);
	        OLSR_topology_tuple*		find_topology_tuple(node_id_t, node_id_t);
	        OLSR_topology_tuple*		find_newer_topology_tuple(node_id_t, uint16_t);
	        void						erase_topology_tuple(OLSR_topology_tuple*);				// Called by rm_topology_tuple()
	        void						erase_older_topology_tuples(node_id_t, uint16_t);
	        void						insert_topology_tuple(OLSR_topology_tuple*);			// Called by add_topology_tuple()

	        static double				emf_to_seconds(uint8_t);
	        static uint8_t				seconds_to_emf(double);

	    	inline uint16_t				msg_seq() 												// Increments message sequence number and returns the new value.
										{
											msg_seq_ = (msg_seq_ + 1)%(OLSR_MAX_SEQ_NUM + 1);
											return msg_seq_;
										}

	    	inline int 					willingness()
										{
											return OLSR_WILL_DEFAULT;
										}


   private:
	    	//int random_limit=1;
	    	//double random(random_limit);
	        void timer_elapsed( void *userdata );      												// Methods called by Timer

	        void timer_expire_link_tuple( OLSR_link_tuple* );										// Link_tuple     expiration
	        void timer_expire_nb2hop_tuple( OLSR_nb2hop_tuple* );									// Nb2hop_tuple   expiration
	        void timer_expire_topology_tuple( OLSR_topology_tuple* );								// Topology_tuple expiration
	        void timer_expire_mprsel_tuple( OLSR_mprsel_tuple* );									// Mprsel_tuple   expiration
	        void timer_expire_dup_tuple( OLSR_dup_tuple* );											// Dup_tuple      expiration

	        void receive( node_id_t from, size_t len, block_data_t *data );      					//@name Methods called by RadioModel
	        void print_routing_table( RoutingTable rt );

	        millis_t startup_time_;
	        millis_t work_period_;
	        millis_t delay_;																		// delay for the retransmission of a message

	        //millis_t link_tuple_expire_;															// expiration of link_tuple
	        //millis_t nb_tuple_expire_;															// expiration of nb_tuple
	        //millis_t nb2hop_tuple_expire_;														// expiration of nb2hop_tuple
	        //millis_t topology_tuple_expire_;														// expiration of link_tuple
	        //millis_t mprsel_tuple_expire_;														// expiration of mprsel_tuple
	        //millis_t dup_tuple_expire_;															// expiration of dup_tuple

	   		int nb_addrs_count_before[OLSR_MAX_HELLOS];

	        short seconds_hello;
	        short seconds_tc;

	        Os *os_;

	        RoutingTable routing_table_;       														// Routing table


	        uint16_t msg_seq_;
	        uint16_t ansn_;
   };

   // -----------------------------------------------------------------------
   // -----------------------------------------------------------------------
   // -----------------------------------------------------------------------
   template<typename OsModel_P,
            typename RoutingTable_P,
            typename Clock_P,
            typename Radio_P,
            typename Debug_P>
   OlsrRouting<OsModel_P, RoutingTable_P, Clock_P, Radio_P, Debug_P>::
   OlsrRouting()
      :  startup_time_		   ( 2000 ),
         work_period_		   ( 5000 ),
         delay_				   ( rand()%50 ),
         os_( 0 )
   {



       msg_seq_		= OLSR_MAX_SEQ_NUM;						// Message sequence number counter
       ansn_		= OLSR_MAX_SEQ_NUM;						// Advertised Neighbor Set sequence number.
   };
   // -----------------------------------------------------------------------
   template<typename OsModel_P,
            typename RoutingTable_P,
            typename Clock_P,
            typename Radio_P,
            typename Debug_P>
   OlsrRouting<OsModel_P, RoutingTable_P, Clock_P, Radio_P, Debug_P>::
   ~OlsrRouting()
   {
#ifdef DEBUG_OLSRROUTING
      Debug::debug( os(), "OlsrRouting: Destroyed\n" );
#endif
   };
   // -----------------------------------------------------------------------
   template<typename OsModel_P,
            typename RoutingTable_P,
            typename Clock_P,
            typename Radio_P,
            typename Debug_P>
   void
   OlsrRouting<OsModel_P, RoutingTable_P, Clock_P, Radio_P, Debug_P>::
   enable( void )
   {
#ifdef DEBUG_OLSRROUTING
      Debug::debug( os(), "OlsrRouting: Boot for %i\n", Radio::id( os() ) );
#endif

      seconds_tc = 0;
      seconds_hello = 0;

      Radio::enable( os() );
      Radio::template reg_recv_callback<self_type, &self_type::receive>( os(), this );
      Timer::template set_timer<self_type, &self_type::timer_elapsed>( os(), startup_time_, this, 0 );
   }
   // -----------------------------------------------------------------------
   template<typename OsModel_P,
            typename RoutingTable_P,
            typename Clock_P,
            typename Radio_P,
            typename Debug_P>
   void
   OlsrRouting<OsModel_P, RoutingTable_P, Clock_P, Radio_P, Debug_P>::
   disable( void )
   {
#ifdef DEBUG_OLSRROUTING
      Debug::debug( os(), "OlsrRouting: Disable\n" );
#endif
   }
   // -----------------------------------------------------------------------
   template<typename OsModel_P,
            typename RoutingTable_P,
            typename Clock_P,
            typename Radio_P,
            typename Debug_P>
   void
   OlsrRouting<OsModel_P, RoutingTable_P, Clock_P,Radio_P, Debug_P>::
   send( node_id_t destination, size_t len, block_data_t *data )			// Send the DATA message according to the routing table entry
   {
	  Debug::debug( os(), "OlsrRouting: START TO SEND DATA.\n" );
      RoutingTableIterator it = routing_table_.find( destination );
      if ( it != routing_table_.end() && it->second.next_addr != Radio::NULL_NODE_ID )
      {
    	 RoutingMessage message;
         message.set_msg_id( DATA );
         message.set_source( Radio::id(os()) );
         message.set_destination( destination );
         message.set_payload( len, data );
         Radio::send( os(), it->second.next_addr, message.buffer_size(), (uint8_t*)&message );

#ifdef DEBUG_OLSRROUTING
         Debug::debug( os(), "OlsrRouting: Send DATA to %i over %i.\n", message.destination(), it->second.next_addr );
#endif
      }
#ifdef DEBUG_OLSRROUTING
      else
         Debug::debug( os(), "OlsrRouting: Send failed. Route to Destination not known.\n" );
#endif
   }

   // ----------------------------------------------------------------------
	template<typename OsModel_P,
			 typename RoutingTable_P,
			 typename Clock_P,
			 typename Radio_P,
			 typename Debug_P>
	void
	OlsrRouting<OsModel_P, RoutingTable_P, Clock_P, Radio_P, Debug_P>::
	broadcast_hello()
	{
		OLSR_msg msg;
		BroadcastHelloMessage hellomessage;

		time_t now = Clock::time( os() );

		hellomessage.set_msg_id( HELLO );
		hellomessage.set_vtime( seconds_to_emf(OLSR_NEIGHB_HOLD_TIME) );
		hellomessage.set_originator_addr( Radio::id(os()) );
		hellomessage.set_ttl(1);
		hellomessage.set_hop_count(0);
		hellomessage.set_msg_seq_num( msg_seq() );
		hellomessage.set_htime( seconds_to_emf(OLSR_HELLO_INTERVAL) );
		hellomessage.set_willingness( willingness() );

		msg.hello().hello_msg_count_ = 0;
		msg.msg_id() = HELLO;

		std::map< uint8_t, int> linkcodes_count;										// <link_code, link_code_cnt>

	    if (!linkset().size())
	    {
	    	Debug::debug( os(), "Empty HELLO!!!!!! \n");
	    }

		for (typename linkset_t::iterator it = linkset().begin(); it != linkset().end(); it++)
		{
		   	OLSR_link_tuple* link_tuple = *it;

		   	if (link_tuple->local_node_addr() == Radio::id(os()) && link_tuple->time() >= now )// for each tuple in the LINK_SET, produce a LINK_CODE and
																							   // generate a hello_msg corresponding to this LINK_CODE
		   	{
		   		uint8_t link_type, nb_type, link_code;

		   		// Establishes LINK_TYPE for this entry
		   		if (link_tuple->lost_time() >= now)
		   			link_type = OLSR_LOST_LINK;													// LINK_TYPE = LOST_LINK
		   		else if (link_tuple->sym_time() >= now)
		   			link_type = OLSR_SYM_LINK;													// LINK_TYPE = SYM_LINK
		   		else if (link_tuple->asym_time() >= now)
		   			link_type = OLSR_ASYM_LINK;													// LINK_TYPE = ASYM_LINK
		   		else
		   			link_type = OLSR_LOST_LINK;													// LINK_TYPE = LOST_LINK

		   		// Establishes NEIGHBOR_TYPE for this entry
		   		if (find_mpr_addr(link_tuple->nb_node_addr()))
		   			nb_type = OLSR_MPR_NEIGH;													// NB_TYPE = MPR_NEIGH
		   		else {
						bool ok = false;
						for (typename nbset_t::iterator nb_it = nbset().begin(); nb_it != nbset().end(); nb_it++)
						{
							OLSR_nb_tuple* nb_tuple = *nb_it;
							if (nb_tuple->nb_node_addr() == link_tuple->nb_node_addr())
							{
								if (nb_tuple->status() == OLSR_STATUS_SYM)
									nb_type = OLSR_SYM_NEIGH;									// NB_TYPE = SYM_NEIGH
								else if (nb_tuple->status() == OLSR_STATUS_NOT_SYM)
									nb_type = OLSR_NOT_NEIGH;									// NB_TYPE = NOT_NEIGH
								else
									{
										fprintf(stderr, "There is a neighbor tuple with an unknown status!\n");
										exit(1);
									}
								ok = true;
								break;
							}
						}

						if (!ok)
						{
							fprintf(stderr, "Link tuple has no corresponding Neighbor tuple\n");
							exit(1);
						}
					}

		   		link_code = (link_type & 0x03) | ((nb_type << 2) & 0x0f);						// LINK_CODE = LINK_TYPE + NB_TYPE, for this link_tuple entry


		   		// For each LINK_CODE, generate a hello_msg
		   		int count = msg.hello().hello_msg_count_;										// count = #_hello_msg in OLSR_hello = hello_msg_count_, initially 0
		   		for ( int p = 0; p < count; p++ )
		   		{
		   			nb_addrs_count_before[count] += msg.hello().hello_msg(p).neighbor_addr_count();
		   		}

		   		typename std::map<uint8_t, int>::iterator pos = linkcodes_count.find(link_code);

				if (pos == linkcodes_count.end())												// No such LINK_CODE
				{
					linkcodes_count[link_code] = count;											// linkcodes_count[link_code, link_code_cnt]
					assert(count >= 0 && count < OLSR_MAX_HELLOS);

					msg.hello().hello_msg(count).nb_addrs_count_ = 0;
					hellomessage.set_link_code(count, nb_addrs_count_before[count], link_code);			 			    // Set the link_code of hello_msg[count]
					msg.hello().hello_msg_count_++;
				}
				else						// there is already existing item in the linkcodes_count with certain link_code
					count = (*pos).second;  // count is set with number of OLSR_hello_msg with the given link_code

				int i = msg.hello().hello_msg(count).nb_addrs_count_;
				msg.hello().hello_msg(count).nb_addrs_count_++;

				assert(count >= 0 && count < OLSR_MAX_HELLOS);
				assert(i >= 0 && i < OLSR_MAX_ADDRS);

				uint32_t pnt = NULL;
				pnt= msg.hello().hello_msg(count).size_pnt();

				node_id_t* pnt2 = NULL;
				pnt2 = &(link_tuple->nb_node_addr());

				hellomessage.set_neighbor_addr_list(count, nb_addrs_count_before[count], i, pnt2);		 					// Set the neighbor_addr_list[i] of hello_msg[count]
				hellomessage.set_link_msg_size(count, nb_addrs_count_before[count], &pnt ); // Set the link_msg_size of hello_msg[count]
				hellomessage.set_neighbor_addr_count(count, nb_addrs_count_before[count], msg.hello().hello_msg(count).nb_addrs_count_);			// Set the neighbor_addr_count of hello[count]

		   	}
		 }

		 hellomessage.set_msg_size( msg.size() );
		 hellomessage.set_hello_msgs_count( msg.hello().hello_msg_count_ );

		 Radio::send( os(), Radio::BROADCAST_ADDRESS, hellomessage.buffer_size(), (uint8_t*)&hellomessage);
		 Debug::debug( os(), "%i send HELLO with %i hello_msg! \n", Radio::id(os()), msg.hello().hello_msg_count_ );
	}
   // ----------------------------------------------------------------------
	template<typename OsModel_P,
			 typename RoutingTable_P,
			 typename Clock_P,
			 typename Radio_P,
			 typename Debug_P>
	void
	OlsrRouting<OsModel_P, RoutingTable_P, Clock_P, Radio_P, Debug_P>::
	broadcast_tc()
	{
	    OLSR_msg msg;
	    BroadcastTcMessage tcmessage;

	    msg.msg_id() = TC;
	    msg.vtime() = seconds_to_emf(OLSR_TOP_HOLD_TIME);
	    msg.originator_addr() = Radio::id(os());
	    msg.ttl() = 255;
	    msg.hop_count() = 0;
	    msg.msg_seq_num() = msg_seq();
	    msg.tc().ansn() = ansn_;
	    msg.tc().adv_nb_addrs_count_ = 0;

	    tcmessage.set_msg_id( TC );
	    tcmessage.set_vtime( seconds_to_emf(OLSR_TOP_HOLD_TIME) );
	    tcmessage.set_originator_addr( Radio::id(os()) );
	    tcmessage.set_ttl( 255 );
	    tcmessage.set_hop_count( 0 );
	    tcmessage.set_msg_seq_num( msg_seq() );
	    tcmessage.set_ansn( ansn_ );

	    if (!mprselset().size())
	    {
	    	Debug::debug( os(), "Empty TC!!!!!! \n");
	    }

	    for (typename mprselset_t::iterator it = mprselset().begin(); it != mprselset().end(); it++)		// TC message contains the MPR Selector nodes of this MPR node
	    {
	    	OLSR_mprsel_tuple* mprsel_tuple = *it;
	      	int count = msg.tc().adv_nb_addrs_count_;

	      	assert(count >= 0 && count < OLSR_MAX_ADDRS);
	      	msg.tc().adv_neighbor_addr_list(count) = mprsel_tuple->node_addr(); 				// each MPR selector nodes' address will be included in the TC message
	      	msg.tc().adv_nb_addrs_count_++;

			Debug::debug( os(), "Put node %i into TC message \n", &(mprsel_tuple->node_addr()) );
	      	tcmessage.set_adv_neighbor_addr_list( count,  &(mprsel_tuple->node_addr()) );
	    }

	    tcmessage.set_msg_size( msg.size() );

		Radio::send( os(), Radio::BROADCAST_ADDRESS, msg.size(), (uint8_t*)&tcmessage );

		Debug::debug( os(), "%i send TC with %i NB address! \n", Radio::id(os()), msg.tc().adv_nb_addrs_count_ );
	}
   // -----------------------------------------------------------------------
   template<typename OsModel_P,
            typename RoutingTable_P,
            typename Clock_P,
            typename Radio_P,
            typename Debug_P>
   void
   OlsrRouting<OsModel_P, RoutingTable_P, Clock_P, Radio_P, Debug_P>::
   timer_elapsed( void* userdata )
   {
	   seconds_hello++;
	   seconds_tc++;

	  if ( seconds_hello == OLSR_HELLO_INTERVAL)
	  {
		  broadcast_hello();
		  seconds_hello = 0;
	  }

	  if ( seconds_tc == OLSR_TC_INTERVAL)
	  {
		  broadcast_tc();
		  seconds_tc = 0;
	  }

      print_routing_table( routing_table_ );

      Timer::template set_timer<self_type, &self_type::timer_elapsed>( os(), work_period_, this, 0 );
   }

   // -----------------------------------------------------------------------
   template<typename OsModel_P,
            typename RoutingTable_P,
            typename Clock_P,
            typename Radio_P,
            typename Debug_P>
   void
   OlsrRouting<OsModel_P, RoutingTable_P, Clock_P, Radio_P, Debug_P>::
   timer_expire_link_tuple( OLSR_link_tuple* tuple )// Removes link_tuple if expired.
													// Else if symmetric time has expired then it is assumed a neighbor loss, the timer is rescheduled to expire at tuple_->time().
													// Otherwise the timer is rescheduled to expire at the minimum between tuple_->time() and tuple_->sym_time().
   {
	   time_t now = Clock::time( os() );

	   if (tuple->time() < now)
		{
			rm_link_tuple(tuple);
			delete tuple;
			delete this;
		}
		else if (tuple->sym_time() < now)
		{
			nb_loss(tuple);
			Timer::template set_timer<self_type, &self_type::timer_elapsed>( os(), ( (tuple->time()) - now ), this, 0 );
		}
		else
		    Timer::template set_timer<self_type, &self_type::timer_elapsed>( os(), ( MIN( tuple->time(), tuple->sym_time()) - now ), this, 0 );

   }

   // -----------------------------------------------------------------------
   /*template<typename OsModel_P,
            typename RoutingTable_P,
            typename Clock_P,
            typename Radio_P,
            typename Debug_P>
   void
   OlsrRouting<OsModel_P, RoutingTable_P, Clock_P, Radio_P, Debug_P>::
   timer_expire_nb_tuple( OLSR_nb_tuple* tuple )
   {
	   time_t now = Clock::time( os() );

	   if (tuple.time() < now)
		{
			rm_nb_tuple(tuple);
			delete tuple;
			delete this;
		}

	   Timer::template set_timer<self_type, &self_type::timer_elapsed>( os(), nb_tuple_expire_, this, 0 );
   }
	*/
   // -----------------------------------------------------------------------
   template<typename OsModel_P,
            typename RoutingTable_P,
            typename Clock_P,
            typename Radio_P,
            typename Debug_P>
   void
   OlsrRouting<OsModel_P, RoutingTable_P, Clock_P, Radio_P, Debug_P>::
   timer_expire_nb2hop_tuple( OLSR_nb2hop_tuple* tuple )
   {
	   time_t now = Clock::time( os() );

	   if (tuple->time() < now)
		{
			rm_nb2hop_tuple(tuple);
			delete tuple;
			delete this;
		}
	   else
	   {
		   Timer::template set_timer<self_type, &self_type::timer_elapsed>( os(), ( (tuple->time()) - now ), this, 0 );
	   }


   }

   // -----------------------------------------------------------------------
   template<typename OsModel_P,
            typename RoutingTable_P,
            typename Clock_P,
            typename Radio_P,
            typename Debug_P>
   void
   OlsrRouting<OsModel_P, RoutingTable_P, Clock_P, Radio_P, Debug_P>::
   timer_expire_topology_tuple( OLSR_topology_tuple* tuple )
   {
	   time_t now = Clock::time( os() );

	   if (tuple->time() < now)
		{
			rm_topology_tuple(tuple);
			delete tuple;
			delete this;
		}
	   else
	   	{
	   		Timer::template set_timer<self_type, &self_type::timer_elapsed>( os(), ( (tuple->time()) - now ), this, 0 );
	   	}
   }

   // -----------------------------------------------------------------------
   template<typename OsModel_P,
            typename RoutingTable_P,
            typename Clock_P,
            typename Radio_P,
            typename Debug_P>
   void
   OlsrRouting<OsModel_P, RoutingTable_P, Clock_P, Radio_P, Debug_P>::
   timer_expire_mprsel_tuple( OLSR_mprsel_tuple* tuple )
   {
	   //time_t now = Clock::time( os() );
	   double  now = Clock::time( os() );

	   if (tuple->time() < now)
		{
			rm_mprsel_tuple(tuple);
			delete tuple;
			delete this;
		}
	   else
	   	{
	   		Timer::template set_timer<self_type, &self_type::timer_elapsed>( os(), ( (tuple->time()) - now ), this, 0 );
	   	}
   }

   // -----------------------------------------------------------------------
   template<typename OsModel_P,
            typename RoutingTable_P,
            typename Clock_P,
            typename Radio_P,
            typename Debug_P>
   void
   OlsrRouting<OsModel_P, RoutingTable_P, Clock_P, Radio_P, Debug_P>::
   timer_expire_dup_tuple( OLSR_dup_tuple* tuple )
   {
	   time_t now = Clock::time( os() );

	   if (tuple->time() < now)
		{
			rm_dup_tuple(tuple);
			delete tuple;
			delete this;
		}
	   else
	   	{
	   		Timer::template set_timer<self_type, &self_type::timer_elapsed>( os(), ( (tuple->time()) - now ), this, 0 );
	   	}
   }
   // -----------------------------------------------------------------------
   template<typename OsModel_P,
            typename RoutingTable_P,
            typename Clock_P,
            typename Radio_P,
            typename Debug_P>
   void
   OlsrRouting<OsModel_P, RoutingTable_P, Clock_P, Radio_P, Debug_P>::
   receive( node_id_t from, size_t len, block_data_t *data )
   {
      if ( from == Radio::id(os()) )											// Coming back to myself, Radio::id(os()) is the id of this receiving node
         return;

      uint8_t msg_id = *data;

      switch (msg_id)
      {
             case HELLO:														// HELLO
             {
             	BroadcastHelloMessage *message = (BroadcastHelloMessage*)data;
             	OLSR_dup_tuple *duplicated = find_dup_tuple( message->originator_addr(), message->msg_seq_num() );

             	if (duplicated == NULL)
             	{
             		process_hello(*message, from);
             	}
             	else
             	{
             		forward_hello(from, *message, duplicated);
             	}

             	break;
             }
             case TC:															// TC
             {
             	BroadcastTcMessage *message = (BroadcastTcMessage*) data;
             	OLSR_dup_tuple *duplicated = find_dup_tuple( message->originator_addr(), message->msg_seq_num() );

             	if (duplicated == NULL)
             	{
             		process_tc(*message, from);
             	}
             	else
             	{
             		forward_tc(from, *message, duplicated);
             	}

                break;
             }
             case DATA:															// DATA
             {
                RoutingMessage *message = (RoutingMessage*) data;
                process_data(*message, from);

                break;
             }
             default:
             {
                 Debug::debug(os(), "%i received UNRECOGNIZED message type [%i]\n", Radio::id(os()), msg_id);
             }
         }

	   	routing_table_computation();											// After processing all OLSR messages, recompute routing table

   }

   // -----------------------------------------------------------------------
   // brief Processes a HELLO message following RFC 3626 specification.
   // Link sensing & population of the Neighbor Set & population of 2-hop Neighbor Set & population of MPR Selector Set

   // param msg 		the OLSR hello message
   // param sender 		the address of the node where the message was sent

   template<typename OsModel_P,
            typename RoutingTable_P,
            typename Clock_P,
            typename Radio_P,
            typename Debug_P>
   void
   OlsrRouting<OsModel_P, RoutingTable_P, Clock_P, Radio_P, Debug_P>::
   process_hello(BroadcastHelloMessage& message, node_id_t sender)
   {
    Debug::debug(os(), "%i received HELLO FROM %i of SIZE %i with %i HELLO_MSG \n", Radio::id(os()), message.originator_addr(), message.msg_size(), message.hello_msgs_count() );

    link_sensing(message, sender);
   	populate_nbset(message);
   	populate_nb2hopset(message);
   	mpr_computation();
   	populate_mprselset(message);
   }

   // -----------------------------------------------------------------------
   // brief	Updates Link Set according to a new received HELLO message (following RFC 3626 specification). Neighbor Set is also updated if needed

   // param msg 		the OLSR message which contains the HELLO message
   // param sender 		the address of the node where the message was sent

   template<typename OsModel_P,
            typename RoutingTable_P,
            typename Clock_P,
            typename Radio_P,
            typename Debug_P>
   void
   OlsrRouting<OsModel_P, RoutingTable_P, Clock_P, Radio_P, Debug_P>::
   link_sensing(BroadcastHelloMessage& message, node_id_t sender)
   {

#ifdef DEBUG_OLSRROUTING
   	Debug::debug(os(), "Link Sensing receiving Hello with %i hello_msgn inside!!!\n", message.hello_msgs_count() );
#endif

	time_t now 			= Clock::time( os() );

	// Construct OLSR_hello from BroadcastHelloMessage& message
	OLSR_hello hello;
	hello.htime_ 			= message.htime();
	hello.willingness_		= message.willingness();
	hello.hello_msg_count_ 	= message.hello_msgs_count();
	for (int i = 0; i < hello.hello_msg_count_; i++ )
	{
		hello.hello_body_[i].link_code_ 		= message.link_code(i, nb_addrs_count_before[i]);
		hello.hello_body_[i].link_msg_size_ 	= message.link_msg_size(i, nb_addrs_count_before[i]);
		hello.hello_body_[i].nb_addrs_count_	= message.neighbor_addr_count(i, nb_addrs_count_before[i]);
		for ( int j = 1; j < message.neighbor_addr_count(i, nb_addrs_count_before[i]); j++ )
		{
			hello.hello_body_[i].nb_addrs_[j] = message.neighbor_addr_list(i, j, nb_addrs_count_before[i]);
		}

	}

   	bool updated		= false;
   	bool created		= false;

   	OLSR_link_tuple* link_tuple = find_link_tuple(sender);
   	if (link_tuple == NULL) {							// Create a new tuple

   		link_tuple 						= new OLSR_link_tuple;
   		link_tuple->nb_node_addr()		= sender;
   		link_tuple->local_node_addr()	= Radio::id(os());
   		link_tuple->sym_time()			= now - 1;
   		link_tuple->lost_time()			= 0.0;
   		link_tuple->time()				= now + emf_to_seconds(message.vtime());
#ifdef DEBUG_OLSRROUTING
   		Debug::debug(os(), "Adding new link_tuple!!! \n");
#endif
   		add_link_tuple(link_tuple, hello.willingness());
   		created = true;
   	}
   	else												// Update an existing tuple
   	{
#ifdef DEBUG_OLSRROUTING
	Debug::debug(os(), "Update an existing tuple!!! \n");
#endif
   		updated = true;
   	}

    link_tuple->asym_time() = now + emf_to_seconds(message.vtime());

   	assert(hello.hello_msg_count_ >= 0 && hello.hello_msg_count_ <= OLSR_MAX_HELLOS);
   	for (int i = 0; i < hello.hello_msg_count_; i++)				// for each the OLSR_hello_msg inside OLSR_hello
   	{
   		OLSR_hello_msg& hello_msg = hello.hello_msg(i);
   		int lt = hello_msg.link_code() & 0x03;
   		int nt = hello_msg.link_code() >> 2;
														// We must not process invalid advertised links
   		if ((lt == OLSR_SYM_LINK && nt == OLSR_NOT_NEIGH) || (nt != OLSR_SYM_NEIGH && nt != OLSR_MPR_NEIGH && nt != OLSR_NOT_NEIGH))
   			continue;

   		assert(hello_msg.nb_addrs_count_ >= 0 && hello_msg.nb_addrs_count_ <= OLSR_MAX_ADDRS);
   		for (int j = 0; j < hello_msg.nb_addrs_count_; j++)		// for each the Neighbor_Interface_Address inside a OLSR_hello_msg
   		{
   			if (hello_msg.neighbor_addr_list(j) == Radio::id(os()))
   			{
   				if (lt == OLSR_LOST_LINK)
   				{
   					link_tuple->sym_time() = now - 1;
   					updated = true;
   				}
   				else if (lt == OLSR_SYM_LINK || lt == OLSR_ASYM_LINK)
   				{
   					link_tuple->sym_time()	= now + emf_to_seconds(message.vtime());
   					link_tuple->time()		= link_tuple->sym_time() + OLSR_NEIGHB_HOLD_TIME;
   					link_tuple->lost_time()	= 0.0;
   					updated = true;
   				}
   				break;
   			}
   		}
   	}

   	link_tuple->time() = MAX(link_tuple->time(), link_tuple->asym_time());

   	if (updated)										// whenever the link tuple is updated, the corresponding neighbor tuple also needs update
   		updated_link_tuple(link_tuple);

   	// Schedules link tuple deletion
   	timer_expire_link_tuple(link_tuple);

   }

   // -----------------------------------------------------------------------
   // brief	Updates the Neighbor Set according to the information contained in a new received HELLO message (following RFC 3626).
   //		basically the "nb_tuple->status" will be updated

   // param  msg the %OLSR message which contains the HELLO message.

   template<typename OsModel_P,
            typename RoutingTable_P,
            typename Clock_P,
            typename Radio_P,
            typename Debug_P>
   void
   OlsrRouting<OsModel_P, RoutingTable_P, Clock_P, Radio_P, Debug_P>::
   populate_nbset(BroadcastHelloMessage& message)
   {
   	OLSR_nb_tuple* nb_tuple = find_nb_tuple(message.originator_addr());
   	if (nb_tuple != NULL)
   		nb_tuple->willingness() = message.willingness();
   }

   // -----------------------------------------------------------------------
   // brief	Updates the 2-hop Neighbor Set according to the information contained in a new received HELLO message (following RFC 3626).

   // param  msg the %OLSR message which contains the HELLO message.

   template<typename OsModel_P,
            typename RoutingTable_P,
            typename Clock_P,
            typename Radio_P,
            typename Debug_P>
   void
   OlsrRouting<OsModel_P, RoutingTable_P, Clock_P, Radio_P, Debug_P>::
   populate_nb2hopset(BroadcastHelloMessage& message)
   {
	time_t now = Clock::time( os() );

	// Construct OLSR_hello from BroadcastHelloMessage& message
	OLSR_hello hello;
	hello.htime_ 			= message.htime();
	hello.willingness_		= message.willingness();
	hello.hello_msg_count_ 	= message.hello_msgs_count();

	for (int i = 0; i < hello.hello_msg_count_; i++ )
	{
		hello.hello_body_[i].link_code_ 		= message.link_code(i, nb_addrs_count_before[i]);
		hello.hello_body_[i].link_msg_size_ 	= message.link_msg_size(i, nb_addrs_count_before[i]);
		hello.hello_body_[i].nb_addrs_count_	= message.neighbor_addr_count(i, nb_addrs_count_before[i]);
		for ( int j = 1; j < message.neighbor_addr_count(i, nb_addrs_count_before[i]); j++ )
		{
			hello.hello_body_[i].nb_addrs_[j] = message.neighbor_addr_list(i, j, nb_addrs_count_before[i]);
		}

	}



   	for (typename linkset_t::iterator it_lt = linkset().begin(); it_lt != linkset().end(); it_lt++)
   	{
   		OLSR_link_tuple* link_tuple = *it_lt;

   		if (link_tuple->nb_node_addr() == message.originator_addr())
   		{
   			if (link_tuple->sym_time() >= now)
   			{
   				assert(hello.hello_msg_count_ >= 0 && hello.hello_msg_count_ <= OLSR_MAX_HELLOS);
   				for (int i = 0; i < hello.hello_msg_count_; i++) 			// hello_msg_count_ is the number of the "hello_msg" included inside each "hello"
   				{
   					OLSR_hello_msg& hello_msg = hello.hello_msg(i);

   					int nt = hello_msg.link_code() >> 2; 		// nt = node_type

   					assert(hello_msg.nb_addrs_count_ >= 0 && hello_msg.nb_addrs_count_ <= OLSR_MAX_ADDRS);
   					for (int j = 0; j < hello_msg.nb_addrs_count_; j++) 	// hello_msg.count is the number of the "Neighbor_Interface_Address" inside each "hello_msg"
																// the "Neighbor_Interface_Address" is actually the 2-hop neighbor address
   					{
   						node_id_t nb2hop_addr = hello_msg.neighbor_addr_list(j);
   						if (nt == OLSR_SYM_NEIGH || nt == OLSR_MPR_NEIGH)
   						{
   							// if the main address of the 2-hop neighbor address = main address of the receiving node: silently discard the 2-hop neighbor address
   							if (nb2hop_addr != Radio::id(os()))
   							{
   								// Otherwise, a 2-hop tuple is created
   								OLSR_nb2hop_tuple* nb2hop_tuple = find_nb2hop_tuple(message.originator_addr(), nb2hop_addr);
   								if (nb2hop_tuple == NULL)
   								{
   									nb2hop_tuple = new OLSR_nb2hop_tuple;
   									nb2hop_tuple->nb_node_addr() = message.originator_addr();
   									nb2hop_tuple->nb2hop_addr() = nb2hop_addr;
   									add_nb2hop_tuple(nb2hop_tuple);
   									nb2hop_tuple->time() = now + emf_to_seconds(message.vtime());

   									// Schedules nb2hop tuple deletion
   									timer_expire_nb2hop_tuple(nb2hop_tuple);

   								}
   								else
   								{
   									nb2hop_tuple->time() = now + emf_to_seconds(message.vtime());
   								}

   							}
   						}
   						else if (nt == OLSR_NOT_NEIGH)
   						{
   							// For each 2-hop node listed in the HELLO message with Neighbor Type equal to NOT_NEIGH all 2-hop tuples where: N_neighbor_node_addr == Originator
   							// Address AND N_2hop_addr  == main address of the 2-hop neighbor are deleted.
   							erase_nb2hop_tuples(message.originator_addr(), nb2hop_addr);
   						}
   					}
   				}
   			}
   		}
   	}
   }

   // -----------------------------------------------------------------------
   // brief Compute MPR set of a node following RFC 3626 hints.

   template<typename OsModel_P,
            typename RoutingTable_P,
            typename Clock_P,
            typename Radio_P,
            typename Debug_P>
   void
   OlsrRouting<OsModel_P, RoutingTable_P, Clock_P, Radio_P, Debug_P>::
   mpr_computation()
   {
	time_t now = Clock::time( os() );

  	clear_mprset();
   	nbset_t N;
   	nb2hopset_t N2;

   	// N is the subset of neighbors of the node, which are neighbor "of the interface I"
   	for (typename nbset_t::iterator it = nbset().begin(); it != nbset().end(); it++)
   		if ((*it)->status() == OLSR_STATUS_SYM) // I think that we need this check
   			N.push_back(*it);

   	// N2 is the set of 2-hop neighbors reachable from "the interface I", excluding:

   	// (i)   the nodes only reachable by members of N with willingness WILL_NEVER
   	// (ii)  the node performing the computation
   	// (iii) all the symmetric neighbors: the nodes for which there exists a symmetric link to this node on some interface.

   	for (typename nb2hopset_t::iterator it = nb2hopset().begin(); it != nb2hopset().end(); it++) {
   		OLSR_nb2hop_tuple* nb2hop_tuple = *it;
   		bool ok = true;
   		OLSR_nb_tuple* nb_tuple = find_sym_nb_tuple(nb2hop_tuple->nb_node_addr());
   		if (nb_tuple == NULL)
   			ok = false;
   		else {
   			nb_tuple = find_nb_tuple(nb2hop_tuple->nb_node_addr(), OLSR_WILL_NEVER);
   			if (nb_tuple != NULL)
   				ok = false;
   			else {
   				nb_tuple = find_sym_nb_tuple(nb2hop_tuple->nb2hop_addr());
   				if (nb_tuple != NULL)
   					ok = false;
   			}
   		}

   		if (ok)
   			N2.push_back(nb2hop_tuple);
   	}

   	// 1. Start with an MPR set made of all members of N with N_willingness equal to WILL_ALWAYS
   	for (typename nbset_t::iterator it = N.begin(); it != N.end(); it++) {
   		OLSR_nb_tuple* nb_tuple = *it;
   		if (nb_tuple->willingness() == OLSR_WILL_ALWAYS)
   			insert_mpr_addr(nb_tuple->nb_node_addr());
   	}

   	// 2. Calculate D(y), where y is a member of N, for all nodes in N.
   	// We will do this later.

   	// 3. Add to the MPR set those nodes in N, which are the *only* nodes to provide reachability to a node in N2.
   	// Remove the nodes from N2 which are now covered by a node in the MPR set.
   	mprset_t foundset;
   	std::set<node_id_t> deleted_addrs;
   	for (typename nb2hopset_t::iterator it = N2.begin(); it != N2.end(); it++)
   	{
   		OLSR_nb2hop_tuple* nb2hop_tuple1 = *it;

   		typename mprset_t::iterator pos = foundset.find(nb2hop_tuple1->nb2hop_addr());
   		if (pos != foundset.end())
   			continue;

   		bool found = false;
   		for (typename nbset_t::iterator it2 = N.begin();it2 != N.end();it2++) {
   			if ((*it2)->nb_node_addr() == nb2hop_tuple1->nb_node_addr()) {
   				found = true;
   				break;
   			}
   		}
   		if (!found)
   			continue;

   		found = false;
   		for (typename nb2hopset_t::iterator it2 = it + 1; it2 != N2.end(); it2++) {
   			OLSR_nb2hop_tuple* nb2hop_tuple2 = *it2;
   			if (nb2hop_tuple1->nb2hop_addr() == nb2hop_tuple2->nb2hop_addr()) {
   				foundset.insert(nb2hop_tuple1->nb2hop_addr());
   				found = true;
   				break;
   			}
   		}
   		if (!found) {
   			insert_mpr_addr(nb2hop_tuple1->nb_node_addr());

   			for (typename nb2hopset_t::iterator it2 = it + 1; it2 != N2.end(); it2++) {
   				OLSR_nb2hop_tuple* nb2hop_tuple2 = *it2;
   				if (nb2hop_tuple1->nb_node_addr() == nb2hop_tuple2->nb_node_addr()) {
   					deleted_addrs.insert(nb2hop_tuple2->nb2hop_addr());
   					it2 = N2.erase(it2);
   					it2--;
   				}
   			}
   			it = N2.erase(it);
   			it--;
   		}

   		for (typename std::set<node_id_t>::iterator it2 = deleted_addrs.begin(); it2 != deleted_addrs.end(); it2++)
   		{
   			for (typename nb2hopset_t::iterator it3 = N2.begin();
   				it3 != N2.end();
   				it3++) {
   				if ((*it3)->nb2hop_addr() == *it2) {
   					it3 = N2.erase(it3);
   					it3--;
   					// I have to reset the external iterator because it
   					// may have been invalidated by the latter deletion
   					it = N2.begin();
   					it--;
   				}
   			}
   		}
   		deleted_addrs.clear();
   	}

   	// 4. While there exist nodes in N2 which are not covered by at
   	// least one node in the MPR set:
   	while (N2.begin() != N2.end()) {
   		// 4.1. For each node in N, calculate the reachability, i.e., the
   		// number of nodes in N2 which are not yet covered by at
   		// least one node in the MPR set, and which are reachable
   		// through this 1-hop neighbor
   		std::map<int, std::vector<OLSR_nb_tuple*> > reachability;
   		std::set<int> rs;
   		for (typename nbset_t::iterator it = N.begin(); it != N.end(); it++) {
   			OLSR_nb_tuple* nb_tuple = *it;
   			int r = 0;
   			for (typename nb2hopset_t::iterator it2 = N2.begin(); it2 != N2.end(); it2++) {
   				OLSR_nb2hop_tuple* nb2hop_tuple = *it2;
   				if (nb_tuple->nb_node_addr() == nb2hop_tuple->nb_node_addr())
   					r++;
   			}
   			rs.insert(r);
   			reachability[r].push_back(nb_tuple);
   		}

   		// 4.2. Select as a MPR the node with highest N_willingness among
   		// the nodes in N with non-zero reachability. In case of
   		// multiple choice select the node which provides
   		// reachability to the maximum number of nodes in N2. In
   		// case of multiple nodes providing the same amount of
   		// reachability, select the node as MPR whose D(y) is
   		// greater. Remove the nodes from N2 which are now covered
   		// by a node in the MPR set.
   		OLSR_nb_tuple* max = NULL;
   		int max_r = 0;
   		for (std::set<int>::iterator it = rs.begin(); it != rs.end(); it++) {
   			int r = *it;
   			if (r > 0) {
   				for (typename std::vector<OLSR_nb_tuple*>::iterator it2 = reachability[r].begin();
   					it2 != reachability[r].end();
   					it2++) {
   					OLSR_nb_tuple* nb_tuple = *it2;
   					if (max == NULL || nb_tuple->willingness() > max->willingness()) {
   						max = nb_tuple;
   						max_r = r;
   					}
   					else if (nb_tuple->willingness() == max->willingness()) {
   						if (r > max_r) {
   							max = nb_tuple;
   							max_r = r;
   						}
   						else if (r == max_r) {
   							if (degree(nb_tuple) > degree(max)) {
   								max = nb_tuple;
   								max_r = r;
   							}
   						}
   					}
   				}
   			}
   		}
   		if (max != NULL) {
   			insert_mpr_addr(max->nb_node_addr());
   			std::set<node_id_t> nb2hop_addrs;
   			for (typename nb2hopset_t::iterator it = N2.begin(); it != N2.end(); it++) {
   				OLSR_nb2hop_tuple* nb2hop_tuple = *it;
   				if (nb2hop_tuple->nb_node_addr() == max->nb_node_addr()) {
   					nb2hop_addrs.insert(nb2hop_tuple->nb2hop_addr());
   					it = N2.erase(it);
   					it--;
   				}
   			}
   			for (typename nb2hopset_t::iterator it = N2.begin(); it != N2.end(); it++) {
   				OLSR_nb2hop_tuple* nb2hop_tuple = *it;
   				typename std::set<node_id_t>::iterator it2 = nb2hop_addrs.find(nb2hop_tuple->nb2hop_addr());
   				if (it2 != nb2hop_addrs.end()) {
   					it = N2.erase(it);
   					it--;
   				}
   			}
   		}
   	}
   }

   // -----------------------------------------------------------------------
   template<typename OsModel_P,
            typename RoutingTable_P,
            typename Clock_P,
            typename Radio_P,
            typename Debug_P>
   bool
   OlsrRouting<OsModel_P, RoutingTable_P, Clock_P, Radio_P, Debug_P>::
   find_mpr_addr(node_id_t addr)
   {
	typename mprset_t::iterator it = mprset_.find(addr);
   	return (it != mprset_.end());
   }
   // -----------------------------------------------------------------------
   template<typename OsModel_P,
            typename RoutingTable_P,
            typename Clock_P,
            typename Radio_P,
            typename Debug_P>
   void
   OlsrRouting<OsModel_P, RoutingTable_P, Clock_P, Radio_P, Debug_P>::
   insert_mpr_addr(node_id_t addr)
   {
   	mprset_.insert(addr);
   }
   // -----------------------------------------------------------------------
   template<typename OsModel_P,
            typename RoutingTable_P,
            typename Clock_P,
            typename Radio_P,
            typename Debug_P>
   void
   OlsrRouting<OsModel_P, RoutingTable_P, Clock_P, Radio_P, Debug_P>::
   clear_mprset()
   {
   	mprset_.clear();
   }
   // -----------------------------------------------------------------------


   // brief	Updates the MPR Selector Set according to the information contained in a new received HELLO message (following RFC 3626).

   // param  msg the %OLSR message which contains the HELLO message.

   template<typename OsModel_P,
            typename RoutingTable_P,
            typename Clock_P,
            typename Radio_P,
            typename Debug_P>
   void
   OlsrRouting<OsModel_P, RoutingTable_P, Clock_P, Radio_P, Debug_P>::
   populate_mprselset(BroadcastHelloMessage& message)
   {
	time_t now 			= Clock::time( os() );

	Debug::debug(os(), "Receive Hello message from %i with %i hello_msg, inside populate_mprselset \n", message.originator_addr(), message.hello_msgs_count() );

	// Construct OLSR_hello from BroadcastHelloMessage& message
	OLSR_hello hello;
	hello.htime_ 			= message.htime();
	hello.willingness_		= message.willingness();
	hello.hello_msg_count_ 	= message.hello_msgs_count();

	for (int i = 0; i < hello.hello_msg_count_; i++ )
	{
		hello.hello_body_[i].link_code_ 		= message.link_code(i, nb_addrs_count_before[i]);
		hello.hello_body_[i].link_msg_size_ 	= message.link_msg_size(i, nb_addrs_count_before[i]);
		hello.hello_body_[i].nb_addrs_count_	= message.neighbor_addr_count(i, nb_addrs_count_before[i]);
		for ( int j = 1; j < message.neighbor_addr_count(i, nb_addrs_count_before[i]); j++ )
		{
			hello.hello_body_[i].nb_addrs_[j] = message.neighbor_addr_list(i, j, nb_addrs_count_before[i]);
		}

	}


   	assert(hello.hello_msg_count_ >= 0 && hello.hello_msg_count_ <= OLSR_MAX_HELLOS);
   	for (int i = 0; i < hello.hello_msg_count_; i++)
   	{
   		OLSR_hello_msg& hello_msg = hello.hello_msg(i);
   		int nt = hello_msg.link_code() >> 2;
   		if (nt == OLSR_MPR_NEIGH)
   		{
   			assert(hello_msg.nb_addrs_count_ >= 0 && hello_msg.nb_addrs_count_ <= OLSR_MAX_ADDRS);
   			for (int j = 0; j < hello_msg.nb_addrs_count_; j++)
   			{
   				if (hello_msg.neighbor_addr_list(j) == Radio::id(os()))
   				{
   					// Create a new entry into the mpr selector set
   					OLSR_mprsel_tuple* mprsel_tuple = find_mprsel_tuple(message.originator_addr());
   					if (mprsel_tuple == NULL)
   					{
   						mprsel_tuple = new OLSR_mprsel_tuple;
   						mprsel_tuple->node_addr() = message.originator_addr();
   						mprsel_tuple->time() = now + emf_to_seconds(message.vtime());
   						add_mprsel_tuple(mprsel_tuple);

   						// Schedules mpr selector tuple deletion
   						timer_expire_mprsel_tuple(mprsel_tuple);

   					}
   					else
   						mprsel_tuple->time() = now + emf_to_seconds(message.vtime());
   				}
   			}
   		}
   	}
   }

   // -----------------------------------------------------------------------

   // brief OLSR's default forwarding algorithm.

   //param msg 			the OLSR message which must be forwarded.
   //param dup_tuple 	NULL if the message has never been considered for forwarding, or a duplicate tuple in other case.

   template<typename OsModel_P,
            typename RoutingTable_P,
            typename Clock_P,
            typename Radio_P,
            typename Debug_P>
   void
   OlsrRouting<OsModel_P, RoutingTable_P, Clock_P, Radio_P, Debug_P>::
   forward_hello(node_id_t from, BroadcastHelloMessage& message, OLSR_dup_tuple* dup_tuple)
   {
	time_t now = Clock::time( os() );

   	OLSR_link_tuple* link_tuple = find_sym_link_tuple(from, now);   	// If the sender address is not in the symmetric 1-hop neighborhood the message must not be forwarded
   	if (link_tuple == NULL)
   		return;

   	if (dup_tuple != NULL && dup_tuple->retransmitted())   				// If the message has already been considered for forwarding, it must not be retransmitted again
   	{
#ifdef DEBUG_OLSRROUTING
   		Debug::debug(os(), "%f: Node %d does not forward a message received from %d because it is duplicated\n", Clock::time( os() ), Radio::id( os() ), dup_tuple->addr() );
#endif

   		return;
   	}

   	bool retransmitted = false;   										// If sender_address is an address of a MPR selector of this node and ttl > 1, the message must be retransmitted
   	if (message.ttl() > 1)
   	{
   		OLSR_mprsel_tuple* mprsel_tuple = find_mprsel_tuple(from);
   		if (mprsel_tuple != NULL) 										// the sender address is an address of a MPR selector of this local node
   		{
   			message.set_ttl( message.ttl() - 1 );
   			message.set_hop_count( message.hop_count() + 1 );

   			Timer::template set_timer<self_type, &self_type::timer_elapsed>( os(), delay_, this, 0 );	// introduce a random delay before retransmit the message
   			Radio::send( os(), Radio::BROADCAST_ADDRESS, message.buffer_size(), (uint8_t*)&message);
   			retransmitted = true;
   		}
   	}

   	if (dup_tuple != NULL)   											// Update duplicate tuple
   	{
   		dup_tuple->time()		= now + OLSR_DUP_HOLD_TIME;
   		dup_tuple->retransmitted()	= retransmitted;
   	}
   	else   																// or create a new one
   	{
   		OLSR_dup_tuple* new_dup 	= new OLSR_dup_tuple;
   		new_dup->addr()				= message.originator_addr();
   		new_dup->seq_num()			= message.msg_seq_num();
   		new_dup->time()				= now + OLSR_DUP_HOLD_TIME;
   		new_dup->retransmitted()	= retransmitted;
   		add_dup_tuple(new_dup);

   		// Schedules dup tuple deletion
   		timer_expire_dup_tuple(new_dup);
   	}
   }
   // -----------------------------------------------------------------------

   // brief OLSR's default forwarding algorithm.

   //param msg 			the OLSR message which must be forwarded.
   //param dup_tuple 	NULL if the message has never been considered for forwarding, or a duplicate tuple in other case.

   template<typename OsModel_P,
            typename RoutingTable_P,
            typename Clock_P,
            typename Radio_P,
            typename Debug_P>
   void
   OlsrRouting<OsModel_P, RoutingTable_P, Clock_P, Radio_P, Debug_P>::
   forward_tc(node_id_t from, BroadcastTcMessage& message, OLSR_dup_tuple* dup_tuple)
   {
	time_t now = Clock::time( os() );

   	OLSR_link_tuple* link_tuple = find_sym_link_tuple(from, now);   	// If the sender address is not in the symmetric 1-hop neighborhood the message must not be forwarded
   	if (link_tuple == NULL)
   		return;

   	if (dup_tuple != NULL && dup_tuple->retransmitted())   				// If the message has already been considered for forwarding, it must not be retransmitted again
   	{
#ifdef DEBUG_OLSRROUTING
   		Debug::debug(os(), "%f: Node %d does not forward a message received from %d because it is duplicated\n", Clock::time( os() ), Radio::id( os() ), dup_tuple->addr() );
#endif

   		return;
   	}

   	bool retransmitted = false;   										// If sender_address is an address of a MPR selector of this node and ttl > 1, the message must be retransmitted
   	if (message.ttl() > 1)
   	{
   		OLSR_mprsel_tuple* mprsel_tuple = find_mprsel_tuple(from);
   		if (mprsel_tuple != NULL) 										// the sender address is an address of a MPR selector of this local node
   		{
   			message.set_ttl( message.ttl() - 1 );
   			message.set_hop_count( message.hop_count() + 1 );

   			Timer::template set_timer<self_type, &self_type::timer_elapsed>( os(), delay_, this, 0 );	// introduce a random delay before retransmit the message
   			Radio::send( os(), Radio::BROADCAST_ADDRESS, message.buffer_size(), (uint8_t*)&message);
   			retransmitted = true;
   		}
   	}

   	if (dup_tuple != NULL)   											// Update duplicate tuple
   	{
   		dup_tuple->time()		= now + OLSR_DUP_HOLD_TIME;
   		dup_tuple->retransmitted()	= retransmitted;
   	}
   	else   																// or create a new one
   	{
   		OLSR_dup_tuple* new_dup 	= new OLSR_dup_tuple;
   		new_dup->addr()				= message.originator_addr();
   		new_dup->seq_num()			= message.msg_seq_num();
   		new_dup->time()				= now + OLSR_DUP_HOLD_TIME;
   		new_dup->retransmitted()	= retransmitted;
   		add_dup_tuple(new_dup);

   		// Schedules dup tuple deletion
   		timer_expire_dup_tuple(new_dup);
   	}
   }

   // -----------------------------------------------------------------------
   // brief Processes a TC message following RFC 3626 specification, The Topology Set is updated (if needed) with the information of the received TC message.

   // param msg 	the OLSR message which contains the TC message.
   // param sender 	the address of the node where the message was sent from.

   template<typename OsModel_P,
             typename RoutingTable_P,
             typename Clock_P,
             typename Radio_P,
             typename Debug_P>
    void
    OlsrRouting<OsModel_P, RoutingTable_P, Clock_P, Radio_P, Debug_P>::
    process_tc(BroadcastTcMessage& message, node_id_t sender)
    {
	Debug::debug(os(), "%i received TC from %i of size %i  \n", Radio::id(os()), message.originator_addr(), message.msg_size() );
	//Debug::debug(os(), "%i received TC from %i \n", Radio::id(os()), message.originator_addr());

    time_t now = Clock::time( os() );

	// Construct OLSR_tc from BroadcastTcMessage& message
	OLSR_tc tc;
	tc.ansn_ 					= message.ansn();
	tc.adv_nb_addrs_count_		= message.adv_neighbor_addr_list_size();
	for ( int i = 1; i < message.adv_neighbor_addr_list_size(); i++ )
	{
		tc.adv_nb_addrs_[i] = message.adv_neighbor_addr_list(i);
	}


   	// 1. If the sender of this message is not in the symmetric 1-hop neighborhood of this node, the message MUST be discarded.
   	OLSR_link_tuple* link_tuple = find_sym_link_tuple(sender, now);
   	if (link_tuple == NULL)
   		return;

   	// 2. If there exist some tuple in the topology set where:
   	// 	  T_last_addr == originator address AND T_seq > ANSN, then further processing of this TC message MUST NOT be performed.
   	OLSR_topology_tuple* topology_tuple = find_newer_topology_tuple(message.originator_addr(), tc.ansn());
   	if (topology_tuple != NULL)
   		return;

   	// 3. All tuples in the topology set where:
   	//	  T_last_addr == originator address AND T_seq < ANSN, MUST be removed from the topology set.
   	erase_older_topology_tuples(message.originator_addr(), tc.ansn());

   	// 4. For each of the advertised neighbor main address received in the TC message:
   	for (int i = 0; i < tc.adv_nb_addrs_count_; i++)
   	{
   		assert(i >= 0 && i < OLSR_MAX_ADDRS);
   		node_id_t addr = tc.adv_neighbor_addr_list(i);

   		// 4.1. If there exist some tuple in the topology set where:
   		// 	T_dest_addr == advertised neighbor main address, AND T_last_addr == originator address,
   		//  then the holding time of that tuple MUST be set to:  T_time = current time + validity time.

   		OLSR_topology_tuple* topology_tuple = find_topology_tuple(addr, message.originator_addr());
   		if (topology_tuple != NULL)
   			topology_tuple->time() = now + emf_to_seconds(message.vtime());

   		// 4.2. Otherwise, a new tuple MUST be recorded in the topology set where:
   		//		T_dest_addr = advertised neighbor main address,
   		//		T_last_addr = originator address,
   		//		T_seq       = ANSN,
   		//		T_time      = current time + validity time.
   		else
   		{
   			OLSR_topology_tuple* topology_tuple = new OLSR_topology_tuple;
   			topology_tuple->dest_addr()	= addr;
   			topology_tuple->last_addr()	= message.originator_addr();
   			topology_tuple->seq()		= tc.ansn();
   			topology_tuple->time()		= now + emf_to_seconds(message.vtime());
   			add_topology_tuple(topology_tuple);

   			// Schedules topology tuple deletion
   			timer_expire_topology_tuple(topology_tuple);
   		}
   	}
   }

   // -----------------------------------------------------------------------
   // param msg 	the OLSR message which contains the TC message.
   // param sender 	the address of the node where the message was sent from.

   template<typename OsModel_P,
            typename RoutingTable_P,
            typename Clock_P,
            typename Radio_P,
            typename Debug_P>
    void
    OlsrRouting<OsModel_P, RoutingTable_P, Clock_P, Radio_P, Debug_P>::
    process_data(RoutingMessage& msg, node_id_t sender)
    {
       Debug::debug(os(), "%i received DATA from %i \n", Radio::id(os()), msg.source());

       if (msg.destination() == Radio::id(os()))																// Got the destination
		{
           Debug::debug(os(), "%i got his DATA from %i \n", Radio::id(os()), msg.source());
		}
       else if (route_exists(msg.destination())) 																// Check any route to destination in the local routing table
		{
           Debug::debug(os(), "%i forwards DATA for %i. next hop [%i] %i hops away \n",
							   Radio::id(os()), msg.destination(), routing_table_[msg.destination()].next_addr, routing_table_[msg.destination()].hops);

           Radio::send(os(), routing_table_[msg.destination()].next_addr, msg.buffer_size(), (uint8_t*) & msg);	//Forward message to next hop in the routing table entry
           //routing_table_[msg.destination()].lifetime = ROUTE_TIMEOUT;
		}
       else
        {
           Debug::debug(os(), "%i ERROR: no route for \n", Radio::id(os()), msg.destination());

        }
    }

   // -----------------------------------------------------------------------
   // brief Creates the routing table of the node following RFC 3626 hints.

   // The routing table is updated in case of:
   // -- neighbor appearance or loss
   // -- 2-hop tuple is created or removed
   // -- topology tuple is created or removed
   // -- multiple interface association information changes

   template<typename OsModel_P,
            typename RoutingTable_P,
            typename Clock_P,
            typename Radio_P,
            typename Debug_P>
    void
    OlsrRouting<OsModel_P, RoutingTable_P, Clock_P, Radio_P, Debug_P>::
    routing_table_computation()
    {
   	 routing_table_.clear();												 			// 1. All the entries from the routing table are removed.

   	 for (typename nbset_t::iterator it = nbset().begin(); it != nbset().end(); it++)   // 2. New routing entries are added starting with the symmetric neighbors (h=1) as the destination nodes.
   	 {
   	 	 OLSR_nb_tuple* nb_tuple = *it;

   		 if (nb_tuple->status() == OLSR_STATUS_SYM)
   		 {
   			 bool nb_node_addr = false;
   			 OLSR_link_tuple* lt = NULL;

   			 for (typename linkset_t::iterator it2 = linkset().begin(); it2 != linkset().end(); it2++)
   			 {
   				 OLSR_link_tuple* link_tuple = *it2;
   				 if (link_tuple->nb_node_addr() == nb_tuple->nb_node_addr() && link_tuple->time() >= Clock::time( os() ))
   				 {
   					 lt = link_tuple;

#ifdef DEBUG_OLSRROUTING
   					 Debug::debug( os(), "OlsrRouting: Add %i because not known\n", link_tuple->nb_node_addr() );
#endif
   					 routing_table_[link_tuple->nb_node_addr()] = RoutingTableEntry(link_tuple->nb_node_addr(), link_tuple->nb_node_addr(), 1); //insert

   					 if (link_tuple->nb_node_addr() == nb_tuple->nb_node_addr())
   						 nb_node_addr = true;
   				 }
   			 }

   			 if (!nb_node_addr && lt != NULL)
   			 {
#ifdef DEBUG_OLSRROUTING
				 Debug::debug( os(), "OlsrRouting: Add %i because not known\n", nb_tuple->nb_node_addr() );
#endif
				 routing_table_[nb_tuple->nb_node_addr()] = RoutingTableEntry(nb_tuple->nb_node_addr(), lt->nb_node_addr(), 1);  //insert
   			 }
   		 }
   	 }

   	// N2 is the set of 2-hop neighbors reachable from this node, excluding:

   	// (i)   the nodes only reachable by members of N with willingness WILL_NEVER
   	// (ii)  the node performing the computation
   	// (iii) all the symmetric neighbors: the nodes for which there exists a symmetric link to this node on some interface.
   	 for (typename nb2hopset_t::iterator it = nb2hopset().begin(); it != nb2hopset().end(); it++)
   	 {
   		 OLSR_nb2hop_tuple* nb2hop_tuple = *it;
   		 bool ok = true;
   		 OLSR_nb_tuple* nb_tuple = find_sym_nb_tuple(nb2hop_tuple->nb_node_addr());
   		 if (nb_tuple == NULL)
   			 ok = false;
   		 else
   		 {
   			 nb_tuple = find_nb_tuple(nb2hop_tuple->nb_node_addr(), OLSR_WILL_NEVER);
   			 if (nb_tuple != NULL)
   				 ok = false;
   			 else
   			 {
   				 nb_tuple = find_sym_nb_tuple(nb2hop_tuple->nb2hop_addr());
   				 if (nb_tuple != NULL)
   					 ok = false;
   			 }
   		 }

   		// Successfully find an entry in Nb_Tuple with willingness != NEVER
   		// 3. For each node in N2 create a new entry in the routing table
   		 if (ok) {
   			 RoutingTableIterator it = routing_table_.find(nb2hop_tuple->nb_node_addr());
   			 assert(it != NULL);

#ifdef DEBUG_OLSRROUTING
				 Debug::debug( os(), "OlsrRouting: Add %i because not known\n", nb2hop_tuple->nb2hop_addr() );
#endif
				 routing_table_[nb2hop_tuple->nb2hop_addr()] = RoutingTableEntry(nb2hop_tuple->nb2hop_addr(), it->second.next_addr, 2);  //insert
   		 }
   	 }

   	 for (uint32_t h = 2; ; h++)
   	 {
   		 bool added = false;

   		// 4.1. For each topology entry in the topology table, if its T_dest_addr does not correspond to R_dest_addr of any
   		// route entry in the routing table AND its T_last_addr corresponds to R_dest_addr of a route entry whose R_dist
   		// is equal to h, then a new route entry MUST be recorded in the routing table (if it does not already exist)
   		 for (typename topologyset_t::iterator it = topologyset().begin(); it != topologyset().end(); it++)
   		 {
   			OLSR_topology_tuple* topology_tuple = *it;
   			RoutingTableIterator it1 = routing_table_.find(topology_tuple->dest_addr());
   			RoutingTableIterator it2 = routing_table_.find(topology_tuple->last_addr());
   			 if (it1 == routing_table_.end() && it2 != routing_table_.end() && it2->second.dest_addr == h)
   			 {
#ifdef DEBUG_OLSRROUTING
				 Debug::debug( os(), "OlsrRouting: Add %i because not known\n", topology_tuple->dest_addr() );
#endif
				 routing_table_[topology_tuple->dest_addr()] = RoutingTableEntry(topology_tuple->dest_addr(), it2->second.next_addr, h+1);  //insert

   				 added = true;
   			 }
   		 }

   		 if (!added)
   			 break;
   	 }
   }

   // -----------------------------------------------------------------------

   template<typename OsModel_P,
            typename RoutingTable_P,
            typename Clock_P,
            typename Radio_P,
            typename Debug_P>
   void
   OlsrRouting<OsModel_P, RoutingTable_P, Clock_P, Radio_P, Debug_P>::
   add_dup_tuple(OLSR_dup_tuple* tuple)
   {
   	insert_dup_tuple(tuple);
   }
   // -----------------------------------------------------------------------
   template<typename OsModel_P,
            typename RoutingTable_P,
            typename Clock_P,
            typename Radio_P,
            typename Debug_P>
   class OlsrRouting<OsModel_P, RoutingTable_P, Clock_P, Radio_P, Debug_P>::OLSR_dup_tuple*
   OlsrRouting<OsModel_P, RoutingTable_P, Clock_P, Radio_P, Debug_P>::
   find_dup_tuple(node_id_t addr, uint16_t seq_num)
   {
   	for (typename dupset_t::iterator it = dupset_.begin(); it != dupset_.end(); it++)
		{
			OLSR_dup_tuple* tuple = *it;
			if (tuple->addr() == addr && tuple->seq_num() == seq_num)
				return tuple;
		}

   	return NULL;
   }

   // -----------------------------------------------------------------------
   template<typename OsModel_P,
            typename RoutingTable_P,
            typename Clock_P,
            typename Radio_P,
            typename Debug_P>
   void
   OlsrRouting<OsModel_P, RoutingTable_P, Clock_P, Radio_P, Debug_P>::
   rm_dup_tuple(OLSR_dup_tuple* tuple)
   {
   	erase_dup_tuple(tuple);
   }
   // -----------------------------------------------------------------------

   template<typename OsModel_P,
            typename RoutingTable_P,
            typename Clock_P,
            typename Radio_P,
            typename Debug_P>
   void
   OlsrRouting<OsModel_P, RoutingTable_P, Clock_P, Radio_P, Debug_P>::
   erase_dup_tuple(OLSR_dup_tuple* tuple)
   {
   	for (typename dupset_t::iterator it = dupset_.begin(); it != dupset_.end(); it++)
   	{
   		if (*it == tuple)
   		{
   			dupset_.erase(it);
   			break;
   		}
   	}
   }

   // -----------------------------------------------------------------------

   template<typename OsModel_P,
            typename RoutingTable_P,
            typename Clock_P,
            typename Radio_P,
            typename Debug_P>
   void
   OlsrRouting<OsModel_P, RoutingTable_P, Clock_P, Radio_P, Debug_P>::
   insert_dup_tuple(OLSR_dup_tuple* tuple)
   {
   	dupset_.push_back(tuple);
   }

   // -----------------------------------------------------------------------
   // brief Adds a link tuple to the Link Set (and an associated neighbor tuple to the Neighbor Set).
   // param tuple the link tuple to be added.
   // param willingness willingness of the node which is going to be inserted in the Neighbor Set.

   template<typename OsModel_P,
            typename RoutingTable_P,
            typename Clock_P,
            typename Radio_P,
            typename Debug_P>
   void
   OlsrRouting<OsModel_P, RoutingTable_P, Clock_P, Radio_P, Debug_P>::
   add_link_tuple(OLSR_link_tuple* tuple, uint8_t  willingness)
   {
	time_t now = Clock::time( os() );

#ifdef DEBUG_OLSRROUTING
	Debug::debug(os(), "%f: Node %d adds link tuple: nb_addr = %d\n", now, Radio::id(os()), tuple->nb_node_addr());
#endif

   	insert_link_tuple(tuple);
   	// Creates associated neighbor tuple
   	OLSR_nb_tuple* nb_tuple		= new OLSR_nb_tuple;
   	nb_tuple->nb_node_addr()	= tuple->nb_node_addr();
   	nb_tuple->willingness()		= willingness;
   	if (tuple->sym_time() >= now)
   		nb_tuple->status() = OLSR_STATUS_SYM;
   	else
   		nb_tuple->status() = OLSR_STATUS_NOT_SYM;
   	add_nb_tuple(nb_tuple);
   }
   // -----------------------------------------------------------------------
   // brief Removes a link tuple from the Link Set.
   // param tuple the link tuple to be removed.

   template<typename OsModel_P,
            typename RoutingTable_P,
            typename Clock_P,
            typename Radio_P,
            typename Debug_P>
   void
   OlsrRouting<OsModel_P, RoutingTable_P, Clock_P, Radio_P, Debug_P>::
   rm_link_tuple(OLSR_link_tuple* tuple)
   {
   	node_id_t nb_addr	= tuple->nb_node_addr();
    time_t now = Clock::time( os() );

#ifdef DEBUG_OLSRROUTING
    Debug::debug(os(), "%f: Node %d removes neighbor tuple: nb_addr = %d\n", now, Radio::id(os()), nb_addr);
#endif

   	erase_link_tuple(tuple);

   	OLSR_nb_tuple* nb_tuple = find_nb_tuple(nb_addr);
   	erase_nb_tuple(nb_tuple);
   	delete nb_tuple;
   }
   // -----------------------------------------------------------------------
   // brief	This function is invoked when a link tuple is updated. Its aim is to also update the corresponding neighbor tuple if it is needed.
   // param tuple the link tuple which has been updated.

   template<typename OsModel_P,
            typename RoutingTable_P,
            typename Clock_P,
            typename Radio_P,
            typename Debug_P>
   void
   OlsrRouting<OsModel_P, RoutingTable_P, Clock_P, Radio_P, Debug_P>::
   updated_link_tuple(OLSR_link_tuple* tuple)
   {
	time_t now = Clock::time( os() );

   	// Each time a link tuple changes, the associated neighbor tuple must be recomputed, basically the "nb_tuple->status" should be updated
   	OLSR_nb_tuple* nb_tuple = find_nb_tuple(tuple->nb_node_addr());
   	if (nb_tuple != NULL) {
   		if (tuple->lost_time() >= now)
   			nb_tuple->status() = OLSR_STATUS_NOT_SYM;
   		else if (tuple->sym_time() >= now)
   			nb_tuple->status() = OLSR_STATUS_SYM;
   		else
   			nb_tuple->status() = OLSR_STATUS_NOT_SYM;
   	}

#ifdef DEBUG_OLSRROUTING
   	Debug::debug(os(), "%f: Node %d has updated link tuple: nb_addr = %d status = %s\n", now, Radio::id(os()), tuple->nb_node_addr(),
   		((nb_tuple->status() == OLSR_STATUS_SYM) ? "sym" : "not_sym"));
#endif
   }
   // -----------------------------------------------------------------------

   template<typename OsModel_P,
            typename RoutingTable_P,
            typename Clock_P,
            typename Radio_P,
            typename Debug_P>
   class OlsrRouting<OsModel_P, RoutingTable_P, Clock_P, Radio_P, Debug_P>::OLSR_link_tuple *
   OlsrRouting<OsModel_P, RoutingTable_P, Clock_P, Radio_P, Debug_P>::
   find_link_tuple(node_id_t node_addr)
   {
   	for (typename linkset_t::iterator it = linkset_.begin(); it != linkset_.end(); it++)
   	{
   		OLSR_link_tuple* tuple = *it;
   		if (tuple->nb_node_addr() == node_addr)
   		{
#ifdef DEBUG_OLSRROUTING
   			Debug::debug(os(), "Find existing tuple!!!\n");
#endif
   			return tuple;
   		}
   		else
   		{
#ifdef DEBUG_OLSRROUTING
   			Debug::debug(os(), "NO existing tuple!!!\n");
#endif
   			return NULL;
   		}
   	}
   }

   // -----------------------------------------------------------------------

   template<typename OsModel_P,
            typename RoutingTable_P,
            typename Clock_P,
            typename Radio_P,
            typename Debug_P>
   class OlsrRouting<OsModel_P, RoutingTable_P, Clock_P, Radio_P, Debug_P>::OLSR_link_tuple *
   OlsrRouting<OsModel_P, RoutingTable_P, Clock_P, Radio_P, Debug_P>::
   find_sym_link_tuple(node_id_t node_addr, double now)
   {

   	for (typename linkset_t::iterator it = linkset_.begin(); it != linkset_.end(); it++)
   	{
   		OLSR_link_tuple* tuple = *it;
   		if (tuple->nb_node_addr() == node_addr)
   		{
   			if (tuple->sym_time() > now)
   				return tuple;
   			else
   				break;
   		}
   	}
   	return NULL;
   }

   // -----------------------------------------------------------------------

   template<typename OsModel_P,
            typename RoutingTable_P,
            typename Clock_P,
            typename Radio_P,
            typename Debug_P>
   void
   OlsrRouting<OsModel_P, RoutingTable_P, Clock_P, Radio_P, Debug_P>::
   erase_link_tuple(OLSR_link_tuple* tuple)
   {
   	for (typename linkset_t::iterator it = linkset_.begin(); it != linkset_.end(); it++)
		{
			if (*it == tuple)
			{
				linkset_.erase(it);
				break;
			}
		}
   }

   // -----------------------------------------------------------------------

   template<typename OsModel_P,
            typename RoutingTable_P,
            typename Clock_P,
            typename Radio_P,
            typename Debug_P>
   void
   OlsrRouting<OsModel_P, RoutingTable_P, Clock_P, Radio_P, Debug_P>::
   insert_link_tuple(OLSR_link_tuple* tuple)
   {
   	linkset_.push_back(tuple);
   }
   // -----------------------------------------------------------------------


   // brief Adds a neighbor tuple to the Neighbor Set.
   // param tuple the neighbor tuple to be added.

   template<typename OsModel_P,
            typename RoutingTable_P,
            typename Clock_P,
            typename Radio_P,
            typename Debug_P>
   void
   OlsrRouting<OsModel_P, RoutingTable_P, Clock_P, Radio_P, Debug_P>::
   add_nb_tuple(OLSR_nb_tuple* tuple)
   {
#ifdef DEBUG_OLSRROUTING
	   Debug::debug(os(), "%f: Node %d adds neighbor tuple: nb_addr = %d status = %s\n", Clock::time( os() ), Radio::id(os()), tuple->nb_node_addr(),
   		((tuple->status() == OLSR_STATUS_SYM) ? "sym" : "not_sym"));
#endif

   	insert_nb_tuple(tuple);
   }
   // -----------------------------------------------------------------------
   // brief Removes a neighbor tuple from the Neighbor Set.
   // param tuple the neighbor tuple to be removed.

   template<typename OsModel_P,
            typename RoutingTable_P,
            typename Clock_P,
            typename Radio_P,
            typename Debug_P>
   void
   OlsrRouting<OsModel_P, RoutingTable_P, Clock_P, Radio_P, Debug_P>::
   rm_nb_tuple(OLSR_nb_tuple* tuple)
   {
#ifdef DEBUG_OLSRROUTING
	   Debug::debug( "%f: Node %d removes neighbor tuple: nb_addr = %d status = %s\n", Clock::time( os() ),
																			 Radio::id(os()),
																			 tuple->nb_node_addr(),
																			 ((tuple->status() == OLSR_STATUS_SYM) ? "sym" : "not_sym")  );
#endif

   	erase_nb_tuple(tuple);
   }
   // -----------------------------------------------------------------------
   template<typename OsModel_P,
            typename RoutingTable_P,
            typename Clock_P,
            typename Radio_P,
            typename Debug_P>
   class OlsrRouting<OsModel_P, RoutingTable_P, Clock_P, Radio_P, Debug_P>::OLSR_nb_tuple *
   OlsrRouting<OsModel_P, RoutingTable_P, Clock_P, Radio_P, Debug_P>::
   find_nb_tuple(node_id_t node_addr) 						// find a neighbor with NO SPECIFIC requirement
   {
   	for (typename nbset_t::iterator it = nbset_.begin(); it != nbset_.end(); it++)
	{
		OLSR_nb_tuple* tuple = *it;
		if (tuple->nb_node_addr() == node_addr)
			return tuple;
	}
   	return NULL;
   }

   // -----------------------------------------------------------------------
   template<typename OsModel_P,
            typename RoutingTable_P,
            typename Clock_P,
            typename Radio_P,
            typename Debug_P>
   class OlsrRouting<OsModel_P, RoutingTable_P, Clock_P, Radio_P, Debug_P>::OLSR_nb_tuple*
   OlsrRouting<OsModel_P, RoutingTable_P, Clock_P, Radio_P, Debug_P>::
   find_nb_tuple(node_id_t node_addr, uint8_t willingness) // find a neighbor with certain WILLNESS requirement
   {
   	for (typename nbset_t::iterator it = nbset_.begin(); it != nbset_.end(); it++)
   	{
   		OLSR_nb_tuple* tuple = *it;
   		if (tuple->nb_node_addr() == node_addr && tuple->willingness() == willingness)
   			return tuple;
   	}
   	return NULL;
   }

   // -----------------------------------------------------------------------
   template<typename OsModel_P,
            typename RoutingTable_P,
            typename Clock_P,
            typename Radio_P,
            typename Debug_P>
   class OlsrRouting<OsModel_P, RoutingTable_P, Clock_P, Radio_P, Debug_P>::OLSR_nb_tuple*
   OlsrRouting<OsModel_P, RoutingTable_P, Clock_P, Radio_P, Debug_P>::
   find_sym_nb_tuple(node_id_t node_addr) 					// find a neighbor with certain SYMMETRIC requirement
   {
   	for (typename nbset_t::iterator it = nbset_.begin(); it != nbset_.end(); it++)
   	{
   		OLSR_nb_tuple* tuple = *it;
   		if (tuple->nb_node_addr() == node_addr && tuple->status() == OLSR_STATUS_SYM)
   			return tuple;
   	}
   	return NULL;
   }

   // -----------------------------------------------------------------------
   template<typename OsModel_P,
            typename RoutingTable_P,
            typename Clock_P,
            typename Radio_P,
            typename Debug_P>
   void
   OlsrRouting<OsModel_P, RoutingTable_P, Clock_P, Radio_P, Debug_P>::
   erase_nb_tuple(OLSR_nb_tuple* tuple)
   {
   	for (typename nbset_t::iterator it = nbset_.begin(); it != nbset_.end(); it++)
   	{
   		if (*it == tuple) {
   			nbset_.erase(it);
   			break;
   		}
   	}
   }

   // -----------------------------------------------------------------------
   template<typename OsModel_P,
            typename RoutingTable_P,
            typename Clock_P,
            typename Radio_P,
            typename Debug_P>
   void
   OlsrRouting<OsModel_P, RoutingTable_P, Clock_P, Radio_P, Debug_P>::
   insert_nb_tuple(OLSR_nb_tuple* tuple)
   {
   	nbset_.push_back(tuple);
   }

   // -----------------------------------------------------------------------

   // brief Adds a 2-hop neighbor tuple to the 2-hop Neighbor Set.
   // param tuple the 2-hop neighbor tuple to be added.

   template<typename OsModel_P,
            typename RoutingTable_P,
            typename Clock_P,
            typename Radio_P,
            typename Debug_P>
   void
   OlsrRouting<OsModel_P, RoutingTable_P, Clock_P, Radio_P, Debug_P>::
   add_nb2hop_tuple(OLSR_nb2hop_tuple* tuple)
   {
#ifdef DEBUG_OLSRROUTING
	   Debug::debug(os(), "%f: Node %d adds 2-hop neighbor tuple: nb_addr = %d nb2hop_addr = %d\n", Clock::time( os() ), Radio::id(os()),
		tuple->nb_node_addr(), tuple->nb2hop_addr());
#endif

   	insert_nb2hop_tuple(tuple);
   }
   // -----------------------------------------------------------------------
   // brief Removes a 2-hop neighbor tuple from the 2-hop Neighbor Set.
   // param tuple the 2-hop neighbor tuple to be removed.

   template<typename OsModel_P,
            typename RoutingTable_P,
            typename Clock_P,
            typename Radio_P,
            typename Debug_P>
   void
   OlsrRouting<OsModel_P, RoutingTable_P, Clock_P, Radio_P, Debug_P>::
   rm_nb2hop_tuple(OLSR_nb2hop_tuple* tuple) {
#ifdef DEBUG_OLSRROUTING
	   Debug::debug(os(), "%f: Node %d removes 2-hop neighbor tuple: nb_addr = %d nb2hop_addr = %d\n", Clock::time( os() ), Radio::id(os()),
   		tuple->nb_node_addr(), tuple->nb2hop_addr());
#endif

   	erase_nb2hop_tuple(tuple);
   }
   // -----------------------------------------------------------------------

   template<typename OsModel_P,
            typename RoutingTable_P,
            typename Clock_P,
            typename Radio_P,
            typename Debug_P>
   class OlsrRouting<OsModel_P, RoutingTable_P, Clock_P, Radio_P, Debug_P>::OLSR_nb2hop_tuple *
   OlsrRouting<OsModel_P, RoutingTable_P, Clock_P, Radio_P, Debug_P>::
   find_nb2hop_tuple(node_id_t nb_node_addr, node_id_t nb2hop_addr)
   {
   	for (typename nb2hopset_t::iterator it = nb2hopset_.begin(); it != nb2hopset_.end(); it++)
   	{
   		OLSR_nb2hop_tuple* tuple = *it;
   		if (tuple->nb_node_addr() == nb_node_addr && tuple->nb2hop_addr() == nb2hop_addr)
   			return tuple;
   	}
   	return NULL;
   }

   // -----------------------------------------------------------------------

   template<typename OsModel_P,
            typename RoutingTable_P,
            typename Clock_P,
            typename Radio_P,
            typename Debug_P>
   void
   OlsrRouting<OsModel_P, RoutingTable_P, Clock_P, Radio_P, Debug_P>::
   erase_nb2hop_tuple(OLSR_nb2hop_tuple* tuple)
   {
   	for (typename nb2hopset_t::iterator it = nb2hopset_.begin(); it != nb2hopset_.end(); it++)
   	{
   		if (*it == tuple)
   		{
   			nb2hopset_.erase(it);
   			break;
   		}
   	}
   }
   // -----------------------------------------------------------------------

   template<typename OsModel_P,
            typename RoutingTable_P,
            typename Clock_P,
            typename Radio_P,
            typename Debug_P>
   void
   OlsrRouting<OsModel_P, RoutingTable_P, Clock_P, Radio_P, Debug_P>::
   erase_nb2hop_tuples(node_id_t nb_node_addr)
   {
	for (typename nb2hopset_t::iterator it = nb2hopset_.begin(); it != nb2hopset_.end(); it++)
		{
			OLSR_nb2hop_tuple* tuple = *it;
			if (tuple->nb_node_addr() == nb_node_addr)
			{
				it = nb2hopset_.erase(it);
				it--;
			}
		}
   }
   // -----------------------------------------------------------------------

   template<typename OsModel_P,
            typename RoutingTable_P,
            typename Clock_P,
            typename Radio_P,
            typename Debug_P>
   void
   OlsrRouting<OsModel_P, RoutingTable_P, Clock_P, Radio_P, Debug_P>::
   erase_nb2hop_tuples(node_id_t nb_node_addr, node_id_t nb2hop_addr)
   {
   	for (typename nb2hopset_t::iterator it = nb2hopset_.begin(); it != nb2hopset_.end(); it++)
		{
			OLSR_nb2hop_tuple* tuple = *it;
			if (tuple->nb_node_addr() == nb_node_addr && tuple->nb2hop_addr() == nb2hop_addr)
			{
				it = nb2hopset_.erase(it);
				it--;
			}
		}
   }
   // -----------------------------------------------------------------------

   template<typename OsModel_P,
            typename RoutingTable_P,
            typename Clock_P,
            typename Radio_P,
            typename Debug_P>
   void
   OlsrRouting<OsModel_P, RoutingTable_P, Clock_P, Radio_P, Debug_P>::
   insert_nb2hop_tuple(OLSR_nb2hop_tuple* tuple)
   {
   	nb2hopset_.push_back(tuple);
   }

   // -----------------------------------------------------------------------
   // brief Adds an MPR selector tuple to the MPR Selector Set, Advertised Neighbor Sequence Number (ANSN) is also updated.
   // param tuple the MPR selector tuple to be added.

   template<typename OsModel_P,
            typename RoutingTable_P,
            typename Clock_P,
            typename Radio_P,
            typename Debug_P>
   void
   OlsrRouting<OsModel_P, RoutingTable_P, Clock_P, Radio_P, Debug_P>::
   add_mprsel_tuple(OLSR_mprsel_tuple* tuple) {
#ifdef DEBUG_OLSRROUTING
	   Debug::debug(os(), "%f: Node %d adds MPR selector tuple: nb_addr = %d\n", Clock::time( os() ), Radio::id(os()), tuple->node_addr() );
#endif

   	insert_mprsel_tuple(tuple);
   	ansn_ = (ansn_ + 1)%(OLSR_MAX_SEQ_NUM + 1);
   }
   // -----------------------------------------------------------------------
   // brief Removes an MPR selector tuple from the MPR Selector Set, Advertised Neighbor Sequence Number (ANSN) is also updated.
   // param tuple the MPR selector tuple to be removed.

   template<typename OsModel_P,
            typename RoutingTable_P,
            typename Clock_P,
            typename Radio_P,
            typename Debug_P>
   void
   OlsrRouting<OsModel_P, RoutingTable_P, Clock_P, Radio_P, Debug_P>::
   rm_mprsel_tuple(OLSR_mprsel_tuple* tuple) {
#ifdef DEBUG_OLSRROUTING
	   Debug::debug(os(), "%f: Node %d removes MPR selector tuple: nb_addr = %d\n", Clock::time( os() ), Radio::id(os()), tuple->node_addr() );
#endif

   	erase_mprsel_tuple(tuple);
   	ansn_ = (ansn_ + 1)%(OLSR_MAX_SEQ_NUM + 1);
   }
   // -----------------------------------------------------------------------

   template<typename OsModel_P,
            typename RoutingTable_P,
            typename Clock_P,
            typename Radio_P,
            typename Debug_P>
   class OlsrRouting<OsModel_P, RoutingTable_P, Clock_P, Radio_P, Debug_P>::OLSR_mprsel_tuple *
   OlsrRouting<OsModel_P, RoutingTable_P, Clock_P, Radio_P, Debug_P>::
   find_mprsel_tuple(node_id_t node_addr) {
   	for (typename mprselset_t::iterator it = mprselset_.begin(); it != mprselset_.end(); it++)
   	{
   		OLSR_mprsel_tuple* tuple = *it;
   		if (tuple->node_addr() == node_addr)
   			return tuple;
   	}
   	return NULL;
   }
   // -----------------------------------------------------------------------
   template<typename OsModel_P,
            typename RoutingTable_P,
            typename Clock_P,
            typename Radio_P,
            typename Debug_P>
   void
   OlsrRouting<OsModel_P, RoutingTable_P, Clock_P, Radio_P, Debug_P>::
   erase_mprsel_tuple(OLSR_mprsel_tuple* tuple)
   {
   	for (typename mprselset_t::iterator it = mprselset_.begin(); it != mprselset_.end(); it++)
   	{
   		if (*it == tuple)
   		{
   			mprselset_.erase(it);
   			break;
   		}
   	}
   }
   // -----------------------------------------------------------------------
   template<typename OsModel_P,
            typename RoutingTable_P,
            typename Clock_P,
            typename Radio_P,
            typename Debug_P>
   void
   OlsrRouting<OsModel_P, RoutingTable_P, Clock_P, Radio_P, Debug_P>::
   erase_mprsel_tuples(node_id_t node_addr)
   {
		for (typename mprselset_t::iterator it = mprselset_.begin(); it != mprselset_.end(); it++)
		{
			OLSR_mprsel_tuple* tuple = *it;
			if (tuple->node_addr() == node_addr) {
				it = mprselset_.erase(it);
				it--;
			}
		}
   }
   // -----------------------------------------------------------------------
   template<typename OsModel_P,
            typename RoutingTable_P,
            typename Clock_P,
            typename Radio_P,
            typename Debug_P>
   void
   OlsrRouting<OsModel_P, RoutingTable_P, Clock_P, Radio_P, Debug_P>::
   insert_mprsel_tuple(OLSR_mprsel_tuple* tuple)
   {
   	mprselset_.push_back(tuple);
   }

   // -----------------------------------------------------------------------
   // brief Adds a topology tuple to the Topology Set.

   // param tuple the topology tuple to be added.

   template<typename OsModel_P,
            typename RoutingTable_P,
            typename Clock_P,
            typename Radio_P,
            typename Debug_P>
   void
   OlsrRouting<OsModel_P, RoutingTable_P, Clock_P, Radio_P, Debug_P>::
   add_topology_tuple(OLSR_topology_tuple* tuple)
   {
#ifdef DEBUG_OLSRROUTING
	   Debug::debug(os(), "%f: Node %d adds topology tuple: dest_addr = %d last_addr = %d seq = %d\n",
   		  Clock::time( os() ),
   		  Radio::id(os()),
   		  tuple->dest_addr(),
   		  tuple->last_addr(),
   		  tuple->seq() );
#endif

   	insert_topology_tuple(tuple);
   }
   // -----------------------------------------------------------------------
   // brief Removes a topology tuple from the Topology Set.

   // param tuple the topology tuple to be removed.

   template<typename OsModel_P,
            typename RoutingTable_P,
            typename Clock_P,
            typename Radio_P,
            typename Debug_P>
   void
   OlsrRouting<OsModel_P, RoutingTable_P, Clock_P, Radio_P, Debug_P>::
   rm_topology_tuple(OLSR_topology_tuple* tuple)
   {
#ifdef DEBUG_OLSRROUTING
	   Debug::debug(os(), "%f: Node %d removes topology tuple: dest_addr = %d last_addr = %d seq = %d\n",
   		  Clock::time( os() ),
   		  Radio::id(os()),
   		  tuple->dest_addr(),
   		  tuple->last_addr(),
   		  tuple->seq() );
#endif

   	erase_topology_tuple(tuple);
   }
   // -----------------------------------------------------------------------

   template<typename OsModel_P,
            typename RoutingTable_P,
            typename Clock_P,
            typename Radio_P,
            typename Debug_P>
   class OlsrRouting<OsModel_P, RoutingTable_P, Clock_P, Radio_P, Debug_P>::OLSR_topology_tuple *
   OlsrRouting<OsModel_P, RoutingTable_P, Clock_P, Radio_P, Debug_P>::
   find_topology_tuple(node_id_t dest_addr, node_id_t last_addr)
   {
   	for (typename topologyset_t::iterator it = topologyset_.begin(); it != topologyset_.end(); it++)
   	{
   		OLSR_topology_tuple* tuple = *it;
   		if (tuple->dest_addr() == dest_addr && tuple->last_addr() == last_addr)
   			return tuple;
   	}
   	return NULL;
   }
   // -----------------------------------------------------------------------

   template<typename OsModel_P,
            typename RoutingTable_P,
            typename Clock_P,
            typename Radio_P,
            typename Debug_P>
   class OlsrRouting<OsModel_P, RoutingTable_P, Clock_P, Radio_P, Debug_P>::OLSR_topology_tuple *
   OlsrRouting<OsModel_P, RoutingTable_P, Clock_P, Radio_P, Debug_P>::
   find_newer_topology_tuple(node_id_t last_addr, u_int16_t ansn)
   {
   	for (typename topologyset_t::iterator it = topologyset_.begin(); it != topologyset_.end(); it++)
   	{
   		OLSR_topology_tuple* tuple = *it;
   		if (tuple->last_addr() == last_addr && tuple->seq() > ansn)
   			return tuple;
   	}
   	return NULL;
   }
   // -----------------------------------------------------------------------
   template<typename OsModel_P,
            typename RoutingTable_P,
            typename Clock_P,
            typename Radio_P,
            typename Debug_P>
   void
   OlsrRouting<OsModel_P, RoutingTable_P, Clock_P, Radio_P, Debug_P>::
   erase_topology_tuple(OLSR_topology_tuple* tuple)
   {
   	for (typename topologyset_t::iterator it = topologyset_.begin(); it != topologyset_.end(); it++)
   	{
   		if (*it == tuple)
   		{
   			topologyset_.erase(it);
   			break;
   		}
   	}
   }
   // -----------------------------------------------------------------------
   template<typename OsModel_P,
            typename RoutingTable_P,
            typename Clock_P,
            typename Radio_P,
            typename Debug_P>
   void
   OlsrRouting<OsModel_P, RoutingTable_P, Clock_P, Radio_P, Debug_P>::
   erase_older_topology_tuples(node_id_t last_addr, uint16_t ansn)
   {
   	for (typename topologyset_t::iterator it = topologyset_.begin(); it != topologyset_.end(); it++)
   	{
   		OLSR_topology_tuple* tuple = *it;
   		if (tuple->last_addr() == last_addr && tuple->seq() < ansn)
   		{
   			it = topologyset_.erase(it);
   			it--;
   		}
   	}
   }
   // -----------------------------------------------------------------------
   template<typename OsModel_P,
            typename RoutingTable_P,
            typename Clock_P,
            typename Radio_P,
            typename Debug_P>
   void
   OlsrRouting<OsModel_P, RoutingTable_P, Clock_P, Radio_P, Debug_P>::
   insert_topology_tuple(OLSR_topology_tuple* tuple)
   {
   	topologyset_.push_back(tuple);
   }
   // -----------------------------------------------------------------------
   template<typename OsModel_P,
			typename RoutingTable_P,
			typename Clock_P,
			typename Radio_P,
			typename Debug_P>
   bool
   OlsrRouting<OsModel_P, RoutingTable_P, Clock_P, Radio_P, Debug_P>::
   route_exists(node_id_t destination)
   {
	   RoutingTableIterator it = routing_table_.find(destination);
       if (it != routing_table_.end())
       {
           return true;
       }
       else
           return false;
   }

   // -----------------------------------------------------------------------
   template<typename OsModel_P,
            typename RoutingTable_P,
            typename Clock_P,
            typename Radio_P,
            typename Debug_P>
   void
   OlsrRouting<OsModel_P, RoutingTable_P, Clock_P, Radio_P, Debug_P>::
   print_routing_table( RoutingTable rt )
   {
#ifdef DEBUG_OLSRROUTING
      int i = 0;
      Debug::debug( os(), "OlsrRouting: Routing Table:\n" );
      Debug::debug( os(), "+++++++++++++++++++++++++++++++++++++++++++++++++\n" );

      if (!rt.size())
		  {
			  Debug::debug( os(), "|          Routing Table is empty!!!            |\n" );
		  }
      else
		  {
    	        for ( RoutingTableIterator it = rt.begin(); it != rt.end(); ++it )
					{
						Debug::debug( os(), "| RoutingTable[%i]: Dest %i SendTo %i Hops %i |\n", i++, it->first, it->second.next_addr, it->second.hops );
					}
		  }

      Debug::debug( os(), "+++++++++++++++++++++++++++++++++++++++++++++++++\n" );
#endif
   }

   // -----------------------------------------------------------------------
   template<typename OsModel_P,
            typename RoutingTable_P,
            typename Clock_P,
            typename Radio_P,
            typename Debug_P>
   int
   OlsrRouting<OsModel_P, RoutingTable_P, Clock_P, Radio_P, Debug_P>::
   degree(OLSR_nb_tuple* tuple)
   {
   	int degree = 0;
   	for (typename nb2hopset_t::iterator it = nb2hopset().begin(); it != nb2hopset().end(); it++)
		{
			OLSR_nb2hop_tuple* nb2hop_tuple = *it;
			if (nb2hop_tuple->nb_node_addr() == tuple->nb_node_addr())
			{
				OLSR_nb_tuple* nb_tuple = find_nb_tuple(nb2hop_tuple->nb_node_addr());
				if (nb_tuple == NULL)
					degree++;
			}
		}

   	return degree;
   }

   // -----------------------------------------------------------------------
   template<typename OsModel_P,
            typename RoutingTable_P,
            typename Clock_P,
            typename Radio_P,
            typename Debug_P>
   double
   OlsrRouting<OsModel_P, RoutingTable_P, Clock_P, Radio_P, Debug_P>::
   emf_to_seconds(uint8_t olsr_format)
   {
   	// This implementation has been taken from unik-olsrd-0.4.5 (mantissa.c), licensed under the GNU Public License (GPL)
   	int a = olsr_format >> 4;
   	int b = olsr_format - a*16;
   	return (double)(OLSR_C*(1+(double)a/16)*(double)pow(2,b));
   }

   // -----------------------------------------------------------------------
   template<typename OsModel_P,
            typename RoutingTable_P,
            typename Clock_P,
            typename Radio_P,
            typename Debug_P>
   uint8_t
   OlsrRouting<OsModel_P, RoutingTable_P, Clock_P, Radio_P, Debug_P>::
   seconds_to_emf(double seconds)
   {
   	// This implementation has been taken from unik-olsrd-0.4.5 (mantissa.c), licensed under the GNU Public License (GPL)
   	int a, b = 0;
    	while (seconds/OLSR_C >= pow((double)2, (double)b))
   		b++;
   	b--;

   	if (b < 0) {
   		a = 1;
   		b = 0;
   	}
   	else if (b > 15) {
   		a = 15;
   		b = 15;
   	}
   	else {
   		a = (int)(16*((double)seconds/(OLSR_C*(double)pow(2, b))-1));
   		while (a >= 16) {
   			a -= 16;
   			b++;
   		}
   	}

   	return (uint8_t)(a*16+b);
   }

   // -----------------------------------------------------------------------
   template<typename OsModel_P,
            typename RoutingTable_P,
            typename Clock_P,
            typename Radio_P,
            typename Debug_P>
   void
   OlsrRouting<OsModel_P, RoutingTable_P, Clock_P, Radio_P, Debug_P>::
   nb_loss(OLSR_link_tuple* tuple)
   {
#ifdef DEBUG_OLSRROUTING
	   Debug::debug(os(), "%f: Node %d detects neighbor %d loss\n", Clock::time( os() ), Radio::id(os()), tuple->nb_node_addr());
#endif

   	updated_link_tuple(tuple);
   	erase_nb2hop_tuples(tuple->nb_node_addr());
   	erase_mprsel_tuples(tuple->nb_node_addr());

   	mpr_computation();
    routing_table_computation();
   }

   // -----------------------------------------------------------------------


}
#endif
