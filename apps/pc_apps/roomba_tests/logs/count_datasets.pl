#!/usr/bin/perl -w
use strict;
use warnings;
use Data::Dumper;

my %data; # groundtype => move => input => velocity
my ($gnd,$move,$in,$velo);

while (<>) {
  if (/\bground_type=([^ ]+) .*\bmove=([^ ]+) .*\b(turn_angle|input_distance)=([^ ]+) .*\bvelocity=([^ ]+) .*/) {
    #print "groundtype $1 move $2 input $4 velocity $5\n";
    $data{$1}{$2}{$4}{$5} += 1;
  } else {
    print "wrong line format! $_\n";
  }
}

#print Dumper(\%data);

foreach $gnd (keys %data) {
  foreach $move (keys %{$data{$gnd}}) {
    print "$gnd, $move:\n";
    foreach $in (sort { $a <=> $b } keys %{$data{$gnd}{$move}}) {
      foreach $velo (sort { $a <=> $b } keys %{$data{$gnd}{$move}{$in}}) {
        print sprintf "  in=%-4d, velo=%-3d: %-2d datapoints\n", $in, $velo, $data{$gnd}{$move}{$in}{$velo};
      }
    }
    print "\n";
  }
}

