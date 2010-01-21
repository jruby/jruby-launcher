/*
 * Copyright 2009-2010 JRuby Team (www.jruby.org).
 */

#include "utilsfuncs.h"
#include "argparser.h"
#include "argnames.h"

using namespace std;

const char *ArgParser::HELP_MSG =
"\nJRuby Launcher usage: jruby.exe {options} arguments\n\
Options:\n\
  -Xhelp                show this help\n\
  -Xjdkhome <path>      path to JDK\n\
  -J<jvm_option>        pass <jvm_option> to JVM\n\
\n\
  -Xcp   <classpath>    set the classpath\n\
  -Xcp:p <classpath>    prepend <classpath> to classpath\n\
  -Xcp:a <classpath>    append <classpath> to classpath\n\
\n\
  -Xfork-java           run java in separate process\n\
  -Xtrace <path>        path for launcher log (for troubleshooting)\n\
  -Xconsole <mode>      jrubyw console attach mode (new|attach|suppress)\n\n\
To see general JRuby options, type 'jruby -h' or 'jruby --help'.\n\
--------------------------------------------------------------------\n\n";

const char *ArgParser::REQ_JAVA_VERSION = "1.5";

const char *ArgParser::OPT_JDK_HOME = "-Djdk.home=";
const char *ArgParser::OPT_JRUBY_HOME = "-Djruby.home=";
const char *ArgParser::OPT_JRUBY_COMMAND_NAME = "-Dsun.java.command=";

const char *ArgParser::OPT_CLASS_PATH = "-Djava.class.path=";
const char *ArgParser::OPT_BOOT_CLASS_PATH = "-Xbootclasspath/a:";

const char *ArgParser::OPT_JFFI_PATH = "-Djffi.boot.library.path=";
const char *ArgParser::OPT_JRUBY_SHELL = "-Djruby.shell=";
const char *ArgParser::OPT_JRUBY_SCRIPT = "-Djruby.script=";
const char *ArgParser::MAIN_CLASS = "org/jruby/Main";
const char *ArgParser::DEFAULT_EXECUTABLE = "jruby";

ArgParser::ArgParser()
    : separateProcess(false)
    , nailgunClient(false)
    , nailgunServer(false)
{
}

ArgParser::ArgParser(const ArgParser& orig) {
}

ArgParser::~ArgParser() {
}

void ArgParser::addEnvVarToOptions(std::list<std::string> & optionsList, const char * envvar) {
    const char * value = getenv(envvar);
    string opts("");
    if (value) {
        opts = value;
    }

    if (opts.size() > 0) {
        if (opts[0] == '"' || opts[0] == '\'') {
            char quote = opts[0];
            if (opts[opts.size() - 1] == quote) {
                opts = opts.substr(1, opts.size() - 2);
            }
        }

        size_t start = 0, pos = 0;
        while ((pos = opts.find(' ', start)) != string::npos) {
            string part(opts.substr(start, pos));
            if (part.size() > 0) {
                logMsg("%s += %s", envvar, part.c_str());
                optionsList.push_back(part);
            }
            start = pos + 1;
        }
        if (start < opts.size()) {
            string part(opts.substr(start, string::npos));
            if (part.size() > 0) {
                logMsg("%s += %s", envvar, part.c_str());
                optionsList.push_back(part);
            }
        }
    }
}

bool ArgParser::parseArgs(int argc, char *argv[]) {
#define CHECK_ARG \
    if (i+1 == argc) {\
        logErr(false, true, "Argument is missing for \"%s\" option.", argv[i]);\
        return false;\
    }

    addEnvVarToOptions(javaOptions, "JAVA_OPTS");
    addEnvVarToOptions(progArgs, "JRUBY_OPTS");

    logMsg("Parsing arguments:");
    for (int i = 0; i < argc; i++) {
        logMsg("\t%s", argv[i]);
    }
    bool doneScanning = false;

    for (int i = 0; i < argc; i++) {
        if (doneScanning) {
            progArgs.push_back(argv[i]);
        } else if (strcmp("--", argv[i]) == 0) {
            progArgs.push_back(argv[i]);
            doneScanning = true;
        } else if (strcmp(ARG_NAME_SEPAR_PROC, argv[i]) == 0) {
            separateProcess = true;
            logMsg("Run Java in separater process");
        } else if (strcmp(ARG_NAME_LAUNCHER_LOG, argv[i]) == 0) {
            CHECK_ARG;
            i++;
        } else if (strcmp(ARG_NAME_BOOTCLASS, argv[i]) == 0) {
            CHECK_ARG;
            bootclass = argv[++i];
        } else if (strcmp(ARG_NAME_JDKHOME, argv[i]) == 0) {
            CHECK_ARG;
            jdkhome = argv[++i];
        } else if (strcmp(ARG_NAME_CP_PREPEND, argv[i]) == 0
                || strcmp(ARG_NAME_CP_PREPEND + 1, argv[i]) == 0) {
            CHECK_ARG;
            if (!cpBefore.empty()) {
                cpBefore += ';';
            }
            cpBefore += argv[++i];
        } else if (strcmp(ARG_NAME_CP_APPEND, argv[i]) == 0
                || strcmp(ARG_NAME_CP_APPEND + 1, argv[i]) == 0) {
            CHECK_ARG;
            if (!cpAfter.empty()) {
                cpAfter += ';';
            }
            cpAfter += argv[++i];
        } else if (strcmp(ARG_NAME_CP, argv[i]) == 0
                || strcmp(ARG_NAME_CP + 1, argv[i]) == 0
                || strcmp(ARG_NAME_CLASSPATH, argv[i]) == 0
                || strcmp(ARG_NAME_CLASSPATH + 1, argv[i]) == 0
                || strncmp(ARG_NAME_CP_APPEND + 1, argv[i], 3) == 0
                || strncmp(ARG_NAME_CP_APPEND, argv[i], 4) == 0) {
            // handling -Xcp, -J-cp or -J-classpath options
            CHECK_ARG;
            if (!cpExplicit.empty()) {
                cpExplicit += ';';
            }
            cpExplicit += argv[++i];
        } else if (strcmp(ARG_NAME_SERVER, argv[i]) == 0
                || strcmp(ARG_NAME_CLIENT, argv[i]) == 0) {
            javaOptions.push_back(argv[i] + 1); // to JVMLauncher, -server instead of --server
        } else if (strcmp(ARG_NAME_SAMPLE, argv[i]) == 0) {
            javaOptions.push_back("-Xprof");
        } else if (strcmp(ARG_NAME_MANAGE, argv[i]) == 0) {
            javaOptions.push_back("-Dcom.sun.management.jmxremote");
            javaOptions.push_back("-Djruby.management.enabled=true");
        } else if (strcmp(ARG_NAME_HEADLESS, argv[i]) == 0) {
            javaOptions.push_back("-Djava.awt.headless=true");
        } else if (strcmp(ARG_NAME_PROFILE, argv[i]) == 0 ||
                strcmp(ARG_NAME_PROFILE "-all", argv[i]) == 0) {
            std::string filterType = strlen(argv[i]) == strlen(ARG_NAME_PROFILE) ? "ruby" : "all";
            javaOptions.push_front("-Dprofile.properties=" + platformDir + "/lib/profile-" + filterType + ".properties");
            javaOptions.push_front("-javaagent:" + platformDir + "/lib/profile.jar");
            progArgs.push_back("-X+C");
            printToConsole("Running with instrumented profiler\n");
        } else if (strcmp(ARG_NAME_NG, argv[i]) == 0) {
            nailgunClient = true;
        } else if (strcmp(ARG_NAME_NG_SERVER, argv[i]) == 0) {
            bootclass = "com/martiansoftware/nailgun/NGServer";
            javaOptions.push_back("-server");
            nailgunServer = true;
        } else if (strncmp("-J", argv[i], 2) == 0) {
            javaOptions.push_back(argv[i] + 2);
        } else if (strcmp(argv[i], "-Xhelp") == 0) {
            printToConsole(HELP_MSG);
            if (!appendHelp.empty()) {
                printToConsole(appendHelp.c_str());
            }
            return false;
        } else {
            progArgs.push_back(argv[i]);
        }
    }

    return true;
}

void ArgParser::prepareOptions() {
    string option = OPT_JDK_HOME;
    option += jdkhome;
    javaOptions.push_back(option);

    option = OPT_JRUBY_HOME;
    option += platformDir;
    javaOptions.push_back(option);

    option = OPT_JRUBY_SCRIPT;
    option += "jruby";
    javaOptions.push_back(option);

    option = OPT_JRUBY_SHELL;
    option += "cmd.exe";
    javaOptions.push_back(option);

    option = OPT_JFFI_PATH;
    option += (platformDir + "\\lib\\native\\i386-Windows;"
            + platformDir + "\\lib\\native\\x86_64-Windows");
    javaOptions.push_back(option);

    setupMaxHeapAndStack();
}

void ArgParser::setupMaxHeapAndStack() {
    // Hard-coded 500m, 1024k is for consistency with jruby shell script.
    string heapSize("500m"), stackSize("1024k");
    bool maxHeap = false, maxStack = false;
    for (list<string>::iterator it = javaOptions.begin(); it != javaOptions.end(); it++) {
        if (!maxHeap && strncmp("-Xmx", it->c_str(), 4) == 0) {
            heapSize = it->substr(4, it->size() - 4);
            maxHeap = true;
        }
        if (!maxStack && strncmp("-Xss", it->c_str(), 4) == 0) {
            stackSize = it->substr(4, it->size() - 4);
            maxStack = true;
        }
    }
    if (!maxHeap) {
        javaOptions.push_back("-Xmx" + heapSize);
    }
    if (!maxStack) {
        javaOptions.push_back("-Xss" + stackSize);
    }
    javaOptions.push_back("-Djruby.memory.max=" + heapSize);
    javaOptions.push_back("-Djruby.stack.max=" + stackSize);
}

void ArgParser::constructBootClassPath() {
    logMsg("constructBootClassPath()");
    addedToBootCP.clear();
    addedToCP.clear();
    classPath = cpBefore;

    string jruby_complete_jar = platformDir + "\\lib\\jruby-complete.jar";
    string jruby_jar = platformDir + "\\lib\\jruby.jar";

    if (fileExists(jruby_complete_jar.c_str())) {
        if (fileExists(jruby_jar.c_str())) {
            printToConsole("ERROR: Both jruby-complete.jar and jruby.jar are present in the 'lib' directory.");
            exit(10);
        }
        addToBootClassPath(jruby_complete_jar.c_str());
    } else {
        addToBootClassPath(jruby_jar.c_str(), true);
    }

    logMsg("BootclassPath: %s", bootClassPath.c_str());
}

void ArgParser::constructClassPath() {
    logMsg("constructClassPath()");

    addJarsToClassPathFrom(platformDir.c_str());

    if (cpExplicit.empty()) {
        logMsg("No explicit classpath option is used, looking up %%CLASSPATH%% env");
        char *envCP = getenv("CLASSPATH");
        if (envCP) {
            addToClassPath(envCP, false);
        }
    } else {
        logMsg("Explicit classpath option is used, ignoring %%CLASSPATH%% env");
        addToClassPath(cpExplicit.c_str(), false);
    }

    if (!cpAfter.empty()) {
        classPath += ";";
        classPath += cpAfter;
    }

    logMsg("ClassPath: %s", classPath.c_str());
}

void ArgParser::addJarsToClassPathFrom(const char *dir) {
    addFilesToClassPath(dir, "lib", "*.jar");
}

void ArgParser::addFilesToClassPath(const char *dir, const char *subdir, const char *pattern) {
    logMsg("addFilesToClassPath()\n\tdir: %s\n\tsubdir: %s\n\tpattern: %s", dir, subdir, pattern);
    string path = dir;
    path += '\\';
    path += subdir;
    path += '\\';

    WIN32_FIND_DATA fd = {0};
    string patternPath = path + pattern;
    HANDLE hFind = FindFirstFile(patternPath.c_str(), &fd);
    if (hFind == INVALID_HANDLE_VALUE) {
        logMsg("Nothing found (%s)", patternPath.c_str());
        return;
    }
    do {
        string name = subdir;
        name += fd.cFileName;
        string fullName = path + fd.cFileName;
        addToClassPath(fullName.c_str());
    } while (FindNextFile(hFind, &fd));
    FindClose(hFind);
}

void ArgParser::addToClassPath(const char *path, bool onlyIfExists) {
    logMsg("addToClassPath()\n\tpath: %s\n\tonlyIfExists: %s", path, onlyIfExists ? "true" : "false");
    if (onlyIfExists && !fileExists(path)) {
        return;
    }

    if (!addedToCP.insert(path).second) {
        logMsg("\"%s\" already added, skipping", path);
        return;
    }

    // check that this hasn't been added to boot class path already
    if (addedToBootCP.find(path) == addedToBootCP.end()) {
        if (!classPath.empty()) {
            classPath += ';';
        }
        classPath += path;
    } else {
        logMsg("No need to add \"%s\" to classpath, it's already in bootclasspath", path);
    }
}

void ArgParser::addToBootClassPath(const char *path, bool onlyIfExists) {
    logMsg("addToBootClassPath()\n\tpath: %s\n\tonlyIfExists: %s", path, onlyIfExists ? "true" : "false");

    if (nailgunServer) {
        logMsg("NOTE: In 'ng-server' mode there is no bootclasspath, adding to classpath...");
        return addToClassPath(path, onlyIfExists);
    }

    if (onlyIfExists && !fileExists(path)) {
        return;
    }

    // only add non-duplicates
    if (addedToBootCP.insert(path).second) {
        if (!bootClassPath.empty()) {
            bootClassPath += ';';
        }
        bootClassPath += path;
    } else {
        logMsg("\"%s\" already in bootclasspath", path);
    }
}

void ArgParser::appendToHelp(const char *msg) {
    if (msg) {
        appendHelp = msg;
    }
}
