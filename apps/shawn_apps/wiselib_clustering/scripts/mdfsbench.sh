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
		echo -en "#Total\theads\tsimple\trounds\ttime\tcl.diam\tavgsize\tmsg\tDISC\tREPLY\tJ_REQ\tJ_REJ\tRESUME\tFLOOD\tCHD\tJD\tIT\tmCHD\tmJD\tmIT\\" > data/"$d_parameter"."$topologies"."$algo_name".dat
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
				msg_disc=0;
				msg_reply=0;
				msg_jreq=0;
				msg_jdeny=0;
				msg_resume=0;
				timestart=0;
				timeend=0;
				chd_rounds=0;
				jd_rounds=0;
				it_rounds=0;
				chd_mess=0;
				jd_mess=0;
				it_mess=0;
				curr_it=0;
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
					if ($2 ~ "NEIGHBOR_DISCOVERY"){
						msg_disc++;
						jd_rounds=cur_it+1;
						
					}
					else if ($2 ~ "NEIGHBOR_REPLY"){
						msg_reply++;
						jd_rounds=cur_it+1;
						
					}
					else if ($2 ~ "JOIN_DENY"){
						msg_jdeny++;
						jd_rounds=cur_it+1;
						
					}
					else if ($2 ~ "JOIN_REQUEST"){
						msg_jreq++;
						jd_rounds=cur_it+1;
						
					}
					else if ($2 ~ "RESUME"){
						msg_resume++;
						
					}
					else if ($2 ~ "FLOOD"){
						msg_flood++;
					}
					
				}
				if ($1 ~ "TIMESTARTED"){
					timestart = $2;
				}
				if ($1 ~ "TIMEENDED"){
					timeend = $2;
				}
				if ($2 ~ "CHEAD"){
					if ($3 ~ "DECIDED"){
						chd_rounds=$5;
						chd_rounds++;
					}
				}
				if ($3 ~ "ITERATION"){
					cur_it=$4;
				}	
				if ($2 ~ "FORMED"){
					it_rounds=cur_it+1;	
				}

			} \
			END	{
				avg_size = total_nodes/heads;
				time_needed = timeend-timestart;
				hops=hops/simple;
				jd_rounds-=chd_rounds;
				it_rounds-=jd_rounds;
				it_rounds-=chd_rounds;		
				chd_mess=msg_flood;
				jd_mess= msg_disc+msg_reply+msg_jreq+msg_jdeny;
				it_mess= msg_resume;
				print total_nodes "\t" heads "\t" simple "\t" round_finished "\t" time_needed "\t" hops "\t" avg_size "\t" msg_ "\t" msg_disc "\t" msg_reply "\t" msg_jdeny "\t" msg_jreq "\t" msg_resume "\t" msg_flood "\t" chd_rounds "\t" jd_rounds "\t" it_rounds "\t" chd_mess "\t" jd_mess "\t" it_mess "\t\\\\";

			}
			' "$algo_name"tempresults >> data/"$d_parameter"."$topologies"."$algo_name".dat

			rm "$algo_name"tempresults
			
			#cp snapshot.pdf snapshots/$item.$d_parameter.pdf
		done

		cat data/"$d_parameter"."$topologies"."$algo_name".dat
	fi
