#include <stdio.h>
#include <stdlib.h>
#include <spawn.h>
#include <sys/wait.h>
#include <unistd.h>
#include <string.h>

#define N 12

extern char **environ;

char *allowed[N] = {"cp","touch","mkdir","ls","pwd","cat","grep","chmod","diff","cd","exit","help"};

int isAllowed(const char*cmd) {
	// TODO
	// return 1 if cmd is one of the allowed commands
	// return 0 otherwise
	for (int i = 0; i < N; i++) {
        if (strcmp(cmd, allowed[i]) == 0) {
            return 1;  // Command is allowed
        }
    }
	return 0;
}

int main() {
    char line[256];

    while (1) {
        fprintf(stderr, "rsh> ");

        // Read input
        if (fgets(line, 256, stdin) == NULL) continue;

        // Handle empty input
        if (strcmp(line, "\n") == 0) continue;

        // Remove newline character at the end
        line[strlen(line) - 1] = '\0';

        // Tokenize the input into arguments
        char *argv[21];  // Maximum 20 arguments + NULL
        int argc = 0;
        char *token = strtok(line, " ");
        while (token != NULL && argc < 20) {
            argv[argc++] = token;
            token = strtok(NULL, " ");
        }
        argv[argc] = NULL;  // Null-terminate the argument list

        // Check if the command is allowed
        if (!isAllowed(argv[0])) {
            printf("NOT ALLOWED!\n");
            continue;
        }

        // Handle built-in commands (cd, exit, help)
        if (strcmp(argv[0], "cd") == 0) {
            if (argc > 2) {
                printf("-rsh: cd: too many arguments\n");
            } else if (argc == 2) {
                if (chdir(argv[1]) != 0) {
                    perror("cd failed");
                }
            }
            continue;
        } else if (strcmp(argv[0], "exit") == 0) {
            return 0;  // Exit the shell
        } else if (strcmp(argv[0], "help") == 0) {
            printf("The allowed commands are:\n");
            for (int i = 0; i < N; i++) {
                printf("%s\n", allowed[i]);
            }
            continue;
        }

        // Spawn processes for allowed commands (1-9)
        pid_t pid;
        int status;
        posix_spawnattr_t attr;

        // Initialize spawn attributes
        posix_spawnattr_init(&attr);

        if (posix_spawnp(&pid, argv[0], NULL, &attr, argv, environ) != 0) {
            perror("posix_spawnp failed");
        } else {
            // Wait for the spawned process to finish
            if (waitpid(pid, &status, 0) == -1) {
                perror("waitpid failed");
            }
            if (WIFEXITED(status)) {
                printf("Process exited with status %d\n", WEXITSTATUS(status));
            }
        }

        // Clean up spawn attributes
        posix_spawnattr_destroy(&attr);
    }
    return 0;
}
