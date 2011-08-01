#!/usr/bin/perl -w
# input: sorted(!) lines in the form "$velocity $input $measured"
# output: sorted lines in the form "$velocity $input min($measured) max($measured) mean($measured)"
my $in = -1;
my $velo = -1;
my $meas = -1;
my $min_meas = 999999;
my $max_meas = 0;
my $cur_in = -1;
my $cur_velo = -1;
my $first = 1;
my $total = 0;
my $num = 0;
while(<>) {
  ($velo, $in, $meas) = split / /;
  if($first) {
    $cur_in = $in;
    $cur_velo = $velo;
    $first = 0;
  }
  if($in != $cur_in or $velo != $cur_velo) {
    print "$cur_velo $cur_in ".int($min_meas)." ".int($max_meas)." ".($total/$num)."\n";
    if($velo != $cur_velo) {
      print "\n\n";  # for stupid gnuplot splot
    }
    $cur_in = $in;
    $cur_velo = $velo;
    $max_meas = 0;
    $min_meas = 999999;
    $num = 0;
    $total = 0;
  }
  $min_meas = (($meas-$cur_in) < $min_meas) ? ($meas-$cur_in) : $min_meas;
  $max_meas = (($meas-$cur_in) > $max_meas) ? ($meas-$cur_in) : $max_meas;
  $total += ($meas - $cur_in);
  $num++;
}
print "$cur_velo $cur_in ".int($min_meas)." ".int($max_meas)." ".($total/$num)."\n" unless ($num == 0);
