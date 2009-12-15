#
#  There exist several targets which are by default empty and which can be 
#  used for execution of your targets. These targets are usually executed 
#  before and after some main targets. They are: 
#
#     .build-pre:              called before 'build' target
#     .build-post:             called after 'build' target
#     .clean-pre:              called before 'clean' target
#     .clean-post:             called after 'clean' target
#     .clobber-pre:            called before 'clobber' target
#     .clobber-post:           called after 'clobber' target
#     .all-pre:                called before 'all' target
#     .all-post:               called after 'all' target
#     .help-pre:                called before 'help' target
#     .help-post:               called after 'help' target
#
#  Targets beginning with '.' are not intended to be called on their own.
#
#  Main targets can be executed directly, and they are:
#  
#     build                    build a specific configuration
#     clean                    remove built files from a configuration
#     clobber                  remove all built files
#     all                      build all configurations
#     help                     print help mesage
#  
#  Targets .build-impl, .clean-impl, .clobber-impl, .all-impl, and
#  .help-impl are implemented in nbproject/makefile-impl.mk.
#
# NOCDDL


# Environment 
MKDIR=mkdir
CP=cp
CCADMIN=CCadmin
RANLIB=ranlib

# build
build: .build-post

.build-pre:
# Add your pre 'build' code here...

.build-post: .build-impl
# Add your post 'build' code here...
	${MAKE} -f nbproject/Makefile-${CONF}.mk SUBPROJECTS=${SUBPROJECTS} jruby.exe jrubyw.exe
	if [ -d ../jruby ]; then cp jruby.exe jrubyw.exe jruby.dll ../jruby/bin/; fi
	if [ -d D:/work/jruby-dev/jruby ]; then cp jruby.exe jrubyw.exe jruby.dll D:/work/jruby-dev/jruby/bin/; fi

jruby.exe: jrubyexe.cpp nbexecloader.h utilsfuncs.cpp
	g++ $(CXXFLAGS) $^ -s -o $@ $(LDLIBSOPTIONS)

jrubyw.exe: jrubyexe.cpp nbexecloader.h utilsfuncs.cpp
	g++ $(CXXFLAGS) -DJRUBYW -mwindows $^ -s -o $@ $(LDLIBSOPTIONS)

# clean
clean: .clean-post

.clean-pre:
# Add your pre 'clean' code here...
	-rm -r build/*

.clean-post: .clean-impl
# Add your post 'clean' code here...
	rm -f jruby.exe jrubyw.exe

# clobber
clobber: .clobber-post

.clobber-pre:
# Add your pre 'clobber' code here...

.clobber-post: .clobber-impl
# Add your post 'clobber' code here...


# all
all: .all-post

.all-pre:
# Add your pre 'all' code here...

.all-post: .all-impl
# Add your post 'all' code here...


# help
help: .help-post

.help-pre:
# Add your pre 'help' code here...

.help-post: .help-impl
# Add your post 'help' code here...



# include project implementation makefile
include nbproject/Makefile-impl.mk
