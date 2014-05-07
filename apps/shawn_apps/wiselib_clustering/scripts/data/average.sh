#!/bin/bash
algo=$1
for file in *.$algo.dat
do
	awk '
	BEGIN{
		nodes = 0;
		counter = 0;

		}
	
		{
			if ($1=="#Total"){
				for(i = 1; i < NF; i++)
					values[i]=0;
				for(i=1;i<NF;i++)
					printf("%s\t",$i);
				printf(" \\\\ \n");

			}
			if ($1!="#Total"){
				if (nodes==0){
					nodes=$1;
					counter=0;
				}
				if (nodes==$1){
					for(i = 1; i < NF; i++)
						values[i]+=$i;					
					counter++;
				}
				else{
					for(i = 1; i < NF; i++)
						printf("%.1f \t",values[i]/counter);
					printf(" \\\\ \n");
					for(i = 1; i < NF; i++)
						values[i]=$i;	
					nodes=$1;		
					counter=1;		
					
				}
			}	
		}
	END{
					for(i = 1; i < NF; i++)
						printf("%.1f \t",values[i]/counter);
					printf(" \\\\ \n");
		}
	'	$file > $file.avg
	
	echo "Average values for file $file saved in file $file.avg"
	
done
