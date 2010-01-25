# Environment
MKDIR=mkdir
CP=cp
CCADMIN=CCadmin
RANLIB=ranlib
CC=gcc
CCC=g++
CXX=g++

# Windows (mingw)?
ifneq (,$(findstring mingw, $(CONF)))
MINGW := true
endif

# Macros
CND_PLATFORM=$(shell uname -s)

# Include project Makefile
include Makefile

# Object Directory
OBJECTDIR=build/${CONF}/${CND_PLATFORM}

# Object Files
OBJECTFILES = ${OBJECTDIR}/argparser.o		\
	      ${OBJECTDIR}/utilsfuncs.o		\
	      ${OBJECTDIR}/ng.o			\
	      ${OBJECTDIR}/jrubyexe.o

INCLUDES = -I${JAVA_HOME}/include

ifdef MINGW
OBJECTFILES += ${OBJECTDIR}/utilsfuncswin.o	\
	       ${OBJECTDIR}/platformlauncher.o	\
	       ${OBJECTDIR}/jvmlauncher.o	\
	       ${OBJECTDIR}/jruby.o
INCLUDES += -I${JAVA_HOME}/include/win32
else
OBJECTFILES += ${OBJECTDIR}/unixlauncher.o
endif

CFLAGS = -O2 -Wall -s $(INCLUDES)
CCFLAGS = $(CFLAGS)
CXXFLAGS = $(CFLAGS)

# Compiler Flags
ifeq (mingw,$(CONF))
CFLAGS += -m32 -mno-cygwin
else ifeq (mingw64,$(CONF))
CFLAGS += -m64 -mno-cygwin
endif

# Resources
WINDRES = windres

# Link Libraries and Options
LDLIBSOPTIONS = -lstdc++

ifdef MINGW
LDLIBSOPTIONS += -lws2_32 -static-libgcc -Wl,--enable-auto-import -Wl,-Bstatic -Wl,-Bdynamic
PROGRAM = jruby.dll
else
PROGRAM = jruby
endif
