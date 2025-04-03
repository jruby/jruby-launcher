/*
 * Copyright 2009-2025 JRuby Team (www.jruby.org).
 */


#ifndef _UNIXLAUNCHER_H_
#define _UNIXLAUNCHER_H_

#ifdef __cplusplus
extern "C"
{
#endif

int unixlauncher_run(int argc, char *argv[], char *envp[]);

#ifdef __cplusplus
}
#endif

#endif // ! _UNIXLAUNCHER_H_
