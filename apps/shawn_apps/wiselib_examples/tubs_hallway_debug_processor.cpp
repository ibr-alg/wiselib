/************************************************************************
 ** This file is part of the network simulator Shawn.                  **
 ** Copyright (C) 2004-2007 by the SwarmNet (www.swarmnet.de) project  **
 ** Shawn is free software; you can redistribute it and/or modify it   **
 ** under the terms of the BSD License. Refer to the shawn-licence.txt **
 ** file in the root of the Shawn source tree for further details.     **
 ************************************************************************/
#include "_legacyapps_enable_cmake.h"
#ifdef ENABLE_WISELIB_EXAMPLES

#include "legacyapps/wiselib_examples/tubs_hallway_debug_processor.h"
#include "sys/simulation/simulation_controller.h"
#include "sys/simulation/simulation_environment.h"
#include "sys/node.h"
#include "sys/taggings/basic_tags.h"
#include "sys/tag.h"


namespace wiselib_examples
{

   TubsHallwayDebugProcessor::
   TubsHallwayDebugProcessor()
      : timer_duration_ ( 1000 ),
         timer_started_ ( false ),
         event_counter_ ( 0 ),
         dms_threshold_ ( 7 ),
         actor_duration_( 1000 ),
         wiselib_radio_( os_ ),
         wiselib_timer_( os_ ),
         wiselib_clock_( os_ ),
         wiselib_debug_( os_ )
   {}
   // ----------------------------------------------------------------------
   TubsHallwayDebugProcessor::
   ~TubsHallwayDebugProcessor()
   {}
   // ----------------------------------------------------------------------
   void
   TubsHallwayDebugProcessor::
   boot( void )
      throw()
   {
      INFO( logger(), "TubsHallwayDebug::booting" );
      INFO( logger(), "TubsHallwayDebug::pstate" << state() );
      INFO( logger(), "TubsHallwayDebug::nstate" << owner().state() );

      os_.proc = this;
      wiselib_radio_.enable_radio();
      wiselib_radio_.reg_recv_callback<TubsHallwayDebugProcessor,
         &TubsHallwayDebugProcessor::receive_message>( this );

      const shawn::SimulationEnvironment& se = owner().world().
                                       simulation_controller().environment();
      timer_duration_ = se.optional_int_param( "timer_duration", timer_duration_ );
      dms_threshold_ = se.optional_int_param( "dms_threshold", dms_threshold_ );
      actor_duration_ = se.optional_int_param( "actor_duration", actor_duration_ );

      owner_w().write_simple_tag<double>( "event", 0.0 );

// TODO
//       set_node( owner_w() );
//       testbedservice_proc_boot();
   }
   // ----------------------------------------------------------------------
   void
   TubsHallwayDebugProcessor::
   receive_message( int src_addr, size_t len, unsigned char *buf )
      throw()
   {
//       INFO( logger(), "TubsHallwayDebug::received message." );
      if ( buf[0] == 142 && len != 7 )
      {
         ERROR( logger(), "  Message of wrong length :(" );
         return;
      }

      uint16_t sensor_id = (buf[1] << 8) | buf[2];
      uint16_t short_term = (buf[3] << 8) | buf[4];
      uint16_t long_term = (buf[5] << 8) | buf[6];

      INFO( logger(), "  Got sensor id " << sensor_id
               << " from  " << src_addr
               << " with " << short_term
               << " and " << long_term
               << " at " << owner().id() );
// std::string value = "2";
// send_text_message( value, testbedservice::MESSAGE_LEVEL_TRACE );

      if ( long_term - short_term > dms_threshold_ && !timer_started_ )
      {
         wiselib_timer_.set_timer<TubsHallwayDebugProcessor,
                              &TubsHallwayDebugProcessor::event_evaluation>(
            timer_duration_, this, 0 );
         timer_started_ = true;
      }

      if ( long_term - short_term > dms_threshold_ )
      {
         event_counter_++;
      }
   }
   // ----------------------------------------------------------------------
   void
   TubsHallwayDebugProcessor::
   event_evaluation( void* userdata )
      throw()
   {
      if ( event_counter_ >= 7 )
      {
         INFO( logger(), owner().id() << " COUNTED " << event_counter_ << " EVENTS." );
         std::string value = "1";
// TODO:
//          send_text_message( value, testbedservice::MESSAGE_LEVEL_TRACE );

         owner_w().write_simple_tag<double>( "event", 1.0 );

         wiselib_timer_.set_timer<TubsHallwayDebugProcessor,
                              &TubsHallwayDebugProcessor::actor_off>(
            actor_duration_, this, 0 );

//          uint8_t buffer = 143;
//          wiselib_radio_.send( Os::Radio::BROADCAST_ADDRESS, 1, &buffer );
      }

      timer_started_ = false;
      event_counter_ = 0;
   }
   // ----------------------------------------------------------------------
   void
   TubsHallwayDebugProcessor::
   actor_off( void* userdata )
      throw()
   {
      owner_w().write_simple_tag<double>( "event", 0.0 );
      INFO( logger(), "Actor off." );
      std::string value = "0";
// TODO
//       send_text_message( value, testbedservice::MESSAGE_LEVEL_TRACE );
   }
}
#endif
