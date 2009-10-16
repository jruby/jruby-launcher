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
"\nUsage: launcher {options} arguments\n\
\n\
General options:\n\
  --help                show this help\n\
  --jdkhome <path>      path to JDK\n\
  -J<jvm_option>        pass <jvm_option> to JVM\n\
\n\
  --cp:p <classpath>    prepend <classpath> to classpath\n\
  --cp:a <classpath>    append <classpath> to classpath\n\
\n\
  --fork-java           run java in separate process\n\
  --trace <path>        path for launcher log (for trouble shooting)\n\
\n";

const char *PlatformLauncher::REQ_JAVA_VERSION = "1.5";

const char *PlatformLauncher::OPT_JDK_HOME = "-Djdk.home=";
const char *PlatformLauncher::OPT_NB_PLATFORM_HOME = "-Djruby.home=";
const char *PlatformLauncher::OPT_NB_USERDIR = "-Dnetbeans.user=";
const char *PlatformLauncher::OPT_HTTP_PROXY = "-Dnetbeans.system_http_proxy=";
const char *PlatformLauncher::OPT_HTTP_NONPROXY = "-Dnetbeans.system_http_non_proxy_hosts=";
const char *PlatformLauncher::OPT_SOCKS_PROXY = "-Dnetbeans.system_socks_proxy=";
const char *PlatformLauncher::OPT_HEAP_DUMP = "-XX:+HeapDumpOnOutOfMemoryError";
const char *PlatformLauncher::OPT_HEAP_DUMP_PATH = "-XX:HeapDumpPath=";
const char *PlatformLauncher::OPT_KEEP_WORKING_SET_ON_MINIMIZE = "-Dsun.awt.keepWorkingSetOnMinimize=true";
const char *PlatformLauncher::OPT_CLASS_PATH = "-Djava.class.path=";
const char *PlatformLauncher::OPT_BOOT_CLASS_PATH = "-Xbootclasspath/a:";

const char *PlatformLauncher::OPT_JRUBY_LIB = "-Djruby.lib=";
const char *PlatformLauncher::OPT_JRUBY_SHELL = "-Djruby.shell=";
const char *PlatformLauncher::OPT_JRUBY_SCRIPT = "-Djruby.script=";

const char *PlatformLauncher::REG_PROXY_KEY = "Software\\Microsoft\\Windows\\CurrentVersion\\Internet settings";
const char *PlatformLauncher::REG_PROXY_ENABLED_NAME = "ProxyEnable";
const char *PlatformLauncher::REG_PROXY_SERVER_NAME = "ProxyServer";
const char *PlatformLauncher::REG_PROXY_OVERRIDE_NAME = "ProxyOverride";
const char *PlatformLauncher::PROXY_DIRECT = "DIRECT";
const char *PlatformLauncher::HEAP_DUMP_PATH =  "\\var\\log\\heapdump.hprof";
const char *PlatformLauncher::RESTART_FILE_PATH =  "\\var\\restart";

const char *PlatformLauncher::UPDATER_MAIN_CLASS = "org/netbeans/updater/UpdaterFrame";
const char *PlatformLauncher::IDE_MAIN_CLASS = "org/jruby/Main";

PlatformLauncher::PlatformLauncher()
    : separateProcess(false)
    , suppressConsole(false)
    , heapDumpPathOptFound(false)
    , exiting(false) {
}

PlatformLauncher::PlatformLauncher(const PlatformLauncher& orig) {
}

PlatformLauncher::~PlatformLauncher() {
}

bool PlatformLauncher::start(char* argv[], int argc, DWORD *retCode) {
    if (!checkLoggingArg(argc, argv, false) || !initPlatformDir() || !parseArgs(argc, argv)) {
        return false;
    }
    disableFolderVirtualization(GetCurrentProcess());

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
            if (!run(false, retCode)) {
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

bool PlatformLauncher::run(bool updater, DWORD *retCode) {
    logMsg(updater ? "Starting updater..." : "Starting application...");
    constructClassPath(updater);
    const char *mainClass;
    if (updater) {
        mainClass = UPDATER_MAIN_CLASS;
        nextAction = ARG_NAME_LA_START_APP;
    } else {
        DeleteFile((userDir + RESTART_FILE_PATH).c_str());
        mainClass = bootclass.empty() ? IDE_MAIN_CLASS : bootclass.c_str();
        nextAction = ARG_NAME_LA_START_AU;
    }

    string option = OPT_CLASS_PATH;
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
    clusters = platformDir = path;
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

    for (int i = 0; i < argc; i++) {
        if (strcmp(ARG_NAME_SEPAR_PROC, argv[i]) == 0) {
            separateProcess = true;
            logMsg("Run Java in separater process");
        } else if (strcmp(ARG_NAME_LAUNCHER_LOG, argv[i]) == 0) {
            CHECK_ARG;
            i++;
        } else if (strcmp(ARG_NAME_LA_START_APP, argv[i]) == 0
                || strcmp(ARG_NAME_LA_START_AU, argv[i]) == 0) {
            nextAction = argv[i++];
            logMsg("Next launcher action: %s", nextAction.c_str());
        } else if (strcmp(ARG_NAME_LA_PPID, argv[i]) == 0) {
            CHECK_ARG;
            suppressConsole = false;
            parentProcID = argv[++i];
            logMsg("Parent process ID found: %s", parentProcID.c_str());
        } else if (strcmp(ARG_NAME_USER_DIR, argv[i]) == 0) {
            CHECK_ARG;
            char tmp[MAX_PATH + 1] = {0};
            strncpy(tmp, argv[++i], MAX_PATH);
            if (strcmp(tmp, "memory") != 0 && !normalizePath(tmp, MAX_PATH)) {
                logErr(false, true, "User directory path \"%s\" is not valid.", argv[i]);
                return false;
            }
            userDir = tmp;
            logMsg("User dir: %s", userDir.c_str());
        } else if (strcmp(ARG_NAME_CLUSTERS, argv[i]) == 0) {
            CHECK_ARG;
            clusters = argv[++i];
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
        } else if (strncmp("-J", argv[i], 2) == 0) {
            javaOptions.push_back(argv[i] + 2);
            if (strncmp(argv[i] + 2, OPT_HEAP_DUMP_PATH, strlen(OPT_HEAP_DUMP_PATH)) == 0) {
                heapDumpPathOptFound = true;
            }
        } else {
            if (strcmp(argv[i], "-h") == 0
                    || strcmp(argv[i], "-help") == 0
                    || strcmp(argv[i], "--help") == 0
                    || strcmp(argv[i], "/?") == 0) {
                printToConsole(HELP_MSG);
                if (!appendHelp.empty()) {
                    printToConsole(appendHelp.c_str());
                }
            }
            progArgs.push_back(argv[i]);
        }
    }
    return true;
}

void PlatformLauncher::prepareOptions() {
    string option = OPT_JDK_HOME;
    option += jdkhome;
    javaOptions.push_back(option);

    option = OPT_NB_PLATFORM_HOME;
    option += platformDir;
    javaOptions.push_back(option);

    option = OPT_JRUBY_SCRIPT;
    option += "jruby.exe";
    javaOptions.push_back(option);

    option = OPT_JRUBY_SHELL;
    option += "cmd.exe";
    javaOptions.push_back(option);

    option = OPT_NB_USERDIR;
    option += userDir;
    javaOptions.push_back(option);

    option = OPT_HEAP_DUMP;
    javaOptions.push_back(option);

    if (!heapDumpPathOptFound) {
        option = OPT_HEAP_DUMP_PATH;
        option += userDir;
        option += HEAP_DUMP_PATH;
        javaOptions.push_back(option);
    }

    string proxy, nonProxy, socksProxy;
    if (!findHttpProxyFromEnv(proxy)) {
        findProxiesFromRegistry(proxy, nonProxy, socksProxy);
    }
    if (!proxy.empty()) {
        option = OPT_HTTP_PROXY;
        option += proxy;
        javaOptions.push_back(option);
    }
    if (!nonProxy.empty()) {
        option = OPT_HTTP_NONPROXY;
        option += nonProxy;
        javaOptions.push_back(option);
    }
    if (!socksProxy.empty()) {
        option = OPT_SOCKS_PROXY;
        option += socksProxy;
        javaOptions.push_back(option);
    }

    option = OPT_KEEP_WORKING_SET_ON_MINIMIZE;
    javaOptions.push_back(option);
}

// Reads value of http_proxy environment variable to use it as proxy setting
bool PlatformLauncher::findHttpProxyFromEnv(string &proxy) {
    logMsg("findHttpProxyFromEnv()");
    char *envVar = getenv("http_proxy");
    if (envVar) {
        // is it URL?
        int prefixLen = strlen("http://");
        if (strncmp(envVar, "http://", prefixLen) == 0 && envVar[strlen(envVar) - 1] == '/'
                && strlen(envVar) > strlen("http://")) {
            // trim URL part to keep only 'host[:port]'
            proxy = envVar + prefixLen;
            proxy.erase(proxy.size() - 1);
            logMsg("Found proxy in environment variable: %s", proxy.c_str());
            return true;
        }
    }
    return false;
}

bool PlatformLauncher::findProxiesFromRegistry(string &proxy, string &nonProxy, string &socksProxy) {
    logMsg("findProxiesFromRegistry()");
    socksProxy = nonProxy = proxy = "";
    DWORD proxyEnable = 0;
    if (!getDwordFromRegistry(HKEY_CURRENT_USER, REG_PROXY_KEY, REG_PROXY_ENABLED_NAME, proxyEnable)) {
        return false;
    }

    if (!proxyEnable) {
        logMsg("Proxy disabled");
        proxy = PROXY_DIRECT;
        return true;
    }

    string proxyServer;
    if (!getStringFromRegistry(HKEY_CURRENT_USER, REG_PROXY_KEY, REG_PROXY_SERVER_NAME, proxyServer)) {
        return false;
    }

    if (proxyServer.find('=') == string::npos) {
        proxy = proxyServer;
    } else {
        string::size_type pos = proxyServer.find("socks=");
        if (pos != string::npos) {
            if (proxyServer.size() > pos + 1 && proxyServer.at(pos) != ';') {
                string::size_type endPos = proxyServer.find(';', pos);
                socksProxy = proxyServer.substr(pos, endPos == string::npos ? string::npos : endPos - pos);
            }
        }
        pos = proxyServer.find("http=");
        if (pos != string::npos) {
            string::size_type endPos = proxyServer.find(';', pos);
            proxy = proxyServer.substr(pos, endPos == string::npos ? string::npos : endPos - pos);
        }
    }
    logMsg("Proxy servers:\n\tproxy: %s\n\tsocks proxy: %s\n\tnonProxy: %s", proxy.c_str(), socksProxy.c_str(), nonProxy.c_str());
    getStringFromRegistry(HKEY_CURRENT_USER, REG_PROXY_KEY, REG_PROXY_OVERRIDE_NAME, nonProxy);
    return true;
}

string & PlatformLauncher::constructClassPath(bool runUpdater) {
    logMsg("constructClassPath()");
    addedToCP.clear();
    classPath = cpBefore;

    // No need to load from the current dir!
    // addJarsToClassPathFrom(userDir.c_str());
    
    addJarsToClassPathFrom(platformDir.c_str());

    classPath += cpAfter;
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
            addToBootClassPath(fullName.c_str());
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

bool PlatformLauncher::restartRequested() {
    return fileExists((userDir + RESTART_FILE_PATH).c_str());
}

void PlatformLauncher::onExit() {
    logMsg("onExit()");
    exiting = true;
    if (separateProcess) {
        logMsg("JVM in separate process, no need to restart");
        return;
    }
}
