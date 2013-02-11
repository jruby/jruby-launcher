JRuby Native Launcher

== Motivation

Maintaning JRuby.BAT was, well, to put it mildly, unpleasant. We had
tens of bugs due to BAT limitations, we had weird behaviors depending
on the version of Windows, we had a bunch of regressions.

See http://jira.codehaus.org/browse/JRUBY-4100 for more details.

On UNIX platforms, we had problems because a shell-script can't be put
as a path in the shebang and couldn't take arguments. (#!/usr/bin/env
jruby -w)

We also wanted to DRY up argument handling, even if it meant ditching
shell script and writing in lowest-common-denominator C++ (!).

== Compile

On UNIX, you should be able to just type 'make' and a 'jruby' binary
will be created in the project directory. Copy this to
$JRUBY_HOME/bin. On Windows, you should also be able to type 'make' if
you have the MinGW compiler toolkit installed.

Or, open the project in Netbeans 6.8 (with C/C++ plugin installed). If
Netbeans warns that no compilers found, follow the instructions and
install the required compilers. Currenty, we support MinGW. More info
here:

http://netbeans.org/community/releases/68/cpp-setup-instructions.html

Then, just build it, and you're ready to go. jruby.exe, jrubyw.exe and
jruby.dll will be created, they need to be copied into $JRUBY_HOME/bin
directory.

Both, 32-bit and 64-bit compilers are supported. Great version of
64-bit mingw can be found here: http://www.cadforte.com/system64.html

To build 64-bit version of the launcher, use the following from the
command line:

  make CONF=mingw64

== Run

The launcher provides a great logger, use it like this:

  jruby -Xtrace LOG_FILE.log ....

== TODO

See TODO.txt file for things that need to be done before this launcher
could replace jruby.bat.

== Thanks

The original code is by Netbeans project.

== License

Read the COPYING file.
