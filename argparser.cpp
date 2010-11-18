/*
 * Copyright 2009-2010 JRuby Team (www.jruby.org).
 */

#include <cstring>
#include <cstdlib>
#include <memory>
#include "utilsfuncs.h"
#include "argparser.h"
#include "argnames.h"
#include "version.h"

#ifndef WIN32
#include <sys/types.h>
#include <sys/utsname.h>
#include <dirent.h>
#define EXEEXT ""
#else
#define EXEEXT ".exe"
#endif

using namespace std;

const char *ArgParser::HELP_MSG =
"JRuby Launcher usage: jruby" EXEEXT " {options} arguments\n\n\
Options:\n\
  -Xhelp                show this help\n\
  -Xversion             print launcher's version\n\
\nJvm Management:\n\
  -Xjdkhome <path>      set path to JDK\n\
  -Xfork-java           run java in separate process\n\
  -J<jvm_option>        pass <jvm_option> to JVM\n\
\nClasspath Management:\n\
  -Xcp   <classpath>    set the classpath\n\
  -Xcp:p <classpath>    prepend <classpath> to classpath\n\
  -Xcp:a <classpath>    append <classpath> to classpath\n\
  -Xnobootclasspath     don't put jruby jars on the bootclasspath\n\
\nMisc:\n\
  -Xtrace <path>        path for launcher log (for troubleshooting)\n\
  -Xcommand             just print the equivalent java command and exit\n\
  -Xprop.erty[=value]   equivalent to -J-Djruby.<prop.erty>[=value]\n\
  -Xproperties          list supported properties (omit \"jruby.\" with -X)\n"
#ifdef WIN32
"  -Xconsole <mode>      jrubyw console attach mode (new|attach|suppress)\n\n"
#endif
"To see general JRuby options, type 'jruby -h' or 'jruby --help'.\n";

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
    , noBootClassPath(false)
    , printCommandLine(false)
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
            string part(opts.substr(start, pos - start));
            if (part.size() > 0) {
                logMsg("%s += %s", envvar, part.c_str());
                optionsList.push_back(part);
            }
            start = pos + 1;
        }
        if (start < opts.size()) {
            string part(opts.substr(start));
            if (part.size() > 0) {
                logMsg("%s += %s", envvar, part.c_str());
                optionsList.push_back(part);
            }
        }
    }
}

#ifdef __MACH__
#include <mach-o/dyld.h>
#endif

bool ArgParser::initPlatformDir() {
#ifdef WIN32
    char path[MAX_PATH] = "";
    getCurrentModulePath(path, MAX_PATH);
#else
    char path[PATH_MAX] = "";
    bool found = false;

    // first try via linux /proc/self/exe
    logMsg("initPlatformDir: trying /proc/self/exe");
    found = readlink("/proc/self/exe", path, PATH_MAX) != -1;

#ifdef __MACH__
    uint32_t sz = PATH_MAX;
    if (_NSGetExecutablePath(path, &sz) == 0) { // OSX-specific
        logMsg("initPlatformDir: using _NSGetExecutablePath");
        string tmpPath(path);
        realpath(tmpPath.c_str(), path);
        found = true;
    }
#endif

#ifdef __SUNOS__
    const char* execname = getexecname();
    if (execname) {
        logMsg("initPlatformDir: using getexecname");
        char * dst = path;
        if (execname[0] != '/') {
            getcwd(path, PATH_MAX - strlen(execname) - 2);
            dst = path + strlen(path);
            *dst++ = '/';
        }

        strncpy(dst, execname, strlen(execname));
        found = true;
    }
#endif

    if (!found && platformDir[0] == '/') { // argv[0]: absolute path
        logMsg("initPlatformDir: argv[0] appears to be an absolute path");
        strncpy(path, platformDir.c_str(), PATH_MAX);
        found = true;
    }

    if (!found && platformDir.find('/') != string::npos) { // argv[0]: relative path
        logMsg("initPlatformDir: argv[0] appears to be a relative path");
        getcwd(path, PATH_MAX - platformDir.length() - 1);
        strncpy(path + strlen(path), platformDir.c_str(), platformDir.length());
        found = true;
    }

    if (!found) {               // try via PATH search
        logMsg("initPlatformDir: trying to find executable on PATH");
        char * location = findOnPath(platformDir.c_str());
        if (location != NULL) {
            strncpy(path, location, PATH_MAX);
            free(location);
            found = true;
        }
    }

    if (!found) {               // try via JRUBY_HOME
        if (getenv("JRUBY_HOME") != NULL) {
            logMsg("initPlatformDir: trying JRUBY_HOME environment variable");
            strncpy(path, getenv("JRUBY_HOME"), PATH_MAX - 11);
            strncpy(path + strlen(path), "/bin/jruby", 10);
            found = true;
        }
    }

    if (!fileExists(path)) {
        printToConsole("Could not figure out a proper location for JRuby.\n"
                       "Try `jruby -Xtrace trace.log ...` and view trace.log for details.");
        return false;
    }
#endif

    logMsg("Module: %s", path);
    char *bslash = strrchr(path, FILE_SEP);
    if (!bslash) {
        return false;
    }
    *bslash = '\0';
    bslash = strrchr(path, FILE_SEP);
    if (!bslash) {
        return false;
    }
    *bslash = '\0';
    platformDir = path;
    logMsg("Platform dir: %s", platformDir.c_str());
    return true;
}

bool ArgParser::parseArgs(int argc, char *argv[]) {
    list<string> args;

#define CHECK_ARG                                                       \
    it++;								\
    if (it == args.end() || it->empty() || it->at(0) == '-') {	\
        logErr(false, true, "Argument is missing for \"%s\" option.", (--it)->c_str()); \
        return false;                                                   \
    }									\
    it--;
#define INCR ++it, ++i

    addEnvVarToOptions(javaOptions, "JAVA_OPTS");
    addEnvVarToOptions(args, "JRUBY_OPTS");

#ifdef __MACH__
    if (getenv("JAVA_ENCODING") == NULL) {
        javaOptions.push_back("-Dfile.encoding=UTF-8");
    }
#endif

    addToArgList(args, argc, argv);

    logMsg("Parsing arguments:");
    for (list<string>::iterator it = args.begin(); it != args.end(); ++it) {
        logMsg("\t%s", it->c_str());
    }

    bool doneScanning = false;

    int i = 0;
    for (list<string>::iterator it = args.begin(); it != args.end(); INCR) {
        if (doneScanning) {
            progArgs.push_back(*it);
        } else if (it->compare("--") == 0 || it->empty() || it->at(0) != '-') {
            progArgs.push_back(*it);
            doneScanning = true;
        } else if (it->compare(ARG_NAME_SEPAR_PROC) == 0) {
            separateProcess = true;
            logMsg("Run Java in separater process");
        } else if (it->compare(ARG_NAME_CMD_ONLY) == 0) {
            printCommandLine = true;
        } else if (it->compare(ARG_NAME_NO_BOOTCLASSPATH) == 0) {
            noBootClassPath = true;
        } else if (it->compare(ARG_NAME_LAUNCHER_LOG) == 0) {
            // We only check the validity of args here,
            // the actual parsing and setting the log file
            // is done earlier, in checkLoggingArg()
            CHECK_ARG;
            INCR;
        } else if (it->compare(ARG_NAME_BOOTCLASS) == 0) {
            CHECK_ARG;
	    INCR;
            bootclass = *it;
        } else if (it->compare(ARG_NAME_JDKHOME) == 0) {
            CHECK_ARG;
	    INCR;
            jdkhome = *it;
        } else if (it->compare(ARG_NAME_CP_PREPEND) == 0) {
            CHECK_ARG;
            if (!cpBefore.empty()) {
                cpBefore += PATH_SEP;
            }
	    INCR;
            cpBefore += *it;
        } else if (it->compare(ARG_NAME_CP_APPEND) == 0) {
            CHECK_ARG;
            if (!cpAfter.empty()) {
                cpAfter += PATH_SEP;
            }
	    INCR;
            cpAfter += *it;
        } else if (it->compare(ARG_NAME_CP) == 0
		   || it->compare(ARG_NAME_CLASSPATH) == 0) {
            CHECK_ARG;
            if (!cpExplicit.empty()) {
                cpExplicit += PATH_SEP;
            }
	    INCR;
            cpExplicit += *it;
        } else if (it->compare(ARG_NAME_SERVER) == 0
                || it->compare(ARG_NAME_CLIENT) == 0) {
            javaOptions.push_back(it->substr(1)); // to JVMLauncher, -server instead of --server
        } else if (it->compare(ARG_NAME_SAMPLE) == 0) {
            javaOptions.push_back("-Xprof");
        } else if (it->compare(ARG_NAME_MANAGE) == 0) {
            javaOptions.push_back("-Dcom.sun.management.jmxremote");
            javaOptions.push_back("-Djruby.management.enabled=true");
        } else if (it->compare(ARG_NAME_HEADLESS) == 0) {
            javaOptions.push_back("-Djava.awt.headless=true");
        } else if (it->compare(ARG_NAME_NG) == 0) {
            nailgunClient = true;
        } else if (it->compare(ARG_NAME_NG_SERVER) == 0) {
            bootclass = "com/martiansoftware/nailgun/NGServer";
            javaOptions.push_back("-server");
            noBootClassPath = true;
        } else if (it->compare(0, 2, "-J", 2) == 0) {
            std::string javaOpt = it->substr(2);
            if (javaOpt.compare(0, 3, "-ea", 3) == 0
                    || javaOpt.compare(0, 17, "-enableassertions", 17) == 0) {
                logMsg("Note: -ea option is specified, there will be no bootclasspath in order to enable assertions");
                noBootClassPath = true;
            }
            javaOptions.push_back(javaOpt);
        } else if (strcmp(it->c_str(), "-Xhelp") == 0) {
            printToConsole(HELP_MSG);
            if (!appendHelp.empty()) {
                printToConsole(appendHelp.c_str());
            }
            return false;
        } else if (strcmp(it->c_str(), "-Xversion") == 0) {
            printToConsole("JRuby Launcher Version " JRUBY_LAUNCHER_VERSION "\n");
            return false;
        } else if (strcmp(it->c_str(), "-Xproperties") == 0) {
			progArgs.push_back(std::string("--properties"));
        } else if (it->compare(0, 2, "-X", 2) == 0 && islower(it->c_str()[2])) {
	        // Any other /-X([a-z].*)/ get turned into a -Djruby.\1 property
			std::string propPart = it->substr(2);
	        std::string propSet = std::string("-Djruby.");
			propSet += propPart;
            javaOptions.push_back(propSet);
        } else {
            progArgs.push_back(*it);
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
#ifdef WIN32
    option += "cmd.exe";
#else
    const char* shell = getenv("SHELL");
    if (shell == NULL) {
        shell = "/bin/sh";
    }
    option += shell;
#endif
    javaOptions.push_back(option);

    option = OPT_JFFI_PATH;
#ifdef WIN32
    option += (platformDir + "\\lib\\native\\i386-Windows;"
            + platformDir + "\\lib\\native\\x86_64-Windows");
#else
    struct utsname name;
    if (uname(&name) == 0) {
        string ffiPath, ffiBase(platformDir + "/lib/native");
        DIR* dir = opendir(ffiBase.c_str());
        struct dirent* ent;
        if (dir != NULL) {
            while ((ent = readdir(dir)) != NULL) {
                string entry(ent->d_name);
                if (entry.find(name.sysname) != string::npos) {
                    if (!ffiPath.empty()) {
                        ffiPath += PATH_SEP;
                    }
                    ffiPath += ffiBase + FILE_SEP + entry;
                }
            }
            closedir(dir);
        }
        option += ffiPath;
    }
#endif
    javaOptions.push_back(option);

    setupMaxHeapAndStack();

    constructBootClassPath();
    constructClassPath();

    if (bootclass.empty()) {
        bootclass = MAIN_CLASS;
    }

    // replace '/' by '.' to report a better name to jps/jconsole
    string cmdName = bootclass;
    size_t position = cmdName.find("/");
    while (position != string::npos) {
      cmdName.replace(position, 1, ".");
      position = cmdName.find("/", position + 1);
    }

    option = OPT_JRUBY_COMMAND_NAME;
    option += cmdName;
    javaOptions.push_back(option);

    option = OPT_CLASS_PATH;
    option += classPath;
    javaOptions.push_back(option);

    if (!bootClassPath.empty()) {
        option = OPT_BOOT_CLASS_PATH;
        option += bootClassPath;
        javaOptions.push_back(option);
    }
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

    string jruby_complete_jar = platformDir + FILE_SEP + "lib" + FILE_SEP + "jruby-complete.jar";
    string jruby_jar = platformDir + FILE_SEP + "lib" + FILE_SEP + "jruby.jar";

    if (fileExists(jruby_jar.c_str())) {
      addToBootClassPath(jruby_jar.c_str(), true);
      if (fileExists(jruby_complete_jar.c_str())) {
          printToConsole("WARNING: Both jruby-complete.jar and jruby.jar are present in the 'lib' directory. Will use jruby.jar\n");
      }
    } else if (fileExists(jruby_complete_jar.c_str())) {
        addToBootClassPath(jruby_complete_jar.c_str());
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
        classPath += PATH_SEP;
        classPath += cpAfter;
    }

    // JRUBY-4709: Include this by default to have PWD as part of classpath
    if (!classPath.empty()) {
        classPath += PATH_SEP;
    }

    logMsg("ClassPath: %s", classPath.c_str());
}

void ArgParser::addJarsToClassPathFrom(const char *dir) {
    logMsg("addJarsToClassPathFrom()\n\tdir: %s", dir);
    string path = dir;
    path += FILE_SEP;
    path += "lib";

#ifdef WIN32
    WIN32_FIND_DATA fd = {0};
    string patternPath = path + FILE_SEP + "*.jar";
    HANDLE hFind = FindFirstFile(patternPath.c_str(), &fd);
    if (hFind == INVALID_HANDLE_VALUE) {
        logMsg("Nothing found (%s)", patternPath.c_str());
        return;
    }
    do {
        string fullName = path + FILE_SEP + fd.cFileName;
        addToClassPath(fullName.c_str());
    } while (FindNextFile(hFind, &fd));
    FindClose(hFind);
#else
    DIR *directory = opendir(path.c_str());
    if (!directory) {
        logMsg("Nothing found (%s)", path.c_str());
        return;
    }

    struct dirent *ent;
    while((ent = readdir(directory)) != NULL) {
        int len = strlen(ent->d_name);
        if (len > 4 && strncmp(".jar", (ent->d_name + (len - 4)), 4) == 0) {
            string fullName = path + FILE_SEP + ent->d_name;
            addToClassPath(fullName.c_str());
        }
    }
    closedir(directory);
#endif
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
            classPath += PATH_SEP;
        }
        classPath += path;
    } else {
        logMsg("No need to add \"%s\" to classpath, it's already in bootclasspath", path);
    }
}

void ArgParser::addToBootClassPath(const char *path, bool onlyIfExists) {
    logMsg("addToBootClassPath()\n\tpath: %s\n\tonlyIfExists: %s", path, onlyIfExists ? "true" : "false");

    if (noBootClassPath) {
        logMsg("NOTE: In this mode there is no bootclasspath, adding to the classpath instead...");
        return addToClassPath(path, onlyIfExists);
    }

    if (onlyIfExists && !fileExists(path)) {
        return;
    }

    // only add non-duplicates
    if (addedToBootCP.insert(path).second) {
        if (!bootClassPath.empty()) {
            bootClassPath += PATH_SEP;
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

void ArgParser::addOptionsToCommandLine(list<string> & commandLine) {
    commandLine.insert(commandLine.end(), javaOptions.begin(), javaOptions.end());
    commandLine.insert(commandLine.end(), bootclass);
    commandLine.insert(commandLine.end(), progArgs.begin(), progArgs.end());
}
