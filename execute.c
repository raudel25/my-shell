//
// Created by raudel on 3/23/2023.
//
#include <stdio.h>
#include <sys/wait.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>

#include "decod.h"
#include "builtin.h"

void redirect_out(char *fileName) {
    char *end_ptr = 0;

    int fd = (int) strtol(fileName, &end_ptr, 10);

    if (*(end_ptr + 1) != '\0') {
        fd = open(fileName, O_WRONLY | O_TRUNC | O_CREAT, 0600);
    }

    dup2(fd, STDOUT_FILENO);
    close(fd);
}

void redirect_out_append(char *fileName) {
    char *end_ptr = 0;

    int fd = (int) strtol(fileName, &end_ptr, 10);

    if (*(end_ptr + 1) != '\0') {
        fd = open(fileName, O_WRONLY | O_APPEND | O_CREAT, 0600);
    }

    dup2(fd, STDOUT_FILENO);
    close(fd);
}

void redirect_in(char *fileName) {
    char *end_ptr = 0;

    int fd = (int) strtol(fileName, &end_ptr, 10);

    if (*(end_ptr + 1) != '\0') {
        fd = open(fileName, O_RDONLY);
    }

    dup2(fd, STDIN_FILENO);
    close(fd);
}


int my_sh_launch(char **args) {
    pid_t pid;
    int status;

    pid = fork();
    if (pid == 0) {
        int len = 0;
        for (int k = 0; args[k] != NULL; ++k) {
            len++;
        }
        char **new_args = (char **) malloc((len + 1) * sizeof(char *));

        int j = 0;

        for (int i = 0; args[i] != 0; ++i) {
            if (strcmp(args[i], "<") == 0) {
                redirect_in(args[i + 1]);
                i++;
                continue;
            }
            if (strcmp(args[i], ">") == 0) {
                redirect_out(args[i + 1]);
                i++;
                continue;
            }
            if (strcmp(args[i], ">>") == 0) {
                redirect_out_append(args[i + 1]);
                i++;
                continue;
            }

            new_args[j] = args[i];
            j++;
        }

        new_args[j] = NULL;

        if (execvp(new_args[0], new_args) == -1) {
            free(new_args);
            perror("my_shell");
        }
        exit(EXIT_FAILURE);
    } else if (pid < 0) {
        perror("my_shell");
    } else {
        do {
            waitpid(pid, &status, WUNTRACED);
        } while (!WIFEXITED(status) && !WIFSIGNALED(status));
    }

    return 1;
}

int my_sh_execute(char **args) {
    int i;

    if (args[0] == NULL)
        return 1;


    for (i = 0; i < my_sh_num_builtins(); i++) {
        if (strcmp(args[0], builtin_str[i]) == 0) {
            return (*builtin_func[i])(args);
        }
    }

    return my_sh_launch(args);
}
