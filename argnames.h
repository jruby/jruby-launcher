/* 
 * File:   argnames.h
 * Author: Holy
 *
 * Created on 4. prosinec 2008, 14:13
 */

#ifndef _ARGNAMES_H
#define	_ARGNAMES_H

/* These are windows-launcher-specific args. They should be prefixed with -X
   so that they don't clash with normal Ruby args (like --trace for Rake). */
#define ARG_NAME_SEPAR_PROC "-Xfork-java"
#define ARG_NAME_CONSOLE    "-Xconsole"
#define ARG_NAME_LAUNCHER_LOG "-Xtrace"
#define ARG_NAME_BOOTCLASS "-Xbootclass"
#define ARG_NAME_JDKHOME "-Xjdkhome"
#define ARG_NAME_CP_PREPEND "-Xcp:p"
#define ARG_NAME_CP_APPEND "-Xcp:a"

/* Below are standard JRuby args handled by the launcher. */
#define ARG_NAME_SERVER "--server"
#define ARG_NAME_CLIENT "--client"
#define ARG_NAME_SAMPLE "--sample"
#define ARG_NAME_MANAGE "--manage"
#define ARG_NAME_HEADLESS "--headless"
#define ARG_NAME_PROFILE "--profile"
#define ARG_NAME_NG_SERVER "--ng-server"
#define ARG_NAME_NG "--ng"

#endif	/* _ARGNAMES_H */

