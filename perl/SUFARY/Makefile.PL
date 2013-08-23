#!/usr/local/bin/perl

use ExtUtils::MakeMaker;

WriteMakefile(
	      'NAME' => 'SUFARY',
	      'INC' => "-Wall -I../../lib",
	      'LIBS' => ["-L. -L../../lib/.libs -lsufary"],
	      'OBJECT' => "SUFARY.o"
);
