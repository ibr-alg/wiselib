#!/bin/bash

OUT_DIR=shdt_test

mkdir -p $OUT_DIR

for mtu in 20 40 60 80 100 120 140 200 300 500 1000; do
	echo -n "$mtu: "
	for tablesize in $(seq 3 200); do
		echo -n "$tablesize "
		for fn in btcsample0 incontextsensing ssp; do
			out/pc/shdt_cat $mtu $tablesize  < ./${fn}.rdf > $OUT_DIR/${fn}.${mtu}.${tablesize}.shdt &
			out/pc/hshdt_cat $mtu $tablesize < ./${fn}.rdf > $OUT_DIR/${fn}.${mtu}.${tablesize}.hshdt
		done
	done
	echo
done
echo


