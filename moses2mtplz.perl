#!/usr/bin/env perl

use strict;

while (my $line = <STDIN>) {
  chomp($line);
  my @cols = split(/\|/, $line);
  print $cols[0] ."|||" .$cols[3];
  print "||| ";
	 
  my @scores = split(" ", $cols[6]);
	#print STDERR "num scores=" .scalar(@scores);
  for (my $i = 0; $i < scalar(@scores); ++$i) {
 	  print log($scores[$i]) ." ";
  }
  print "\n";
}

