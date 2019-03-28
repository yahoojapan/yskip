#!/usr/bin/env perl

use strict;
use warnings;
use utf8;
use Encode;

binmode STDIN, ":utf8";
binmode STDOUT, ":utf8";
binmode STDERR, ":utf8";


&main();


sub main {

    my $min_count = 1;
    for (my$i = 0; $i < @ARGV;) {
	if ($ARGV[$i] eq "--min-count") {
	    $min_count = $ARGV[$i+1];
	    $i += 2;
	}else {
	    print STDERR "$0 [--min-count MIN_COUNT]\n";
	    return 1;
	}
    }
    
    my $line_num = 0;
    while (<STDIN>) {
	chomp;
	++$line_num;
	if ($line_num == 1) {
	    my ($vocab_size, $max_vocab_size, $vec_size, $window_size, $neg_sample_num, $alpha, $subsampling_threshold, $eta, $unigram_table_size) = split(/\t/, $_);
	    print "$vocab_size $vec_size\n";
	}else {
	    my ($word, $count, $in, $out) = split(/\t/, $_);
	    if ($min_count <= $count) {
		my @vec_in = split(/ /,  $in);
		my @vec_out = split(/ /,  $out);
		print "$word";
		for (my$i = 0; $i < @vec_in; ++$i) {
		    print " ",($vec_in[$i]+$vec_out[$i])/2;
		}
		print "\n";
	    }
	}
    }
}
