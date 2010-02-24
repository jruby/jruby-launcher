# These line gets substituted with the actual Config::CONFIG items location by extconf.rb
PREFIX = notspecified
BINDIR = $(PREFIX)/bin
SITELIBDIR = $(PREFIX)/lib/ruby/site_ruby/1.8

build: .build-post

.build-pre:

.build-post: .build-impl build-exe test

build-exe:
	@if [ "$(findstring mingw, $(CONF))" ]; then											\
	${MAKE} -f ${SUB_CONFMK} SUBPROJECTS=${SUBPROJECTS} jruby.exe jrubyw.exe;					\
	if [ -d ../jruby ]; then cp jruby.exe jrubyw.exe jruby.dll ../jruby/bin/; fi;					\
	if [ -d D:/work/jruby-dev/jruby ]; then cp jruby.exe jrubyw.exe jruby.dll D:/work/jruby-dev/jruby/bin/; fi;	\
	fi

jruby.res: resources/jruby.rc
	windres $^ -O coff -o $@

jruby.exe: jrubyexe.cpp nbexecloader.h utilsfuncs.cpp utilsfuncswin.cpp jruby.res
	g++ $(CXXFLAGS) $^ -s -o $@ $(LDLIBSOPTIONS)

jrubyw.exe: jrubyexe.cpp nbexecloader.h utilsfuncs.cpp utilsfuncswin.cpp jruby.res
	g++ $(CXXFLAGS) -DJRUBYW -mwindows $^ -s -o $@ $(LDLIBSOPTIONS)

install:
	@if [ ! -f ./jruby ]; then echo "Please run 'make' first."; exit 1; fi
	@if [ x$(BINDIR) = xnotspecified/bin ]; then echo "Please define where to install by passing PREFIX=<jruby-home>."; exit 1; fi
	@if [ ! -w $(BINDIR) ]; then echo "'$(BINDIR)' does not exist or cannot write to '$(BINDIR)'."; exit 1; fi
	@if [ -f $(BINDIR)/jruby -a ! -w $(BINDIR)/jruby ]; then echo "Cannot write to '$(BINDIR)/jruby'."; exit 1; fi
	cp ./jruby $(BINDIR)/jruby
	@if [ x$(SITELIBDIR) = xnotspecified/lib/ruby/site_ruby/1.8 ]; then echo "Please define where to install by passing PREFIX=<jruby-home>."; exit 1; fi
	@if [ ! -w $(SITELIBDIR) ]; then echo "'$(SITELIBDIR)' does not exist or cannot write to '$(SITELIBDIR)'."; exit 1; fi
	cp ./lib/rubygems/defaults/jruby_native.rb $(SITELIBDIR)/rubygems/defaults

test:
	rake

# Universal binary on OSX
FAT_ARCHES=i386 ppc x86_64

fat: $(FAT_ARCHES)
	lipo -create $(foreach arch,$(FAT_ARCHES),build/unix/Darwin-$(arch)/jruby-launcher) -output jruby
	rake

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
CONF=mingw
else
CONF=unix
endif

ifdef NETBEANS
SUB_CONFMK=nbproject/Makefile-${CONF}.mk
endif
