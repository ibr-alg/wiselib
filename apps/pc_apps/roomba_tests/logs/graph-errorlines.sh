#!/bin/bash

fit_and_plot() {
  FILE=$1
  echo ==================================
  echo Fitting $FILE...
  #FIT=`echo "data <- read.table('$FILE'); lm(data[,3] ~ data[,1] + data[,2])"|R --vanilla --batch|grep -E '^( +[-0-9.]+){3}'`
  #INTERCEPT=`echo $FIT|awk '{ print $1; }'`
  #X=`echo $FIT|awk '{ print $2; }'`
  #Y=`echo $FIT|awk '{ print $3; }'`
  #echo intercept: $INTERCEPT, x: $X, y: $Y
  if [ -n "$TOFILE" ]; then
    OUTFILE=${FILE//./_}.pdf
    TOFILE="set term pdf; set output '$OUTFILE'";
    echo "Writing to file $OUTFILE";
  fi
  echo > .$FILE.gp "$TOFILE
set title '$2'
set logscale x
set xlabel 'input [mm]'
set ylabel 'error of measured value [mm]'
plot \\
  '$FILE' index 0 using 2:5:3:4 title 'velocity=20' with yerrorlines, \\
  '$FILE' index 1 using 2:5:3:4 title 'velocity=50' with yerrorlines, \\
  '$FILE' index 2 using 2:5:3:4 title 'velocity=70' with yerrorlines, \\
  '$FILE' index 3 using 2:5:3:4 title 'velocity=100' with yerrorlines, \\
  '$FILE' index 4 using 2:5:3:4 title 'velocity=150' with yerrorlines, \\
  '$FILE' index 5 using 2:5:3:4 title 'velocity=200' with yerrorlines, \\
  '$FILE' index 6 using 2:5:3:4 title 'velocity=300' with yerrorlines, \\
  '$FILE' index 7 using 2:5:3:4 title 'velocity=400' with yerrorlines
"
  gnuplot -persist .$FILE.gp &
}

TITLE="Original behaviour"
PREFIX=
TOFILE=
while [ "$#" -gt 0 ]; do
  echo test: $1
  case "$1" in
    --help|-h)
      echo "Usage: $0 [--help|-h|--mean|-m|--soft|-s] [--to-file|-o]"
      exit
      ;;
    --mean|-m)
      TITLE="Arithmetic Mean Correction"
      PREFIX=-mean
      ;;
    --soft|-s)
      TITLE="Soft Start/Stop"
      PREFIX=-soft
      ;;
    --to-file|-o)
      TOFILE="1"
      ;;
    *)
      ;;
  esac
  shift
done;

cat roomba$PREFIX-drive-* | grep seminarraum | sed 's/[a-z_]\+=//g' | awk '{ print $14 " " $13 " " $15 }' | sort -V | ./aggregate-minmax.pl > seminarraum.drive$PREFIX.data
fit_and_plot "seminarraum.drive$PREFIX.data" "$TITLE, carpet floor, straight drive"

cat roomba$PREFIX-turn-* | grep seminarraum | sed 's/[a-z_]\+=//g' | awk '{ print $15 " " $13 " " $14 }' | sort -V | ./aggregate-minmax.pl > seminarraum.turn$PREFIX.data
fit_and_plot "seminarraum.turn$PREFIX.data" "$TITLE, carpet floor, turn on spot"

cat roomba$PREFIX-drive-* | grep iz250flur | sed 's/[a-z_]\+=//g' | awk '{ print $14 " " $13 " " $15 }' | sort -V | ./aggregate-minmax.pl > iz250flur.drive$PREFIX.data
fit_and_plot "iz250flur.drive$PREFIX.data" "$TITLE, laminate floor, straight drive"

cat roomba$PREFIX-turn-* | grep iz250flur | sed 's/[a-z_]\+=//g' | awk '{ print $15 " " $13 " " $14 }' | sort -V | ./aggregate-minmax.pl > iz250flur.turn$PREFIX.data
fit_and_plot "iz250flur.turn$PREFIX.data" "$TITLE, laminate floor, turn on spot"

