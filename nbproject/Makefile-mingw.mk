#
# Generated Makefile - do not edit!
#
# Edit the Makefile in the project folder instead (../Makefile). Each target
# has a -pre and a -post target defined where you can add customized code.
#
# This makefile implements configuration specific macros and targets.


# Environment
MKDIR=mkdir
CP=cp
CCADMIN=CCadmin
RANLIB=ranlib
CC=gcc
CCC=g++
CXX=g++
FC=
AS=as.exe

# Macros
CND_PLATFORM=MinGW-Windows
CND_CONF=mingw
CND_DISTDIR=dist

# Include project Makefile
include Makefile

# Object Directory
OBJECTDIR=build/${CND_CONF}/${CND_PLATFORM}

# Object Files
OBJECTFILES= \
	${OBJECTDIR}/argparser.o \
	${OBJECTDIR}/utilsfuncswin.o \
	${OBJECTDIR}/platformlauncher.o \
	${OBJECTDIR}/ng.o \
	${OBJECTDIR}/jrubyexe.o \
	${OBJECTDIR}/strlcpy.o \
	${OBJECTDIR}/jvmlauncher.o \
	${OBJECTDIR}/unixlauncher.o \
	${OBJECTDIR}/utilsfuncs.o \
	${OBJECTDIR}/jruby.o

# C Compiler Flags
CFLAGS=-m32 -mno-cygwin

# CC Compiler Flags
CCFLAGS=-m32 -mno-cygwin
CXXFLAGS=-m32 -mno-cygwin

# Fortran Compiler Flags
FFLAGS=

# Assembler Flags
ASFLAGS=

# Link Libraries and Options
LDLIBSOPTIONS=-lws2_32 -static-libgcc -lws2_32 -Wl,--enable-auto-import -Wl,-Bstatic -lstdc++ -Wl,-Bdynamic

# Build Targets
.build-conf: ${BUILD_SUBPROJECTS}
	${MAKE}  -f nbproject/Makefile-mingw.mk jruby.dll

jruby.dll: ${OBJECTFILES}
	${LINK.cc} -shared -o jruby.dll -s ${OBJECTFILES} ${LDLIBSOPTIONS} 

${OBJECTDIR}/argparser.o: nbproject/Makefile-${CND_CONF}.mk argparser.cpp 
	${MKDIR} -p ${OBJECTDIR}
	${RM} $@.d
	$(COMPILE.cc) -O2 -Wall -s -I${JAVA_HOME}/include -I${JAVA_HOME}/include/win32  -MMD -MP -MF $@.d -o ${OBJECTDIR}/argparser.o argparser.cpp

${OBJECTDIR}/utilsfuncswin.o: nbproject/Makefile-${CND_CONF}.mk utilsfuncswin.cpp 
	${MKDIR} -p ${OBJECTDIR}
	${RM} $@.d
	$(COMPILE.cc) -O2 -Wall -s -I${JAVA_HOME}/include -I${JAVA_HOME}/include/win32  -MMD -MP -MF $@.d -o ${OBJECTDIR}/utilsfuncswin.o utilsfuncswin.cpp

${OBJECTDIR}/platformlauncher.o: nbproject/Makefile-${CND_CONF}.mk platformlauncher.cpp 
	${MKDIR} -p ${OBJECTDIR}
	${RM} $@.d
	$(COMPILE.cc) -O2 -Wall -s -I${JAVA_HOME}/include -I${JAVA_HOME}/include/win32  -MMD -MP -MF $@.d -o ${OBJECTDIR}/platformlauncher.o platformlauncher.cpp

${OBJECTDIR}/ng.o: nbproject/Makefile-${CND_CONF}.mk ng.c 
	${MKDIR} -p ${OBJECTDIR}
	${RM} $@.d
	$(COMPILE.c) -O2 -Wall -s  -MMD -MP -MF $@.d -o ${OBJECTDIR}/ng.o ng.c

${OBJECTDIR}/jrubyexe.o: nbproject/Makefile-${CND_CONF}.mk jrubyexe.cpp 
	${MKDIR} -p ${OBJECTDIR}
	${RM} $@.d
	$(COMPILE.cc) -O2 -Wall -s -I${JAVA_HOME}/include -I${JAVA_HOME}/include/win32  -MMD -MP -MF $@.d -o ${OBJECTDIR}/jrubyexe.o jrubyexe.cpp

${OBJECTDIR}/strlcpy.o: nbproject/Makefile-${CND_CONF}.mk strlcpy.c 
	${MKDIR} -p ${OBJECTDIR}
	${RM} $@.d
	$(COMPILE.c) -O2 -Wall -s  -MMD -MP -MF $@.d -o ${OBJECTDIR}/strlcpy.o strlcpy.c

${OBJECTDIR}/jvmlauncher.o: nbproject/Makefile-${CND_CONF}.mk jvmlauncher.cpp 
	${MKDIR} -p ${OBJECTDIR}
	${RM} $@.d
	$(COMPILE.cc) -O2 -Wall -s -I${JAVA_HOME}/include -I${JAVA_HOME}/include/win32  -MMD -MP -MF $@.d -o ${OBJECTDIR}/jvmlauncher.o jvmlauncher.cpp

${OBJECTDIR}/unixlauncher.o: nbproject/Makefile-${CND_CONF}.mk unixlauncher.cpp 
	${MKDIR} -p ${OBJECTDIR}
	${RM} $@.d
	$(COMPILE.cc) -O2 -Wall -s -I${JAVA_HOME}/include -I${JAVA_HOME}/include/win32  -MMD -MP -MF $@.d -o ${OBJECTDIR}/unixlauncher.o unixlauncher.cpp

${OBJECTDIR}/utilsfuncs.o: nbproject/Makefile-${CND_CONF}.mk utilsfuncs.cpp 
	${MKDIR} -p ${OBJECTDIR}
	${RM} $@.d
	$(COMPILE.cc) -O2 -Wall -s -I${JAVA_HOME}/include -I${JAVA_HOME}/include/win32  -MMD -MP -MF $@.d -o ${OBJECTDIR}/utilsfuncs.o utilsfuncs.cpp

${OBJECTDIR}/jruby.o: nbproject/Makefile-${CND_CONF}.mk jruby.cpp 
	${MKDIR} -p ${OBJECTDIR}
	${RM} $@.d
	$(COMPILE.cc) -O2 -Wall -s -I${JAVA_HOME}/include -I${JAVA_HOME}/include/win32  -MMD -MP -MF $@.d -o ${OBJECTDIR}/jruby.o jruby.cpp

# Subprojects
.build-subprojects:

# Clean Targets
.clean-conf: ${CLEAN_SUBPROJECTS}
	${RM} -r build/mingw
	${RM} jruby.dll

# Subprojects
.clean-subprojects:

# Enable dependency checking
.dep.inc: .depcheck-impl

include .dep.inc
