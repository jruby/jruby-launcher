/*
 * Copyright 2009-2012 JRuby Team (www.jruby.org).
 */


#ifndef _ARGPARSER_H_
#define _ARGPARSER_H_

#include <string>
#include <list>
#include <set>

class ArgParser {
protected:
    static const char *REQ_JAVA_VERSION;
    static const char *HELP_MSG;

    static const char *OPT_JDK_HOME;
    static const char *OPT_JRUBY_HOME;
    static const char *OPT_JRUBY_COMMAND_NAME;

    static const char *OPT_CMDLINE_CLASS_PATH;
    static const char *OPT_CMDLINE_MODULE_PATH;
    static const char *OPT_CLASS_PATH;
    static const char *OPT_BOOT_CLASS_PATH;

    static const char *OPT_JFFI_PATH;
    static const char *OPT_JRUBY_SHELL;
    static const char *OPT_JRUBY_SCRIPT;

    static const char *MAIN_CLASS;
    static const char *DEFAULT_EXECUTABLE;

public:
    ArgParser();
    virtual ~ArgParser();
    bool parseArgs(int argc, char *argv[]);
    void appendToHelp(const char *msg);
    std::string* buildCommandLine(int argc, char* argv[]);

protected:
    ArgParser(const ArgParser& orig);

    bool initPlatformDir();
    void prepareOptions();
    void setupMaxHeapAndStack(std::list<std::string> userOptions);
    void addEnvVarToOptions(std::list<std::string> & optionsList, const char * envvar);
    void constructClassPath();
    void constructBootClassPath();
    void addFilesToClassPath(const char *dir, const char *subdir, const char *pattern);
    void addToClassPath(const char *path, bool onlyIfExists = false);
    void addToBootClassPath(const char *path, bool onlyIfExists = false);
    void addJarsToClassPathFrom(const char *dir);
    void addOptionsToCommandLine(std::list<std::string> & commandLine);
    bool endsWith(const std::string &string, const std::string &end);
    void useModulesIfPresent();

protected:
    bool separateProcess;
    bool nailgunClient;
    bool noBootClassPath;
    bool printCommandLine;
    bool useModulePath;
    std::string platformDir;
    std::string bootclass;
    std::string jdkhome;
    std::string cpBefore;
    std::string cpExplicit;
    std::string cpAfter;
    std::string nextAction;

    std::list<std::string> javaOptions;
    std::set<std::string> addedToCP;
    std::string classPath;
    std::string bootClassPath;
    std::set<std::string> addedToBootCP;
    std::string appendHelp;
    std::list<std::string> progArgs;
};

#endif // ! _ARGPARSER_H_
