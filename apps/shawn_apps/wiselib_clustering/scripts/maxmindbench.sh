#!/bin/bash
if [ $# -eq 4 ]
then
	d_parameter=$1
	topologies=$2
#	times=$2
	max_iterations=$3
	algo_name=$4
	



	
else 
	echo "Not Enough Arguments";
	echo "./""$algo_name""bench.sh parameter topologies max_shawn_iterations algo_name";
fi

	times=$(ls -l topologies/$topologies/*.xml | wc -l)

	if [ $times == 0 ]
	then
		echo "Error no topologies found !"
	fi

	echo $algo_name"bench.sh "
	echo "FORMAT is ./"$algo_name"bench d_parameter number_of_experiments"
	echo "You selected d_parameter="$d_parameter" and number_of_experiments="$times
	echo -n "Run ? [Y/n]> "
	read -t 1 ans
	if [ -n $ans ] || [ $ans != "n" ]
	then
		echo -en "#Total\theads\tsimple\trounds\ttime\tcl.diam\tavgsize\tmsg\tFLOOD\tINFORM\tCONVERGECAST\tREJOIN\t\\" > data/"$d_parameter"."$topologies"."$algo_name".dat
		echo -e "\\" >> data/"$d_parameter"."$topologies"."$algo_name".dat
		echo "Running Experiments..."
			
		for item in topologies/$topologies/*.xml
		do
			echo "Build Config File "$item"..."
			./buildconfig.sh $item $d_parameter $max_iterations $algo_name
			echo "Run Experiment "$item"..."
			echo -n "TIMESTARTED " > "$algo_name"tempresults
			date +%s >> "$algo_name"tempresults
			./shawn -f $algo_name.conf >> "$algo_name"tempresults
			echo -n "TIMEENDED " >> "$algo_name"tempresults
			date +%s >> "$algo_name"tempresults
		
			awk '
			BEGIN 	{
				heads = 0;
				simple = 0;
				dfinish = "";
				round_finished = 0;
				hasfinished = "false";
				total_nodes = 0;
				time_needed = 0;
				avg_size = 0;
				msg_=0;
				msg_flood=0;
				msg_inform=0;
				msg_converge=0;
				msg_rejoin=0;
				timestart=0;
				timeend=0;
				hops=0;
			} \
			{
				if (hasfinished !~ "true")
				if (dfinish ~ "BEGIN"){
					if ($2 ~ "DONE"){
						round_finished = $4;
						hasfinished = "true";
					}
				}
				dfinish = $2;
				if ($2 ~ "RESULTS"){
					if ($4 ~ $7){
					heads++;
					}
					else {
					simple++;
					}
					total_nodes++;
					hops+=$15;
				}
				
				if ($1 ~ "real"){
					time_needed = $2;
				}
				
				if ($1 ~ "SEND"){
					msg_++;
					if ($2 ~ "FLOOD"){
						msg_flood++;
						
					}
					else if ($2 ~ "INFORM"){
						msg_inform++;
						
					}
					else if ($2 ~ "CONVERGECAST"){
						msg_converge++;
						
					}
					else if ($2 ~ "REJOIN"){
						msg_rejoin++;
						
					}
				}
				if ($1 ~ "TIMESTARTED"){
					timestart = $2;
				}
				if ($1 ~ "TIMEENDED"){
					timeend = $2;
				}

			} \
			END	{
				avg_size = total_nodes/heads;
				time_needed = timeend-timestart;
				hops=hops/simple;
				print total_nodes "\t" heads "\t" simple "\t" round_finished "\t" time_needed "\t" hops "\t" avg_size "\t" msg_ "\t" msg_flood "\t" msg_inform "\t" msg_converge "\t" msg_rejoin "\t\\\\";

			}
			' "$algo_name"tempresults >> data/"$d_parameter"."$topologies"."$algo_name".dat

			rm "$algo_name"tempresults
			
			#cp snapshot.pdf snapshots/$item.$d_parameter.pdf
		done

		cat data/"$d_parameter"."$topologies"."$algo_name".dat
	fi
