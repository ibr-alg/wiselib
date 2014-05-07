#!/bin/sh

F_INPUT=$1
ELEMENTS=out/pc/hash_test
F_ELEMENTS=$1.elements.uniq_c

zcat $F_INPUT|$ELEMENTS|sort|uniq -c > $F_ELEMENTS
sort -rn $F_ELEMENTS > $F_ELEMENTS.sorted

