# $Id$

# Obvious program stuff.
bin_PROGRAMS = prwd
# dist_man1_MANS = prwd.1

# Distribution tarball options.
EXTRA_DIST = \
	CHANGES FAQ README TODO examples \
	array.h prwd.h prwd.1
dist-hook:
	grep "^#found_debug=" configure
	find $(distdir) -name .svn -type d|xargs rm -Rf

# Preprocessor flags.
CPPFLAGS += @XOPEN_DEFINES@

# glibc as usual does things ass-backwards and hides useful things by default,
# so everyone has to add this.
if IS_GLIBC
CFLAGS += -D_GNU_SOURCE
endif

# Set flags for gcc. gcc4 whines abouts silly stuff so it needs slightly
# different flags.
if IS_GCC
CFLAGS += -std=gnu99
if IS_DEBUG
CFLAGS += -O0 -g
CFLAGS += -Wno-long-long -Wall -W -Wnested-externs -Wformat=2
CFLAGS += -Wmissing-prototypes -Wstrict-prototypes -Wmissing-declarations
CFLAGS += -Wwrite-strings -Wshadow -Wpointer-arith -Wsign-compare
CFLAGS += -Wundef -Wbad-function-cast -Winline -Wcast-align
CFLAGS += -Wdeclaration-after-statement
CPPFLAGS += -DDEBUG
else
CFLAGS += -O2
endif
if IS_GCC4
CPPFLAGS += -iquote. -I/usr/local/include
if IS_DEBUG
CFLAGS += -Wno-pointer-sign
endif
else
CPPFLAGS += -I. -I- -I/usr/local/include
endif
endif

# Set flags for Solaris.
if IS_SUNOS
CPPFLAGS += -D_XPG4_2 -D__EXTENSIONS__ -D_POSIX_PTHREAD_SEMANTICS
endif

# Set flags for Sun CC.
if IS_SUNCC
CFLAGS += -erroff=E_EMPTY_DECLARATION
endif

# List of sources.
dist_prwd_SOURCES = \
	main.c \
	wcslcpy.c \
	strdelim.c

nodist_prwd_SOURCES =

if NO_STRLCPY
nodist_prwd_SOURCES += strlcpy.c
endif
