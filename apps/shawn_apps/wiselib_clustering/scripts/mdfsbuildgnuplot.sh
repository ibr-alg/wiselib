#!/bin/bash
algo=$1
topologies=$2
function addfiles {
	flag=0
	echo -n "plot " >> "$topologies"."$algo".experiments.p
	#((tparam=param))
	for file in data/*.$topologies.$algo.dat.avg
	do
	tval=${file#*/}
	dval=${tval%%.*}
		if [ $flag -eq 0 ]; then
			echo -n "\"$file\" using 1:$target title 'd=$dval' with linespoints " >> "$topologies"."$algo".experiments.p
			flag=1
		else 
			echo " , \\" >> "$topologies"."$algo".experiments.p 
			echo -n " \"$file\" using 1:$target title 'd=$dval' with linespoints " >> "$topologies"."$algo".experiments.p
		fi
		#(( tparam+=rate ))
	done
	echo "" >> "$topologies"."$algo".experiments.p
}

#param=$1
#rate=$2



target=2

echo "

set autoscale xy                      # scale axes automatically
unset log                              # remove any log-scaling
unset label                            # remove any previous labels
set xtic auto                          # set xtics automatically
set ytic auto                          # set ytics automatically
set title \"Clusters In Topology\"
set xlabel \"Nodes In Topology\"
set ylabel \"Clusters Formed\"
set size 1.0, 0.6
set terminal png  size 800, 600 crop
set tmargin 0
set output \"plots/clusters_"$algo"_"$topologies".png\"
	" > "$topologies"."$algo".experiments.p

addfiles

target=4

echo "
unset log                              # remove any log-scaling
unset label                            # remove any previous labels
set xtic auto                          # set xtics automatically
set ytic auto                          # set ytics automatically
set title \"Rounds For Clustering\"
set xlabel \"Nodes In Topology\"
set ylabel \"Shawn Rounds\"
set size 1.0, 0.6
set terminal png 
set output \"plots/rounds_"$algo"_"$topologies".png\"
	" >> "$topologies"."$algo".experiments.p

addfiles

target=7

echo "
unset log                              # remove any log-scaling
unset label                            # remove any previous labels
set xtic auto                          # set xtics automatically
set ytic auto                          # set ytics automatically
set title \"Average Cluster Size\"
set xlabel \"Nodes In Topology\"
set ylabel \"Nodes In Cluster\"
set size 1.0, 0.6
set terminal png 
set output \"plots/averagesize_"$algo"_"$topologies".png\"
	" >> "$topologies"."$algo".experiments.p
addfiles

target=6

echo "
unset log                              # remove any log-scaling
unset label                            # remove any previous labels
set xtic auto                          # set xtics automatically
set ytic auto                          # set ytics automatically
set title \"Average Cluster Diameter\"
set xlabel \"Nodes In Topology\"
set ylabel \"Hops to Cluster Head\"
set size 1.0, 0.6
set terminal png 
set output \"plots/averagediam_"$algo"_"$topologies".png\"
	" >> "$topologies"."$algo".experiments.p
addfiles


target=5

echo "
unset log                              # remove any log-scaling
unset label                            # remove any previous labels
set xtic auto                          # set xtics automatically
set ytic auto                          # set ytics automatically
set title \"Time needed for Clustering\"
set xlabel \"Nodes In Topology\"
set ylabel \"Time in seconds\"
set size 1.0, 0.6
set terminal png 
set output \"plots/time_"$algo"_"$topologies".png\"
	" >> "$topologies"."$algo".experiments.p
addfiles

target=8

echo "
unset log                              # remove any log-scaling
unset label                            # remove any previous labels
set xtic auto                          # set xtics automatically
set ytic auto                          # set ytics automatically
set title \"Total messages sent\"
set xlabel \"Nodes In Topology\"
set ylabel \"Messages\"
set size 1.0, 0.6
set terminal png 
set output \"plots/messages_all_"$algo"_"$topologies".png\"
	" >> "$topologies"."$algo".experiments.p
addfiles


target=9

echo "
unset log                              # remove any log-scaling
unset label                            # remove any previous labels
set xtic auto                          # set xtics automatically
set ytic auto                          # set ytics automatically
set title \"Neighbhor Discovery Messages sent\"
set xlabel \"Nodes In Topology\"
set ylabel \"Messages\"
set size 1.0, 0.6
set terminal png 
set output \"plots/messages_neighbhor_discovery_"$algo"_"$topologies".png\"
	" >> "$topologies"."$algo".experiments.p
addfiles

target=10

echo "
unset log                              # remove any log-scaling
unset label                            # remove any previous labels
set xtic auto                          # set xtics automatically
set ytic auto                          # set ytics automatically
set title \"Neighbhor Reply Messages sent\"
set xlabel \"Nodes In Topology\"
set ylabel \"Messages\"
set size 1.0, 0.6
set terminal png 
set output \"plots/messages_neighbor_reply_"$algo"_"$topologies".png\"
	" >> "$topologies"."$algo".experiments.p
addfiles

target=11

echo "
unset log                              # remove any log-scaling
unset label                            # remove any previous labels
set xtic auto                          # set xtics automatically
set ytic auto                          # set ytics automatically
set title \"JJoin Request Messages sent\"
set xlabel \"Nodes In Topology\"
set ylabel \"Messages\"
set size 1.0, 0.6
set terminal png 
set output \"plots/messages_join_request_"$algo"_"$topologies".png\"
	" >> "$topologies"."$algo".experiments.p
addfiles

target=12

echo "
unset log                              # remove any log-scaling
unset label                            # remove any previous labels
set xtic auto                          # set xtics automatically
set ytic auto                          # set ytics automatically
set title \"Join Reject Messages sent\"
set xlabel \"Nodes In Topology\"
set ylabel \"Messages\"
set size 1.0, 0.6
set terminal png 
set output \"plots/messages_join_reject_"$algo"_"$topologies".png\"
	" >> "$topologies"."$algo".experiments.p
addfiles

target=13

echo "
unset log                              # remove any log-scaling
unset label                            # remove any previous labels
set xtic auto                          # set xtics automatically
set ytic auto                          # set ytics automatically
set title \"Resume Messages sent\"
set xlabel \"Nodes In Topology\"
set ylabel \"Messages\"
set size 1.0, 0.6
set terminal png 
set output \"plots/messages_resume_"$algo"_"$topologies".png\"
	" >> "$topologies"."$algo".experiments.p
addfiles

target=14

echo "
unset log                              # remove any log-scaling
unset label                            # remove any previous labels
set xtic auto                          # set xtics automatically
set ytic auto                          # set ytics automatically
set title \"Attribute based Cluster Head Decision\"
set xlabel \"Nodes In Topology\"
set ylabel \"Rounds\"
set size 1.0, 0.6
set terminal png 
set output \"plots/att_chd_rounds_"$algo"_"$topologies".png\"
	" >> "$topologies"."$algo".experiments.p
addfiles

target=15

echo "
unset log                              # remove any log-scaling
unset label                            # remove any previous labels
set xtic auto                          # set xtics automatically
set ytic auto                          # set ytics automatically
set title \"Dfs Join Decision\"
set xlabel \"Nodes In Topology\"
set ylabel \"Rounds\"
set size 1.0, 0.6
set terminal png 
set output \"plots/dfs_jd_rounds_"$algo"_"$topologies".png\"
	" >> "$topologies"."$algo".experiments.p
addfiles

target=17

echo "
unset log                              # remove any log-scaling
unset label                            # remove any previous labels
set xtic auto                          # set xtics automatically
set ytic auto                          # set ytics automatically
set title \"Attribute based Cluster Head Decision\"
set xlabel \"Nodes In Topology\"
set ylabel \"Messages\"
set size 1.0, 0.6
set terminal png 
set output \"plots/att_chd_mess_"$algo"_"$topologies".png\"
	" >> "$topologies"."$algo".experiments.p
addfiles

target=18

echo "
unset log                              # remove any log-scaling
unset label                            # remove any previous labels
set xtic auto                          # set xtics automatically
set ytic auto                          # set ytics automatically
set title \"Dfs Join Decision\"
set xlabel \"Nodes In Topology\"
set ylabel \"Messages\"
set size 1.0, 0.6
set terminal png 
set output \"plots/dfs_jd_mess_"$algo"_"$topologies".png\"
	" >> "$topologies"."$algo".experiments.p
addfiles
