#!/usr/bin/perl -w
# input: sorted(!) lines in the form "$val1 $val2 $val3 ..."
# output: sorted lines in the form "$val1 $val2 $val3 ..."
#  with blocks of different $val1 separated by two blank lines
my $in1;
my $cur_in1;
my @inN;
my $first = 1;
while(<>) {
  ($in1, @inN) = split / /;
  if($first) {
    $cur_in1 = $in1;
    $first = 0;
  }
  if($in1 != $cur_in1) {
    print "\n\n";  # new gnuplot record
    $cur_in1 = $in1;
  }
  print "$in1 @inN";
}
