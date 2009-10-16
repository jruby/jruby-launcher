#ifndef _NBEXECLOADER_H
#define	_NBEXECLOADER_H

#include "utilsfuncs.h"

#define HELP_MSG ""

class NBExecLoader {
    typedef int (*StartPlatform)(int argc, char *argv[], const char *help);

public:
    NBExecLoader()
        : hLib(0) {
    }
    ~NBExecLoader() {
        if (hLib) {
            FreeLibrary(hLib);
        }
    }
    int start(const char *path, int argc, char *argv[]) {
        if (!hLib) {
            hLib = LoadLibrary(path);
            if (!hLib) {
                logErr(true, true, "Cannot load \"%s\".", path);
                return -1;
            }
        }

        StartPlatform startPlatform = (StartPlatform) GetProcAddress(hLib, "startPlatform");
        if (!startPlatform) {
            logErr(true, true, "Cannot start platform, failed to find startPlatform() in %s", path);
            return -1;
        }
        logMsg("Starting platform...\n");
        return startPlatform(argc, argv, HELP_MSG);
    }

private:
    HMODULE hLib;
};

#endif	/* _NBEXECLOADER_H */

