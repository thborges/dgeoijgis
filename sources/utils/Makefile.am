AUTOMAKE_OPTIONS = subdir-objects

lib_LIBRARIES = librtreeutils.a

AM_CPPFLAGS = $(DEPS_CFLAGS) $(GDAL_CFLAGS) -I../utils

librtreeutils_a_SOURCES = \
	./ogrext.cpp \
	./utils.c \
	./dataset.c \
	./histcommon.c \
	./histogram_grid.c \
	./geosext.cpp \
	./wkbconvert.c \
	./glibwrap.c \
	./dataset_persist.c \
	./joincommon.c

if !ENABLE_CPLEX
librtreeutils_a_SOURCES += glpk_cplex_wrap.c
endif
