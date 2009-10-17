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
CND_CONF=cygwin
CND_DISTDIR=dist

# Include project Makefile
include Makefile

# Object Directory
OBJECTDIR=build/${CND_CONF}/${CND_PLATFORM}

# Object Files
OBJECTFILES= \
	${OBJECTDIR}/jvmlauncher.o \
	${OBJECTDIR}/platformlauncher.o \
	${OBJECTDIR}/utilsfuncs.o \
	${OBJECTDIR}/jruby.o \
	${OBJECTDIR}/jrubyexe.o

# C Compiler Flags
CFLAGS=-m32

# CC Compiler Flags
CCFLAGS=-m32 -s -mno-cygwin
CXXFLAGS=-m32 -s -mno-cygwin

# Fortran Compiler Flags
FFLAGS=

# Assembler Flags
ASFLAGS=

# Link Libraries and Options
LDLIBSOPTIONS=

# Build Targets
.build-conf: ${BUILD_SUBPROJECTS}
	${MAKE}  -f nbproject/Makefile-cygwin.mk jruby.dll

jruby.dll: ${OBJECTFILES}
	${LINK.cc} -shared -o jruby.dll -fPIC ${OBJECTFILES} ${LDLIBSOPTIONS} 

${OBJECTDIR}/jvmlauncher.o: nbproject/Makefile-${CND_CONF}.mk jvmlauncher.cpp 
	${MKDIR} -p ${OBJECTDIR}
	${RM} $@.d
	$(COMPILE.cc) -O2 -Wall -s -I/c/cygwin/usr/include/mingw -I/D/re/java6/include -I/D/re/java6/include/win32 -fPIC  -MMD -MP -MF $@.d -o ${OBJECTDIR}/jvmlauncher.o jvmlauncher.cpp

${OBJECTDIR}/platformlauncher.o: nbproject/Makefile-${CND_CONF}.mk platformlauncher.cpp 
	${MKDIR} -p ${OBJECTDIR}
	${RM} $@.d
	$(COMPILE.cc) -O2 -Wall -s -I/c/cygwin/usr/include/mingw -I/D/re/java6/include -I/D/re/java6/include/win32 -fPIC  -MMD -MP -MF $@.d -o ${OBJECTDIR}/platformlauncher.o platformlauncher.cpp

${OBJECTDIR}/utilsfuncs.o: nbproject/Makefile-${CND_CONF}.mk utilsfuncs.cpp 
	${MKDIR} -p ${OBJECTDIR}
	${RM} $@.d
	$(COMPILE.cc) -O2 -Wall -s -I/c/cygwin/usr/include/mingw -I/D/re/java6/include -I/D/re/java6/include/win32 -fPIC  -MMD -MP -MF $@.d -o ${OBJECTDIR}/utilsfuncs.o utilsfuncs.cpp

${OBJECTDIR}/jruby.o: nbproject/Makefile-${CND_CONF}.mk jruby.cpp 
	${MKDIR} -p ${OBJECTDIR}
	${RM} $@.d
	$(COMPILE.cc) -O2 -Wall -s -I/c/cygwin/usr/include/mingw -I/D/re/java6/include -I/D/re/java6/include/win32 -fPIC  -MMD -MP -MF $@.d -o ${OBJECTDIR}/jruby.o jruby.cpp

${OBJECTDIR}/jrubyexe.o: nbproject/Makefile-${CND_CONF}.mk jrubyexe.cpp 
	${MKDIR} -p ${OBJECTDIR}
	${RM} $@.d
	$(COMPILE.cc) -O2 -Wall -s -I/c/cygwin/usr/include/mingw -I/D/re/java6/include -I/D/re/java6/include/win32 -fPIC  -MMD -MP -MF $@.d -o ${OBJECTDIR}/jrubyexe.o jrubyexe.cpp

# Subprojects
.build-subprojects:

# Clean Targets
.clean-conf:
	${RM} -r build/cygwin
	${RM} jruby.dll

# Subprojects
.clean-subprojects:

# Enable dependency checking
.dep.inc: .depcheck-impl

include .dep.inc
