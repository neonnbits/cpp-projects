#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <unistd.h>

pid_t background_pids[100];
int bg_idx = 0;

void parse_command(char *input, char **args) {
    int i = 0;
    char *token = strtok(input, " ");

    while (token != NULL && i < 63) { // Leave space for NULL
        args[i] = token;
        i++;
        token = strtok(NULL, " ");
    }
    args[i] = NULL; // execvp needs NULL-terminated array
}

void childExit(int sig) {
    int status;
    pid_t pid;

    while ((pid = waitpid(-1, &status, WNOHANG)) > 0) {
        int bg_ps = 0;
        for (int i = 0; i < bg_idx; i++) {
            if (pid == background_pids[i]) {
                bg_ps = 1;
                background_pids[i] = background_pids[--bg_idx];
                break;
            }
        }

        if (bg_ps) {
            printf("\n[Process %d] Done\n", pid);
            printf("prompt > ");
            fflush(stdout);
        }
    }
}

int main() {
    char input[1024];
    signal(SIGINT, SIG_IGN);
    signal(SIGCHLD, childExit);
    while (1) {
        printf("prompt > ");
        fflush(stdout);

        if (fgets(input, sizeof(input), stdin) == NULL)
            break;

        input[strcspn(input, "\n")] = 0;

        if (strcmp(input, "exit") == 0) {
            printf("Goodbye!\n");
            break;
        }

        if (strncmp(input, "cd ", 3) == 0) {
            char *path = input + 3;
            if (chdir(path) == 0) {

            } else {
                printf("cd: %s: No such file or directory\n", path);
            }
            continue;
        }

        if (strcmp(input, "pwd") == 0) {
            char cwd[1024];
            if (getcwd(cwd, sizeof(cwd)) != NULL) {
                printf("%s\n", cwd);
            } else {
                printf("Error: Cannot get current working directory.\n");
            }
            continue;
        }

        char *args[64];
        char input_copy[1024];
        strcpy(input_copy, input);

        parse_command(input_copy, args);

        int background = 0;
        for (int i = 0; args[i] != NULL; i++) {
            if (strcmp(args[i], "&") == 0) {
                args[i] = NULL;
                background = 1;
                break;
            }
        }

        pid_t pid = fork();

        if (pid == 0) {
            signal(SIGINT, SIG_DFL);
            execvp(args[0], args);
            printf("Command not found: %s\n", input);
            exit(1);
        } else if (pid > 0) {
            if (background) {
                background_pids[bg_idx++] = pid;
                printf("PS ID: [%d] \n", pid);
            } else {
                // Foreground process - wait as normal
                int status;
                wait(&status);
                if (WIFSIGNALED(status)) {
                    printf("\n");
                }
            }
        } else {
            printf("Fork Failed\n");
        }
    }

    return 0;
}