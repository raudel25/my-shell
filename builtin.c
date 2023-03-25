//
// Created by raudel on 3/25/2023.
//

#include <stdio.h>
#include <unistd.h>
#include <string.h>

#include "builtin.h"

char *builtin_str[3] = {
        "cd",
        "help",
        "exit"
};

int (*builtin_func[3])(char **) = {
        &my_sh_cd,
        &my_sh_help,
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