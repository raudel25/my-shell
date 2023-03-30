//
// Created by raudel on 3/25/2023.
//

#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/wait.h>

#include "builtin.h"
#include "decod.h"

#define HISTORY_FILE ".my_sh_history"
#define MY_SH_TOK_BUF_SIZE 1024

List *background_pid = NULL;

GList *background_command = NULL;

char *variables[26];

char *home = NULL;

char *builtin_str[7] = {
        "cd",
        "help",
        "history",
        "unset",
        "fg",
        "jobs",
        "exit"
};

int (*builtin_func[7])(char **) = {
        &my_sh_cd,
        &my_sh_help,
        &my_sh_history,
        &my_sh_unset,
        &my_sh_foreground,
        &my_sh_jobs,
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

            return 0;
        }
    } else {
        if (chdir(args[1]) != 0) {
            perror("my_shell");

            return 0;
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
    exit(EXIT_SUCCESS);
}

void save_history(char *line) {
    char *end_ptr = 0;
    char *path = my_sh_path_history();

    int fd = (int) strtol(path, &end_ptr, 10);

    if (*(end_ptr + 1) != '\0') {
        fd = open(path, O_WRONLY | O_APPEND | O_CREAT, 0600);
    }

    write(fd, "#", 1);
    write(fd, line, strlen(line));
    close(fd);
    free(path);
}

char **get_history() {
    char *buffer=(char *) malloc(MY_SH_TOK_BUF_SIZE);
    char *path = my_sh_path_history();

    char *end_ptr = 0;

    int fd = (int) strtol(path, &end_ptr, 10);

    if (*(end_ptr + 1) != '\0') {
        fd = open(path, O_RDONLY);
    }

    read(fd,buffer,MY_SH_TOK_BUF_SIZE);
    close(fd);

    free(path);
    char **args = my_sh_split_line(buffer, "\t\r\n\a");

    return args;
}

int my_sh_history() {
    char **args = get_history();

    int i;
    for (i = 0; args[i] != NULL; i++) {
        if (args[i][0] != '#') break;
    }

    int top = i < 10 ? 0 : i - 10;
    for (int j = top; j < i; j++) {
        char *aux = sub_str(args[j], 1, (int) strlen(args[j]) - 1);
        printf("%d: %s\n", j - top + 1, aux);
        free(aux);
    }

    free(args);

    return 1;
}

char *get_again(int ind) {
    char **args = get_history();
    char *again = NULL;

    int i;
    for (i = 0; args[i] != NULL; i++) {
        if(args[i][0]!='#') break;
    }

    int top = i < 10 ? 0 : i - 10;
    for (int j = top; j < i; j++) {
        if (ind == j - top + 1) {
            char *aux = sub_str(args[j], 1, (int) strlen(args[j]) - 1);
            again = (char *) malloc(strlen(aux) + 1);
            strcpy(again, aux);
            strcat(again, "\n");
            free(aux);
        }
    }

    free(args);

    return again;
}

void my_sh_init_variables() {
    for (int i = 0; i < 26; i++) {
        variables[i] = NULL;
    }
}

int check_variable(char *variable) {
    if (strlen(variable) != 1 || variable[0] - 'a' < 0 || variable[0] - 'a' > 'z') {
        write(2, "my_sh: the variables must by letters of english alphabet\n", 57);

        return 0;
    }

    return 1;
}

int my_sh_unset(char **args) {
    if (args[1] != NULL) {
        if (check_variable(args[1])) {
            if (variables[args[1][0] - 'a'] != NULL) {
                free(variables[args[1][0] - 'a']);
                variables[args[1][0] - 'a'] = NULL;
            } else {
                printf("my_sh: incorrect command unset\n");

                return 0;
            }
        }
    } else {
        printf("my_sh: incorrect command unset\n");

        return 0;
    }

    return 1;
}

int my_sh_foreground(char **args) {
    if (background_pid->len == 0)
        return 1;

    int c_pid;
    int status = 1;

    int index = args[1] == NULL ? background_pid->len - 1 : (int) strtol(args[1], 0, 10) - 1;
    if (index >= background_pid->len)
        return 1;

    c_pid = background_pid->array[index];
    do {
        waitpid(c_pid, &status, WUNTRACED);
    } while (!WIFEXITED(status) && !WIFSIGNALED(status));

    removeAtIndex(background_pid, index);
    removeAtIndexG(background_command, index);

    return status;
}

int my_sh_jobs() {
    for (int i = 0; i < background_pid->len; i++) {
        printf("[%d]\t%s\t%d\n", i + 1, (char *) background_command->array[i], background_pid->array[i]);
    }

    return 1;
}