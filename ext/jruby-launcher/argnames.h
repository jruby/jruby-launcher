/*
 * Copyright 2009-2010 JRuby Team (www.jruby.org).
 */

#ifndef _ARGNAMES_H
#define _ARGNAMES_H

/* Standard Java classpath options */
#define ARG_NAME_CP "-J-cp"
#define ARG_NAME_CLASSPATH "-J-classpath"

/* These are windows-launcher-specific args. They should be prefixed with -X
   so that they don't clash with normal Ruby args (like --trace for Rake). */
#define ARG_NAME_SEPAR_PROC "-Xfork-java"
#define ARG_NAME_CONSOLE    "-Xconsole"
#define ARG_NAME_LAUNCHER_LOG "-Xtrace"
#define ARG_NAME_BOOTCLASS "-Xbootclass"
#define ARG_NAME_JDKHOME "-Xjdkhome"
#define ARG_NAME_CP_PREPEND "-Xcp:p"
#define ARG_NAME_CP_APPEND "-Xcp:a"
#define ARG_NAME_CMD_ONLY "-Xcommand"
#define ARG_NAME_NO_BOOTCLASSPATH "-Xnobootclasspath"

/* Below are standard JRuby args handled by the launcher. */
#define ARG_NAME_SERVER "--server"
#define ARG_NAME_CLIENT "--client"
#define ARG_NAME_DEV "--dev"
#define ARG_NAME_SAMPLE "--sample"
#define ARG_NAME_MANAGE "--manage"
#define ARG_NAME_HEADLESS "--headless"
#define ARG_NAME_NG_SERVER "--ng-server"
#define ARG_NAME_NG "--ng"

#endif  /* _ARGNAMES_H */

