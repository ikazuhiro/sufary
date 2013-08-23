#!/usr/local/bin/perl

use ExtUtils::MakeMaker;

WriteMakefile(
	      'NAME' => 'SUFARY',
	      'INC' => "-Wall -I../../lib",
	      'OBJECT' => "SUFARY.o ../../lib/.libs/libsufary.a"
);
