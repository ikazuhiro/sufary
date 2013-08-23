# Process this file with automake to produce Makefile.in

man_MANS = sass.1
EXTRA_DIST = $(man_MANS)

update-man:
	LANG=C help2man --no-info ../src/sass | perl -pe s/FSF/SUFARY/ > sass.1


