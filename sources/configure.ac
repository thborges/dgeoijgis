#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ([2.69])
AC_INIT([dgeo],[1.0],[email@email.com])
AC_CONFIG_HEADERS([config.h])
AM_INIT_AUTOMAKE([1.10 no-define subdir-objects])
AM_SILENT_RULES([yes])

# Checks for programs.
AC_PROG_CC(gcc)
AC_PROG_LEX
AC_PROG_YACC
AC_PROG_RANLIB
AC_PROG_CC_STDC
AC_PROG_CXX(g++)
AC_CANONICAL_HOST

# Checks for libraries.
AC_CHECK_LIB([gdal], [OGRRegisterAll], [], [AC_MSG_ERROR([libgdal was not found.])])
AC_CHECK_LIB([geos_c], [GEOSversion], [], [AC_MSG_ERROR([libgeos_c was not found.])])
AC_CHECK_LIB([geos], [GEOSversion], [], [AC_MSG_ERROR([libgeos was not found.])])

# checks for dependencies
#PKG_CHECK_MODULES([CHECK], [check >= 0.9.6])
AC_SEARCH_LIBS([mergesort], [bsd])
AC_SEARCH_LIBS([pthread_create], [pthread])

# Checks for header files.
AC_CHECK_HEADERS([float.h inttypes.h stddef.h stdlib.h string.h sys/time.h])
AC_HEADER_TIME

# Checks for typedefs, structures, and compiler characteristics.
AC_CHECK_HEADER_STDBOOL
AC_TYPE_SIZE_T
AC_TYPE_UINT16_T
AC_TYPE_UINT32_T
AC_TYPE_UINT8_T
AC_CHECK_TYPES([ptrdiff_t])

# Checks for library functions.
AC_CHECK_FUNCS([memset pow setlocale sqrt strdup])

GDAL_CFLAGS="`gdal-config --cflags`"
GDAL_LIBS="`gdal-config --libs`"
AC_SUBST([GDAL_CFLAGS])
AC_SUBST([GDAL_LIBS])

# Conditional link with CPLEX
AC_ARG_WITH([cplex],
  [AS_HELP_STRING([--with-cplex],
    [enable cplex @<:@default=check@:>@])],
  [],
  [with_cplex=check])
AS_CASE(["$with_cplex"],
  [yes], [AC_CHECK_LIB([cplex], [CPXopenCPLEX], [HAVE_CPLEX=1], [HAVE_CPLEX=0], [-lm -lpthread])],
  [no], [HAVE_CPLEX=0],
  [AC_CHECK_LIB([cplex], [CPXopenCPLEX], [HAVE_CPLEX=1], [HAVE_CPLEX=0])])
AM_CONDITIONAL([ENABLE_CPLEX], [test "$HAVE_CPLEX" -eq 1])

if test "$HAVE_CPLEX" -eq 1; then
  AC_DEFINE([ENABLE_CPLEX], 1, [Use CPLEX on build])
fi

# Conditional var to OSX
build_mac=no
case "${host_os}" in
    darwin*)
        build_mac=yes
        ;;
esac
AM_CONDITIONAL([OSX], [test "$build_mac" = "yes"])

AC_CONFIG_FILES([Makefile
                 utils/Makefile
                 stats/Makefile])
                 
AC_OUTPUT
