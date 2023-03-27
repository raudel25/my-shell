//
// Created by raudel on 3/25/2023.
//

#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <fcntl.h>

#include "builtin.h"
#include "decod.h"

#define HISTORY_FILE ".my_sh_history"

char *home = NULL;

char *builtin_str[4] = {
        "cd",
        "help",
        "history",
        "exit"
};

int (*builtin_func[4])(char **) = {
        &my_sh_cd,
        &my_sh_help,
        &show_history,
        &my_sh_exit
};

char *my_sh_path_history() {
    char *path = (char *) malloc(strlen(home) + strlen(HISTORY_FILE) + 1);

    strcpy(path, home);
    strcat(path, "/");
    strcat(path, HISTORY_FILE);

    return path;
}

int my_sh_num_builtins() {
    return sizeof(builtin_str) / sizeof(char *);
}

int my_sh_cd(char **args) {
    if (args[1] == NULL) {
        if (chdir(home) != 0) {
            perror("my_shell");
        }
    } else {
        if (chdir(args[1]) != 0) {
            perror("my_shell");
        }
    }
    return 1;
}

int my_sh_help() {
    int i;
    printf("Stephen Brennan's my_sh\n");
    printf("Type program names and arguments, and hit enter.\n");
    printf("The following are built in:\n");

    for (i = 0; i < my_sh_num_builtins(); i++) {
        printf("  %s\n", builtin_str[i]);
    }

    printf("Use the man command for information on other programs.\n");
    return 1;
}

int my_sh_exit() {
    return 0;
}

void save_history(char *line) {
    char *end_ptr = 0;
    char *path = my_sh_path_history();

    int fd = (int) strtol(path, &end_ptr, 10);

    if (*(end_ptr + 1) != '\0') {
        fd = open(path, O_WRONLY | O_APPEND | O_CREAT, 0600);
    }

    write(fd, line, strlen(line));
    close(fd);
    free(path);
}

char **get_history() {
    char buffer[1024];
    char *path = my_sh_path_history();

    char *end_ptr = 0;

    int fd = (int) strtol(path, &end_ptr, 10);

    if (*(end_ptr + 1) != '\0') {
        fd = open(path, O_RDONLY);
    }

    read(fd,buffer,1024);
    close(fd);

    free(path);
    char **args = my_sh_split_line(buffer, "\n");

    return args;
}

int show_history() {
    char **args = get_history();
    char **history = (char **) malloc(10 * sizeof(char *));

    int i;
    for (i = 0; args[i] != NULL; i++) {
    }

    int top = i < 10 ? 0 : i - 10;
    for (int j = top; j < i; j++) {
        printf("%d: %s\n", j-top+1, args[j]);
    }

    free(args);
    free(history);

    return 1;
}