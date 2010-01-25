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

#ifndef _UTILSFUNCS_H
#define	_UTILSFUNCS_H

#include <string>
#include <list>

bool dirExists(const char *path);
bool fileExists(const char *path);
void logMsg(const char *format, ...);
void logErr(bool appendSysError, bool showMsgBox, const char *format, ...);
bool checkLoggingArg(int argc, char *argv[], bool delFile);
bool printToConsole(const char *msg);
const char** convertToArgvArray(std::list<std::string> args);

#ifndef WIN32
#define FILE_SEP '/'
#define PATH_SEP ':'
#else  // WIN32
#include <windows.h>
#define FILE_SEP '\\'
#define PATH_SEP ';'

#ifdef JRUBYW
bool setupProcess(int &argc, char *argv[], DWORD &parentProcID, const char *attachMsg = 0);
bool getParentProcessID(DWORD &id);
#endif /* JRUBYW */

bool normalizePath(char *path, int len);
bool disableFolderVirtualization(HANDLE hProcess);
char * getCurrentModulePath(char *path, int pathLen);
bool getStringFromRegistry(HKEY rootKey, const char *keyName, const char *valueName, std::string &value);
bool getDwordFromRegistry(HKEY rootKey, const char *keyName, const char *valueName, DWORD &value);
bool isConsoleAttached();
#endif	// WIN32

#endif	/* _UTILSFUNCS_H */
