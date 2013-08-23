# $Id: Makefile.am,v 1.20 2000/11/28 07:56:42 tatuo-y Exp $ 
AUTOMAKE_OPTIONS = no-dependencies

include_HEADERS = sufary.h
lib_LTLIBRARIES = libsufary.la
libsufary_la_SOURCES = file.c search.c regex.c \
make-index.c make-index.h \
index-point.c \
region.c sufary.h \
util.h util.c \
mmp.h mmp.c \
my-malloc.h my-malloc.c \
did.h did.c make-did.c 

noinst_PROGRAMS = test
test_SOURCES = test.c
test_LDADD   = libsufary.la

libsufary_la_LDFLAGS = -version-info 0:0:0
