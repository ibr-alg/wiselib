/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2007 Georgia Tech Research Corporation
 * Copyright (c) 2010 Adrian Sai-wah Tam
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation;
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 * Author: Dizhi Zhou (dizhi.zhou@gmail.com)
 */

#include <iostream>
#include "external_interface/ns3/ns3_os.h"
#include "external_interface/ns3/ns3_debug.h"
#include "external_interface/ns3/ns3_timer.h"
#include "external_interface/ns3/ns3_radio.h"
#include "external_interface/ns3/ns3_clock.h"
#include "external_interface/ns3/ns3_position.h"
#include "external_interface/ns3/ns3_distance.h"
#include "external_interface/ns3/ns3_rand.h"
#include "external_interface/ns3/ns3_facet_provider.h"
#include "external_interface/ns3/ns3_debug_com_uart.h"
#include "external_interface/ns3/ns3_wiselib_application.h"
#include "algorithms/localization/distance_based/math/vec.h"

#include "ns3/simulator.h"
#include "ns3/ptr.h"
#include "ns3/wiselib-ext-iface.h"

using namespace wiselib;

typedef Ns3OsModel Os;
#define MAX_NODES 3
#define ENABLE_EXTDATA_RADIO false
#define ENABLE_TXPOWER_RADIO false

class Ns3ExampleApplication
{
  public:

    void init (Os::AppMainParameter& value) 
      {  
        debug_ = &wiselib::FacetProvider<Os, Os::Debug>::get_facet( value );
        timer_ = &wiselib::FacetProvider<Os, Os::Timer>::get_facet( value );
        clock_ = &wiselib::FacetProvider<Os, Os::Clock>::get_facet( value );
        rand_ = &wiselib::FacetProvider<Os, Os::Rand>::get_facet( value );

        // each node has one radio instance
        // sender: radio[0]    receiver: radio[1], radio[2]
        double pos_x = 0;
        double pos_y = 0;
        double pos_z = 0;
        for (uint16_t i = 0; i < MAX_NODES; i++)
          {


            radio[i] = &wiselib::FacetProvider<Os, Os::Radio>::get_facet( value );
            radio[i]->enable_radio ();

            if ( ENABLE_TXPOWER_RADIO )
              {
                /* tx power radio facet test */
                debug_->debug ("\n%f: TxPowerRadio facet test", clock_->time () );\
                TxPowerClass txpower;
                txpower.SetTxPowerStart (17);
                txpower.SetTxPowerEnd (17);
                radio[i]->set_power (txpower);
                TxPowerClass txpower1;
                txpower1 = radio[i]->power ();
                debug_->debug("Start %f , end %f ",txpower1.GetTxPowerStart (), txpower1.GetTxPowerEnd () );
              }


            // use different receive callbacks for receiver nodes
            if ( i == 1)
              {
                if ( !ENABLE_EXTDATA_RADIO )
                  radio[i]->reg_recv_callback <Ns3ExampleApplication,
                            &Ns3ExampleApplication::receive_radio1_message>(this);
                else
                  radio[i]->reg_recv_callback <Ns3ExampleApplication,
                            &Ns3ExampleApplication::receive_extdata_radio1_message>(this);
              }
            else if ( i == 2)
             {
                if ( !ENABLE_EXTDATA_RADIO )
                  radio[i]->reg_recv_callback <Ns3ExampleApplication,
                            &Ns3ExampleApplication::receive_radio2_message>(this);
                else
                  radio[i]->reg_recv_callback <Ns3ExampleApplication,
                            &Ns3ExampleApplication::receive_extdata_radio2_message>(this);
             }

            position[i] = &wiselib::FacetProvider<Os, Os::Position>::get_facet( value );
            position[i]->bind (radio[i]->id ()); // bind position facet and radio facet on the same node
            position[i]->set_position (pos_x,pos_y,pos_z, radio[i]->id ()); // position can only be setted after enable_radio () operation

            distance[i] = &wiselib::FacetProvider<Os, Os::Distance>::get_facet( value );
            distance[i]->bind (radio[i]->id ()); // bind position facet and radio facet on the same node

            pos_x += 10;
            pos_y += 10;
            pos_z += 10;

            debugComUart[i] = &wiselib::FacetProvider<Os, Os::DebugComUart>::get_facet( value );
            debugComUart[i]->enable_serial_comm();
            debugComUart[i]->init (*debug_);
          }

        debug_->debug( "%f: Init simulation", clock_->time ());        

        timer_->set_timer<Ns3ExampleApplication,
                        &Ns3ExampleApplication::start_radio_facet>( 5000, this, 0 );

        timer_->set_timer<Ns3ExampleApplication,    
                  &Ns3ExampleApplication::start_clock_facet>( 6543.2, this, 0 );

        timer_->set_timer<Ns3ExampleApplication,
                        &Ns3ExampleApplication::start_distance_position_facet>( 7000, this, 0 );

        timer_->set_timer<Ns3ExampleApplication,
                        &Ns3ExampleApplication::start_rand_facet>( 8000, this, 0 );

        timer_->set_timer<Ns3ExampleApplication,
                        &Ns3ExampleApplication::start_serial_comm_facet>( 9000, this, 0 );

        if  ( ENABLE_EXTDATA_RADIO )
          timer_->set_timer<Ns3ExampleApplication,
                          &Ns3ExampleApplication::start_extdata_radio_facet>( 10000, this, 0 );

      };


    void start_radio_facet (void*)
    {
       debug_->debug ("\n%f: Radio facet test", clock_->time () );
       debug_->debug( "  %f: Broadcast message at node %d", clock_->time (), radio[0]->id() );
       Os::Radio::block_data_t message1[] = "hello world broadcast!\0";
               radio[0]->send( Os::Radio::BROADCAST_ADDRESS, sizeof(message1), message1 );

       debug_->debug( "  %f: Send unicast message at node %d to %d",clock_->time (), radio[0]->id(), radio[1]->id() );
       Os::Radio::block_data_t message2[] = "hello world unicast!\0";
               radio[0]->send( radio[1]->id(), sizeof(message2), message2 );
    }

    void receive_radio1_message( Os::Radio::node_id_t from, Os::Radio::size_t len, Os::Radio::block_data_t *buf )
      {
         debug_->debug( "  %f: Received msg1 at %u from %u",clock_->time (), radio[1]->id(), from );
         debug_->debug( "    message is %s", buf );
      }

    void receive_radio2_message( Os::Radio::node_id_t from, Os::Radio::size_t len, Os::Radio::block_data_t *buf )
      {
         debug_->debug( "  %f: received msg2 at %u from %u",clock_->time (),  radio[2]->id(), from );
         debug_->debug( "    message is %s", buf );
      }

    void start_clock_facet (void*)
      {
         debug_->debug ("\n%f: Clock facet test", clock_->time () );
         debug_->debug( "  %d seconds since second %f", clock_->seconds (clock_->time ()), 0.0);
         debug_->debug( "  %d milliseconds from last second %d", clock_->milliseconds (clock_->time ()), 
                                                            (uint16_t)(clock_->time ()));
         debug_->debug( "  %d microseconds from last millisecond %d", clock_->microseconds (clock_->time () * 1000),
                                                            (uint16_t)(clock_->time () * 1000));
      }

    void start_distance_position_facet (void*)
      {
        debug_->debug ("\n%f: Positioan and distance facet test", clock_->time () );

        for (uint16_t i = 0; i < MAX_NODES; i++)
          {
            debug_->debug ("  Set node %d at position ( %f, %f, %f )", 
                                  radio[i]->id (), 
                                  (*position[i])().x(), 
                                  (*position[i])().y(),
                                  (*position[i])().z());
            debug_->debug ("    Distance from %d to %d is %f", radio[i]->id (), radio[0]->id (), 
                                                      (*distance[i])(radio[0]->id ()) );
          }
      }

    void start_rand_facet (void*)
      {
         debug_->debug ("\n%f: Rand facet test", clock_->time () );
         debug_->debug( "  Generate random number (0,100): %d", (*rand_)(100));
      }

    void start_serial_comm_facet (void*)
    {
       debug_->debug ("\n%f: Serial communication facet test", clock_->time () );
        for (uint16_t i = 0; i < MAX_NODES; i++)
          {
            Os::DebugComUart::block_data_t message[] = "test\0";
            debugComUart[i]->write (sizeof(message), message);
            int idx = debugComUart[i]->reg_read_callback <Ns3ExampleApplication,
                         &Ns3ExampleApplication::read_message>(this);
            debugComUart[i]->receive (sizeof(message), message);
            
            // re-register read callback for node 2
            if (i == 2)
              {
                debugComUart[i]->unreg_read_callback (idx);
                debugComUart[i]->reg_read_callback <Ns3ExampleApplication,
                         &Ns3ExampleApplication::read_message1>(this);
              }
  
            debugComUart[i]->receive (sizeof(message), message);
          }
    }

    void read_message (Os::DebugComUart::size_t size, Os::DebugComUart::block_data_t* data)
    {
      debug_->debug( "    receive message: %s", data );
    }

    void read_message1 (Os::DebugComUart::size_t size, Os::DebugComUart::block_data_t* data)
    {
      debug_->debug( "    receive re-registered message: %s", data );
    }

    void start_extdata_radio_facet (void*)
    {
       debug_->debug ("\n%f: Extended data Radio facet test", clock_->time () );
       debug_->debug( "  %f: Broadcast message at node %d", clock_->time (), radio[0]->id() );
       Os::Radio::block_data_t message1[] = "hello world broadcast!\0";
               radio[0]->send( Os::Radio::BROADCAST_ADDRESS, sizeof(message1), message1 );

       debug_->debug( "  %f: Send unicast message at node %d to %d",clock_->time (), radio[0]->id(), 
                    radio[1]->id() );
       Os::Radio::block_data_t message2[] = "hello world unicast!\0";
               radio[0]->send( radio[1]->id(), sizeof(message2), message2 );
      
    }

    void receive_extdata_radio1_message( Os::Radio::node_id_t from, Os::Radio::size_t len, 
            Os::Radio::block_data_t *buf, ExtendedDataClass *extdata )
      {
         debug_->debug( "  %f: Received msg1 at %u from %u",clock_->time (), radio[1]->id(), from );
         debug_->debug( "    message is %s", buf );
         debug_->debug( "    RSS is %f", extdata->GetRss () );
      }

    void receive_extdata_radio2_message( Os::Radio::node_id_t from, Os::Radio::size_t len, 
           Os::Radio::block_data_t *buf, ExtendedDataClass *extdata )
      {
         debug_->debug( "  %f: received msg2 at %u from %u",clock_->time (),  radio[2]->id(), from );
         debug_->debug( "    message is %s", buf );
         debug_->debug( "    RSS is %f", extdata->GetRss () );
      }



  private:
    Os::Debug::self_pointer_t debug_;
    Os::Timer::self_pointer_t timer_;
    Os::Radio::self_pointer_t radio[MAX_NODES];
    Os::Clock::self_pointer_t clock_;
    Os::Position::self_pointer_t position[MAX_NODES];
    Os::Distance::self_pointer_t distance[MAX_NODES];
    Os::Rand::self_pointer_t rand_;
    Os::DebugComUart::self_pointer_t debugComUart[MAX_NODES];

};

wiselib::WiselibApplication<Os, Ns3ExampleApplication> example_app;

int main(int argc, char *argv[] )
{
   std::cout << std::endl; 

   Os::AppMainParameter value;  // the external interface instance

   example_app.init (value); // configure Wiselib script

   ns3::Simulator::Run ();   // start NS-3 simulation

   std::cout << std::endl;
}







