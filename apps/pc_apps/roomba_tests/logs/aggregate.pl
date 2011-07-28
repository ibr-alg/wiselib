#!/usr/bin/perl -w
# input: sorted(!) lines in the form "$input $velocity $measured"
# output: sorted lines in the form "$input $velocity mean($measured)"
my $in = -1;
my $velo = -1;
my $meas = -1;
my $cur_in = -1;
my $cur_velo = -1;
my $first = 1;
my $total = 0;
my $num = 0;
while(<>) {
  ($in, $velo, $meas) = split / /;
  if($first) {
    $cur_in = $in;
    $cur_velo = $velo;
    $first = 0;
  }
  if($in != $cur_in or $velo != $cur_velo) {
    print "$cur_in $cur_velo ".($total / $num)."\n";
    if($in != $cur_in) {
      print "\n";  # for stupid gnuplot splot
    }
    $cur_in = $in;
    $cur_velo = $velo;
    $num = 0;
    $total = 0;
  }
  $total += $meas - $cur_in;
  $num++;
}
print "$cur_in $cur_velo ".($total / $num)."\n" unless ($num == 0);
