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

using namespace std;

bool dirExists(const char *path) {
    WIN32_FIND_DATA fd = {0};
    HANDLE hFind = 0;
    hFind = FindFirstFile(path, &fd);
    if (hFind == INVALID_HANDLE_VALUE) {
        logMsg("Dir \"%s\" does not exist", path);
        return false;
    }
    logMsg("Dir \"%s\" exists", path);
    FindClose(hFind);
    return (fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) != 0;
}

bool fileExists(const char *path) {
    WIN32_FIND_DATA fd = {0};
    HANDLE hFind = 0;
    hFind = FindFirstFile(path, &fd);
    if (hFind == INVALID_HANDLE_VALUE) {
        logMsg("File \"%s\" does not exist", path);
        return false;
    }

    logMsg("File \"%s\" exists", path);
    FindClose(hFind);
    return true;
}

bool normalizePath(char *path, int len) {
    char tmp[MAX_PATH] = "";
    int i = 0;
    while (path[i] && i < MAX_PATH - 1) {
        tmp[i] = path[i] == '/' ? '\\' : path[i];
        i++;
    }
    tmp[i] = '\0';
    return _fullpath(path, tmp, len) != NULL;
}

bool createPath(const char *path) {
    logMsg("Creating directory \"%s\"", path);
    char dir[MAX_PATH] = "";
    const char *sep = strchr(path, '\\');
    while (sep) {
        strncpy(dir, path, sep - path);
        if (!CreateDirectory(dir, 0) && GetLastError() != ERROR_ALREADY_EXISTS) {
            logErr(true, false, "Failed to create directory %s", dir);
            return false;
        }
        sep = strchr(sep + 1, '\\');
    }
    return true;
}

char * skipWhitespaces(char *str) {
    while (*str != '\0' && (*str == ' ' || *str == '\t' || *str == '\n' || *str == '\r')) {
        str++;
    }
    return str;
}

char * trimWhitespaces(char *str) {
    char *end = str + strlen(str) - 1;
    while (end >= str && (*end == ' ' || *end == '\t' || *end == '\n' || *end == '\r')) {
        *end = '\0';
        end--;
    }
    return end;
}

char* getSysError(char *str, int strSize) {
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

    if (showMsgBox) {
        ::MessageBox(NULL, msg, "JRuby Error", MB_OK | MB_ICONSTOP);
    }
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
                DeleteFile(gLogFileName.c_str());
            }
            break;
        }
    }
    return true;
}
