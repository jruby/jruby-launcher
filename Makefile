
build: .build-post

.build-pre:

.build-post: .build-impl
	${MAKE} -f ${SUB_CONFMK} SUBPROJECTS=${SUBPROJECTS} jruby.exe jrubyw.exe
	if [ -d ../jruby ]; then cp jruby.exe jrubyw.exe jruby.dll ../jruby/bin/; fi
	if [ -d D:/work/jruby-dev/jruby ]; then cp jruby.exe jrubyw.exe jruby.dll D:/work/jruby-dev/jruby/bin/; fi

jruby.res: resources/jruby.rc
	windres $^ -O coff -o $@

jruby.exe: jrubyexe.cpp nbexecloader.h utilsfuncs.cpp utilsfuncswin.cpp jruby.res
	g++ $(CXXFLAGS) $^ -s -o $@ $(LDLIBSOPTIONS)

jrubyw.exe: jrubyexe.cpp nbexecloader.h utilsfuncs.cpp utilsfuncswin.cpp jruby.res
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

# To use the manually-maintained inc/*.mk makefiles.
# Comment out to use NB's generated nbproject/*.mk
USE_INCMK=true

ifdef USE_INCMK
SUB_IMPLMK=inc/Makefile-impl.mk
SUB_CONFMK=inc/Makefile-rules.mk
else
SUB_IMPLMK=nbproject/Makefile-impl.mk
endif

# include project implementation makefile
include $(SUB_IMPLMK)

# Pick conf based on OS. for mingw64, must manually override for now.
ifeq ($(OS),Windows_NT)
CONF=mingw
else
CONF=unix
endif

ifndef USE_INCMK
SUB_CONFMK=nbproject/Makefile-${CONF}.mk
endif
