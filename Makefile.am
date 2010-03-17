# $Id: Makefile.am,v 1.37 2006/08/28 19:13:04 jonz Exp $
#
# Makefile.am
# Jonathan A. Zdziarski <zdziarski@mitre.org>
#
# autoconf (GNU Autoconf) 2.59
# ltmain.sh (GNU libtool) 1.5.6
# automake (GNU automake) 1.9.2

SUBDIRS = .
DIST_SUBDIRS = .

etcdir = $(sysconfdir)
libversion = 1:0:0

EXTRA_DIST = autogen.sh test-GET.sh test-POST.sh test-DELETE.sh test-PUT.sh \
    hstore.conf README

MAINTAINERCLEANFILES = Makefile.in aclocal.m4 auto-config.h.in \
	config.guess config.sub configure depcomp install-sh   \
	ltmain.sh missing mkinstalldirs

lib_LTLIBRARIES = libhstore_plugins.la

bin_PROGRAMS = hstore

include_HEADERS = config.h \
                  hstore.h \
                  log.h \
                  util.h

libhstore_plugins_la_SOURCES = hstore_plugins.c

#   current:revision:age
libhstore_plugins_la_LDFLAGS = -rpath '$(libdir)' -version-info $(libversion)

hstore_SOURCES = \
    hstore.c  hstore.h \
    log.c     log.h \
    config.c  config.h \
    util.c    util.h

hstore_LDFLAGS = -static

ACLOCAL_AMFLAGS = -I m4
