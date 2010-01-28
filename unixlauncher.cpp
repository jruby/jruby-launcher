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
	if (printCommandLine) {
	    return printArgvToConsole(nailArgv);
	}
        return nailgunClientMain(progArgs.size(), (char**)nailArgv, envp);
    }

    prepareOptions();

    char * java = findOnPath("java");
    if (java == NULL) {
	printToConsole("No `java' executable found on PATH.");
	return 255;
    }

    list<string> commandLine;
    commandLine.push_back(java);
    addOptionsToCommandLine(commandLine);

    logMsg("Command line:");
    for (list<string>::iterator it = commandLine.begin(); it != commandLine.end(); ++it) {
	logMsg(it->c_str());
    }

    char** newArgv = convertToArgvArray(commandLine);

    if (printCommandLine) {
	return printArgvToConsole(newArgv);
    }

    execv(java, newArgv);

    // shouldn't get here unless something bad happened with execv
    logErr(true, true, "execv failed:");
    return 255;
}
