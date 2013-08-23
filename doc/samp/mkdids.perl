#!/usr/local/bin/perl

$SADIR = "/home/tatuo-y/work/sufary/sufary";

while(<>){
    chomp;
    if(/^TXT\s([^\s]+)\s*/){
	$textfile = $1;
	print ">>> TEXT_FILE = $textfile\n";
    }elsif(/^(\d+)\s([^\s]+)\s([^\s]+)\s*/){
	print ">>> $SADIR/mkdid/mkdid -o $3 '<$2>' '</$2>' $textfile\n";
	`$SADIR/mkdid/mkdid -o $3 '<$2>' '</$2>' $textfile`;
    }
}

sub DB {print "@_\n";}
