/*
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS HEADER.
 *
 * Copyright 2009-2012 JRuby Team (www.jruby.org).
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

#include <stdio.h>
#include "utilsfuncs.h"
#include "platformlauncher.h"
#include "rb_w32_cmdvector.h"

using namespace std;

extern "C" int nailgunClientMain(int argc, char *argv[], char *env[]);

PlatformLauncher::PlatformLauncher()
    : ArgParser()
    , suppressConsole(false)
{
    separateProcess = false;
}

PlatformLauncher::PlatformLauncher(const PlatformLauncher& orig)
    : ArgParser(orig)
{
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

bool PlatformLauncher::start(char* argv[], int argc, DWORD *retCode, const char* binaryName) {

    // subvert cmd.exe's feeble attempt at command line parsing,
    // the code is taken from MRI
    argc = rb_w32_cmdvector(GetCommandLine(), &argv);

    // remove the first argument ('jruby')
    argc -= 1;
    argv += 1;

    platformDir = binaryName;
    checkLoggingArg(argc, argv, false);
    if (!initPlatformDir()
        || !parseArgs(argc, argv)
        || !checkJDKHome()) {
        return false;
    }
    disableFolderVirtualization(GetCurrentProcess());

    if (nailgunClient) {
        progArgs.push_front("org.jruby.util.NailMain");
        char ** nailArgv = convertToArgvArray(progArgs);
        if (printCommandLine) {
            printListToConsole(progArgs);
            return true;
        }

        list<string>* envList = GetEnvStringsAsList();
        char ** nailEnv  = convertToArgvArray(*envList);
        nailgunClientMain(progArgs.size(), nailArgv, nailEnv);
        return true;
    }

    if (binaryName) {
        // clean up the binaryName first,
        // remove '.exe' from the end, and the possible path.
        string bn = binaryName;

        size_t found = bn.find_last_of("/\\");
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

        if (strnicmp(binaryName, DEFAULT_EXECUTABLE, strlen(DEFAULT_EXECUTABLE)) != 0) {
            logMsg("PlatformLauncher:\n\tNon-default executable name: %s", binaryName);
            logMsg("\tHence, launching with extra parameters: -S %s", binaryName);
            progArgs.push_front(binaryName);
            progArgs.push_front("-S");
        }
    }

    prepareOptions();

    string java("");

    if (getenv("JAVACMD") != NULL) {
        java = getenv("JAVACMD");
        jvmLauncher.setJavaCmd(java);
        separateProcess = true;
        suppressConsole = false;
    } else {
        if (jdkhome.empty()) {
            if (!jvmLauncher.initialize(REQ_JAVA_VERSION)) {
                logErr(false, true, "Cannot find Java %s or higher.", REQ_JAVA_VERSION);
                return false;
            }
        }

        jvmLauncher.getJavaPath(jdkhome);
        java = jdkhome + "\\bin\\java";
    }

    if (printCommandLine) {
        list<string> commandLine;
        commandLine.push_back(java);
        addOptionsToCommandLine(commandLine);
        for (list<string>::iterator it = commandLine.begin(); it != commandLine.end(); it++) {
            printf("%s\n", it->c_str());
        }
        return true;
    }

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

    jvmLauncher.setSuppressConsole(suppressConsole);

    bool rc = jvmLauncher.start(bootclass.c_str(), progArgs, javaOptions, separateProcess, retCode);
    if (!separateProcess) {
        exit(0);
    }
    return rc;
}

bool PlatformLauncher::checkJDKHome() {
    if (!jdkhome.empty() && !jvmLauncher.initialize(jdkhome.c_str())) {
        logMsg("Cannot locate java installation in specified jdkhome: %s", jdkhome.c_str());
        string errMsg = "ERROR: Cannot locate Java installation in specified jdkhome:\n";
        errMsg += jdkhome;
        string errMsgFull = errMsg + "\nDo you want to try to use default version?";
        jdkhome = "";
        // Pop-up the message box only if there is no console
        if (!isConsoleAttached()) {
            if (::MessageBox(NULL, errMsgFull.c_str(), "Invalid jdkhome specified", MB_ICONQUESTION | MB_YESNO) == IDNO) {
                return false;
            }
        } else {
            fprintf(stdout, "%s\n", errMsg.c_str());
            return false;
        }
    }

    if (jdkhome.empty()) {
        logMsg("-Xjdkhome is not set, checking for %%JAVA_HOME%%...");
        char *origJavaHome = getenv("JAVA_HOME");
        if (origJavaHome) {
            const char *javaHome = trimTrailingBackslashes(origJavaHome).c_str();
            logMsg("%%JAVA_HOME%% is set: %s", javaHome);
            if (!jvmLauncher.initialize(javaHome)) {
                logMsg("ERROR: Cannot locate java installation, specified by JAVA_HOME: %s", javaHome);
                string errMsg = "Cannot locate Java installation, specified by JAVA_HOME:\n";
                errMsg += javaHome;
                string errMsgFull = errMsg + "\nDo you want to try to use default version?";
                jdkhome = "";
                // Pop-up the message box only if there is no console
                if (!isConsoleAttached()) {
                    if (::MessageBox(NULL, errMsgFull.c_str(),
                            "Invalid jdkhome specified", MB_ICONQUESTION | MB_YESNO) == IDNO) {
                        return false;
                    }
                } else {
                    fprintf(stdout, "%s\n", errMsg.c_str());
                    return false;
                }
            } else {
                jdkhome = javaHome;
            }
        }
    }

    return true;
}

void PlatformLauncher::onExit() {
    logMsg("onExit()");
}
