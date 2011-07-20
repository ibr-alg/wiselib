/************************************************************************
 ** This file is part of the network simulator Shawn.                  **
 ** Copyright (C) 2004-2007 by the SwarmNet (www.swarmnet.de) project  **
 ** Shawn is free software; you can redistribute it and/or modify it   **
 ** under the terms of the BSD License. Refer to the shawn-licence.txt **
 ** file in the root of the Shawn source tree for further details.     **
 ************************************************************************/
#include "_legacyapps_enable_cmake.h"
#ifdef ENABLE_WISELIB_EXAMPLES

#include "legacyapps/wiselib_examples/localization_components_processor.h"
#include "sys/simulation/simulation_controller.h"
#include "sys/simulation/simulation_environment.h"
#include "sys/node.h"
#include "sys/taggings/basic_tags.h"
#include "sys/tag.h"

namespace wiselib_examples
{

   LocalizationComponentsProcessor::
   LocalizationComponentsProcessor()
      : timer_duration_ ( 1000 ),
         timer_started_ ( false ),
         event_counter_ ( 0 ),
         dms_threshold_ ( 7 ),
         actor_duration_( 3000 )
   {}
   // ----------------------------------------------------------------------
   LocalizationComponentsProcessor::
   ~LocalizationComponentsProcessor()
   {}
   // ----------------------------------------------------------------------
   void
   LocalizationComponentsProcessor::
   boot( void )
      throw()
   {
      INFO( logger(), "TubsHallwayDebug::booting" );

//       os_.proc = this;
//       localization_base_.set_os(&os_);
//       localization_base_.enable();

//       Os::Radio::enable( &os_ );
//       Os::Radio::reg_recv_callback<LocalizationComponentsProcessor, &LocalizationComponentsProcessor::receive_message>( &os_, this );

//       const SimulationEnvironment& se = owner().world().simulation_controller().environment();
//
//       dist_algo_ = DistanceAlgorithm(
//          get_index(
//             se.optional_string_param( "loc_dist_algo", "" ),
//             DIST_ALGOS,
//             sizeof( DIST_ALGOS ) / sizeof( DIST_ALGOS[0] ),
//             int( dist_algo_ ) ) );
//       pos_algo_ = PositionAlgorithm(
//          get_index(
//             se.optional_string_param( "loc_pos_algo", "" ),
//             POS_ALGOS,
//             sizeof( POS_ALGOS ) / sizeof( POS_ALGOS[0] ),
//             int( pos_algo_ ) ) );
//       ref_algo_ = RefinementAlgorithm(
//          get_index(
//             se.optional_string_param( "loc_ref_algo", "" ),
//             REF_ALGOS,
//             sizeof( REF_ALGOS ) / sizeof( REF_ALGOS[0] ),
//             int( ref_algo_ ) ) );
// 
//       std::string dist_est_name = se.required_string_param( "loc_est_dist" );
//       dist_est_ = owner().world().simulation_controller().
//                      distance_estimate_keeper().find( dist_est_name );
//       assert( dist_est_ != NULL );
// 
//       check_residue_ = se.optional_bool_param( "loc_check_residue", check_residue_ );
//       floodlimit_ = se.optional_int_param( "loc_floodlimit", floodlimit_ );
//       idle_time_ = se.optional_int_param( "loc_idle_time", idle_time_ );
//       idle_shutdown_time_ = se.optional_int_param( "loc_idle_shutdown_time", idle_shutdown_time_ );
// 
//       startup_anchor_frac_ = se.optional_double_param( "loc_startup_anchor_frac", startup_anchor_frac_ );
// 
//       rollback_period_ = se.optional_int_param( "loc_rollback_period", rollback_period_ );
//       rollback_limit_ = se.optional_int_param( "loc_rollback_limit", rollback_limit_ );
// 
//       if ( owner().world().communication_model().exists_communication_upper_bound() )
//          comm_range_ = owner().world().communication_model().communication_upper_bound();
// 
//       assert( startup_anchor_frac_ >= 0 && startup_anchor_frac_ <= 1 );
//       assert( floodlimit_ >= 0 && floodlimit_ <= std::numeric_limits<unsigned int>::max() );
//       assert( idle_time_ >= 0 && idle_time_ <= std::numeric_limits<int>::max() );
//       assert( rollback_period_ >= 0 && rollback_period_ <= std::numeric_limits<int>::max() );
//       assert( rollback_limit_ >= 0 && rollback_limit_ <= std::numeric_limits<int>::max() );



// init_proc_type( void )
//       throw()
//    {
//       if ( startup_anchor_frac_ > 0 && startup_anchor_frac_ > *random_ )
//       {
//          DEBUG( logger(), "set " << owner().label() << " as anchor on startup" );
//          set_proc_type( anchor );
//       }
//    //  if( owner().is_special_node())
//    //   set_proc_type( server);
// 
//       switch ( proc_type() )
//       {
//          case anchor:
//          {
//             confidence_ = 1;
//          double position_error= owner().world().simulation_controller().environment().optional_double_param("anchor_pos_err",-1);
//          if(position_error>0.0){
//             /*shawn::UniformRandomVariable* urv = new shawn::UniformRandomVariable();
//             urv->set_upper_bound(2*position_error);
//             urv->set_upper_bound_inclusive(false);
//             urv->set_lower_bound_inclusive(false);
//             urv->init();
//             double dx = (*urv)-position_error;
//             double dy = (*urv) - position_error;
//             double dz =(*urv) - position_error;
//             */
// 
//             shawn::UniformRandomVariable urv;
//             urv.set_upper_bound(2*position_error);
//             urv.set_upper_bound_inclusive(false);
//             urv.set_lower_bound_inclusive(false);
//             urv.init();
//             double dx = (urv)-position_error;
//             double dy = (urv) - position_error;
//             double dz =(urv) - position_error;
// 
//             Vec tmp=owner().real_position();
//             if(tmp.z()==0)
//                dz=0;
//             Vec pos(tmp.x()+dx, tmp.y()+dy,tmp.z()+dz);
//             owner_w().set_est_position(pos);
//          }
//          else
//             owner_w().set_est_position( owner().real_position() );
//          }
// 
//          case unknown:
//          {
//             confidence_ = 0.1;
//          }

   }
   // ----------------------------------------------------------------------
   void
   LocalizationComponentsProcessor::
   receive_message( int src_addr, size_t len, unsigned char *buf )
      throw()
   {}
   // ----------------------------------------------------------------------
   void
   LocalizationComponentsProcessor::
   event_evaluation( void* userdata )
      throw()
   {}
   // ----------------------------------------------------------------------
   void
   LocalizationComponentsProcessor::
   actor_off( void* userdata )
      throw()
   {}
}

#endif
