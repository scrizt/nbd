EXTRA_PROGRAMS = make-integrityhuge
TESTS_ENVIRONMENT=$(srcdir)/simple_test
AM_CPPFLAGS = -I$(top_srcdir)/src
TESTS = cmd cfg1 cfgmulti cfgnew cfgsize write flush integrity dirconfig list #integrityhuge
check_PROGRAMS = nbd-tester-client
nbd_tester_client_SOURCES = nbd-tester-client.c cliserv.h netdb-compat.h
nbd_tester_client_CFLAGS = @CFLAGS@ @GLIB_CFLAGS@
nbd_tester_client_LDADD = @GLIB_LIBS@
make_integrityhuge_SOURCES = make-integrityhuge.c cliserv.h nbd.h
EXTRA_DIST = simple_test integrity-test.tr integrityhuge-test.tr maketr
cmd:
cfg1:
cfgmulti:
cfgnew:
cfgsize:
write:
flush:
integrity:
integrityhuge:
dirconfig:
list:
