#!/usr/bin/perl

&init_labels;
foreach $key (keys $label) {
	print "$key = $label($key)\n";
}

sub init_labels {
    open(LABELS, "labels.pl");
    <LABELS>;
    <LABELS>;
    <LABELS>;
    while (<LABELS>) {
	$key = s/$key = q\/(\w)\/;/\1/;
	chop(<LABELS>);
	$node = s/$external_labels{$key} = '(\w).html'/\1/;
	$label{$key} = $node;
    }
    close(LABELS);
}

