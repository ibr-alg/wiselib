#!/bin/sh
for f in "seminarraum" "iz250flur"; do
  cat roomba-soft-drive-* | grep $f | sed 's/[a-z_]\+=//g' | awk '{ print $13 " " $14 " " $15 }' | sort -V | ./aggregate.pl > $f.soft.drive.data
  cat roomba-mean-drive-* | grep $f | sed 's/[a-z_]\+=//g' | awk '{ print $13 " " $14 " " $15 }' | sort -V | ./aggregate.pl > $f.mean.drive.data
  gnuplot -p -e "set xlabel 'input'; set ylabel 'velocity'; set zlabel 'measured'; ideal(x,y)=0; splot ideal(x,y) with lines, '$f.soft.drive.data' with lines, '$f.mean.drive.data' with lines" &

  cat roomba-soft-turn-* | grep $f | sed 's/[a-z_]\+=//g' | awk '{ print $13 " " $15 " " $14 }' | sort -V | ./aggregate.pl > $f.soft.turn.data
  cat roomba-mean-turn-* | grep $f | sed 's/[a-z_]\+=//g' | awk '{ print $13 " " $15 " " $14 }' | sort -V | ./aggregate.pl > $f.mean.turn.data
  gnuplot -p -e "set xlabel 'input'; set ylabel 'velocity'; set zlabel 'measured'; ideal(x,y)=0; splot ideal(x,y) with lines, '$f.soft.turn.data' with lines, '$f.mean.turn.data' with lines" &
done;
