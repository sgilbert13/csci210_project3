#include <stdio.h>
#include <stdlib.h>
#include <spawn.h>
#include <sys/wait.h>
#include <unistd.h>
#include <string.h>

#define N 12

extern char **environ;

char *allowed[N] = {
    "cp", "touch", "mkdir", "ls", "pwd", "cat", "grep", "chmod", "diff", "cd", "exit", "help"
};

// Function to check if a command is allowed
int isAllowed(const char *cmd) {
    for (int i = 0; i < N; i++) {
        if (strcmp(cmd, allowed[i]) == 0) {
            return 1;
        }
    }
    return 0;
}

int main() {
    char line[256];
    char *argv[21];  // Max 20 arguments (including the command itself)
    int status;

    while (1) {
        fprintf(stderr, "rsh>");

        // Read the input line
        if (fgets(line, 256, stdin) == NULL) continue;

        // Ignore empty lines
        if (strcmp(line, "\n") == 0) continue;

        // Remove newline character from fgets input
        line[strlen(line) - 1] = '\0';

        // Tokenize the input line into arguments
        int argc = 0;
        char *token = strtok(line, " ");
        while (token != NULL && argc < 20) {
            argv[argc++] = token;
            token = strtok(NULL, " ");
        }
        argv[argc] = NULL;  // Null-terminate the argument list

        // Check if the command is allowed
        if (!isAllowed(argv[0])) {
            fprintf(stderr, "NOT ALLOWED!\n");
            continue;
        }

        // Handle the "cd" command
        if (strcmp(argv[0], "cd") == 0) {
            if (argc > 2) {
                fprintf(stderr, "-rsh: cd: too many arguments\n");
            } else {
                if (chdir(argv[1]) != 0) {
                    perror("chdir");
                }
            }
            continue;
        }

        // Handle the "exit" command
        if (strcmp(argv[0], "exit") == 0) {
            return 0;
        }

        // Handle the "help" command
        if (strcmp(argv[0], "help") == 0) {
            printf("The allowed commands are:\n");
            for (int i = 0; i < N; i++) {
                printf("%d: %s\n", i + 1, allowed[i]);
            }
            continue;
        }

        // For other commands (those that spawn processes)
        pid_t pid;
        posix_spawnattr_t attr;
        posix_spawn_file_actions_t actions;

        // Initialize posix_spawn attributes and file actions
        posix_spawnattr_init(&attr);
        posix_spawn_file_actions_init(&actions);

        // Spawn the command as a child process
        if (posix_spawnp(&pid, argv[0], &actions, &attr, argv, environ) != 0) {
            perror("posix_spawnp");
            continue;
        }

        // Wait for the child process to finish
        waitpid(pid, &status, 0);
    }

    return 0;
}

