#include "unixlauncher.h"
#include "utilsfuncs.h"

using namespace std;

extern "C" int nailgunClientMain(int argc, char *argv[], char *env[]);

char* findOnPath(const char* name) {
    string path(getenv("PATH"));
    size_t start = 0;
    size_t sep;
    char * found;

    while ((sep = path.find(":", start)) != string::npos) {
	string elem(path.substr(start, sep - start));
	if (elem[elem.length() - 1] != '/') {
	    elem += '/';
	}
	elem += name;

	if (fileExists(elem.c_str())) {
	    found = (char*) malloc(elem.length());
	    strncpy(found, elem.c_str(), elem.length());
	    return found;
	}

	start = sep + 1;
    }

    return NULL;
}

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
    if (!initPlatformDir() || !parseArgs(argc, argv)) {
	return 255;
    }

    if (nailgunClient) {
        progArgs.push_front("org.jruby.util.NailMain");
        const char ** nailArgv = convertToArgvArray(progArgs);
        return nailgunClientMain(progArgs.size(), (char**)nailArgv, envp);
    }

    prepareOptions();

    char * java = findOnPath("java");
    if (java == NULL) {
	printToConsole("No `java' executable found on PATH.");
	return 254;
    }

    list<string> commandLine;
    commandLine.insert(commandLine.end(), javaOptions.begin(), javaOptions.end());
    commandLine.insert(commandLine.end(), bootclass);
    commandLine.insert(commandLine.end(), progArgs.begin(), progArgs.end());

    logMsg("Command line:");
    logMsg(java);
    for (list<string>::iterator it = commandLine.begin(); it != commandLine.end(); ++it) {
	logMsg(it->c_str());
    }
    return execv(java, (char**)convertToArgvArray(commandLine));
}
