//
// Created by raudel on 3/25/2023.
//

#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/wait.h>
#include <pwd.h>

#include "builtin.h"
#include "decode.h"
#include "help.c"
#include "execute.h"

#define BOLD_BLUE "\033[1;34m"
#define RESET "\033[0m"
#define ERROR "\033[1;31mmy_sh\033[0m"

#define HISTORY_FILE ".my_sh_history"
#define MY_SH_TOK_BUF_SIZE 1024
#define MY_SH_MAX_HISTORY 20
#define MY_SH_TOK_DELIM " \t\r\n\a"

int current_pid;

int last_pid;

List *background_pid = NULL;

char *history[MY_SH_MAX_HISTORY];

int history_len;

char *variables[26];

struct passwd *pw = NULL;

int num_commands() {
    return sizeof(commands) / sizeof(char *);
}

char *builtin_str[7] = {
        "cd",
        "unset",
        "fg",
        "exit",
        "true",
        "false",
        "set"
};

char *builtin_str_out[4] = {
        "help",
        "history",
        "jobs",
        "get"
};

int (*builtin_func[7])(char **) = {
        &my_sh_cd,
        &my_sh_unset,
        &my_sh_foreground,
        &my_sh_exit,
        &my_sh_true,
        &my_sh_false,
        &my_sh_set
};

int (*builtin_func_out[4])(char **) = {
        &my_sh_help,
        &my_sh_history,
        &my_sh_jobs,
        &my_sh_get
};


char *my_sh_path_history() {
    char *path = (char *) malloc(sizeof(char) * (strlen(pw->pw_dir) + strlen(HISTORY_FILE) + 1));

    strcpy(path, pw->pw_dir);
    strcat(path, "/");
    strcat(path, HISTORY_FILE);

    return path;
}

int my_sh_num_builtins() {
    return sizeof(builtin_str) / sizeof(char *);
}

int my_sh_num_builtins_out() {
    return sizeof(builtin_str) / sizeof(char *);
}


int my_sh_cd(char **args) {
    if (args[1] == NULL) {
        if (chdir(pw->pw_dir) != 0) {
            perror(ERROR);

            return 1;
        }
    } else {
        if (chdir(args[1]) != 0) {
            perror(ERROR);

            return 1;
        }
    }
    return 0;
}

int my_sh_help(char **args) {
    if (args[1] == NULL) {
        printf("\n");

        for (int i = 0; i < num_commands(); i++) {
            printf("%s%s%s: %s\n", BOLD_BLUE, commands[i], RESET, commands_help[i]);
        }

        return 0;
    }

    for (int i = 0; i < num_commands(); i++) {
        if (strcmp(commands[i], args[1]) == 0) {
            printf("%s", commands_help[i]);
            return 0;
        }
    }

    fprintf(stderr, "%s: command not found\n", ERROR);

    return 1;
}

int my_sh_exit() {
    exit(EXIT_SUCCESS);
}

void my_sh_save_history(char *line) {
    if (history_len == MY_SH_MAX_HISTORY) {
        char *aux = history[0];
        for (int i = 1; i < history_len; i++) {
            history[i - 1] = history[i];
        }
        history_len--;

        history[history_len] = aux;
    }

    strcpy(history[history_len++], line);

    char *path = my_sh_path_history();
    int fd = open(path, O_WRONLY | O_CREAT | O_TRUNC, S_IRWXU);
    for (int i = 0; i < history_len; i++) {
        write(fd, history[i], strlen(history[i]));
    }
    write(fd, "\n", 1);
    close(fd);
}

void my_sh_load_history() {
    for(int i=0;i<MY_SH_MAX_HISTORY;i++){
        history[i]=(char *) malloc(MY_SH_TOK_BUF_SIZE);
    }

    char *path = my_sh_path_history();

    int status = 0;
    FILE *file;
    file = fopen(path, "r");
    int i = 0;
    if (file != NULL) {
        while (status != -1) {
            char *line = NULL;
            size_t buf_size = 0;
            status = (int) getline(&line, &buf_size, file);
            if (status == -1) {
                i--;
                free(line);
                continue;
            }
            if (i == MY_SH_MAX_HISTORY) break;

            strcpy(history[i], line);
            free(line);
            i++;
        }
        history_len = i;
        fclose(file);
    }

    free(path);
}

int my_sh_history() {
    for (int j = 0; j < history_len; j++) {
        printf("%d: %s", j + 1, history[j]);
    }

    return 0;
}

char *my_sh_get_again(int ind, int last) {
    char *again = NULL;
    if (last) ind = history_len;

    for (int j = 0; j < history_len; j++) {
        if (ind == j + 1) {
            char *aux = sub_str(history[j], 0,
                                (int) strlen(history[j]) - 2);
            again = (char *) malloc(sizeof(char) * (strlen(aux) + 1));
            strcpy(again, aux);
            strcat(again, "\n");
        }
    }

    return again;
}

void my_sh_init_variables() {
    for (int i = 0; i < 26; i++) {
        variables[i] = NULL;
    }
}

int check_variable(char *variable) {
    if (strlen(variable) != 1 || variable[0] - 'a' < 0 || variable[0] - 'a' > 'z') {
        fprintf(stderr, "%s: the variables must by letters of english alphabet\n", ERROR);

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
                fprintf(stderr, "%s: incorrect command unset\n", ERROR);

                return 1;
            }
        }
    } else {
        fprintf(stderr, "%s: incorrect command unset\n", ERROR);

        return 1;
    }

    return 0;
}

int my_sh_foreground(char **args) {
    if (background_pid->len == 0) {
        fprintf(stderr, "%s: the process does not exist in the background\n", ERROR);
        return 1;
    }

    int c_pid;
    int status = 1;

    int index = args[1] == NULL ? background_pid->len - 1 : (int) strtol(args[1], 0, 10) - 1;
    if (index >= background_pid->len) {
        fprintf(stderr, "%s: the process does not exist in the background\n", ERROR);
        return 1;
    }

    c_pid = background_pid->array[index];
    current_pid = c_pid;
    do {
        waitpid(c_pid, &status, WUNTRACED);
    } while (!WIFEXITED(status) && !WIFSIGNALED(status));

    removeAtIndex(background_pid, index);

    return status;
}

int my_sh_jobs() {
    for (int i = 0; i < background_pid->len; i++) {
        printf("[%d]\t%d\n", i + 1, background_pid->array[i]);
    }

    return 0;
}

int my_sh_get(char **args) {
    if (args[1] == NULL) {
        for (int i = 0; i < 26; i++) {
            if (variables[i] != NULL) {
                printf("%s%c%s = %s", BOLD_BLUE, (char) (i + 'a'), RESET, variables[i]);
                if (variables[i][strlen(variables[i]) - 1] != '\n') printf("\n");
            }
        }

        return 0;
    }

    if (check_variable(args[1])) {
        for (int i = 0; i < 26; i++) {
            if (i != args[1][0] - 'a')
                continue;

            if (variables[i] != NULL) {
                printf("%s", variables[i]);
                if (variables[i][strlen(variables[i]) - 1] != '\n') printf("\n");

                return 0;
            }
        }

        fprintf(stderr, "%s: variable not found\n", ERROR);
    }

    return 1;
}

int my_sh_true() {
    return 0;
}

int my_sh_false() {
    return 1;
}

int my_sh_set(char **args) {
    int status = 0;

    if (args[1] != NULL && args[2] != NULL) {
        if (check_variable(args[1])) {
            if (args[2][0] != '`') {
                if (variables[args[1][0] - 'a'] != NULL) {
                    free(variables[args[1][0] - 'a']);
                }

                variables[args[1][0] - 'a'] = (char *) malloc(sizeof(char) * strlen(args[2]));
                strcpy(variables[args[1][0] - 'a'], args[2]);
            } else {
                char *new_command = NULL;

                if (args[2][strlen(args[2]) - 1] == '`') {
                    new_command = sub_str(args[2], 1, (int) strlen(args[2]) - 2);
                    my_sh_decode_set(new_command);
                }

                if (new_command != NULL) {
                    char *new_command_format = my_sh_decode_line(new_command);

                    char *buffer = (char *) malloc(MY_SH_TOK_BUF_SIZE);
                    char c = 1;
                    int i = 0;
                    fflush(stdout);

                    int temp_stdout;
                    temp_stdout = dup(fileno(stdout));

                    int fd[2];
                    pipe(fd);
                    dup2(fd[1], fileno(stdout));

                    my_sh_execute(new_command_format, 0, 1);

                    write(fd[1], "", 1);
                    close(fd[1]);

                    fflush(stdout);
                    dup2(temp_stdout, fileno(stdout));
                    close(temp_stdout);

                    while (1) {
                        read(fd[0], &c, 1);
                        buffer[i] = c;
                        if (c == 0)
                            break;
                        i++;
                        if (i % MY_SH_TOK_BUF_SIZE == 0) {
                            buffer = (char *) realloc(buffer, i * 2);
                        }
                    }

                    close(fd[0]);

                    if (i != 0) {
                        buffer[i] = 0;
                        if (buffer[i - 1] == '\n')
                            buffer[i - 1] = 0;

                        if (variables[args[1][0] - 'a'] != NULL) {
                            free(variables[args[1][0] - 'a']);
                        }

                        variables[args[1][0] - 'a'] = (char *) malloc(sizeof(char) * strlen(buffer));
                        strcpy(variables[args[1][0] - 'a'], buffer);
                    } else {
                        fprintf(stderr, "%s: the output of the command is null\n", ERROR);
                        status = 1;
                    }

                    free(buffer);
                    free(new_command);
                    free(new_command_format);
                } else {
                    fprintf(stderr, "%s: incorrect command set\n", ERROR);
                    status = 1;
                }
            }
        } else {
            status = 1;
        }
    } else {
        fprintf(stderr, "%s: incorrect command set\n", ERROR);
        status = 1;
    }

    return status;
}

int my_sh_background(char **args, int init, int end) {
    char *new_args[end - init];

    for (int i = 0; i < end - init; i++) {
        new_args[i] = (char *) malloc(sizeof(char) * strlen(args[init + i]));
        strcpy(new_args[i], args[init + i]);
    }

    int pid;

    pid = fork();
    if (pid == 0) {
        setpgid(0, 0);

        exit(my_sh_parser(new_args, 0, end - init, -1, -1));
    } else if (pid > 0) {
        setpgid(pid, pid);
        append(background_pid, pid);
        printf("[%d]\t%d\n", background_pid->len, pid);
    }

    return 0;
}

char *my_sh_again(char *line) {
    char **args = my_sh_split_line(line, MY_SH_TOK_DELIM);
    char *aux = (char *) malloc(MY_SH_TOK_BUF_SIZE);
    strcpy(aux, "");

    int q = 0;
    int last;
    int error = 0;

    int len = array_size(args);
    for (int i = 0; i < len; i++) {
        if (strcmp(args[i], "again") == 0) {
            last = 0;

            if (args[i + 1] != NULL) {
                char *p;
                q = (int) strtol(args[i + 1], &p, 10);

                if (q == 0) last = 1;
                else i++;
            } else last = 1;

            char *c_again = my_sh_get_again(q, last);

            if (c_again != NULL) {
                c_again[strlen(c_again)-1]=0;
                strcat(aux, c_again);
                free(c_again);
            } else {
                error = 1;
                strcat(aux, "false");
            }
        } else {
            strcat(aux, args[i]);
        }

        if (i != len - 1) {
            strcat(aux, " ");
        }
    }
    strcat(aux, "\n");

    if (error) {
        fprintf(stderr, "%s: incorrect command again\n", ERROR);
    }

    free(args);
    return aux;
}

void my_sh_update_background() {
    int status;

    if (background_pid->len > 0) {
        for (int i = 0; i < background_pid->len; ++i) {
            waitpid(background_pid->array[i], &status, WNOHANG);
            if (WIFEXITED(status)) {
                printf("[%d]\tDone\t%d\n", i + 1, background_pid->array[i]);
                removeAtIndex(background_pid, i);

                i = -1;
            }
        }
    }
}

