#!/bin/sh

mkdir -p dot
rm dot/*.dot dot/*.png dot/*.pdf
python ./tree.py $@


COUNT=$(( $(ls dot/*.dot|wc -l) / 10 ))
echo "Rendering  will take > ${COUNT}s (="$(echo "scale=2; $COUNT / 60"|bc)"m ="$(echo "scale=2; $COUNT / 3600"|bc)"h), sure?"
read

for f in dot/*.dot; do
	echo rendering $f...
	#dot -Tpdf -o dot/$(basename $f .dot).pdf $f &
	dot -Tpng -o dot/$(basename $f .dot).png $f &
	sleep .1
done

echo waiting for completion...
wait


