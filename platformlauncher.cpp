/*
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS HEADER.
 *
 * Copyright 1997-2008 Sun Microsystems, Inc. All rights reserved.
 *
 * The contents of this file are subject to the terms of either the GNU
 * General Public License Version 2 only ("GPL") or the Common
 * Development and Distribution License("CDDL") (collectively, the
 * "License"). You may not use this file except in compliance with the
 * License. You can obtain a copy of the License at
 * http://www.netbeans.org/cddl-gplv2.html
 * or nbbuild/licenses/CDDL-GPL-2-CP. See the License for the
 * specific language governing permissions and limitations under the
 * License.  When distributing the software, include this License Header
 * Notice in each file and include the License file at
 * nbbuild/licenses/CDDL-GPL-2-CP.  Sun designates this
 * particular file as subject to the "Classpath" exception as provided
 * by Sun in the GPL Version 2 section of the License file that
 * accompanied this code. If applicable, add the following below the
 * License Header, with the fields enclosed by brackets [] replaced by
 * your own identifying information:
 * "Portions Copyrighted [year] [name of copyright owner]"
 *
 * Contributor(s):
 *
 * The Original Software is NetBeans. The Initial Developer of the Original
 * Software is Sun Microsystems, Inc. Portions Copyright 1997-2008 Sun
 * Microsystems, Inc. All Rights Reserved.
 *
 * If you wish your version of this file to be governed by only the CDDL
 * or only the GPL Version 2, indicate your decision by adding
 * "[Contributor] elects to include this software in this distribution
 * under the [CDDL or GPL Version 2] license." If you do not indicate a
 * single choice of license, a recipient has the option to distribute
 * your version of this file under either the CDDL, the GPL Version 2 or
 * to extend the choice of license to its licensees as provided above.
 * However, if you add GPL Version 2 code and therefore, elected the GPL
 * Version 2 license, then the option applies only if the new code is
 * made subject to such option by the copyright holder.
 *
 * Author: Tomas Holy
 */

#include "utilsfuncs.h"
#include "platformlauncher.h"
#include "argnames.h"

using namespace std;

const char *PlatformLauncher::HELP_MSG =
"\nJRuby Launcher usage: jruby.exe {options} arguments\n\
General options:\n\
  --help                show this help\n\
  --jdkhome <path>      path to JDK\n\
  -J<jvm_option>        pass <jvm_option> to JVM\n\
\n\
  --cp:p <classpath>    prepend <classpath> to classpath\n\
  --cp:a <classpath>    append <classpath> to classpath\n\
\n\
  --fork-java           run java in separate process\n\
  --trace <path>        path for launcher log (for troubleshooting)\n\n\
To see general JRuby options, type 'jruby -h' or 'jruby --help'.\n\
--------------------------------------------------------------------\
\n\n";

const char *PlatformLauncher::REQ_JAVA_VERSION = "1.5";

const char *PlatformLauncher::OPT_JDK_HOME = "-Djdk.home=";
const char *PlatformLauncher::OPT_JRUBY_HOME = "-Djruby.home=";
const char *PlatformLauncher::OPT_JRUBY_COMMAND_NAME = "-Dsun.java.command=";

const char *PlatformLauncher::OPT_CLASS_PATH = "-Djava.class.path=";
const char *PlatformLauncher::OPT_BOOT_CLASS_PATH = "-Xbootclasspath/a:";

const char *PlatformLauncher::OPT_JRUBY_LIB = "-Djruby.lib=";
const char *PlatformLauncher::OPT_JRUBY_SHELL = "-Djruby.shell=";
const char *PlatformLauncher::OPT_JRUBY_SCRIPT = "-Djruby.script=";
const char *PlatformLauncher::MAIN_CLASS = "org/jruby/Main";
const char *PlatformLauncher::DEFAULT_EXECUTABLE = "jruby";

extern "C" int nailgunClientMain(int argc, char *argv[], char *env[]);

PlatformLauncher::PlatformLauncher()
    : separateProcess(false)
    , suppressConsole(false)
    , nailgunClient(false)
    , nailgunServer(false)
{
}

PlatformLauncher::PlatformLauncher(const PlatformLauncher& orig) {
}

PlatformLauncher::~PlatformLauncher() {
}

list<string>* GetEnvStringsAsList() {
    char * env = GetEnvironmentStrings();
    list<string> * envList = new list<string>();
    while (*env) {
        envList->push_back(env);
        env = env + strlen(env) + 1;
    }
    return envList;
}

const char** convertToArgvArray(list<string> args) {
    const char ** argv = (const char**) malloc(sizeof (char*) * args.size());
    int i = 0;
    for (list<string>::iterator it = args.begin(); it != args.end(); ++it, ++i) {
        argv[i] = it->c_str();
    }
    return argv;
}

bool PlatformLauncher::start(char* argv[], int argc, DWORD *retCode, const char* binaryName) {
    if (!checkLoggingArg(argc, argv, false) || !initPlatformDir() || !parseArgs(argc, argv)) {
        return false;
    }
    disableFolderVirtualization(GetCurrentProcess());

    if (nailgunClient) {
        progArgs.push_front("org.jruby.util.NailMain");
        const char ** nailArgv = convertToArgvArray(progArgs);
        list<string>* envList = GetEnvStringsAsList();
        const char ** nailEnv  = convertToArgvArray(*envList);
        nailgunClientMain(progArgs.size(), (char**)nailArgv, (char**)nailEnv);
        return true;
    }

    if (binaryName) {
        // clean up the binaryName first,
		// remove '.exe' from the end, and the possible path.
        string bn = binaryName;

        int found = bn.find_last_of("/\\");
        if (found != string::npos) {
            logMsg("The binary name contains slashes, will remove: %s", binaryName);
            bn = bn.substr(found + 1);
            binaryName = bn.c_str();
        }

        found = bn.find(".exe", bn.length() - 4);
        if (found != string::npos) {
            bn.erase(found, 4);
            binaryName = bn.c_str();
            logMsg("*** Cleaned up the binary name: %s", binaryName);
        } else {
            logMsg("*** No need to clean the binary name: %s", binaryName);
        }

        if (strcmp(binaryName, DEFAULT_EXECUTABLE) != 0) {
            logMsg("PlatformLauncher:\n\tNon-default executable name: %s", binaryName);
            logMsg("\tHence, launching with extra parameters: -S %s", binaryName);
            progArgs.push_front(binaryName);
            progArgs.push_front("-S");
        }
    }

    if (jdkhome.empty()) {
        if (!jvmLauncher.initialize(REQ_JAVA_VERSION)) {
            logErr(false, true, "Cannot find Java %s or higher.", REQ_JAVA_VERSION);
            return false;
        }
    }
    jvmLauncher.getJavaPath(jdkhome);

    prepareOptions();

    if (nextAction.empty()) {
        while (true) {
            // run app
            if (!run(retCode)) {
                return false;
            }

            break;
        }
    } else {
        logErr(false, true, "We should not get here.");
        return false;
    }

    return true;
}

bool PlatformLauncher::run(DWORD *retCode) {
    logMsg("Starting application...");
    constructClassPath();
    const char *mainClass;
    mainClass = bootclass.empty() ? MAIN_CLASS : bootclass.c_str();

    // replace '/' by '.' to report a better name to jps/jconsole
    string cmdName = mainClass;
    int position = cmdName.find("/");
    while (position != string::npos) {
      cmdName.replace(position, 1, ".");
      position = cmdName.find("/", position + 1);
    }

    string option = OPT_JRUBY_COMMAND_NAME;
    option += cmdName;
    javaOptions.push_back(option);

    option = OPT_CLASS_PATH;
    option += classPath;
    javaOptions.push_back(option);

    option = OPT_BOOT_CLASS_PATH;
    option += bootClassPath;
    javaOptions.push_back(option);

    jvmLauncher.setSuppressConsole(suppressConsole);
    bool rc = jvmLauncher.start(mainClass, progArgs, javaOptions, separateProcess, retCode);
    if (!separateProcess) {
        exit(0);
    }

    javaOptions.pop_back();
    javaOptions.pop_back();
    return rc;
}

bool PlatformLauncher::initPlatformDir() {
    char path[MAX_PATH] = "";
    getCurrentModulePath(path, MAX_PATH);
    logMsg("Module: %s", path);
    char *bslash = strrchr(path, '\\');
    if (!bslash) {
        return false;
    }
    *bslash = '\0';
    bslash = strrchr(path, '\\');
    if (!bslash) {
        return false;
    }
    *bslash = '\0';
    platformDir = path;
    logMsg("Platform dir: %s", platformDir.c_str());
    logMsg("classPath: %s", classPath.c_str());
    return true;
}

bool PlatformLauncher::parseArgs(int argc, char *argv[]) {
#define CHECK_ARG \
    if (i+1 == argc) {\
        logErr(false, true, "Argument is missing for \"%s\" option.", argv[i]);\
        return false;\
    }

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
            if (!jvmLauncher.initialize(jdkhome.c_str())) {
                logMsg("Cannot locate java installation in specified jdkhome: %s", jdkhome.c_str());
                string errMsg = "Cannot locate java installation in specified jdkhome:\n";
                errMsg += jdkhome;
                errMsg += "\nDo you want to try to use default version?";
                jdkhome = "";
                if (::MessageBox(NULL, errMsg.c_str(), "Invalid jdkhome specified", MB_ICONQUESTION | MB_YESNO) == IDNO) {
                    return false;
                }
            }
        } else if (strcmp(ARG_NAME_CP_PREPEND, argv[i]) == 0
                || strcmp(ARG_NAME_CP_PREPEND + 1, argv[i]) == 0) {
            CHECK_ARG;
            cpBefore += argv[++i];
        } else if (strcmp(ARG_NAME_CP_APPEND, argv[i]) == 0
                || strcmp(ARG_NAME_CP_APPEND + 1, argv[i]) == 0
                || strncmp(ARG_NAME_CP_APPEND + 1, argv[i], 3) == 0
                || strncmp(ARG_NAME_CP_APPEND, argv[i], 4) == 0) {
            CHECK_ARG;
            cpAfter += argv[++i];
        } else if (strcmp(ARG_NAME_SERVER, argv[i]) == 0
                || strcmp(ARG_NAME_CLIENT, argv[i]) == 0) {
            javaOptions.push_back(argv[i] + 1); // to JVMLauncher, -server instead of --server
        } else if (strcmp(ARG_NAME_SAMPLE, argv[i]) == 0) {
            javaOptions.push_back("-Xprof");
        } else if (strcmp(ARG_NAME_MANAGE, argv[i]) == 0) {
            javaOptions.push_back("-Dcom.sun.management.jmxremote");
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
        } else if (strcmp(argv[i], "-help") == 0) {
            printToConsole(HELP_MSG);
            if (!appendHelp.empty()) {
                printToConsole(appendHelp.c_str());
            }
            return false;
        } else {
            progArgs.push_back(argv[i]);
        }
    }

    if (jdkhome.empty()) {
        logMsg("--jdkhome is not set, checking for %%JAVA_HOME%%...");
        char *javaHome = getenv("JAVA_HOME");
        if (javaHome) {
            logMsg("%%JAVA_HOME%% is set: %s", javaHome);
            if (!jvmLauncher.initialize(javaHome)) {
                logMsg("Cannot locate java installation, specified by JAVA_HOME: %s", javaHome);
                string errMsg = "Cannot locate java installation, specified by JAVA_HOME:\n";
                errMsg += javaHome;
                errMsg += "\nDo you want to try to use default version?";
                jdkhome = "";
                if (::MessageBox(NULL, errMsg.c_str(),
                        "Invalid jdkhome specified", MB_ICONQUESTION | MB_YESNO) == IDNO) {
                    return false;
                }
            } else {
                jdkhome = javaHome;
            }
        }
    }

    return true;
}

void PlatformLauncher::prepareOptions() {
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
}

string & PlatformLauncher::constructClassPath() {
    logMsg("constructClassPath()");
    addedToCP.clear();
    classPath = cpBefore;
    
    addJarsToClassPathFrom(platformDir.c_str());

    char *envCP = getenv("CLASSPATH");
    if (envCP) {
        addToClassPath(envCP, false);
    }

    if (!cpAfter.empty()) {
        classPath += ";";
        classPath += cpAfter;
    }

    logMsg("ClassPath: %s", classPath.c_str());
    return classPath;
}

void PlatformLauncher::addJarsToClassPathFrom(const char *dir) {
    addFilesToClassPath(dir, "lib", "*.jar");
}

void PlatformLauncher::addFilesToClassPath(const char *dir, const char *subdir, const char *pattern) {
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
        if (addedToCP.insert(name).second) {
            addToClassPath(fullName.c_str());
            if (!nailgunServer) {
                addToBootClassPath(fullName.c_str());
            }
        } else {
            logMsg("\"%s\" already added, skipping \"%s\"", name.c_str(), fullName.c_str());
        }
    } while (FindNextFile(hFind, &fd));
    FindClose(hFind);
}

void PlatformLauncher::addToClassPath(const char *path, bool onlyIfExists) {
    logMsg("addToClassPath()\n\tpath: %s\n\tonlyIfExists: %s", path, onlyIfExists ? "true" : "false");
    if (onlyIfExists && !fileExists(path)) {
        return;
    }

    if (!classPath.empty()) {
        classPath += ';';
    }
    classPath += path;
}

void PlatformLauncher::addToBootClassPath(const char *path, bool onlyIfExists) {
    logMsg("addToBootClassPath()\n\tpath: %s\n\tonlyIfExists: %s", path, onlyIfExists ? "true" : "false");
    if (onlyIfExists && !fileExists(path)) {
        return;
    }

    if (!bootClassPath.empty()) {
        bootClassPath += ';';
    }
    bootClassPath += path;
}

void PlatformLauncher::appendToHelp(const char *msg) {
    if (msg) {
        appendHelp = msg;
    }
}

void PlatformLauncher::onExit() {
    logMsg("onExit()");
    if (separateProcess) {
        logMsg("JVM in separate process, no need to restart");
        return;
    }
}
