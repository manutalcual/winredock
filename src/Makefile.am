AM_CPPFLAGGS = -DG_LOG_DOMAIN=\"wm\" -I$(top_srcdir)
AM_CXXFLAGS = -Wall -g -mwindows -std=c++11

bin_PROGRAMS = wm
wm_SOURCES = main.hh main.cc wm.rc resource.h

.rc.o:
	windres $^ -O coff -o $@
%.o : %.rc
	windres $^ -O coff -o $@
