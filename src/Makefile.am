AM_CPPFLAGGS = -DG_LOG_DOMAIN=\"wm\" -I$(top_srcdir)
AM_CXXFLAGS = -Wall -g -mwindows -std=c++11

bin_PROGRAMS = wm
#wm_LDFLAGS = -L/lib
wm_LDADD = -lcfgmgr32
wm_SOURCES = main.hh main.cc \
	wm.rc resource.h \
	dev.hh dev.cc \
	common.hh common.cc \
	deserializer.hh deserializer.cc \
	window.hh window.cc \
	poshandler.hh poshandler.cc \
	serializer.hh serializer.cc

.rc.o:
	windres $^ -O coff -o $@
%.o : %.rc
	windres $^ -O coff -o $@
