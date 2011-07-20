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
#ifndef __ALGORITHMS_COLORING_IMJUDGED_COLORING_H__
#define __ALGORITHMS_COLORING_IMJUDGED_COLORING_H__

#include "algorithms/coloring/imjudged/IMjudged_coloring_message.h"
#include "algorithms/routing/tree/tree_routing.h"
#include "util/pstl/vector_static.h"
#include "util/pstl/pair.h"
#include "color_table.h"

//#define DEBUG_IMJUDGEDCOLORING
#define INFO_IMJUDGEDCOLORING

#define MAX_NB 5
#define MAX_NODES 5

#define ENABLE_AGGREGATION

namespace wiselib
{
   /**
    * \brief IM Judged Coloring Algorithm
    *
    *  \ingroup coloring_concept
    *  \ingroup basic_algorithm_concept
    *  \ingroup coloring_algorithm
    *
    * An IM judged coloring algorithm.
    */
    template<typename OsModel_P,
             typename Radio_P,
             typename Debug_P,
             typename Routing_P>
    class IMJudgedColoring
    {
    public:
      typedef OsModel_P OsModel;
      typedef Radio_P Radio;
      typedef Debug_P Debug;
      typedef Routing_P routing_t;
      
      typedef typename OsModel_P::Timer Timer;
      typedef typename OsModel::Rand Rand;
      typedef typename OsModel::Os Os;

      typedef IMJudgedColoring<OsModel, Radio, Debug, Routing_P> self_type;

      typedef typename Radio::node_id_t node_id_t;
      typedef typename Radio::size_t size_t;
      typedef typename Radio::block_data_t block_data_t;
      typedef typename Timer::millis_t millis_t;

      typedef TreeRouting<OsModel, Timer, Radio, Debug> tree_routing_t;
      typedef typename tree_routing_t::children_t  children_t;
      typedef typename tree_routing_t::children_t::iterator  children_iterator_t;

      typedef node_id_t color_value_type;
      typedef IMJudgedColoringMessage<OsModel, Radio, color_value_type> coloring_message;

      typedef pair<node_id_t, color_value_type> node_color;
      typedef vector_static<OsModel, node_color, MAX_NB> Neighborhood_t;
      typedef typename Neighborhood_t::iterator Neighborhood_iterator_t;

      typedef ColorsTable<OsModel, color_value_type, MAX_NODES> color_table_t;
      
      typedef pair<color_value_type, color_value_type> old_new_color;

      struct stats {
          uint32_t IMJCMsgIdBroadcastCL;
          uint32_t IMJCMsgIdColorRQ;
          uint32_t IMJCMsgIdColorRSP;
          uint32_t IMJCMsgIdColorCHG;
      }msg_info;
      bool cycle_changes_detected;
      routing_t *tree_routing;
      
  //    typedef int uint;
        ///@name Construction / Destruction
        ///@{
        IMJudgedColoring();
        ~IMJudgedColoring();
        ///@}

        ///@name 
        ///@{
        void enable(void);
        void disable(void);
        ///@}

        ///@name Methods called by Timer
        ///@{
        void timer_elapsed(void *userdata);
        ///@}

        ///@name Methods called by Timer
        ///@{
        void color_timeout(void *userdata);
        ///@}

        ///@name Methods called by Timer
        ///@{
        void neighborhood_timeout(void *userdata);
        ///@}

        ///@name Sinartisi lipsis minimatwn
        ///@{
        void receive(node_id_t from, size_t len, block_data_t *data);
        ///@}

        ///@name Sinartisi lipsis minimatwn
        ///@{
        void rcv_routing_message(node_id_t from, size_t len, block_data_t *data);
        ///@}

        ///@name Sinartisi lipsis minimatwn
        ///@{
        void tree_state_chage(uint8_t state);
        ///@}
        
        ///@Get the number of neighboors and who are they if debug
        ///@{
        const uint16_t get_neighboors();
        ///@}

        ///@Get the number of neighboors and who are they if debug
        ///@{
        uint16_t get_color_nodes();
        ///@}

#ifdef ENABLE_AGGREGATION
        void check_rcd_fragments();
#endif        

        bool exist_color_in_neighborhood( color_value_type ncolor )
        {
            for (Neighborhood_iterator_t
                    it = neighborhood.begin(); 
                    it != neighborhood.end();
                    it++ )
            {
                if ( (*it).second == ncolor )
                {
                    return true;
                }
            }
            return false;
        }

        color_value_type change_color()
        {
            uint16_t mycard = colors.cardinality(color);
            for (uint16_t i = 0; i < colors.size(); i++ )
            {
                if( (colors[i] != color)
                        && (!exist_color_in_neighborhood( colors[i] )) )
                {
                    if ( colors.cardinalityAt(i)  >= mycard )
                    {
                        return colors[i];
                    }
                    else
                    {
                        break;
                    }
                }
            }
            return 255;
        }

        void set_timer_in(millis_t future)
        {
            Timer().template set_timer<self_type, &self_type::timer_elapsed>(
                                   os(), future, this, 0 );
        }

        bool check_for_cycle_changes()
        {
            if ( colors.size() != old_colors.size() )
                return false;
            
            for (uint16_t i = 0; i < colors.size(); i++ )
            {
                color_value_type cl = colors[i];                
                if( colors.cardinality(cl) != old_colors.cardinality(cl) )
                {
                    return false;
                }
            }

            return true;
        }
/*
        inline int fm(int arr[], int b, int n)
        {
            int f = b;
            int c;

            for (c = b + 1; c < n; c++)
                if (arr[c] > arr[f])
                    f = c;

            return f;
        }

        inline void isort(int arr[], int n)
        {
            int s, w;
            int sm;

            for (s = 0; s < n - 1; s++) {
                w = fm(arr, s, n);
                sm = arr[w];
                arr[w] = arr[s];
                arr[s] = sm;
            }
        }*/

        inline void set_os(Os* os)
        { os_ = os; }

        inline Os* os()
        { return os_; }
        
        inline void set_judge()
        { is_judge = true; }
        
        inline bool completed()
        { return completed_; }
        
        inline uint16_t ncolors()
        {
            uint16_t i = 0;            
            for (; i < colors.size() && colors.cardinalityAt(i)!=0 ; i++ )
                ;

            return i;
        }

        inline bool judge()
        { return is_judge; }

        inline void set_color(color_value_type color_)
        { color = color_; }

        inline color_value_type get_color()
        { return color; }

        inline void set_tree_routing(routing_t *tr)
        { tree_routing = tr; }

    private:

      /** \brief Message IDs
      */
      enum IMJCMsgIds {
         IMJCMsgIdBroadcastCL = 110, ///< Msg type for broadcasting tree state
         IMJCMsgIdColorRQ   = 111,  ///< Msg type for routing messages to the gateway
         IMJCMsgIdColorRSP   = 112,  ///< Msg type for routing messages to every node in the subtree
         IMJCMsgIdColorCHG   = 113,  ///< Msg type for child response
         IMJCMsgIdColorRSPF  = 114
      };

      enum IMJCState {
          GatheringColors,
          ChangingColors
      };

        Radio& radio()
        { return *radio_; }

        Timer& timer()
        { return *timer_; }

        Debug& debug()
        { return *debug_; }

        Radio * radio_;
        Timer * timer_;
        Debug * debug_;

      Rand rand;
      uint32_t seed;
      Os *os_;
      Neighborhood_t neighborhood;
      uint8_t state;
      color_value_type color,old_color;
      color_table_t colors,old_colors;
      uint16_t my_id,colors_chages_in_round,node_count,msgs_received;
      old_new_color aggragated_values[MAX_NODES];
      old_new_color* aggragated_values_finish_;
      uint8_t children_;
      bool changed_color_in_the_prv_round,is_judge,color_changed,completed_,first_round;
      
    };
    // -----------------------------------------------------------------------
    // -----------Judged Coloring Constructor---------------------------------
    // -----------------------------------------------------------------------

    template<typename OsModel_P,
             typename Radio_P,
             typename Debug_P,
             typename Routing_P>
    IMJudgedColoring<OsModel_P, Radio_P, Debug_P, Routing_P>::
    IMJudgedColoring()
        : os_                  ( 0 ),
           state               ( GatheringColors ),
           color               ( 255 ),
           is_judge            ( false ),
           color_changed       ( false ),
           completed_          ( false ),
           first_round         ( true )
    {
        msg_info.IMJCMsgIdBroadcastCL = 0;
        msg_info.IMJCMsgIdColorRQ = 0;
        msg_info.IMJCMsgIdColorRSP = 0;
        msg_info.IMJCMsgIdColorCHG = 0;
        cycle_changes_detected = false;
        changed_color_in_the_prv_round = false;
        aggragated_values_finish_ = &(aggragated_values[0]);
        node_count = 0;
        colors_chages_in_round = 0;
    };
    // -----------Judged Coloring De-Constructor------------------------------

    template<typename OsModel_P,
             typename Radio_P,
             typename Debug_P,
             typename Routing_P>
    IMJudgedColoring<OsModel_P, Radio_P, Debug_P, Routing_P>::
    ~IMJudgedColoring()
    {
    };
    // -----------------------------------------------------------------------


#ifdef ENABLE_AGGREGATION
    template<typename OsModel_P,
             typename Radio_P,
             typename Debug_P,
             typename Routing_P>
    void
    IMJudgedColoring<OsModel_P, Radio_P, Debug_P, Routing_P>::
    check_rcd_fragments()
    {
        if( children_ == 0 )
        {
            if ( is_judge )
            {
                old_new_color *tmp = &(aggragated_values[0]);

                for ( ;tmp!=aggragated_values_finish_ ;tmp++ )
                {
                    colors.remove( tmp->first );
                    colors.insert( tmp->second );
                    colors_chages_in_round++;
                    color_changed = true;
                }

                if ( !color_changed )
                {
                    completed_ = true;
                }
                else
                {
                    if ( color_changed )
                    {
                        if ( check_for_cycle_changes() )
                        {
                            cycle_changes_detected = true;
                        }
                        old_colors.parse_array(colors.data(), colors.bytes());
                    }


                    coloring_message message( IMJCMsgIdColorRQ, 0 );
                    if( cycle_changes_detected )
                    {
                        message.set_color(1);
                        cycle_changes_detected = false;
                    }

                    message.set_payload( colors.bytes(), colors.data() );
                    msg_info.IMJCMsgIdColorRQ++;
                    tree_routing->send( Radio().BROADCAST_ADDRESS, message.buffer_size(), (uint8_t*)&message );

                    if ( change_color() != 255 && state == GatheringColors )
                    {
                        set_timer_in(1000);
//                        Timer().template set_timer<self_type, &self_type::timer_elapsed>(
//                                               os(), 2000, this, 0 );
                        state = ChangingColors;
                    }
                    else
                    {
                        children_--;
                    }
#ifdef INFO_IMJUDGEDCOLORING
//                Debug().debug(os(), "IMJC: Fr sending color vector (size: %i)\n" , message.buffer_size() );
                Debug().debug(os(), "IMJC Fr Nodes that changed color: %i colors: %i\n" , colors_chages_in_round, colors.size() );
#endif
//                Debug().debug(os(), "IMJC: Node %i want to change from %i to %i\n", my_id, color, change_color() );

                }
//                    new_round = true;
                colors_chages_in_round = 0;
                color_changed = false;
                msgs_received = 0;
                get_color_nodes();
            }
            else
            {
                msg_info.IMJCMsgIdColorRSP++;
                coloring_message message( IMJCMsgIdColorRSPF, color );
                message.set_payload((aggragated_values_finish_-&(aggragated_values[0]))*sizeof(old_new_color),(uint8_t*)&(aggragated_values[0]));
                tree_routing->send( 1, message.buffer_size(), (uint8_t*)&message );
            }

            children_ += tree_routing->get_children().size() + 1;
            aggragated_values_finish_ = &(aggragated_values[0]);
        }
    }
#endif


    template<typename OsModel_P,
             typename Radio_P,
             typename Debug_P,
             typename Routing_P>
    void
    IMJudgedColoring<OsModel_P, Radio_P, Debug_P, Routing_P>::
    rcv_routing_message( node_id_t from, size_t len, block_data_t* data)
    {
        if ( from == my_id )
            return;

#ifdef DEBUG_IMJUDGEDCOLORING
        Debug().debug(os(), "IMJC: Node %i Received message from %i with type %i len %i\n", my_id, from, *data, len);
#endif

        coloring_message *message = reinterpret_cast<coloring_message*>(data);
        
        if( *data == IMJCMsgIdColorRSP )
        {
            if( is_judge )
            {
                if ( message->source() != 255 )
                {
                    colors.remove( message->source() );
                }
                colors.insert( message->color() );

                if( message->source() != message->color() )
                {
                    colors_chages_in_round++;
                    color_changed = true;
                }
                
                msgs_received++;


#ifdef INFO_IMJUDGEDCOLORING
                Debug().debug(os(), "IMJC: Node %i has color %i (:%i,%i)\n", from, message->color(), msgs_received, node_count );
#endif
                //CHECK: do we need the tree check? add delay to tree completed
                if( ( ( node_count - 1 ) == msgs_received ) )
                {
                    if ( !color_changed )
                    {
                        completed_ = true;
                    }
                    else
                    {
                        //CHECK: do we need this? we can
                        if ( first_round )
                        {
                            old_colors.parse_array(colors.data(), colors.bytes());
                            first_round = false;
                        }
                        else if ( color_changed )
                        {
                            if ( check_for_cycle_changes() )
                            {
                                cycle_changes_detected = true;
                            }
                            old_colors.parse_array(colors.data(), colors.bytes());
                        }

                        coloring_message message( IMJCMsgIdColorRQ, 0 );
                        if( cycle_changes_detected )
                        {
                            message.set_color(1);
                            cycle_changes_detected = false;
                        }
                        message.set_payload( colors.bytes(), colors.data() );
                        tree_routing->send( Radio().BROADCAST_ADDRESS, message.buffer_size(), (uint8_t*)&message );
                        msg_info.IMJCMsgIdColorRQ++;

                        if ( change_color() != 255 && state == GatheringColors )
                        {
                            set_timer_in(2001);
//                            Timer().template set_timer<self_type, &self_type::timer_elapsed>(
  //                                                 os(), 2001, this, 0 );
                            state = ChangingColors;
                        }
                        else
                        {
                            children_--;
                        }
//                    Debug().debug(os(), "IMJC: Node %i want to change from %i to %i\n", my_id, msgs_received, node_count );

    #ifdef INFO_IMJUDGEDCOLORING
                        Debug().debug(os(), "IMJC: sending color vector (size: %i)\n" , message.buffer_size() );
                        Debug().debug(os(), "IMJC: Nodes that changed color: %i colors: %i\n" , colors_chages_in_round, colors.size() );
    #endif
                    }
                    colors_chages_in_round = 0;
                    color_changed = false;
                    msgs_received = 0;
//                    get_color_nodes();
               }
            }
        }
#ifdef ENABLE_AGGREGATION
        else if ( *data == IMJCMsgIdColorRSPF )
        {
            children_--;
            uint16_t elements = message->payload_size()/sizeof(old_new_color);
//TODO: remove
//            Debug().debug(os(), "The all mighty gw %i: from %i size: %i\n" ,children_,  from, message->payload_size() );
            if ( elements != 0 )
            {
                memcpy(aggragated_values_finish_,message->payload(),message->payload_size());
                aggragated_values_finish_ += elements;
            }
            check_rcd_fragments();
        }
#endif
        else if( *data == IMJCMsgIdColorRQ )
        {
            colors.parse_array( message->payload(), message->payload_size() );
            
            if ( change_color() != 255 && state == GatheringColors )
            {
                if ( (message->color() != 0) && changed_color_in_the_prv_round )
                {
                    seed += rand.rand(100);
                    rand.srand( seed );
                    uint32_t ran_num = rand.rand(2);
                    if ( ran_num == 0 )
                    {
                        state = ChangingColors;
                        set_timer_in(1000);
//                        Timer().template set_timer<self_type, &self_type::timer_elapsed>(
  //                                             os(), 1000, this, 0 );
                    }
                    else
                    {
#ifdef ENABLE_AGGREGATION
                        children_--;
                        check_rcd_fragments();
#else
                        coloring_message message( IMJCMsgIdColorRSP, color );
                        message.set_source( color );
                        msg_info.IMJCMsgIdColorRSP++;
                        tree_routing->send( 0, message.buffer_size(), (uint8_t*)&message );
#endif
                    }
                }
                else
                {
                    state = ChangingColors;

                        set_timer_in(1000);
//                    Timer().template set_timer<self_type, &self_type::timer_elapsed>(
//                                           os(), 1000, this, 0 );
                }
            }
            else
            {
#ifdef ENABLE_AGGREGATION
                children_--;
                check_rcd_fragments();
#else
                coloring_message message( IMJCMsgIdColorRSP, color );
                message.set_source( color );
                msg_info.IMJCMsgIdColorRSP++;
                tree_routing->send( 0, message.buffer_size(), (uint8_t*)&message );
#endif
            }
            
            changed_color_in_the_prv_round = false;
#ifdef DEBUG_IMJUDGEDCOLORING
            if ( change_color() != 255 )
                Debug().debug(os(), "GatheringColors IMJC: Node %i want to change from %i to %i\n", my_id, color, change_color() );
#endif
        }
#ifdef DEBUG_IMJUDGEDCOLORING
        else
        {
            Debug().debug(os(), "IMJC: Node %i received msg of Unknown type [:= %i] \n", my_id, *data);
        }
#endif

    }
    // -----------------------------------------------------------------------

    template<typename OsModel_P,
             typename Radio_P,
             typename Debug_P,
             typename Routing_P>
    void
    IMJudgedColoring<OsModel_P, Radio_P, Debug_P, Routing_P>::
    enable( void )
    {
        Radio().enable( os() );
        Radio().template reg_recv_callback<self_type, &self_type::receive>( os(), this );

        tree_routing->set_os( os() );
        tree_routing->template reg_recv_callback<self_type, &self_type::rcv_routing_message>( this );
        tree_routing->template reg_state_callback<self_type, &self_type::tree_state_chage>( this );

        colors.clear();
        seed = my_id = color = Radio().id( os() );
        old_color = 255;

        if ( is_judge )
        {
            tree_routing->set_sink( true );
            colors.insert( color );
            old_colors.clear();
            Timer().template set_timer<self_type, &self_type::neighborhood_timeout>(
                                       os(), 200, this, 0 );
        }
        else
        {
            Timer().template set_timer<self_type, &self_type::neighborhood_timeout>(
                                       os(), 100, this, 0 );
        }
        
#ifdef INFO_IMJUDGEDCOLORING
        Debug().debug(os(), "IMJC: Enabled - starting on node %i with color %i\n", my_id, color );
#endif
    }
    // -----------------------------------------------------------------------

    template<typename OsModel_P,
             typename Radio_P,
             typename Debug_P,
             typename Routing_P>
    void
    IMJudgedColoring<OsModel_P, Radio_P, Debug_P, Routing_P>::
    neighborhood_timeout(void *userdata)
    {

        coloring_message message( IMJCMsgIdBroadcastCL, color );
        Radio().send( os(), Radio().BROADCAST_ADDRESS, message.buffer_size(), (uint8_t*)&message );
        msg_info.IMJCMsgIdBroadcastCL++;

        tree_routing->enable();
    }
    // -----------------------------------------------------------------------

    template<typename OsModel_P,
             typename Radio_P,
             typename Debug_P,
             typename Routing_P>
    void
    IMJudgedColoring<OsModel_P, Radio_P, Debug_P, Routing_P>::
    disable(void)
    {
#ifdef DEBUG_IMJUDGEDCOLORING
        Debug().debug(os(), "IMJC: Disabled\n");
#endif       
    }
    // -----------------------------------------------------------------------

    template<typename OsModel_P,
             typename Radio_P,
             typename Debug_P,
             typename Routing_P>
    void
    IMJudgedColoring<OsModel_P, Radio_P, Debug_P, Routing_P>::
    timer_elapsed(void *userdata)
    {
#ifdef DEBUG_IMJUDGEDCOLORING
        Debug().debug(os(), "IMJC: timer_elapsed\n");
#endif

        if ( change_color() != 255 && state == ChangingColors )
        {
            msg_info.IMJCMsgIdColorCHG++;
            coloring_message message( IMJCMsgIdColorCHG, color );
            message.set_source(change_color());
            Radio().send( os(), Radio().BROADCAST_ADDRESS, message.buffer_size(), (uint8_t*)&message );
            Timer().template set_timer<self_type, &self_type::color_timeout>(
                                       os(), 2000, this, 0 );
#ifdef DEBUG_IMJUDGEDCOLORING
            Debug().debug(os(), "IMJC: %i: not changing color\n", my_id );
#endif
        }
        else
        {
#ifdef ENABLE_AGGREGATION
            children_--;
            check_rcd_fragments();
#else
            if ( !is_judge )
            {
                msg_info.IMJCMsgIdColorRSP++;
                coloring_message message( IMJCMsgIdColorRSP, color );
                message.set_source( color );
                tree_routing->send( 0, message.buffer_size(), (uint8_t*)&message );
            }
#endif

#ifdef DEBUG_IMJUDGEDCOLORING
            Debug().debug(os(), "IMJC: %i: not changing color\n", my_id );
#endif
        }
    }
    // -----------------------------------------------------------------------

    template<typename OsModel_P,
             typename Radio_P,
             typename Debug_P,
             typename Routing_P>
    void
    IMJudgedColoring<OsModel_P, Radio_P, Debug_P, Routing_P>::
    color_timeout(void *userdata)
    {
#ifdef DEBUG_IMJUDGEDCOLORING
        Debug().debug(os(), "IMJC: color_timeout\n");
#endif

        if ( change_color() != 255 && state == ChangingColors )
        {
            state = GatheringColors;
#ifdef DEBUG_IMJUDGEDCOLORING
            Debug().debug(os(), "IMJC: %i: changing color from %i to %i\n", my_id, color, change_color());
#endif

#ifdef ENABLE_AGGREGATION
            aggragated_values_finish_->first = color;
            aggragated_values_finish_->second = change_color();
            aggragated_values_finish_++;
            children_--;
            check_rcd_fragments();
#else
            if ( !is_judge )
            {
                msg_info.IMJCMsgIdColorRSP++;
                coloring_message message2( IMJCMsgIdColorRSP, change_color() );
                message2.set_source( color );
                tree_routing->send( 0, message2.buffer_size(), (uint8_t*)&message2 );
            }
            else
            {
                colors.remove( color );
                colors.insert( change_color() );
            }
#endif

            changed_color_in_the_prv_round = true;
            old_color = color;
            color = change_color();
            coloring_message message( IMJCMsgIdBroadcastCL, color );
            Radio().send( os(), Radio().BROADCAST_ADDRESS, message.buffer_size(), (uint8_t*)&message );
            msg_info.IMJCMsgIdBroadcastCL++;
        }
        else
        {

#ifdef ENABLE_AGGREGATION
            children_--;
            check_rcd_fragments();
#else
            if ( !is_judge )
            {
                msg_info.IMJCMsgIdColorRSP++;
                coloring_message message( IMJCMsgIdColorRSP, color );
                message.set_source( color );
                tree_routing->send( 0, message.buffer_size(), (uint8_t*)&message );
            }
#endif
            
#ifdef DEBUG_IMJUDGEDCOLORING
            Debug().debug(os(), "IMJC: %i: not changing color\n", my_id );
#endif
        }
    }

    // -----------------------------------------------------------------------

    template<typename OsModel_P,
             typename Radio_P,
             typename Debug_P,
             typename Routing_P>
    void
    IMJudgedColoring<OsModel_P, Radio_P, Debug_P, Routing_P>::
    tree_state_chage(uint8_t state)
    {
        switch ( state )
        {
            case routing_t::TrNodeConnected :
            {

#ifdef INFO_IMJUDGEDCOLORING
                Debug().debug(os(), "IMJC: Node %i Tree state changed: TrNodeConnected tree size: %i\n",
                my_id, tree_routing->get_tree_size() );
                
                children_t children = tree_routing->get_children();
                for (children_iterator_t it = children.begin(); it != children.end(); it++)
                {
                    Debug().debug(os(), "  child: %i \n",(node_id_t)*it);
                }
#endif                
                node_count = tree_routing->get_tree_size();
                children_ = tree_routing->get_children().size() + 1;

                if( !is_judge )
                {
                    coloring_message message( IMJCMsgIdColorRSP, color );
                    message.set_source(255);
                    tree_routing->send( 0, message.buffer_size(), (uint8_t*)&message );
                    msg_info.IMJCMsgIdColorRSP++;
                }
                break;
            }
            case routing_t::TrLeafConnected :
            {
#ifdef DEBUG_IMJUDGEDCOLORING
                Debug().debug(os(), "IMJC: Node %i Tree state changed: TrLeafConnected\n",my_id);
#endif

                node_count = 0;
                children_ = 1;
                
                coloring_message message( IMJCMsgIdColorRSP, color );
                message.set_source(255);
                tree_routing->send( 0, message.buffer_size(), (uint8_t*)&message );
                msg_info.IMJCMsgIdColorRSP++;
                break;
            }
#ifdef DEBUG_IMJUDGEDCOLORING
            default :
                Debug().debug(os(), "IMJC: Node %i Tree state changed: Error\n",my_id);
#endif

        }

    }

    // -----------------------------------------------------------------------
    
    template<typename OsModel_P,
             typename Radio_P,
             typename Debug_P,
             typename Routing_P>
    void
    IMJudgedColoring<OsModel_P, Radio_P, Debug_P, Routing_P>::
    receive(node_id_t from, size_t len, block_data_t *data)
    {
        
        if ( from == my_id )
            return;

        uint8_t msg_id = *data;
        coloring_message *message = reinterpret_cast<coloring_message*>(data);;

        if ( IMJCMsgIdColorCHG == msg_id )
        {
            if ( state == ChangingColors )
            {
                if ( from > my_id )
                {
                        state = GatheringColors;
                }
            }
        }
        else if ( IMJCMsgIdBroadcastCL == msg_id )
        {
            //CHECK: do we need this?
            state = GatheringColors;
            message = reinterpret_cast<coloring_message*>(data);

            Neighborhood_iterator_t it = neighborhood.begin();
            for ( ; it != neighborhood.end() ; it++ )
            {
                if ( (*it).first == from )
                {
                    (*it).second = message->color();
                    break;
                }
            }
            if ( it == neighborhood.end() )
            {
                neighborhood.push_back( node_color( from, message->color() ) );
            }

#ifdef INFO_IMJUDGEDCOLORING
            Debug().debug(os(), "IMJC: %i: received color %i from %i\n", my_id, message->color(), from );
#endif

        }
        
    }
    // -----------------------------------------------------------------------
    
    template<typename OsModel_P,
             typename Radio_P,
             typename Debug_P,
             typename Routing_P>
    const uint16_t //FIX: change type to uint8_t
    IMJudgedColoring<OsModel_P, Radio_P, Debug_P, Routing_P>::
    get_neighboors()
    {

        for (Neighborhood_iterator_t it = neighborhood.begin(); it != neighborhood.end(); it++ )
        {
            Debug().debug(os(), "IMJC: %i: received color %i from neigbor %i \n", my_id, (*it).first, (*it).second );
        }
        return neighborhood.size();
    }
    // -----------------------------------------------------------------------

    template<typename OsModel_P,
             typename Radio_P,
             typename Debug_P,
             typename Routing_P>
    uint16_t
    IMJudgedColoring<OsModel_P, Radio_P, Debug_P, Routing_P>::
    get_color_nodes()
    {

#ifdef INFO_IMJUDGEDCOLORING
        Debug().debug(os(), "IMJC: %i: My color vector has %i elements:\n", my_id, colors.size() );

        for (uint16_t i = 0; i < colors.size(); i++ )
            Debug().debug(os(), "IMJC: Color %i has Count %i\n", colors[i], colors.cardinalityAt(i) );
#endif

        return colors.size();
    }
    
}
#endif
