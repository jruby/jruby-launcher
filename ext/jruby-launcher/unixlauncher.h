/*
 * Copyright 2009-2010 JRuby Team (www.jruby.org).
 */


#ifndef _UNIXLAUNCHER_H_
#define _UNIXLAUNCHER_H_

#include "argparser.h"

class UnixLauncher : public ArgParser {
public:
    UnixLauncher();
    virtual ~UnixLauncher();

    int run(int argc, char* argv[], char* envp[]);

private:
    UnixLauncher(const UnixLauncher& orig);
};

#endif // ! _UNIXLAUNCHER_H_
