#!/usr/local/bin/perl

use ExtUtils::MakeMaker;

WriteMakefile(
	      'NAME' => 'DID',
	      'INC' => "-Wall -I../../lib",
	      'LIBS' => ["-L. -L../../lib/.libs -lsufary"],
	      'OBJECT' => "DID.o"
);
