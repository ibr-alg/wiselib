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
		echo -en "#Total\theads\tsimple\trounds\ttime\tcl.diam\tavgsize\tmsg\tJOIN\tACC\tDENY\tRESUME\t\\" > data/"$d_parameter"."$topologies"."$algo_name".dat
		echo -e "\\" >> data/"$d_parameter"."$topologies"."$algo_name".dat
		echo "Running Experiments..."
			
		for item in topologies/$topologies/*.xml
		do
			echo "Build Config File "$item"..."
			./buildconfig.sh $item $d_parameter $max_iterations $algo_name
			echo "Run Experiment "$item"..."
			echo -n "TIMESTARTED " > tempresults
			date +%s >> tempresults
			./shawn -f "$algo_name".conf >> tempresults
			echo -n "TIMEENDED " >> tempresults
			date +%s >> tempresults
		
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
				msg_join=0;
				msg_jacc=0;
				msg_jdeny=0;
				msg_resume=0;
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
					if ($2 ~ "JOIN_ACCEPT"){
						msg_jacc++;
						
					}
					else if ($2 ~ "JOIN_DENY"){
						msg_jdeny++;
						
					}
					else if ($2 ~ "JOIN"){
						msg_join++;
						
					}
					if ($2 ~ "RESUME"){
						msg_resume++;
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
				print total_nodes "\t" heads "\t" simple "\t" round_finished "\t" time_needed "\t" hops "\t" avg_size "\t" msg_ "\t" msg_join "\t" msg_jacc "\t" msg_jdeny "\t" msg_resume "\t\\\\";

			}
			' tempresults >> data/"$d_parameter"."$topologies"."$algo_name".dat

			rm tempresults
			
			#cp snapshot.pdf snapshots/$item.$d_parameter.pdf
		done

		cat data/"$d_parameter"."$topologies"."$algo_name".dat
	fi
