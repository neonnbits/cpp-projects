#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <unistd.h>

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

void handle_sigint(int sig) {
    printf("\nCaught Ctrl+C! But I'm not going to quit.\n");
}

int main() {
    char input[1024];
    signal(SIGINT, SIG_IGN);
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

        pid_t pid = fork();

        if (pid == 0) {
            signal(SIGINT, SIG_DFL);
            execvp(args[0], args);
            printf("Command not found: %s\n", input);
            exit(1);
        } else if (pid > 0) {
            int status;
            wait(&status);
            // print a newline if the child process was killed with a exit
            // signal
            if (WIFSIGNALED(status)) {
                printf("\n");
            }
        } else {
            printf("Fork Failed\n");
        }
    }

    return 0;
}