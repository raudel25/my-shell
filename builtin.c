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

#define HISTORY_FILE "history.txt"

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

int my_sh_num_builtins() {
    return sizeof(builtin_str) / sizeof(char *);
}

int my_sh_cd(char **args) {
    if (args[1] == NULL) {
        fprintf(stderr, "my_shell: expected argument to \"cd\"\n");
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

    int fd = (int) strtol(HISTORY_FILE, &end_ptr, 10);

    if (*(end_ptr + 1) != '\0') {
        fd = open(HISTORY_FILE, O_WRONLY | O_APPEND | O_CREAT, 0600);
    }

    write(fd, line, strlen(line));
    close(fd);
}

char **get_history() {
    FILE *arch;
    char buffer[1024];

    arch = fopen(HISTORY_FILE, "r");
    fread(buffer, sizeof(char), 1024, arch);
    fclose(arch);

    char **args = my_sh_split_line(buffer, "\n");

    return args;
}

int show_history() {
    char **args = get_history();
    char **history = (char **) malloc(10 * sizeof(char *));

    int i;
    for (i = 0; args[i] != NULL; i++) {
    }

    i--;
    int top = i < 10 ? 0 : i - 10;
    for (int j = i - 1; j >= top; j--) {
        printf("%d: %s\n", i - j, args[j]);
    }

    free(args);
    free(history);

    return 1;
}