/************************************************************************
 ** This file is part of the network simulator Shawn.                  **
 ** Copyright (C) 2004,2005 by  SwarmNet (www.swarmnet.de)             **
 **                         and SWARMS   (www.swarms.de)               **
 ** Shawn is free software; you can redistribute it and/or modify it   **
 ** under the terms of the GNU General Public License, version 2.      **
 ************************************************************************/
#include "../buildfiles/_legacyapps_enable_cmake.h"
#ifdef ENABLE_DUTY_CYCLING

#include "legacyapps/duty_cycling/duty_cycling_task.h"
#include "legacyapps/duty_cycling/duty_cycling_processor.h"
#include "legacyapps/duty_cycling/wiselib_duty_cycling_processor.h"
#include "sys/communication_model.h"
#include "sys/edge_model.h"
#include "sys/world.h"
#include <fstream>
#include <limits>
#include <sstream>


namespace duty_cycling
{

   DutyCyclingTask::
   DutyCyclingTask()
   {}
   // ----------------------------------------------------------------------
   DutyCyclingTask::
   ~DutyCyclingTask()
   {}
   // ----------------------------------------------------------------------
   std::string
   DutyCyclingTask::
   name( void )
      const throw()
   {
      return "duty_evaluation";
   }
   // ----------------------------------------------------------------------
   std::string
   DutyCyclingTask::
   description( void )
      const throw()
   {
      return "...";
   }
   // ----------------------------------------------------------------------
   void
   DutyCyclingTask::
   run( shawn::SimulationController& sc )
      throw( std::runtime_error )
   {
      require_world( sc );

      double activity = 0.0;
      double battery = 0.0;
      double sun = 0.0;
      int active = 0, inactive = 0;
      int count = 0;

      double e_idle = 0.0;
      double e_active = 0.0;
      double e_tx = 0.0;
      double e_rx = 0.0;
      double e_app = 0.0;

      for( shawn::World::node_iterator
               it = sc.world_w().begin_nodes_w();
               it != sc.world_w().end_nodes_w();
               ++it )
      {
//          DutyCyclingProcessor* lp =
//             (*it).get_processor_of_type_w<DutyCyclingProcessor>();
//          if ( lp == NULL ) continue;
// 
//          activity += lp->S_;
//          battery += lp->battery_;
//          if (lp->active_)
//             active++;
//          else
//             inactive++;
// 
//          sun = lp->sun( sc.world().simulation_round() );
// 
//          count++;


         WiselibDutyCyclingProcessor* wlp =
            (*it).get_processor_of_type_w<WiselibDutyCyclingProcessor>();
         if ( wlp == NULL ) continue;

         activity += wlp->ants_duty_cycling_.activity();
         battery += wlp->ants_duty_cycling_.battery();
         if ( wlp->ants_duty_cycling_.active() )
            active++;
         else
            inactive++;

         sun = wlp->ants_duty_cycling_.sun( sc.world().simulation_round() );

         e_idle += wlp->owner().read_simple_tag<double>( "consumed-idle" );
         e_active += wlp->owner().read_simple_tag<double>( "consumed-active" );
         e_tx += wlp->owner().read_simple_tag<double>( "consumed-tx" );
         e_rx += wlp->owner().read_simple_tag<double>( "consumed-rx" );
         e_app += wlp->owner().read_simple_tag<double>( "consumed-app" );

         count++;
      }

      double result = activity / (double)count;
      INFO( logger(), "Mean battery: " << battery / (double)count );
      INFO( logger(), "Mean activity: " << result );
      INFO( logger(), "Active: " << active << "; Inactive: " << inactive );
      INFO( logger(), "Active frac: " << active/(double)count
         << "; Inactive frac: " << inactive/(double)count );

      std::string fname = sc.environment().required_string_param( "duty_out" );

      if ( sc.world().simulation_round() == 0 )
         std::ofstream ofs( fname.c_str(), std::ios::trunc );

      std::ofstream ofs( fname.c_str(), std::ios::app );
      if ( !ofs )
         throw std::runtime_error( "Cannot open file " + fname );

      ofs << sc.world().simulation_round() << " "
         << active/(double)count << " "
         << battery/(double)count << " "
         << sun << std::endl;


      // ------ Energy Consumption ------ \\
      e_idle = e_idle / (double)count;
      e_active = e_active / (double)count;
      e_tx = e_tx / (double)count;
      e_rx = e_rx / (double)count;
      e_app = e_app / (double)count;
      double e_all = e_idle + e_active + e_tx + e_rx + e_app;
      INFO( logger(), "Mean Consumption: All " << e_all
         << "; App " << e_app << "; Tx " << e_tx << "; Rx " << e_rx
         << "; Id " << e_idle << "; Ac " << e_active );
      e_idle = (e_idle / e_all) * 100;
      e_active = (e_active / e_all) * 100;
      e_tx = (e_tx / e_all) * 100;
      e_rx = (e_rx / e_all) * 100;
      e_app = (e_app / e_all) * 100;

      INFO( logger(), "Consumption Percentage: App " << e_app
         << "; Tx " << e_tx << "; Rx " << e_rx
         << "; Id " << e_idle << "; Ac " << e_active );

      std::ofstream ofs_consumed( "consumption", std::ios::app );
      if ( !ofs_consumed )
         throw std::runtime_error( "Cannot open file " + fname );
      ofs_consumed << sc.world().simulation_round() << " "
         << e_app << " " << e_tx << " " << e_rx << " " << e_idle << " "
         << e_active << " " << std::endl;
   }

}
#endif
/*-----------------------------------------------------------------------
 * Source  $Source: /cvs/shawn/shawn/tubsapps/duty_cycling/connectivity_task.cpp,v $
 * Version $Revision: 1.5 $
 * Date    $Date: 2006/06/07 20:02:03 $
 *-----------------------------------------------------------------------
 * $Log: connectivity_task.cpp,v $
 * Revision 1.5  2006/06/07 20:02:03  tbaum
 * added min/max
 *
 * Revision 1.4  2005/08/05 10:00:52  ali
 * 2005 copyright notice
 *
 * Revision 1.3  2005/06/09 15:28:08  tbaum
 * added module functionality
 *
 * Revision 1.2  2005/02/01 09:51:39  tbaum
 *  changed degree() to edge_model().nof_adjacent_nodes()
 *
 * Revision 1.1  2004/11/25 11:16:52  tbaum
 * added duty_cycling
 *
 *-----------------------------------------------------------------------*/
