# Environment
MKDIR=mkdir
CP=cp
CCADMIN=CCadmin
RANLIB=ranlib

# Windows (mingw)?
ifneq (,$(findstring mingw, $(CONF)))
MINGW := true
endif

# Macros
ifndef CND_PLATFORM
CND_PLATFORM=$(shell uname -s)
endif

# Include project Makefile
include Makefile

# Object Directory
OBJECTDIR=build/${CONF}/${CND_PLATFORM}

# Object Files
OBJECTFILES = ${OBJECTDIR}/jrubyexe.o

ifdef JAVA_HOME
JAVA_INCLUDE = $(subst \,/,${JAVA_HOME})/include
INCLUDES = "-I${JAVA_INCLUDE}"
endif

ifdef MINGW
OBJECTFILES += 
# Object Files
OBJECTFILES = ${OBJECTDIR}/argparser.o		\
	          ${OBJECTDIR}/utilsfuncs.o		\
	          ${OBJECTDIR}/ng.o			\
	          ${OBJECTDIR}/strlcpy.o	\
	          ${OBJECTDIR}/utilsfuncswin.o	\
	          ${OBJECTDIR}/platformlauncher.o	\
	          ${OBJECTDIR}/jvmlauncher.o	\
	          ${OBJECTDIR}/jruby.o
INCLUDES += "-I${JAVA_INCLUDE}/win32"
else
OBJECTFILES += ${OBJECTDIR}/unixlauncher.o
endif

CFLAGS += -O2 -Wall $(INCLUDES)
CCFLAGS = $(CFLAGS)
CXXFLAGS = $(CFLAGS)

# Compiler Flags
ifeq (mingw,$(CONF))
CFLAGS += -m32 -s
endif
ifeq (mingw64,$(CONF))
CFLAGS += -m64 -s
endif

# Resources
WINDRES = windres

# Link Libraries and Options
LDLIBSOPTIONS = -lstdc++

ifdef MINGW
LDLIBSOPTIONS += -lws2_32 -static-libgcc -Wl,--enable-auto-import -Wl,-Bstatic -Wl,-Bdynamic -static
PROGRAM = jruby.dll
else
PROGRAM = jruby
endif

ifneq (,$(findstring SunOS,$(CND_PLATFORM)))
CFLAGS += -D__SUNOS__
LDLIBSOPTIONS += -lsocket -lnsl
endif

# define JRUBY_MODULE appropriately for target JRuby
ifeq (1,$(JRUBY_MODULE))
CFLAGS += -DJRUBY_MODULE=${JRUBY_MODULE}
endif
