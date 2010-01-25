/*
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS HEADER.
 *
 * Copyright 2009-2010 JRuby Team (www.jruby.org).
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
#include "argnames.h"

#ifndef WIN32
#include <sys/stat.h>
#include <errno.h>
#include <string.h>
#endif

using namespace std;

bool checkExists(const char* path, unsigned int flags) {
#ifdef WIN32
    WIN32_FIND_DATA fd = {0};
    HANDLE hFind = 0;
    hFind = FindFirstFile(path, &fd);
    if (hFind == INVALID_HANDLE_VALUE) {
        return false;
    }
    FindClose(hFind);
    if (flags == 0) {
	return true;
    }
    return (fd.dwFileAttributes & flags) != 0;
#else
    struct stat dir;
    if (stat(path, &dir) != 0) {
	return false;
    }
    if (flags == 0) {
	return true;
    }
    return dir.st_mode & flags;
#endif
}

bool checkDirectory(const char* path) {
#ifdef WIN32
    unsigned int flags = FILE_ATTRIBUTE_DIRECTORY;
#else
    unsigned int flags = S_IFDIR;
#endif
    return checkExists(path, flags);
}

bool dirExists(const char *path) {
    if (!checkDirectory(path)) {
        logMsg("Dir \"%s\" does not exist", path);
        return false;
    }
    logMsg("Dir \"%s\" exists", path);
    return true;
}

bool fileExists(const char *path) {
    if (!checkExists(path, 0)) {
        logMsg("File \"%s\" does not exist", path);
        return false;
    }
    logMsg("File \"%s\" exists", path);
    return true;
}

char* findOnPath(const char* name) {
    string path(getenv("PATH"));
    size_t start = 0;
    size_t sep;
    char * found;

    while (start < path.length()) {
	sep = path.find(":", start);
	if (sep == string::npos) {
	    sep = path.length();
	}

	string elem(path.substr(start, sep - start));
	if (elem[elem.length() - 1] != '/') {
	    elem += '/';
	}
	elem += name;

	if (checkExists(elem.c_str(), 0)) {
	    found = (char*) malloc(elem.length());
	    strncpy(found, elem.c_str(), elem.length() + 1);
	    return found;
	}

	start = sep + 1;
    }
    return NULL;
}

const char* getSysError(char *str, int strSize) {
#ifdef WIN32
    int err = GetLastError();
    LPTSTR lpMsgBuf;
    FormatMessage(
            FORMAT_MESSAGE_ALLOCATE_BUFFER |
            FORMAT_MESSAGE_FROM_SYSTEM |
            FORMAT_MESSAGE_IGNORE_INSERTS,
            NULL,
            err,
            MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
            (LPTSTR) & lpMsgBuf,
            0,
            NULL
            );
    LPTSTR tmp = strchr(lpMsgBuf, '\r');
    if (tmp != NULL) {
        *tmp = '\0';
    }

    _snprintf(str, strSize, " %s (%u)", lpMsgBuf, err);
    LocalFree(lpMsgBuf);
#else
    const char* error = strerror(errno);
    snprintf(str, strSize, " %s (%u)", error, errno);
#endif
    return str;
}

string gLogFileName;

void logV(bool appendSysError, bool showMsgBox, const char *format, va_list args) {
    char msg[4096] = "";
    vsnprintf(msg, 4096, format, args);

    if (appendSysError) {
        char sysErr[512] = "";
        getSysError(sysErr, 512);
        strncat(msg, sysErr, 4096 - strlen(msg));
    }

    if (!gLogFileName.empty()) {
        FILE *file = fopen(gLogFileName.c_str(), "a");
        if (file) {
            fprintf(file, "%s\n", msg);
            fclose(file);
        }
    }

#ifdef WIN32
    if (showMsgBox) {
        ::MessageBox(NULL, msg, "JRuby Error", MB_OK | MB_ICONSTOP);
    }
#endif
}

void logErr(bool appendSysError, bool showMsgBox, const char *format, ...) {
    va_list args;
    va_start(args, format);
    logV(appendSysError, showMsgBox, format, args);
}

void logMsg(const char *format, ...) {
    va_list args;
    va_start(args, format);
    logV(false, false, format, args);
}

bool checkLoggingArg(int argc, char *argv[], bool delFile) {
    for (int i = 0; i < argc; i++) {
        if (strcmp("--", argv[i]) == 0) {
            break;
        }
        if (strcmp(ARG_NAME_LAUNCHER_LOG, argv[i]) == 0) {
            if (i + 1 == argc) {
                logErr(false, true, "Argument is missing for \"%s\" option.", argv[i]);
                return false;
            }
            gLogFileName = argv[++i];
            if (delFile) {
#ifdef WIN32
                DeleteFile(gLogFileName.c_str());
#else
		unlink(gLogFileName.c_str());
#endif
            }
            break;
        }
    }
    return true;
}

bool printToConsole(const char *msg) {
#ifdef WIN32
    FILE *console = fopen("CON", "a");
    if (!console) {
        return false;
    }
    fprintf(console, "%s", msg);
    fclose(console);
#else
    fprintf(stderr, "%s", msg);
#endif
    return false;
}

const char** convertToArgvArray(list<string> args) {
    const char ** argv = (const char**) malloc(sizeof (char*) * args.size());
    int i = 0;
    for (list<string>::iterator it = args.begin(); it != args.end(); ++it, ++i) {
        argv[i] = it->c_str();
    }
    return argv;
}
