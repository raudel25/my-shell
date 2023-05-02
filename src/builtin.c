//
// Created by raudel on 3/25/2023.
//

#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <pwd.h>
#include <readline/history.h>

#include "builtin.h"
#include "utils.h"
#include "help.c"
#include "execute.h"

#define GREEN "\033[0;32m"
#define RESET "\033[0m"
#define ERROR "\033[0;31mmy_sh\033[0m"

#define HISTORY_FILE ".my_sh_history"
#define MY_SH_TOK_BUF_SIZE 1024
#define MY_SH_MAX_HISTORY 100
#define MY_SH_TOK_DELIM " \t\r\n\a"

int current_pid;

int last_pid;

List *background_pid = NULL;

GList *variables_key;

GList *variables_value;

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

void my_sh_load_history(){
    char *path = my_sh_path_history();
    read_history(path);
    stifle_history(MY_SH_MAX_HISTORY);
    free(path);
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
            printf("%s%s%s: %s\n", GREEN, commands[i], RESET, commands_help[i]);
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

void my_sh_save_history(char *line){
    add_history(line);
}

int my_sh_exit() {
    char *path = my_sh_path_history();
    write_history(path);
    free(path);

    exit(EXIT_SUCCESS);
}

int my_sh_history() {
    HIST_ENTRY **list = history_list();
    for (int j = 0; j < history_length; j++) {
        printf("%d: %s\n", j + 1, list[j]->line);
    }

    return 0;
}

void my_sh_init_variables() {
    variables_key = createListG();
    variables_value = createListG();
}

int contains_key(char *key) {
    for (int i = 0; i < variables_key->len; i++) {
        if (strcmp(key, variables_key->array[i]) == 0) return i;
    }

    return -1;
}

int my_sh_unset(char **args) {
    if (args[1] != NULL) {
        int index = contains_key(args[1]);

        if (index != -1) {
            removeAtIndexG(variables_key, index);
            removeAtIndexG(variables_value, index);
        } else {
            fprintf(stderr, "%s: variable not found\n", ERROR);

            return 1;
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
        for (int i = 0; i < variables_key->len; i++) {
            printf("%s%s%s = %s\n", GREEN, variables_key->array[i], RESET, variables_value->array[i]);
        }

        return 0;
    }

    int index = contains_key(args[1]);
    if (index != -1) {
        printf("%s\n", variables_value->array[index]);

        return 0;
    } else {
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
    int len = array_size(args);

    if (args[1] != NULL && args[2] != NULL) {
        if (strcmp(args[2], "(") != 0) {
            int index = contains_key(args[1]);
            if (index != -1) {
                removeAtIndexG(variables_key, index);
                removeAtIndexG(variables_value, index);
            }
            char *aux_value = array_to_str(args, 2, len);
            char *aux_key = (char *) malloc(sizeof(char) * strlen(args[1]));
            strcpy(aux_key, args[1]);
            appendG(variables_key, aux_key);
            appendG(variables_value, aux_value);
        } else {
            if (strcmp(args[len - 1], ")") == 0 && len >= 5) {
                char *buffer = (char *) malloc(MY_SH_TOK_BUF_SIZE);
                char c = 1;
                int i = 0;
                fflush(stdout);

                int temp_stdout;
                temp_stdout = dup(fileno(stdout));

                int fd[2];
                pipe(fd);
                dup2(fd[1], fileno(stdout));

                my_sh_parser(args, 3, len - 1, -1, -1);

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
                    int index = contains_key(args[1]);
                    if (index != -1) {
                        removeAtIndexG(variables_key, index);
                        removeAtIndexG(variables_value, index);
                    }
                    char *aux_key = (char *) malloc(sizeof(char) * strlen(buffer));
                    strcpy(aux_key, args[1]);
                    appendG(variables_key, aux_key);
                    appendG(variables_value, buffer);
                } else {
                    fprintf(stderr, "%s: the output of the command is null\n", ERROR);
                    status = 1;
                }
            } else {
                fprintf(stderr, "%s: incorrect command set\n", ERROR);
                status = 1;
            }
            }
    } else {
        status = 1;
    }

    return status;
}

int my_sh_background(char **args, int init, int end) {
    int pid;

    pid = fork();
    if (pid == 0) {
        setpgid(0, 0);

        exit(my_sh_parser(args, init, end, -1, -1));
    } else if (pid < 0) {
        perror(ERROR);
    } else {
        setpgid(pid, pid);
        append(background_pid, pid);
        printf("[%d]\t%d\n", background_pid->len, pid);
    }

    return 0;
}

int special_char(char c) {
    return c == '|' || c == ';' || c == '&' || c == '#' || c == '`';
}

void get_again(char *line, int index, int *j) {
    if (index <= history_length && index > 0) {
        HIST_ENTRY **list = history_list();
        strcat(line, list[index - 1]->line);
        (*j) += (int) strlen(list[index - 1]->line);
    } else {
        fprintf(stderr, "%s: incorrect command again\n", ERROR);
        strcat(line, "false");
        (*j) += 5;
    }
}

char *my_sh_again(char *line) {
    char *aux = (char *) malloc(MY_SH_TOK_BUF_SIZE);
    aux[0] = 0;
    int j = 0;

    char *pat = "again";
    char *pat_help = "help";
    int len = (int) strlen(line);
    int len_aux = 5;
    int len_help = 4;

    int c = 0;

    int q;
    for (int i = 0; i < len; i++) {
        if (line[i] == '"')c = !c;

        int equal = pat_equal(line, pat, i);
        if (c) equal = 0;

        if (equal) {
            int w = i - 1;

            while (w >= len_help - 1) {
                w--;
                if (line[w] != ' ') break;
            }

            int help = pat_equal(line, pat_help, w - len_help + 1);
            if (help) {
                push_str(aux, line[i], &j);
                continue;
            }
        }

        if (equal) {
            if (i + len_aux == len || special_char(line[i + len_aux])) {
                get_again(aux, history_length, &j);
                i += (len_aux - 1);
                continue;
            }
            int s1 = i + len_aux;
            while (line[s1] == ' ') {
                s1++;
                if (s1 == len) break;
            }

            if (s1 == len) {
                get_again(aux, history_length, &j);
                break;
            }

            int s2 = s1;
            while (line[s2] != ' ' && !special_char(line[s2])) {
                s2++;
                if (s2 == len) break;
            }

            char *num = sub_str(line, s1, s2 - 1);
            char *p;
            q = (int) strtol(num, &p, 10);

            if (strlen(p) != 0 || s1 == s2) {
                get_again(aux, history_length, &j);

                i += (len_aux - 1);
            } else {
                get_again(aux, q, &j);
                i = s2 - 1;
            }

            free(num);
        } else {
            push_str(aux, line[i], &j);
        }
    }

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

