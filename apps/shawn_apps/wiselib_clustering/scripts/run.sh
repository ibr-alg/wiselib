#!/bin/bash
if [ $# -eq 5 ]
then
start_d=$1
max_d=$2
step_d=$3
topologies=$4
algo=$5

max_it=1001

	rm data/*."$topologies"."$algo".dat
	rm data/*."$topologies"."$algo".dat.avg

	for (( d_=start_d; d_<=max_d; d_+=step_d ))
	do
		echo "Runing ""$algo""bench.sh for d=" $d_ " will repeat until " $max_d " with step " $step_d
		./"$algo"bench.sh $d_ $topologies $max_it $algo
	done
		echo "Descend into data"
		cd data
		echo "Get Averages"
		./average.sh $algo
		echo "Move back"
		cd ..
		rm plots/*_"$algo"_"$topologies".png
		./"$algo"buildgnuplot.sh $algo $topologies
		gnuplot "$topologies"."$algo".experiments.p
		echo "Gnuplot Finished"
		./"$algo"buildreport.sh $algo $topologies
		echo "Report Ready"
		rm data/archives/$topologies.$start_d.$max_d.$step_d.$algo.tar
		tar cf data/archives/$topologies.$start_d.$max_d.$step_d.$algo.tar data/*."$topologies"."$algo".dat
		tar rf data/archives/$topologies.$start_d.$max_d.$step_d.$algo.tar data/*."$topologies"."$algo".dat.avg
		echo "Data Backup ok!"

		rm plots/archives/$topologies.$start_d.$max_d.$step_d.$algo.tar
		tar cf plots/archives/$topologies.$start_d.$max_d.$step_d.$algo.tar plots/*"_$algo_$topologies.png"
		echo "Plots Backup ok!"		


		
else

	echo "Not Enough Arguments";
	echo "./run.sh start_d max_d step_d topologies algoname";
fi
