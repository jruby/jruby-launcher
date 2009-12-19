# Environment 
MKDIR=mkdir
CP=cp
CCADMIN=CCadmin
RANLIB=ranlib

build: .build-post

.build-pre:

.build-post: .build-impl
	${MAKE} -f nbproject/Makefile-${CONF}.mk SUBPROJECTS=${SUBPROJECTS} jruby.exe jrubyw.exe
	if [ -d ../jruby ]; then cp jruby.exe jrubyw.exe jruby.dll ../jruby/bin/; fi
	if [ -d D:/work/jruby-dev/jruby ]; then cp jruby.exe jrubyw.exe jruby.dll D:/work/jruby-dev/jruby/bin/; fi

jruby.res: resources/jruby.rc
	windres $^ -O coff -o $@

jruby.exe: jrubyexe.cpp nbexecloader.h utilsfuncs.cpp jruby.res
	g++ $(CXXFLAGS) $^ -s -o $@ $(LDLIBSOPTIONS)

jrubyw.exe: jrubyexe.cpp nbexecloader.h utilsfuncs.cpp jruby.res
	g++ $(CXXFLAGS) -DJRUBYW -mwindows $^ -s -o $@ $(LDLIBSOPTIONS)

clean: .clean-post

.clean-pre:
	-rm -r build/*

.clean-post: .clean-impl
	rm -f jruby.exe jrubyw.exe jruby.res

clobber: .clobber-post

.clobber-pre:

.clobber-post: .clobber-impl

all: .all-post

.all-pre:

.all-post: .all-impl

help: .help-post

.help-pre:

.help-post: .help-impl

# include project implementation makefile
include nbproject/Makefile-impl.mk
