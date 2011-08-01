#!/bin/sh
INTERCEPT=
X=
Y=

fit() {
  FILE=$1
  echo ==================================
  echo Fitting $FILE...
  FIT=`echo "data <- read.table('$FILE'); lm(data[,3] ~ data[,1] + data[,2])"|R --vanilla --batch|grep -E '^( +[-0-9.]+){3}'`
  INTERCEPT=`echo $FIT|awk '{ print $1; }'`
  X=`echo $FIT|awk '{ print $2; }'`
  Y=`echo $FIT|awk '{ print $3; }'`
  echo intercept: $INTERCEPT, x: $X, y: $Y
}
  
for f in "seminarraum" "iz250flur"; do
  cat roomba-drive-* | grep $f | sed 's/[a-z_]\+=//g' | awk '{ print $13 " " $14 " " $15 }' | sort -V | ./aggregate.pl > $f.drive.data
  cat roomba-soft-drive-* | grep $f | sed 's/[a-z_]\+=//g' | awk '{ print $13 " " $14 " " $15 }' | sort -V | ./aggregate.pl > $f.soft.drive.data
  fit "$f.drive.data";
  gnuplot -p -e "set xlabel 'input'; set ylabel 'velocity'; set zlabel 'measured'; fit(x,y)=$INTERCEPT+$X*x+$Y*y; ideal(x,y)=0; splot '$f.drive.data' with lines, '$f.soft.drive.data' with pm3d, fit(x,y) with lines, ideal(x,y) with lines" &

  cat roomba-turn-* | grep $f | sed 's/[a-z_]\+=//g' | awk '{ print $13 " " $15 " " $14 }' | sort -V | ./aggregate.pl > $f.turn.data
  cat roomba-soft-turn-* | grep $f | sed 's/[a-z_]\+=//g' | awk '{ print $13 " " $15 " " $14 }' | sort -V | ./aggregate.pl > $f.soft.turn.data
  fit "$f.turn.data";
  gnuplot -p -e "set xlabel 'input'; set ylabel 'velocity'; set zlabel 'measured'; fit(x,y)=$INTERCEPT+$X*x+$Y*y; ideal(x,y)=0; splot '$f.turn.data' with lines, '$f.soft.turn.data' with pm3d, fit(x,y) with lines, ideal(x,y) with lines" &

done;
