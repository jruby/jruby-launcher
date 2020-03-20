#include "utilsfuncs.h"
#include "argnames.h"
#include <tlhelp32.h>

using namespace std;

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

bool disableFolderVirtualization(HANDLE hProcess) {
    OSVERSIONINFO osvi = {0};
    osvi.dwOSVersionInfoSize = sizeof (OSVERSIONINFO);
    if (GetVersionEx(&osvi) && osvi.dwMajorVersion == 6) // check it is Win VISTA
    {
        HANDLE hToken;
        if (OpenProcessToken(hProcess, TOKEN_ALL_ACCESS, &hToken)) {
            DWORD tokenInfoVal = 0;
            if (!SetTokenInformation(hToken, (TOKEN_INFORMATION_CLASS) 24, &tokenInfoVal, sizeof (DWORD))) {
                // invalid token information class (24) is OK, it means there is no folder virtualization on current system
                if (GetLastError() != ERROR_INVALID_PARAMETER) {
                    logErr(true, true, "Failed to set token information.");
                    return false;
                }
            }
            CloseHandle(hToken);
        } else {
            logErr(true, true, "Failed to open process token.");
            return false;
        }
    }
    return true;
}

typedef BOOL (WINAPI *LPFN_ISWOW64PROCESS) (HANDLE, PBOOL);

LPFN_ISWOW64PROCESS IsWow64Process;

BOOL is64bits() {
    BOOL is64Bits = FALSE;
    IsWow64Process = (LPFN_ISWOW64PROCESS) GetProcAddress(GetModuleHandle(TEXT("kernel32")),"IsWow64Process");

    if (IsWow64Process && !IsWow64Process(GetCurrentProcess(),&is64Bits)) {}

    return is64Bits;
}

#define KEY_WOW64_64KEY 0x0100
#define KEY_WOW64_32KEY 0x0200

bool getStringFromRegistry(HKEY rootKey, const char *keyName, const char *valueName, string &value) {
    logMsg("getStringFromRegistry()\n\tkeyName: %s\n\tvalueName: %s", keyName, valueName);
    DWORD openFlags = KEY_READ | (is64bits() ? KEY_WOW64_64KEY : KEY_WOW64_32KEY);
    HKEY hKey = 0;
    if (RegOpenKeyEx(rootKey, keyName, 0, openFlags, &hKey) == ERROR_SUCCESS) {
        DWORD valSize = 4096;
        DWORD type = 0;
        char val[4096] = "";
        if (RegQueryValueEx(hKey, valueName, 0, &type, (BYTE *) val, &valSize) == ERROR_SUCCESS
                && type == REG_SZ) {
            logMsg("%s: %s", valueName, val);
            RegCloseKey(hKey);
            value = val;
            return true;
        } else {
            logErr(true, false, "RegQueryValueEx() failed.");
        }
        RegCloseKey(hKey);
    } else {
        logErr(true, false, "RegOpenKeyEx() failed.");
    }
    return false;
}

bool getDwordFromRegistry(HKEY rootKey, const char *keyName, const char *valueName, DWORD &value) {
    logMsg("getDwordFromRegistry()\n\tkeyName: %s\n\tvalueName: %s", keyName, valueName);
    DWORD openFlags = KEY_READ | (is64bits() ? KEY_WOW64_64KEY : KEY_WOW64_32KEY);
    HKEY hKey = 0;
    if (RegOpenKeyEx(rootKey, keyName, 0, openFlags, &hKey) == ERROR_SUCCESS) {
        DWORD valSize = sizeof(DWORD);
        DWORD type = 0;
        if (RegQueryValueEx(hKey, valueName, 0, &type, (BYTE *) &value, &valSize) == ERROR_SUCCESS
                && type == REG_DWORD) {
            logMsg("%s: %u", valueName, value);
            RegCloseKey(hKey);
            return true;
        } else {
            logErr(true, false, "RegQueryValueEx() failed.");
        }
        RegCloseKey(hKey);
    } else {
        logErr(true, false, "RegOpenKeyEx() failed.");
    }
    return false;
}

char * getCurrentModulePath(char *path, int pathLen) {
    MEMORY_BASIC_INFORMATION mbi;
    static int dummy;
    VirtualQuery(&dummy, &mbi, sizeof (mbi));
    HMODULE hModule = (HMODULE) mbi.AllocationBase;
    GetModuleFileName(hModule, path, pathLen);
    return path;
}

#ifdef JRUBYW
bool setupProcess(int &argc, char *argv[], DWORD &parentProcID, const char *attachMsg) {
#define CHECK_ARG \
    if (i+1 == argc) {\
        logErr(false, true, "Argument is missing for \"%s\" option.", argv[i]);\
        return false;\
    }

    parentProcID = 0;
    DWORD cmdLineArgPPID = 0;
    for (int i = 0; i < argc; i++) {
        // break arg parsing, once "--" is found
        if (strcmp("--", argv[i]) == 0) {
            break;
        }
        if (strcmp(ARG_NAME_CONSOLE, argv[i]) == 0) {
            CHECK_ARG;
            if (strcmp("new", argv[i + 1]) == 0){
                logMsg("Allocating new console...");
                AllocConsole();
            } else if (strcmp("suppress", argv[i + 1]) == 0) {
                logMsg("Suppressing the attachment to console...");
                // nothing, no console should be attached
            } else if (strcmp("attach", argv[i + 1]) == 0) {
                logMsg("Trying to attach to the existing console...");
                // attach to parent process console if exists
                // AttachConsole exists since WinXP, so be nice and do it dynamically
                typedef BOOL(WINAPI * LPFAC)(DWORD dwProcessId);
                HINSTANCE hKernel32 = GetModuleHandle("kernel32");
                if (hKernel32) {
                    LPFAC attachConsole = (LPFAC) GetProcAddress(hKernel32, "AttachConsole");
                    if (attachConsole) {
                        if (cmdLineArgPPID) {
                            if (!attachConsole(cmdLineArgPPID)) {
                                logErr(true, false, "AttachConsole of PPID: %u failed.", cmdLineArgPPID);
                            }
                        } else {
                            if (!attachConsole((DWORD) - 1)) {
                                logErr(true, true, "AttachConsole of PP failed.");
                            } else {
                                getParentProcessID(parentProcID);
                                if (attachMsg) {
                                    printToConsole(attachMsg);
                                }
                            }
                        }
                    } else {
                        logErr(true, false, "GetProcAddress() for AttachConsole failed.");
                    }
                }
            } else {
                logErr(false, true, "Invalid argument for \"%s\" option.", argv[i]);
                return false;
            }
            // remove options
            for (int k = i + 2; k < argc; k++) {
                argv[k-2] = argv[k];
            }
            argc -= 2;
            return true;
        }
    }
#undef CHECK_ARG

    return true;
}
#endif /* JRUBYW */

bool isConsoleAttached() {
    typedef HWND (WINAPI *GetConsoleWindowT)();
    HINSTANCE hKernel32 = GetModuleHandle("kernel32");
    if (hKernel32) {
        GetConsoleWindowT getConsoleWindow = (GetConsoleWindowT) GetProcAddress(hKernel32, "GetConsoleWindow");
        if (getConsoleWindow) {
            if (getConsoleWindow() != NULL) {
                logMsg("Console is attached.");
                return true;
            }
        } else {
            logErr(true, false, "GetProcAddress() for GetConsoleWindow failed.");
        }
    }
    return false;
}

#ifdef JRUBYW
bool getParentProcessID(DWORD &id) {
    typedef HANDLE (WINAPI * CreateToolhelp32SnapshotT)(DWORD, DWORD);
    typedef BOOL (WINAPI * Process32FirstT)(HANDLE, LPPROCESSENTRY32);
    typedef BOOL (WINAPI * Process32NextT)(HANDLE, LPPROCESSENTRY32);

    HINSTANCE hKernel32 = GetModuleHandle("kernel32");
    if (!hKernel32) {
        return false;
    }

    CreateToolhelp32SnapshotT createToolhelp32Snapshot = (CreateToolhelp32SnapshotT) GetProcAddress(hKernel32, "CreateToolhelp32Snapshot");
    Process32FirstT process32First = (Process32FirstT) GetProcAddress(hKernel32, "Process32First");
    Process32NextT process32Next = (Process32NextT) GetProcAddress(hKernel32, "Process32Next");

    if (createToolhelp32Snapshot == NULL || process32First == NULL || process32Next == NULL) {
        logErr(true, false, "Failed to obtain Toolhelp32 functions.");
        return false;
    }

    HANDLE hSnapshot = createToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (hSnapshot == INVALID_HANDLE_VALUE) {
        logErr(true, false, "Failed to obtain process snapshot.");
        return false;
    }

    PROCESSENTRY32 entry = {0};
    entry.dwSize = sizeof (PROCESSENTRY32);
    if (!process32First(hSnapshot, &entry)) {
        CloseHandle(hSnapshot);
        return false;
    }

    DWORD curID = GetCurrentProcessId();
    logMsg("Current process ID: %u", curID);

    do {
        if (entry.th32ProcessID == curID) {
            id = entry.th32ParentProcessID;
            logMsg("Parent process ID: %u", id);
            CloseHandle(hSnapshot);
            return true;
        }
    } while (process32Next(hSnapshot, &entry));

    CloseHandle(hSnapshot);
    return false;
}
#endif /* JRUBYW */

