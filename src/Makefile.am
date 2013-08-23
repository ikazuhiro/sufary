# $Id: Makefile.am,v 1.7 1999/12/04 23:42:06 satoru-t Exp $ 
AUTOMAKE_OPTIONS = no-dependencies
INCLUDES = -I../lib
bin_PROGRAMS = sass af sang mkdid mkary # array
bin_SCRIPTS  = mkdids

sass_SOURCES = sass.c getopt.c getopt1.c getopt.h
sass_LDADD   = $(top_srcdir)/lib/libsufary.la

af_SOURCES   = af.c
af_LDADD     = $(top_srcdir)/lib/libsufary.la

sang_SOURCES = sang.c
sang_LDADD   = $(top_srcdir)/lib/libsufary.la

mkdid_SOURCES = mkdid.c
mkdid_LDADD   = $(top_srcdir)/lib/libsufary.la

mkary_SOURCES = mkary.c
mkary_LDADD   = $(top_srcdir)/lib/libsufary.la

# array_SOURCES = array-main.c array-misc.c array-show.c array.h
# array_LDADD   = $(top_srcdir)/lib/libsufary.la
# array_LIBS    = -lm

