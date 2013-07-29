#!/bin/sh

mkdir -p dot
rm dot/*.dot dot/*.png dot/*.pdf
python ./tree.py

for f in dot/*.dot; do
	echo rendering $f...
	#dot -Tpdf -o dot/$(basename $f .dot).pdf $f &
	dot -Tpng -o dot/$(basename $f .dot).png $f &
done

echo waiting for completion...
wait


