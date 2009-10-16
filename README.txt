JRuby native launcher for Windows.

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
