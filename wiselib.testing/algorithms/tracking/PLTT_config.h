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

//enable switches from general to specific to more specific


//general switches
#define PLTT_OPT
//#define PLTT_METRICS
#define PLTT_DEBUG
#define PLTT_SECURE
//#define PLTT_TARGET_MINI_RUN
#define ISENSE_APP
//#define SHAWN_APP
#define LUEBECK_DEMO


//specific switches
//debug
#ifdef PLTT_DEBUG
	#define ISENSE_PLTT_PASSIVE_DEBUG
	#define ISENSE_PLTT_TARGET_DEBUG
	//#define ISENSE_PLTT_TRACKER_DEBUG
#endif
//optimizations
#ifdef PLTT_OPT
	#define OPT_SPREAD_RANDOM_RECEIVERS
	#define OPT_LQI_INHIBITION
	#define OPT_PATH_CORRECTION
	#define OPT_TARGET_LIST_AGGREGATION
	#define OPT_NON_MERGED_TREE
	#define OPT_RELIABLE_TRACKING
	#define OPT_FLOOD_NEIGHBORS
#endif

//metrics
#ifdef PLTT_METRICS
	#define PLTT_TRACKING_METRICS
	#define PLTT_SPREAD_METRICS
#endif

//metrics dependencies
#ifdef PLTT_TRACKING_METRICS
	#define PLTT_PASSIVE_TRACKING_METRICS
	#define PLTT_PASSIVE_TRACKING_METRICS_LIGHT
	#define PLTT_TRACKER_TRACKING_METRICS
	#define PLTT_AGENT_TRACKING_METRICS
	#define PLTT_TARGET_TRACKING_METRICS
#endif
#ifdef PLTT_SPREAD_METRICS
	#define PLTT_PASSIVE_SPREAD_METRICS
	#define PLTT_PASSIVE_SPREAD_METRICS_LIGHT
	#define PLTT_TARGET_SPREAD_METRICS
#endif

//metrics debug dependencies
#ifdef PLTT_METRICS
	#define ISENSE_PLTT_METRICS_DEBUG
#endif
#ifdef PLTT_SECURE
	#define ISENSE_PLTT_SECURE_DEBUG
#endif

//more specific switches
//debug
#ifdef ISENSE_PLTT_PASSIVE_DEBUG
	#define ISENSE_PLTT_PASSIVE_DEBUG_NEIGHBORHOOD_DISCOVERY
	#define ISENSE_PLTT_PASSIVE_DEBUG_SPREAD
	#define ISENSE_PLTT_PASSIVE_DEBUG_INHIBITION
	#define ISENSE_PLTT_PASSIVE_DEBUG_TRACK_ECHO
	#define ISENSE_PLTT_PASSIVE_DEBUG_TRACK_QUERY
	#define ISENSE_PLTT_PASSIVE_DEBUG_TRACK_QUERY_REPORT
	#define ISENSE_PLTT_PASSIVE_DEBUG_MISC
	#ifdef ISENSE_PLTT_SECURE_DEBUG
		#define ISENSE_PLTT_PASSIVE_DEBUG_SECURE
	#endif
	#ifdef ISENSE_PLTT_METRICS_DEBUG
		#define ISENSE_PLTT_METRICS_PASSIVE_DEBUG
	#endif
	#ifdef OPT_RELIABLE_TRACKING
		#define ISENSE_PLTT_PASSIVE_RELIABLE_TRACKING_DEBUG
	#endif
#endif
#ifdef ISENSE_PLTT_TARGET_DEBUG
	#define ISENSE_PLTT_TARGET_DEBUG_SEND
	#define ISENSE_PLTT_TARGET_DEBUG_MISC
	#ifdef PLTT_SECURE
		#define ISENSE_PLTT_TARGET_DEBUG_SECURE
	#endif
	#ifdef ISENSE_PLTT_METRICS_DEBUG
		#define ISENSE_PLTT_TARGET_METRICS_DEBUG
	#endif
#endif
#ifdef ISENSE_PLTT_TRACKER_DEBUG
	#define ISENSE_PLTT_TRACKER_DEBUG_REPORT
	#define ISENSE_PLTT_TRACKER_DEBUG_QUERY
	#define ISENSE_PLTT_TRACKER_DEBUG_MISC
	#ifdef ISENSE_PLTT_SECURE_DEBUG
		#define ISENSE_PLTT_TRACKER_DEBUG_SECURE
	#endif
	#ifdef ISENSE_PLTT_METRICS_DEBUG
		#define ISENSE_PLTT_METRICS_TRACKER_DEBUG
	#endif
#endif

