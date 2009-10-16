JRuby native launcher for Windows.

== Motivation

Maintaning JRuby.BAT is, well, to put it mildly, unpleasant. We had tens of bugs due to BAT limitations, we had weird behaviors depending on the version of Windows, we had a bunch of regressions. 

See http://jira.codehaus.org/browse/JRUBY-4100 for more details.

== Compile

Open the project in Netbeans 6.7+ (with C/C++ plugin installed). If Netbeans warns that no copilers found, follow the instructions and install the required compilers. I used compilers from Cygwin. More info here:

http://www.netbeans.org/community/releases/65/cpp-setup-instructions.html

Then, just build it, and you're ready to go. jruby.exe and jruby.dll will be created, they need to be copied into $JRUBY_HOME/bin directory.

== Run

The launcher provides a great logger, use it like this:

  jruby.exe --trace LOG_FILE.log ....

== TODO

See TODO.txt file for things that need to be done before this launcher could replace jruby.bat.

== Thanks

The original code is by Netbeans project.

== License

Read the COPYING file.
