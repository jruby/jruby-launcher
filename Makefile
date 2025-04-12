# These line gets substituted with the actual Config::CONFIG items location by extconf.rb
PREFIX = notspecified
BINDIR = $(PREFIX)/bin
JRUBY_VERSION = notspecified
JRUBY_MODULE = 1

ifeq (true,$(shell test -x $(BINDIR)/jruby && echo true))
RAKE=$(BINDIR)/jruby -S rake
else
RAKE=rake
endif

build: .build-post

.build-pre:

.build-post: .build-impl build-exe

build-exe:
	@if [ "$(findstring mingw, $(CONF))" ]; then											\
	${MAKE} -f ${SUB_CONFMK} SUBPROJECTS=${SUBPROJECTS} jruby.exe jrubyw.exe;					\
	if [ -d ../jruby ]; then cp jruby.exe jrubyw.exe jruby.dll ../jruby/bin/; fi;					\
	if [ -d D:/work/jruby-dev/jruby ]; then cp jruby.exe jrubyw.exe jruby.dll D:/work/jruby-dev/jruby/bin/; fi;	\
	fi

jruby.res: resources/jruby.rc
	windres $^ -O coff -o $@

jruby.exe: jrubyexe.cpp nbexecloader.h utilsfuncs.cpp utilsfuncswin.cpp jruby.res
	$(CXX) $(CXXFLAGS) $^ -s -o $@ $(LDLIBSOPTIONS) -static

jrubyw.exe: jrubyexe.cpp nbexecloader.h utilsfuncs.cpp utilsfuncswin.cpp jruby.res
	$(CXX) $(CXXFLAGS) -DJRUBYW -mwindows $^ -s -o $@ $(LDLIBSOPTIONS) -static

install:
	@if [ ! -f ./jruby ]; then echo "Please run 'make' first."; exit 1; fi
	@if [ x$(BINDIR) = xnotspecified/bin ]; then echo "Please define where to install by passing PREFIX=<jruby-home>."; exit 1; fi
	@if [ ! -w $(BINDIR) ]; then echo "'$(BINDIR)' does not exist or cannot write to '$(BINDIR)'."; exit 1; fi
	@if [ -f $(BINDIR)/jruby -a ! -w $(BINDIR)/jruby ]; then echo "Cannot write to '$(BINDIR)/jruby'."; exit 1; fi
	cp -a ./jruby $(BINDIR)/jruby
	@if [ x$(PREFIX) = xnotspecified ]; then echo "Please define where to install by passing PREFIX=<jruby-home>."; exit 1; fi
	# Restore jruby.sh if it has been deleted
	@if [ ! -f $(BINDIR)/jruby.sh ]; then cp -a ./exe/jruby.sh $(BINDIR)/jruby.sh; fi
	# Install a backup jruby.sh in case it is deleted later
	cp -a ./exe/jruby.sh $(BINDIR)/jruby.sh.bak

test:
	$(RAKE)

# Universal binary on OSX
FAT_ARCHES=i386 ppc x86_64

fat: $(FAT_ARCHES)
	lipo -create $(foreach arch,$(FAT_ARCHES),build/unix/Darwin-$(arch)/jruby-launcher) -output jruby
	$(RAKE)

$(FAT_ARCHES):
	$(MAKE) -f $(SUB_CONFMK) CND_PLATFORM=Darwin-$@ CFLAGS="-arch $@" build/unix/Darwin-$@/jruby-launcher

clean: .clean-post

.clean-pre:
	-rm -rf build/*

.clean-post: .clean-impl
	rm -f *.exe *.res

clobber: .clobber-post

.clobber-pre:

.clobber-post: .clobber-impl

all: .all-post

.all-pre:

.all-post: .all-impl

help: .help-post

.help-pre:

.help-post: .help-impl

# Use the manually-maintained inc/*.mk makefiles.
# Pass NETBEANS=true on the command-line to use NB's generated
# nbproject/*.mk

ifdef NETBEANS
SUB_IMPLMK=nbproject/Makefile-impl.mk
else
SUB_IMPLMK=inc/Makefile-impl.mk
SUB_CONFMK=inc/Makefile-rules.mk
endif

# include project implementation makefile
include $(SUB_IMPLMK)

# Pick conf based on OS. for mingw64, must manually override for now.
ifeq ($(OS),Windows_NT)
CC=gcc
CONF=mingw
else
CONF=unix
endif

ifdef NETBEANS
SUB_CONFMK=nbproject/Makefile-${CONF}.mk
endif
