JRuby Native Launcher for Windows.

== Motivation

Maintaning JRuby.BAT is, well, to put it mildly, unpleasant. We had tens of bugs due to BAT limitations, we had weird behaviors depending on the version of Windows, we had a bunch of regressions. 

See http://jira.codehaus.org/browse/JRUBY-4100 for more details.

== Compile

Open the project in Netbeans 6.8 (with C/C++ plugin installed). If Netbeans warns that no compilers found, follow the instructions and install the required compilers. Currenty, we support MinGW. More info here:

http://netbeans.org/community/releases/68/cpp-setup-instructions.html

Then, just build it, and you're ready to go. jruby.exe, jrubyw.exe and jruby.dll will be created, they need to be copied into $JRUBY_HOME/bin directory.

Both, 32-bit and 64-bit compilers are supported. Great version of 64-bit mingw can be found here:
http://www.cadforte.com/system64.html

To build 64-bit version of the launcher, use the following from the command line:

  make CONFG=mingw64

== Run

The launcher provides a great logger, use it like this:

  jruby.exe -Xtrace LOG_FILE.log ....

== TODO

See TODO.txt file for things that need to be done before this launcher could replace jruby.bat.

== Thanks

The original code is by Netbeans project.

== License

Read the COPYING file.
