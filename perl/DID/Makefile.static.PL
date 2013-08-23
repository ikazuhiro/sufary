#!/usr/local/bin/perl

use ExtUtils::MakeMaker;

WriteMakefile(
	      'NAME' => 'DID',
	      'INC' => "-Wall -I../../lib",
	      'OBJECT' => "DID.o ../../lib/.libs/libsufary.a"
);
