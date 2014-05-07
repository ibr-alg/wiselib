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
		echo -en "#Total\theads\tsimple\trounds\ttime\tavgsize\tmsg\tJOIN\tREQ\tCHD\tJD\tIT\tmCHD\tmJD\tmIT\t\\" > data/"$d_parameter"."$topologies"."$algo_name".dat
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
				uncovered_nodes = 0;
				over_clusters=0;
				msg_=0;
				msg_join=0;
				msg_req=0;
				timestart=0;
				timeend=0;
				chd_round=0;
				jd_round=0;
				it_round=0;
				chd_mess=0;
				jd_mess=0;
				it_mess=0;
				curr_it=0;

			} \
			{
				if ($2 ~ "RESULTS"){
					if ($7 == "2"){
						heads++;
					}
					else {
						simple++;
					}
					total_nodes++;
				}
				
				if ($1 ~ "real"){
					time_needed = $2;
				}
				
				if ($1 ~ "SEND"){
					msg_++;
					if ($2 ~ "JOIN_REQUEST"){
						msg_req++;
						
					}
					else if ($2 ~ "JOIN"){
						msg_join++;
						
					}
				}
				if ($1 ~ "TIMESTARTED"){
					timestart = $2;
				}
				if ($1 ~ "TIMEENDED"){
					timeend = $2;
				}
				if ($3 ~ "ITERATION"){
					curr_it=$4+1;
				}
				if ($2 ~ "Finished"){
					round_finished=curr_it;
				}

				if ($1 ~ "stage"){
					if ($2 ~ "2"){
						chd_rounds=curr_it;
					}
					else if ($2 ~ "3"){
						jd_rounds=curr_it;
					}
				}
				
				if ($2 ~ "UNCOVERED"){
					uncovered_nodes++;
				}
				if ($1 ~ "kCluster"){
					over_clusters++;
				}
				


			} \
			END	{
				avg_size = total_nodes/heads;
				time_needed = timeend-timestart;
				it_rounds= round_finished - jd_rounds;
				jd_rounds-=chd_rounds;
				chd_mess=0;
				jd_mess=msg_join;
				it_mess=msg_req;				
				print total_nodes "\t" heads "\t" simple "\t" round_finished "\t" time_needed "\t" avg_size "\t" msg_ "\t" msg_join "\t" msg_req "\t" chd_rounds "\t" jd_rounds "\t" it_rounds "\t" chd_mess "\t" jd_mess "\t" it_mess "\t" uncovered_nodes "\t" over_clusters/heads "\t\\\\";

			}
			' tempresults >> data/"$d_parameter"."$topologies"."$algo_name".dat

			rm tempresults
			
			#cp snapshot.pdf snapshots/$item.$d_parameter.pdf
		done

		cat data/"$d_parameter"."$topologies"."$algo_name".dat
	fi
