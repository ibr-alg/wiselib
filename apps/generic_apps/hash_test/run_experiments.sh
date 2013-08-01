#!/bin/bash

HASHES="fnv32 fnv64 jenkins murmur larson"
HASH="./out/pc/hash_test"
ELEMENTS="./elements"
INPUT_PATH=/home/henning/data/billion_triple_challenge/projects/btc-2012/datahub
INPUT_FILES="$(echo $INPUT_PATH/data-?.nq)"
#INPUT_FILES="$INPUT_PATH/data-0.nq.gz $INPUT_PATH/data-4.nq.gz"
DATA_PATH="./data/datahub"

function find_collisions { 
	echo '{'
	for input_file in $INPUT_FILES; do
		fn=$(basename $input_file)
		echo '"'$fn'": {'
		#echo '  "tuples": ' $(gunzip -c $input_file | wc -l | awk '{ print $1 }') ','
		echo '  "tuples": ' $(wc -l $input_file | awk '{ print $1 }') ','
		#gunzip -c $input_file | $BINARY_PATH/$ELEMENTS > $DATA_PATH/${fn}.elements
		$ELEMENTS < $input_file > $DATA_PATH/${fn}.elements
		
		echo '  "elements_total": ' $(wc -l $DATA_PATH/${fn}.elements | awk '{ print $1 }') ','
		sort -u $DATA_PATH/${fn}.elements > $DATA_PATH/${fn}.elements.unique
		echo '  "elements_unique": ' $(wc -l $DATA_PATH/${fn}.elements.unique | awk '{ print $1 }') ','
		
		
		echo '  "hashes": {'
		comma=""
		for hashfn in $HASHES; do
			echo $comma'     "'$hashfn'": {'
			
			TIMEFORMAT='  "hash_time_real": %R, "hash_time_user": %U, "hash_time_sys": %S,'
			( time $HASH $hashfn < $DATA_PATH/${fn}.elements.unique > $DATA_PATH/${fn}.${hashfn}) 2>&1
			sort $DATA_PATH/${fn}.${hashfn} > $DATA_PATH/${fn}.${hashfn}.sorted
			
			collisions=0
			collision_hashes=0
			uniq -cd $DATA_PATH/${fn}.${hashfn} | while read n v; do
				collisions=$(($collisions + $n - 1))
				collision_hashes=$(($collision_hashes + 1))
			done
			echo '        "collisions": ' $collisions ','
			echo '        "hashes": ' $collision_hashes
			echo '      }'
			comma=","
		done
		
		#echo '  "hash_collisions": {'
		#comma=""
		#for hashfn in $HASHES; do
			#echo $comma'     "'$hashfn'": ['
			#$BINARY_PATH/$hashfn < $DATA_PATH/${fn}.elements.unique > $DATA_PATH/${fn}.${hashfn}
			#sort $DATA_PATH/${fn}.${hashfn} > $DATA_PATH/${fn}.${hashfn}.sorted
			#comma=""
			#uniq -cd $DATA_PATH/${fn}.${hashfn}.sorted | tee blah.log | while read n v; do
				#echo $comma'        { "hash_value": '$v', "occurences": '$n'},'
				#comma=","
			#done
			#echo '   ]'
			#comma=","
		#done
		#echo '  } '
		
		echo '},'
	done
	
	echo '"__all__": {'
		elements_total=0
		elements_files=""
		for input_file in $INPUT_FILES; do
			fn=$(basename $input_file)
			elements_total=$(($elements_total + $(wc -l $DATA_PATH/${fn}.elements | awk '{ print $1 }') ))
			elements_files="$elements_files $DATA_PATH/${fn}.elements.unique"
			
		done
		echo '  "elements_total": ' $elements_total ','
		sort -mu $elements_files > $DATA_PATH/__all__.elements.unique 
		echo '  "elements_unique": ' $(wc -l $DATA_PATH/__all__.elements.unique | awk '{ print $1 }') ','
		
		
		echo '  "hash_collisions": {'
		
		comma=""
		for hashfn in $HASHES; do
			echo $comma'     "'$hashfn'": {'
			
			TIMEFORMAT='  "hash_time_real": %R, "hash_time_user": %U, "hash_time_sys": %S,'
			( time $HASH $hashfn < $DATA_PATH/__all__.elements.unique > $DATA_PATH/__all__.${hashfn}) 2>&1
			sort $DATA_PATH/__all__.${hashfn} > $DATA_PATH/__all__.${hashfn}.sorted
			
			collisions=0
			collision_hashes=0
			uniq -cd $DATA_PATH/__all__.${hashfn} | while read n v; do
				collisions=$(($collisions + $n - 1))
				collision_hashes=$(($collision_hashes + 1))
			done
			echo '  "collisions": ' $collisions ','
			echo '  "hashes": ' $collision_hashes ','
			echo '  }'
			comma=","
		done
		
		echo '  }'
		
		#echo '  "hash_collisions": {'
		
		#comma=""
		#for hashfn in $HASHES; do
			#echo $comma'     "'$hashfn'": ['
			#$BINARY_PATH/$hashfn < $DATA_PATH/__all__.elements.unique > $DATA_PATH/__all__.${hashfn}
			#sort $DATA_PATH/__all__.${hashfn} > $DATA_PATH/__all__.${hashfn}.sorted
			#comma=""
			#uniq -cd $DATA_PATH/__all__.${hashfn}.sorted | while read n v; do
				#echo $comma'        { "hash_value": '$v', "occurences": '$n'},'
				#comma=","
			#done
			#echo ' ]'
			#comma=","
		#done
		#echo '  } }'
	
	echo '}'
}

find_collisions

