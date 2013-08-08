#!/bin/bash

HASHES=" \
bernstein8 bernstein16 bernstein32 bernstein64 \
bernstein2_8 bernstein2_16 bernstein2_32 bernstein2_64 \
elf32 firstchar8 fletcher16 \
fnv1_16 fnv1_32 fnv1_64 \
fnv1a_16 fnv1a_32 fnv1a_64 \
lookup2_32 lookup3_32 oneatatime_32 \
kr8 kr16 kr32 kr64 \
larson8 larson16 larson32 larson64 \
novak8 novak16 novak32 novak64 \
sdbm8 sdbm16 sdbm32 sdbm64 \
"
#HASH="./hash_test"
HASH="./out/pc/hash_test"
ELEMENTS="./elements"
#INPUT_PATH=/home/henning/data/ec2_hashes_2013_08_03
INPUT_PATH=/home/henning/annexe/rdf_datasets/btc2012/projects/btc-2012/datahub
#INPUT_PATH=/mnt
#INPUT_PATH=/home/henning/data/billion_triple_challenge/projects/btc-2012/datahub
#INPUT_FILES="$(echo $INPUT_PATH/data-?.nq)"
#INPUT_FILES="$INPUT_PATH/data-0.nq.gz $INPUT_PATH/data-4.nq.gz"
DATA_PATH=data/ec2
#DATA_PATH="./data/datahub"


function find_collisions { 
	echo '{'
	for fn in __all__; do
	#for input_file in $INPUT_FILES; do
		#fn=$(basename $input_file)
		echo '"'$fn'": {'
		#echo '  "tuples": ' $(gunzip -c $input_file | wc -l | awk '{ print $1 }') ','
#		echo '  "tuples": ' $(wc -l $input_file | awk '{ print $1 }') ','
		#gunzip -c $input_file | $BINARY_PATH/$ELEMENTS > $DATA_PATH/${fn}.elements
#		$ELEMENTS < $input_file > $DATA_PATH/${fn}.elements
		
#		echo '  "elements_total": ' $(wc -l $INPUT_PATH/${fn}.elements | awk '{ print $1 }') ','
#		sort -u $INPUT_PATH/${fn}.elements > $INPUT_PATH/${fn}.elements.unique
		export elements_unique=$(wc -l $INPUT_PATH/${fn}.elements.unique | awk '{ print $1 }')
		echo '  "elements_unique": ' $elements_unique ','
		
		echo '  "hashes": {'
		comma=""
		for hashfn in $HASHES; do
			echo $comma'     "'$hashfn'": {'
			
			bits=32
			if [[ $hashfn == *8 ]]; then
				bits=8
			elif [[ $hashfn == *16 ]]; then
				bits=16
			elif [[ $hashfn == *64 ]]; then
				bits=64
			fi
				
			echo '        "bits": ' $bits ','
			export total_hashes=$(echo "2 ^ $bits" |bc -q)
			echo '        "total_hashes": ' $total_hashes ','
			
			TIMEFORMAT='  "hash_time_real": %R, "hash_time_user": %U, "hash_time_sys": %S,'
			( time $HASH $hashfn < $INPUT_PATH/${fn}.elements.unique > $DATA_PATH/${fn}.${hashfn}) 2>&1
			sort $DATA_PATH/${fn}.${hashfn} > $DATA_PATH/${fn}.${hashfn}.sorted
			
			export sum=0
			export sqsum=0
			export collisions=0
			export collision_hashes=0
			export elems=0;
			export hashes=0
			# uniq -cd
			uniq -c $DATA_PATH/${fn}.${hashfn}.sorted > $DATA_PATH/${fn}.${hashfn}.collisions
			
			awk 'BEGIN { collisions=0; collision_hashes=0; sum=0; sqsum=0; hashes=0 } \
				{ if($1 > 1) { collisions += $1 - 1; collision_hashes++; } \
					hashes++; sum += $1; sqsum += $1 * $1; } \
				END { print collisions, collision_hashes, hashes, sum, sqsum }' < $DATA_PATH/${fn}.${hashfn}.collisions >/tmp/awk.out 
			
			read collisions collision_hashes hashes sum sqsum < /tmp/awk.out
			
			#while read n v; do
				#if [ "$n" -gt 1 ]; then
					#export collisions=$(echo "$collisions + $n - 1" |bc -q)
					#export collision_hashes=$(echo "$collision_hashes + 1" |bc -q)
				#fi
				#export hashes=$(echo "$hashes + 1" |bc -q)
				##export elems=$(echo "$elems + 1" |bc -q)
				#export sum=$(echo "$sum + $n" |bc -q)
				#export sqsum=$(echo "$sqsum + $n * $n" |bc -q)
			#done < $DATA_PATH/${fn}.${hashfn}.collisions
			
			
			export noncolliding=$(echo "$total_hashes - $hashes"|bc -q)
			
			echo '        "used_hashes": ' $hashes ','
			echo '        "unused_hashes": ' $(echo "$total_hashes - $hashes"| bc -q) ','
			echo '        "collisions": ' $collisions ','
			echo '        "colliding_hashes": ' $collision_hashes ','
			echo '        "non_colliding_hashes": ' $noncolliding ','
			echo '        "values_per_hash_mean": ' $(echo "scale=20; $sum / $total_hashes" |bc -q) ','
			echo '        "values_per_hash_variance": ' $(echo "scale=20; ($sqsum / $total_hashes) - (($sum * $sum) / ($total_hashes * $total_hashes))" |bc -q)
			echo '      }'
			comma=","
		done
		
		echo '}}'
	done
	echo '}'
	
	#echo '"__all__": {'
		#elements_total=0
		#elements_files=""
		#for input_file in $INPUT_FILES; do
			#fn=$(basename $input_file)
			#elements_total=$(($elements_total + $(wc -l $DATA_PATH/${fn}.elements | awk '{ print $1 }') ))
			#elements_files="$elements_files $DATA_PATH/${fn}.elements.unique"
			
		#done
		#echo '  "elements_total": ' $elements_total ','
		#sort -mu $elements_files > $DATA_PATH/__all__.elements.unique 
		#echo '  "elements_unique": ' $(wc -l $DATA_PATH/__all__.elements.unique | awk '{ print $1 }') ','
		
		
		#echo '  "hash_collisions": {'
		
		#comma=""
		#for hashfn in $HASHES; do
			#echo $comma'     "'$hashfn'": {'
			
			#TIMEFORMAT='  "hash_time_real": %R, "hash_time_user": %U, "hash_time_sys": %S,'
			#( time $HASH $hashfn < $DATA_PATH/__all__.elements.unique > $DATA_PATH/__all__.${hashfn}) 2>&1
			#sort $DATA_PATH/__all__.${hashfn} > $DATA_PATH/__all__.${hashfn}.sorted
			
			#collisions=0
			#collision_hashes=0
			#uniq -cd $DATA_PATH/__all__.${hashfn}.sorted > $DATA_PATH/__all__.${hashfn}.collisions
			#while read n v; do
				#collisions=$(($collisions + $n - 1))
				#collision_hashes=$(($collision_hashes + 1))
			#done < $DATA_PATH/__all__.${hashfn}.collisions
			#echo '  "collisions": ' $collisions ','
			#echo '  "hashes": ' $collision_hashes ','
			#echo '  }'
			#comma=","
		#done
		
		#echo '  }'
		
		##echo '  "hash_collisions": {'
		
		##comma=""
		##for hashfn in $HASHES; do
			##echo $comma'     "'$hashfn'": ['
			##$BINARY_PATH/$hashfn < $DATA_PATH/__all__.elements.unique > $DATA_PATH/__all__.${hashfn}
			##sort $DATA_PATH/__all__.${hashfn} > $DATA_PATH/__all__.${hashfn}.sorted
			##comma=""
			##uniq -cd $DATA_PATH/__all__.${hashfn}.sorted | while read n v; do
				##echo $comma'        { "hash_value": '$v', "occurences": '$n'},'
				##comma=","
			##done
			##echo ' ]'
			##comma=","
		##done
		##echo '  } }'
	
	#echo '}'
}

find_collisions

