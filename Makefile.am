## Process this file with automake to produce Makefile.in

@SET_MAKE@
AUTOMAKE_OPTIONS = 1.4 no-dependencies

always_built_SUBDIRS = lib src man

SUBDIRS = $(always_built_SUBDIRS)

EXTRA_DIST = ChangeLog.1 ChangeLog.2 ChangeLog.3 README-ja sufary.spec \
*.dsp *.dsw

EXTRA_DIRS = doc misc kwicview perl ruby

pkgdatadir = @datadir@/@PACKAGE@

install-data-local:
	$(mkinstalldirs) $(pkgdatadir)/etc
	for subdir in $(EXTRA_DIRS); do \
	    cp -rp $(srcdir)/$$subdir $(pkgdatadir); \
	    rm -rf $(pkgdatadir)/$$subdir/CVS; \
	    rm -rf $(pkgdatadir)/$$subdir/*/CVS; \
	    rm -f $(pkgdatadir)/$$subdir/Makefile*; \
	done
	cp ChangeLog* COPYING NEWS README README-ja TODO \
		$(pkgdatadir)/etc

dist-hook:
	for subdir in $(EXTRA_DIRS); do \
	    cp -rp $$subdir $(distdir)/$$subdir; \
	    rm -f $(distdir)/$$subdir/*~; \
	    rm -f $(distdir)/$$subdir/*.{bak,orig}; \
	    rm -rf $(distdir)/$$subdir/CVS; \
	    rm -rf $(distdir)/$$subdir/*/CVS; \
	done

##Bug in automake: Can't use `if MAINTAINER_MODE'
@MAINTAINER_MODE_TRUE@include $(srcdir)/maintMakefile
