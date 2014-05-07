#!/bin/bash
counter=0;
if [ $# -eq 4 ]
then
	targetfile=$1
	theta=$2
	max_iter=$3
	algo_name=$4
	
	
	
	# files are arranged in ascending order (small to big topologies)
		echo "Topology Filename:"$targetfile
		echo "
		random_seed action=load filename=.rseed
		prepare_world edge_model=simple comm_model=disk_graph transm_model=stats_chain range=2
		chain_transm_model name=reliable

		load_world file=$targetfile processors=wiselibclustering

		simulation clustering_algorithm=${algo_name} theta=${theta} kappa=2 max_iterations=${max_iter}

		vis_create
		vis_simple_camera
		vis_tag_color_vec elem_regex=node.* dynamictag=csid prop=background prio=2
		vis_show_comradius
		#vis_create_label
		vis_create_edges_tree
		vis_single_snapshot

			"> $algo_name.conf

else 
	echo "Not Enough Arguments";
	echo "./buildconfig.sh targetfile parameter max_iterations algo_name";
fi



