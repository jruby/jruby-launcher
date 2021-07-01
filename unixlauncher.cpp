#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <limits.h>
#include <string.h>
#include "unixlauncher.h"
#include "utilsfuncs.h"

using namespace std;

extern "C" int nailgunClientMain(int argc, char *argv[], char *env[]);

UnixLauncher::UnixLauncher()
    : ArgParser()
{
}

UnixLauncher::UnixLauncher(const UnixLauncher& orig)
    : ArgParser(orig)
{
}

UnixLauncher::~UnixLauncher() {
}

int UnixLauncher::run(int argc, char* argv[], char* envp[]) {
    platformDir = argv[0];
    if (!initPlatformDir() || !parseArgs(argc - 1, argv + 1)) {
        return 255;
    }

    if (nailgunClient) {
        progArgs.push_front("org.jruby.util.NailMain");
        char ** nailArgv = convertToArgvArray(progArgs);
        int nailArgc = progArgs.size();

        if (printCommandLine) {
            printListToConsole(progArgs);
            for (int i = 0; i < nailArgc; i++) {
                free(nailArgv[i]);
            }
            delete[] nailArgv;
            return 0;
        }
        return nailgunClientMain(progArgs.size(), (char**)nailArgv, envp);
    }

    string java("");

    if (getenv("JAVACMD") != NULL) {
        java = getenv("JAVACMD");
        if (java.find_last_of('/') == -1) {
            java = findOnPath(java.c_str());
        }
    } else {
        if (!jdkhome.empty()) {
            java = jdkhome + "/bin/java";
        } else if (getenv("JAVA_HOME") != NULL) {
            string java_home = string(getenv("JAVA_HOME"));
            jdkhome = java_home;
            java_home = trimTrailingBackslashes(java_home);
            java = java_home + "/bin/java";
        } else {
            java = findOnPath("java");
        }
    }

    if (java.empty()) {
        printToConsole("No `java' executable found on PATH.");
        return 255;
    }

    // still no jdk home, use other means to resolve it
    if (jdkhome.empty()) {
        char javaHomeCommand[] = "/usr/libexec/java_home";
        if (access(javaHomeCommand, R_OK | X_OK) != -1 && !checkDirectory(javaHomeCommand)) {
            // try java_home command when not set (on MacOS)
            FILE *fp;
            char tmp[PATH_MAX + 1];

            fp = popen(javaHomeCommand, "r");
            if (fp != NULL) {
                fgets(tmp, sizeof(tmp), fp);
                tmp[strcspn(tmp, "\n")] = 0;
                jdkhome = tmp;
                pclose(fp);
            } else {
                logErr(true, false, "failed to run %s", javaHomeCommand);
            }
        } else {
            java = resolveSymlinks(java);
            int home_index = java.find_last_of('/', java.find_last_of('/') - 1);
            jdkhome = java.substr(0, home_index);
        }
    }

    prepareOptions();

    list<string> commandLine;
    commandLine.push_back(java);
    addOptionsToCommandLine(commandLine);

    logMsg("Command line:");
    for (list<string>::iterator it = commandLine.begin(); it != commandLine.end(); ++it) {
        logMsg("\t%s", it->c_str());
    }

    char** newArgv = convertToArgvArray(commandLine);
    int newArgc = commandLine.size();

    if (printCommandLine) {
        printListToConsole(commandLine);
        for (int i = 0; i < newArgc; i++) {
            free(newArgv[i]);
        }
        delete[] newArgv;
        return 0;
    }

    if (!fileExists(java.c_str())) {
        string msg = "No `java' exists at " + java + ", please double-check JAVA_HOME.\n";
        printToConsole(msg.c_str());
        return 255;
    }

    execv(java.c_str(), newArgv);

    // shouldn't get here unless something bad happened with execv
    logErr(true, true, "execv failed:");
    return 255;
}
