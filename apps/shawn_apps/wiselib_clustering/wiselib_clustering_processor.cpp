
/************************************************************************
 ** This file is part of the network simulator Shawn.                  **
 ** Copyright (C) 2004-2007 by the SwarmNet (www.swarmnet.de) project  **
 ** Shawn is free software; you can redistribute it and/or modify it   **
 ** under the terms of the BSD License. Refer to the shawn-licence.txt **
 ** file in the root of the Shawn source tree for further details.     **
 ************************************************************************/
#include "_legacyapps_enable_cmake.h"
#ifdef ENABLE_WISELIBCLUSTERING
#include "legacyapps/wiselib_clustering/wiselib_clustering_processor.h"
#include "sys/simulation/simulation_controller.h"
#include "sys/simulation/simulation_environment.h"
#include "sys/node.h"
#include "sys/taggings/basic_tags.h"


#define RESULT_ITERATION 35

namespace wiselib {

    WiselibClusteringProcessor::
    WiselibClusteringProcessor()
    : wiselib_radio_(os_),
    wiselib_timer_(os_),
    wiselib_debug_(os_),
    wiselib_clock_(os_),
    wiselib_rand_(os_),
    wiselib_distance_(NULL)
    {
    }
    // ----------------------------------------------------------------------

    WiselibClusteringProcessor::
    ~WiselibClusteringProcessor() {
    }
    // ----------------------------------------------------------------------

    void
    WiselibClusteringProcessor::
    boot(void)
    throw () {
        os_.proc = this;
	if(wiselib_distance_ == NULL)
	    wiselib_distance_ = new Os::Distance(os_);
        const shawn::SimulationEnvironment& se = owner().world().
                simulation_controller().environment();
        cluster_algo_ = se.optional_string_param("clustering_algorithm", "bfs");

        if (false) {
        }
#ifdef ENABLE_BFS
        else if (cluster_algo_ == "bfs") {
            int theta = se.optional_int_param("theta", 100);
            shawn::DoubleTag *csidtag = new shawn::DoubleTag("csid", (double) 0);
            owner_w().add_tag(csidtag);
            shawn::StringTag *parenttag = new shawn::StringTag("predecessor", "");
            owner_w().add_tag(parenttag);

            bfsclustering_.init(wiselib_radio_, wiselib_timer_, wiselib_debug_);

            bfsclustering_.set_theta(theta);

            bfsclustering_.reg_changed_callback<WiselibClusteringProcessor,
                    &WiselibClusteringProcessor::cluster_state_changed > (this);

            bfsclustering_.enable();
            INFO(logger(), "Start BFS clustering on " << owner().id());
        }
#endif
#ifdef ENABLE_DFS
        else if (cluster_algo_ == "dfs") {
            // get theta value from topology
            theta_ = se.optional_int_param("theta", 100);
            // tags
            shawn::DoubleTag *csidtag = new shawn::DoubleTag("csid", (double) 0);
            owner_w().add_tag(csidtag);
            shawn::StringTag *parenttag = new shawn::StringTag("predecessor", "");
            owner_w().add_tag(parenttag);
            dfsclustering_.init(wiselib_radio_, wiselib_timer_, wiselib_debug_);
            // set theta clustering parameter
            dfsclustering_.set_theta(theta_);
            // register state changed function
            dfsclustering_.reg_changed_callback<WiselibClusteringProcessor,
                    &WiselibClusteringProcessor::cluster_state_changed > (this);
            // start algorithm
            dfsclustering_.enable();
            INFO(logger(), "Start DFS clustering on " << owner().id());
        }
#endif
#ifdef ENABLE_MAXMIND
        else if (cluster_algo_ == "maxmind") {
            // get theta value from topology
            theta_ = se.optional_int_param("theta", 100);
            // tags
            shawn::DoubleTag *csidtag = new shawn::DoubleTag("csid", (double) 0);
            owner_w().add_tag(csidtag);
            shawn::StringTag *parenttag = new shawn::StringTag("predecessor", "");
            owner_w().add_tag(parenttag);
            maxmindclustering_.init(wiselib_radio_, wiselib_timer_, wiselib_debug_);
            // set theta clustering parameter
            /*maxmindclustering_.set_theta(theta_);
            // register state changed function
            maxmindclustering_.reg_changed_callback<WiselibClusteringProcessor,
                    &WiselibClusteringProcessor::cluster_state_changed > (this);
            // start algorithm*/
            maxmindclustering_.enable();
            INFO(logger(), "Start maxmind clustering on " << owner().id());

        }
#endif
#ifdef ENABLE_MBFS
        else if (cluster_algo_ == "mbfs") {

            // get theta value from topology
            theta_ = se.optional_int_param("theta", 100);
            // tags
            shawn::DoubleTag *csidtag = new shawn::DoubleTag("csid", (double) 0);
            owner_w().add_tag(csidtag);
            shawn::StringTag *parenttag = new shawn::StringTag("predecessor", "");
            owner_w().add_tag(parenttag);

            INFO(logger(), "Start Cluster Formation on " << owner().id());

            Mbfsclustering_.init(wiselib_radio_, wiselib_timer_, wiselib_debug_);
            // set theta clustering parameter
            Mbfsclustering_.set_theta(theta_);
            // register state changed function
            Mbfsclustering_.reg_changed_callback<WiselibClusteringProcessor,
                    &WiselibClusteringProcessor::cluster_state_changed > (this);
            // set the HeadDecision Module
            Mbfsclustering_.set_cluster_head_decision(attrCHD_);
            // set the JoinDecision Module
            Mbfsclustering_.set_join_decision(simpleJD_);
            // set the Iterator Module
            Mbfsclustering_.set_iterator(simpleIT_);

            // start algorithm
            Mbfsclustering_.enable();

        }
#endif
#ifdef ENABLE_MDFS
        else if (cluster_algo_ == "mdfs") {

            // get theta value from topology
            theta_ = se.optional_int_param("theta", 100);
            // tags
            shawn::DoubleTag *csidtag = new shawn::DoubleTag("csid", (double) 0);
            owner_w().add_tag(csidtag);
            shawn::StringTag *parenttag = new shawn::StringTag("predecessor", "");
            owner_w().add_tag(parenttag);

            INFO(logger(), "Start Cluster Formation on " << owner().id());

            Mdfsclustering_.init(wiselib_radio_, wiselib_timer_, wiselib_debug_);
            // set theta clustering parameter
            Mdfsclustering_.set_theta(theta_);
            // register state changed function
            Mdfsclustering_.reg_changed_callback<WiselibClusteringProcessor,
                    &WiselibClusteringProcessor::cluster_state_changed > (this);
            // set the HeadDecision Module
            Mdfsclustering_.set_cluster_head_decision(probaCHD_);
            // set the JoinDecision Module
            Mdfsclustering_.set_join_decision(dfsJD_);
            // set the Iterator Module
            Mdfsclustering_.set_iterator(simpleIT_);

            // start algorithm
            Mdfsclustering_.enable();


        }
#endif
#ifdef ENABLE_MMAXMIND
        else if (cluster_algo_ == "mmaxmind") {

            // get theta value from topology
            theta_ = se.optional_int_param("theta", 100);

            // tags
            shawn::DoubleTag *csidtag = new shawn::DoubleTag("csid", (double) 0);
            owner_w().add_tag(csidtag);
            shawn::StringTag *parenttag = new shawn::StringTag("predecessor", "");
            owner_w().add_tag(parenttag);

            Mmaxmindclustering_.init(wiselib_radio_, wiselib_timer_, wiselib_debug_);


            // set theta clustering parameter
            maxmindCHD_.set_theta(theta_);
	    maxmindIT_.set_theta(theta_);
	    maxmindJD_.set_theta(theta_);

            // register state changed function
            Mmaxmindclustering_.reg_state_changed_callback<WiselibClusteringProcessor,
                    &WiselibClusteringProcessor::cluster_state_changed > (this);

            // set the HeadDecision Module
            Mmaxmindclustering_.set_cluster_head_decision(maxmindCHD_);
            // set the JoinDecision Module
            Mmaxmindclustering_.set_join_decision(maxmindJD_);
            // set the Iterator Module
            Mmaxmindclustering_.set_iterator(maxmindIT_);

            // start algorithm
            Mmaxmindclustering_.enable();
        }
#endif
#ifdef ENABLE_MMOCA
        else if (cluster_algo_ == "mmoca") {

            // get theta value from topology
            theta_ = se.optional_int_param("theta", 4);
            int kappa_ = se.optional_int_param("kappa", 5);

            // tags
            shawn::DoubleTag *csidtag = new shawn::DoubleTag("csid", (double) 0);
            owner_w().add_tag(csidtag);
            shawn::StringTag *parenttag = new shawn::StringTag("predecessor", "");
            owner_w().add_tag(parenttag);

            Mmocaclustering_.init(wiselib_radio_, wiselib_timer_, wiselib_debug_);


            // set theta clustering parameter
            Mmocaclustering_.set_maxhops(kappa_);
            Mmocaclustering_.set_probability(theta_);
	    
            // register state changed function
            Mmocaclustering_.reg_state_changed_callback<WiselibClusteringProcessor,
                    &WiselibClusteringProcessor::cluster_state_changed > (this);

            // set the HeadDecision Module
            Mmocaclustering_.set_cluster_head_decision(probabilisticCHD_);
            // set the JoinDecision Module
            Mmocaclustering_.set_join_decision(mocaJD_);
            // set the Iterator Module
            Mmocaclustering_.set_iterator(overlappingIT_);

            // start algorithm
            Mmocaclustering_.enable();
        }
#endif
#ifdef ENABLE_MCAWT
        else if (cluster_algo_ == "mcawt") {

            // get theta value from topology
            theta_ = se.optional_int_param("min_waiting", 4000);
            int kappa_ = se.optional_int_param("max_waiting", 50000);
	    cawt_max_round_ = kappa_ / 100;
            // tags
            shawn::DoubleTag *nodetypetag = new shawn::DoubleTag("nodetype", (double) 0);
            owner_w().add_tag(nodetypetag);
	    shawn::DoubleTag *csidtag = new shawn::DoubleTag("csid", (double) 0);
            owner_w().add_tag(csidtag);
            shawn::StringTag *parenttag = new shawn::StringTag("predecessor", "");
            owner_w().add_tag(parenttag);

            Mcawtclustering_.init(wiselib_radio_, wiselib_timer_, wiselib_debug_, wiselib_clock_, wiselib_rand_);

	    Mcawtclustering_.set_hello_message_factor(se.optional_double_param("lambda", 0.2));
	    
            // register state changed function
            Mcawtclustering_.reg_state_changed_callback<WiselibClusteringProcessor,
                    &WiselibClusteringProcessor::cluster_state_changed > (this);

            // set the HeadDecision Module
            Mcawtclustering_.set_cluster_head_decision(wtCHD_);
            // set the JoinDecision Module
            Mcawtclustering_.set_join_decision(cawtJD_);
            // set the Iterator Module
            Mcawtclustering_.set_iterator(normIT_);
	    
	    wtCHD_.set_min_waiting_time(theta_*10);
	    wtCHD_.set_max_waiting_time(kappa_*10);
	    wtCHD_.set_timer_decrease_factor(se.optional_double_param("beta", 0.9));
            // start algorithm
            Mcawtclustering_.enable();
        }
#endif
#ifdef ENABLE_AEEC
	else if (cluster_algo_ == "aeec")
	{	    
	    shawn::DoubleTag *nodetypetag = new shawn::DoubleTag("nodetype", (double) 0);
            owner_w().add_tag(nodetypetag);
	    shawn::DoubleTag *csidtag = new shawn::DoubleTag("csid", (double) 0);
            owner_w().add_tag(csidtag);
            shawn::StringTag *parenttag = new shawn::StringTag("predecessor", "");
            owner_w().add_tag(parenttag);
	    
	    int cluster_time = se.optional_int_param("max_waiting", 25000);
	    Aeecclustering_.set_cluster_head_decision(jrCHD_);
	    Aeecclustering_.set_join_decision(aeecJD_);
	    Aeecclustering_.set_iterator(aeecIT_);
	    Aeecclustering_.init(wiselib_radio_, wiselib_timer_, wiselib_debug_, wiselib_clock_, wiselib_rand_, *wiselib_distance_);
	    Aeecclustering_.set_cluster_time(cluster_time);
	    
	    Aeecclustering_.enable();
	}
#endif
#ifdef ENABLE_KOCA
	else if (cluster_algo_ == "koca")
	{	    
	    shawn::DoubleTag *nodetypetag = new shawn::DoubleTag("nodetype", (double) 0);
            owner_w().add_tag(nodetypetag);
	    shawn::DoubleTag *csidtag = new shawn::DoubleTag("csid", (double) 0);
            owner_w().add_tag(csidtag);
            shawn::StringTag *parenttag = new shawn::StringTag("predecessor", "");
            owner_w().add_tag(parenttag);
	    
	    int probability = se.optional_int_param("probability", 10);
	    int max_hops = se.optional_int_param("max_hops", 3);
	    
	    koca_chd_.set_probability(probability);
	    
	    koca_clustering_.set_cluster_head_decision(koca_chd_);
	    koca_clustering_.set_join_decision(koca_jd_);
	    koca_clustering_.set_iterator(koca_it_);
	    koca_clustering_.set_max_hops(max_hops);
	    
	    koca_clustering_.init(wiselib_radio_, wiselib_timer_, wiselib_debug_, wiselib_clock_, wiselib_rand_, *wiselib_distance_);
	    
	    koca_clustering_.enable();
	}
#endif
        else {
            ERROR(logger(), "Given routing algorithm '" << cluster_algo_ << "' not known.");
            ERROR(logger(), "Try 'tree', 'dsdv', or 'dsr'.");
            abort();
        }
    }
    // ----------------------------------------------------------------------

    void
    WiselibClusteringProcessor::
    work(void)
    throw () {
        if (false) {
        }
#ifdef ENABLE_BFS
        else if (cluster_algo_ == "bfs") {
            if (simulation_round() < RESULT_ITERATION) {
                owner_w().write_simple_tag("csid", ((double) bfsclustering_.cluster_id()) / owner().world().active_nodes_count());
                if (bfsclustering_.parent() == -1) {
                    owner_w().write_simple_tag("predecessor", owner().label());
                } else {
                    owner_w().write_simple_tag("predecessor", owner().world().find_node_by_id(bfsclustering_.parent())->label());
                }
            }
//#ifdef DEBUG_BFSCLUSTERING
            if (simulation_round() == RESULT_ITERATION) {
                INFO(logger(), "RESULTS Node " << owner().id()
                        << " joined sector " << bfsclustering_.cluster_id()
                        << " and has parent " << bfsclustering_.parent()
                        << " distance from head " << bfsclustering_.hops()
                        );


                INFO(logger(), "MESSAGES JOIN " << bfsclustering_.mess_join()
                        << " ACC " << bfsclustering_.mess_join_acc()
                        << " DENY " << bfsclustering_.mess_join_deny()
                        << " RESUME " << bfsclustering_.mess_resume()
                        );

                //bfsclustering_.present_neighbors();
            }
//#endif
        }
#endif
#ifdef ENABLE_DFS
        else if (cluster_algo_ == "dfs") {
            if (simulation_round() < RESULT_ITERATION) {
                owner_w().write_simple_tag("csid", ((double) dfsclustering_.cluster_id()) / owner().world().active_nodes_count());
                if (dfsclustering_.parent() == -1) {
                    owner_w().write_simple_tag("predecessor", owner().label());
                } else {
                    owner_w().write_simple_tag("predecessor", owner().world().find_node_by_id(dfsclustering_.parent())->label());
                }

            }
#ifdef DEBUG_DFSCLUSTERING
            if (simulation_round() == RESULT_ITERATION) {
                INFO(logger(), "RESULTS Node " << owner().id()
                        << " joined sector " << dfsclustering_.cluster_id()
                        << " and has parent " << dfsclustering_.parent()
                        << " distance from head " << dfsclustering_.hops()
                        );

                INFO(logger(), "MESSAGES NEI_DISC " << dfsclustering_.mess_neighbor_discovery()
                        << " NEI_REPLY " << dfsclustering_.mess_neighbor_reply()
                        << " JOIN_REQ " << dfsclustering_.mess_join_req()
                        << " JOIN_DENY " << dfsclustering_.mess_join_deny()
                        << " RESUME " << dfsclustering_.mess_resume()
                        );

                //dfsclustering_.present_neighbors();
            }
#endif
        }
#endif
#ifdef ENABLE_MAXMIND
        else if (cluster_algo_ == "maxmind") {
            if (simulation_round() < RESULT_ITERATION) {
                //owner_w().write_simple_tag("csid", ((double) mmd_form_.cluster_id() / owner().world().active_nodes_count()) );
                owner_w().write_simple_tag("csid", ((double) (maxmindclustering_.cluster_id() % 10) / 10));
                //owner_w().write_simple_tag("csid", ((double) (mmd_form_.cluster_id()*3 ) / 10));
                if (maxmindclustering_.parent() == -1) {
                    owner_w().write_simple_tag("predecessor", owner().label());
                } else {
                    owner_w().write_simple_tag("predecessor", owner().world().find_node_by_id(maxmindclustering_.parent())->label());
                }

            }
            if (simulation_round() == RESULT_ITERATION) {
                INFO(logger(), "RESULTS Node " << owner().id()
                        << " joined sector " << maxmindclustering_.cluster_id()
                        << " and has parent " << maxmindclustering_.parent()

                        );
            }


        }
#endif
#ifdef ENABLE_MBFS
        else if (cluster_algo_ == "mbfs") {
            if (simulation_round() < RESULT_ITERATION) {
                owner_w().write_simple_tag("csid", ((double) Mbfsclustering_.cluster_id()) / owner().world().active_nodes_count());
                if (Mbfsclustering_.parent() == -1) {
                    owner_w().write_simple_tag("predecessor", owner().label());
                } else {
                    owner_w().write_simple_tag("predecessor", owner().world().find_node_by_id(Mbfsclustering_.parent())->label());
                }

            }
            if (simulation_round() == RESULT_ITERATION) {
#ifdef DEBUG_MBFSCLUSTERING
                INFO(logger(), "RESULTS Node " << owner().id()
                        << " joined sector " << Mbfsclustering_.cluster_id()
                        << " and has parent " << Mbfsclustering_.parent()
                        << " distance from head " << Mbfsclustering_.hops()
                        );

                INFO(logger(), "MESSAGES JOIN " << Mbfsclustering_.mess_join()
                        << " ACC " << Mbfsclustering_.mess_join_acc()
                        << " DENY " << Mbfsclustering_.mess_join_deny()
                        << " RESUME " << Mbfsclustering_.mess_resume()
                        );

                //Mbfsclustering_.present_neighbors();
#endif
            }

        }
#endif
#ifdef ENABLE_MDFS
        else if (cluster_algo_ == "mdfs") {
            if (simulation_round() < RESULT_ITERATION) {
                owner_w().write_simple_tag("csid", ((double) Mdfsclustering_.cluster_id()) / owner().world().active_nodes_count());
                if (Mdfsclustering_.parent() == -1) {
                    owner_w().write_simple_tag("predecessor", owner().label());
                } else {
                    owner_w().write_simple_tag("predecessor", owner().world().find_node_by_id(Mdfsclustering_.parent())->label());
                }

            }
#ifdef DEBUG_MDFSCLUSTERING
            if (simulation_round() == RESULT_ITERATION) {
                INFO(logger(), "RESULTS Node " << owner().id()
                        << " joined sector " << Mdfsclustering_.cluster_id()
                        << " and has parent " << Mdfsclustering_.parent()
                        << " distance from head " << Mdfsclustering_.hops()
                        );
                //Mdfsclustering_.present_neighbors();

                INFO(logger(), "MESSAGES NEI_DISC " << Mdfsclustering_.mess_neighbor_discovery()
                        << " NEI_REPLY " << Mdfsclustering_.mess_neighbor_reply()
                        << " JOIN_REQ " << Mdfsclustering_.mess_join_req()
                        << " JOIN_DENY " << Mdfsclustering_.mess_join_deny()
                        << " RESUME " << Mdfsclustering_.mess_resume()
                        );
            }
#endif

        }
#endif
#ifdef ENABLE_MMAXMIND
        else if (cluster_algo_ == "mmaxmind") {
            //INFO( logger(), "Node "<<owner().id()<<" runs work for round "<< simulation_round() );

            if (simulation_round() == RESULT_ITERATION) {
                //owner_w().write_simple_tag("csid", ((double) mmd_form_.cluster_id() / owner().world().active_nodes_count()) );
                double t_csid = (double) (Mmaxmindclustering_.cluster_id() % 10) / 10;
                owner_w().write_simple_tag("csid", t_csid);
		std::cout << t_csid << std::endl;
                //owner_w().write_simple_tag("csid", ((double) (mmd_form_.cluster_id()*3 ) / 10));
                if (Mmaxmindclustering_.parent() == -1) {
                    owner_w().write_simple_tag("predecessor", owner().label());
                } else {
                    owner_w().write_simple_tag("predecessor", owner().world().find_node_by_id(Mmaxmindclustering_.parent())->label());
                }

            }
            if (simulation_round() == RESULT_ITERATION) {
                INFO(logger(), "RESULTS Node " << owner().id()
                        << " joined sector " << Mmaxmindclustering_.cluster_id()
                        << " and has parent " << Mmaxmindclustering_.parent());
            }


        }
#endif
#ifdef ENABLE_MMOCA

        else if (cluster_algo_ == "mmoca") {
            //INFO( logger(), "Node "<<owner().id()<<" runs work for round "<< simulation_round() );

            if (simulation_round() <= RESULT_ITERATION) {
                //owner_w().write_simple_tag("csid", ((double) (Mmocaclustering_.node_type()*3 ) / 10));
		owner_w().write_simple_tag("csid", ((double) (Mmocaclustering_.cluster_id() % 10 ) / 10));
            }
            if (simulation_round() == RESULT_ITERATION) {
                INFO(logger(), "RESULTS Node " << owner().id()
                        << " has type " << Mmocaclustering_.node_type());
            }


        }
#endif

#ifdef ENABLE_MCAWT
        else if (cluster_algo_ == "mcawt") {
            //INFO( logger(), "Node "<<owner().id()<<" runs work for round "<< simulation_round() );

            if (simulation_round() <= cawt_max_round_ + 10) {
                owner_w().write_simple_tag("nodetype", ((double) (Mcawtclustering_.node_type()-1)));
                owner_w().write_simple_tag("csid", ((double) (Mcawtclustering_.cluster_id())));
		//owner_w().write_simple_tag("csid", ((double) (Mcawtclustering_.cluster_id() % 10 ) / 10));
            }
            if (simulation_round() == cawt_max_round_ + 10) {
		owner_w().write_simple_tag("predecessor", owner().world().find_node_by_id(Mcawtclustering_.parent())->label());
		
		std::string typestring = "";
		if(Mcawtclustering_.node_type() == 0)
		{
		    typestring = "UNASSIGNED";
		}
		else if (Mcawtclustering_.node_type() == 1)
		{
		    typestring = "NODE";
		}
		else
		{
		    typestring = "HEAD";
		}
		
		int hops = 1;
		if(Mcawtclustering_.cluster_id() != Mcawtclustering_.parent())
		{
		    hops++;
		}
		
		INFO(logger(), "RESULTS: " << owner().id() << " | " << typestring << " | " 
			<< Mcawtclustering_.cluster_id() << " | " << hops);
		
                /*INFO(logger(), "RESULTS Node " << owner().id()
                        << " has type " << Mcawtclustering_.node_type()
			<< " and belongs to cluster ");*/
		//TODO: Parent-ID; Cluster-ID
            }


        }
#endif
#ifdef ENABLE_AEEC
            else if (cluster_algo_ == "aeec") {
            //INFO( logger(), "Node "<<owner().id()<<" runs work for round "<< simulation_round() );

            if (simulation_round() <= Aeecclustering_.cluster_time()/1000 + 5) {
                 owner_w().write_simple_tag("nodetype", ((double) (Aeecclustering_.node_type()-1)));
                 owner_w().write_simple_tag("csid", ((double) (Aeecclustering_.cluster_id())));
            }
            if (simulation_round() == Aeecclustering_.cluster_time()/1000 + 5) {
// 		INFO(logger(), "RESULTS: ");
 		owner_w().write_simple_tag("predecessor", owner().world().find_node_by_id(Aeecclustering_.parent())->label());
// 		
 		std::string typestring = "";
		if(Aeecclustering_.node_type() == 0)
		{
		    typestring = "UNASSIGNED";
		}
		else if (Aeecclustering_.node_type() == 1)
		{
		    typestring = "NODE";
		}
		else
		{
		    typestring = "HEAD";
		}
		
		int hops = 1;
		if(Aeecclustering_.cluster_id() != Aeecclustering_.parent())
		{
		    hops++;
		}
		
		INFO(logger(), "RESULTS: " << owner().id() << " | " << typestring << " | " 
			<< Aeecclustering_.cluster_id() << " | " << hops);
		
                /*INFO(logger(), "RESULTS Node " << owner().id()
                        << " has type " << Mcawtclustering_.node_type()
			<< " and belongs to cluster ");*/
		//TODO: Parent-ID; Cluster-ID
            }


        }
#endif

    }
    // ----------------------------------------------------------------------

    void
    WiselibClusteringProcessor::
    cluster_state_changed(int event)
    throw () {
        if (false) {
        }
#ifdef ENABLE_BFS
        else if (cluster_algo_ == "bfs")/*||(cluster_algo_=="cluster_radio")*/ {
            if (event == NODE_JOINED)
                INFO(logger(), "Node " << owner().id() << " joined sector " << bfsclustering_.cluster_id()
                    << " at simulation round " << owner().world().simulation_round());
        }
#endif
#ifdef ENABLE_DFS
        else if (cluster_algo_ == "dfs") {
            if (event == NODE_JOINED) {
                INFO(logger(), "Node " << owner().id() << " joined sector " << dfsclustering_.cluster_id()
                        << " at simulation round " << owner().world().simulation_round());
            } else
                if (event == ELECTED_CLUSTER_HEAD) {
                INFO(logger(), "Node " << owner().id() << " is now head if cluster " << dfsclustering_.cluster_id()
                        << " at simulation round " << owner().world().simulation_round());
            } else
                if (event == NODE_LEFT) {
                INFO(logger(), "Node " << owner().id() << " left sector " << dfsclustering_.cluster_id()
                        << " at simulation round " << owner().world().simulation_round());
            }
        }
#endif
#ifdef ENABLE_MBFS
        else if (cluster_algo_ == "mbfs") {
            if (event == NODE_JOINED) {
                INFO(logger(), "Node " << owner().id() << " joined sector " << Mbfsclustering_.cluster_id()
                        << " at simulation round " << owner().world().simulation_round());
            } else
                if (event == ELECTED_CLUSTER_HEAD) {
                INFO(logger(), "Node " << owner().id() << " is now head if cluster " << Mbfsclustering_.cluster_id()
                        << " at simulation round " << owner().world().simulation_round());
                INFO(logger(), "CHEAD DECIDED round " << owner().world().simulation_round());
            } else
                if (event == NODE_LEFT) {
                INFO(logger(), "Node " << owner().id() << " left sector " << Mbfsclustering_.cluster_id()
                        << " at simulation round " << owner().world().simulation_round());
            }
        }
#endif
#ifdef ENABLE_MDFS
        else if (cluster_algo_ == "mdfs") {
            if (event == Mdfsclustering_t::NODE_JOINED) {
                INFO(logger(), "Node " << owner().id() << " joined sector " << Mdfsclustering_.cluster_id()
                        << " at simulation round " << owner().world().simulation_round());
            } else
                if (event == Mdfsclustering_t::CLUSTER_HEAD_CHANGED) {
                INFO(logger(), "Node " << owner().id() << " is now head if cluster " << Mdfsclustering_.cluster_id()
                        << " at simulation round " << owner().world().simulation_round());
                INFO(logger(), "CHEAD DECIDED round " << owner().world().simulation_round());
            } else
                if (event == Mdfsclustering_t::NODE_LEFT) {
                INFO(logger(), "Node " << owner().id() << " left sector " << Mdfsclustering_.cluster_id()
                        << " at simulation round " << owner().world().simulation_round());
            }
        }
#endif
#ifdef ENABLE_MMAXMIND
        else if (cluster_algo_ == "mmaxmind") {
            if (event == ELECTED_CLUSTER_HEAD) {
                INFO(logger(), "Node " << owner().id() << " is now Head of cluster " << Mmaxmindclustering_.cluster_id()
                        << " at simulation round " << owner().world().simulation_round());
                INFO(logger(), "CHEAD DECIDED round " << owner().world().simulation_round());
            } else if (event == NODE_JOINED) {
                INFO(logger(), "Node " << owner().id() << " joined sector " << Mmaxmindclustering_.cluster_id()
                        << " at simulation round " << owner().world().simulation_round());
            } else if (event == LAST_MSG) {
                INFO(logger(), "CLUSTER FORMED ");
            }

        }
#endif

    }

}
#endif
