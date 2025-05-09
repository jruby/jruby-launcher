#include <errno.h>
#include <libgen.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

/*
 * Copyright 2009-2025 JRuby Team (www.jruby.org).
 *
 * This program finds JRUBY_HOME and executes the JRuby launcher script
 * contained within it.
 */

#define LENGTH(a) (sizeof(a) / sizeof(0[a]))

static const char *script_names[] = { "jruby.sh", "jruby.sh.bak" };
static const char *no_scripts_error = "jruby.sh or jruby.sh.bak";

static char *which(const char *const executable) {
    const size_t exe_length = strlen(executable);
    char *exe_path = NULL;
    size_t exe_path_size = 0;

    // Iterate through PATH to find executable
    char *dirs = getenv("PATH");
    if (dirs == NULL) {
        return NULL;
    }
    size_t dirs_length = strlen(dirs);
    // Temporarily replace null terminator with colon
    dirs[dirs_length] = ':';

    size_t dir_head = 0;
    for (size_t i = 0; i <= dirs_length; i++) {
        if (dirs[i] == ':') {
            // Declare convenient path variables
            char *const dir = dirs + dir_head;
            const size_t dir_length = i - dir_head;
            const size_t new_path_length = dir_length + exe_length + 1;

            // Allocate enough space for concatenated path
            if (exe_path_size < new_path_length + 1) {
                // Leave space for null terminator
                exe_path = realloc(exe_path, new_path_length + 1);
                exe_path_size = new_path_length + 1;
            }

            // Concatenate path and executable
            memcpy(exe_path, dir, dir_length);
            exe_path[dir_length] = '/';
            memcpy(exe_path + dir_length + 1, executable, exe_length);
            exe_path[new_path_length] = '\0';

            // Check if we can execute
            if (0 == access(exe_path, R_OK | X_OK)) {
                goto success;
            }

            dir_head = i + 1;
        }
    };

    // Lookup has failed, free if necessary and return NULL
    if (exe_path != NULL) {
        free(exe_path);
        exe_path = NULL;
    }
success:
    // Restore null terminator
    dirs[dirs_length] = '\0';

    return exe_path;
}


int unixlauncher_run(int argc, char *argv[], char *envp[]) {
    if (argc == 0 || argv[0][0] == '\0') {
        fputs("Error: No executable provided!", stderr);
        return 2;
    }

    // Find ourselves
    char *original_self = argv[0];
    char *self_path;

    // Detect whether argv[0] contains forward slashes
    bool self_is_path = false;
    for (size_t i = 0; original_self[i]; i++) {
        if (original_self[i] == '/') {
            self_is_path = true;
            break;
        }
    }

    if (self_is_path) {  // argv[0] is a path to an executable
        self_path = realpath(original_self, NULL);
    } else {  // argv[0] is basename of executable
        // Iterate through PATH to find script
        self_path = which(argv[0]);

        if (self_path == NULL) {
            fprintf(stderr, "Error: Could not find %s executable\n", argv[0]);
            return 1;
        }

        // Juggle malloc'd paths
        char *real_path = realpath(self_path, NULL);
        free(self_path);
        self_path = real_path;
    }

    // Find our parent directory
    char *script_dir = dirname(self_path);
    if (self_path != script_dir) {
        // Free malloc'd self_path if dirname returned statically allocated string
        free(self_path);
    }
    size_t script_dir_length = strlen(script_dir);

    // Try main script and backup script before giving up
    for (int i = 0; i < LENGTH(script_names); i++) {
        const char *script_name = script_names[i];

        // Allocate space for complete script path
        size_t script_path_length = strlen(script_name) + script_dir_length + 1;
        // Leave space for null terminator
        char *script_path = malloc(script_path_length + 1);

        // Concatenate script dir and script name
        memcpy(script_path, script_dir, script_dir_length);
        script_path[script_dir_length] = '/';
        memcpy(script_path + script_dir_length + 1, script_name, strlen(script_name));
        script_path[script_path_length] = '\0';

        // Reuse argv for script command line
        argv[0] = script_path;
        execv(argv[0], argv);

        free(script_path);
    }

    // If we get here neither script was available, so we error
    fprintf(stderr, "%s: No executable %s found in %s\n", original_self, no_scripts_error, script_dir);

    return EXIT_FAILURE;
}
