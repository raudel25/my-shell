//
// Created by raudel on 3/23/2023.
//
#include <stdio.h>
#include <sys/wait.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>

#include "decode.h"
#include "builtin.h"
#include "execute.h"

#define MY_SH_TOK_DELIM " \t\r\n\a"
#define ERROR "\033[1;31mmy_sh\033[0m"
#define MY_SH_TOK_BUF_SIZE 1024


int redirect_instr(char *args) {
    if (strcmp(args, "<") == 0)
        return 0;

    if (strcmp(args, ">") == 0)
        return 1;

    if (strcmp(args, ">>") == 0)
        return 2;

    if (strcmp(args, "|") == 0)
        return 3;

    return -1;
}

int redirect_out(char *fileName) {
    char *end_ptr = 0;

    int fd = (int) strtol(fileName, &end_ptr, 10);

    if (*(end_ptr + 1) != '\0') {
        fd = open(fileName, O_WRONLY | O_TRUNC | O_CREAT, 0600);
    }

    return fd;
}

int redirect_out_append(char *fileName) {
    char *end_ptr = 0;

    int fd = (int) strtol(fileName, &end_ptr, 10);

    if (*(end_ptr + 1) != '\0') {
        fd = open(fileName, O_WRONLY | O_APPEND | O_CREAT, 0600);
    }

    return fd;
}

int redirect_in(char *fileName) {
    char *end_ptr = 0;

    int fd = (int) strtol(fileName, &end_ptr, 10);

    if (*(end_ptr + 1) != '\0') {
        fd = open(fileName, O_RDONLY);
    }

    return fd;
}

int my_sh_launch(char **args, int init, int end, int fd_in, int fd_out) {
    pid_t pid;
    int status = 0;

    pid = fork();
    if (pid == 0) {
        if (fd_in != -1) {
            dup2(fd_in, STDIN_FILENO);
            close(fd_in);
        }

        if (fd_out != -1) {
            dup2(fd_out, STDOUT_FILENO);
            close(fd_out);
        }

        char *new_args[end - init];

        for (int i = init; i < end; i++) {
            new_args[i - init] = args[i];
        }
        new_args[end - init] = NULL;

        for (int i = 0; i < my_sh_num_builtins_out(); i++) {
            if (strcmp(new_args[0], builtin_str_out[i]) == 0) {
                exit((*builtin_func_out[i])(new_args));
            }
        }

        if (execvp(new_args[0], new_args) == -1) {
            perror(ERROR);
        }

        exit(EXIT_FAILURE);
    } else if (pid < 0) {
        perror(ERROR);
    } else {

        if (fd_in != -1) {
            close(fd_in);
        }
        if (fd_out != -1) {
            close(fd_out);
        }

        do {
            waitpid(pid, &status, WUNTRACED);
        } while (!WIFEXITED(status) && !WIFSIGNALED(status));

    }

    return status;
}

int my_sh_launch_not_out(char **args, int init, int end, int fd_in, int fd_out) {
    char *new_args[end - init];

    for (int i = init; i < end; i++) {
        new_args[i - init] = args[i];
    }
    new_args[end - init] = NULL;


    for (int i = 0; i < my_sh_num_builtins(); i++) {
        if (strcmp(new_args[0], builtin_str[i]) == 0) {

            if (fd_in != -1) close(fd_in);
            if (fd_out != -1) close(fd_out);

            return (*builtin_func[i])(new_args);
        }
    }

    return -1;
}

void my_sh_new_args(int init, char **args, int fd_in, int fd[3], int aux[2]) {
    fd[0] = fd_in != -1 ? fd_in : -1;
    fd[1] = -1;
    fd[2] = -1;

    int k = init;
    int instr = -1;

    while (args[k] != NULL) {
        instr = redirect_instr(args[k]);
        if (instr != -1)
            break;

        k++;
    }

    aux[0] = k;
    aux[1] = k + 1;

    if (instr == 0) {
        fd[0] = redirect_in(args[k + 1]);

        if (args[k + 2] != NULL) {
            instr = redirect_instr(args[k + 2]);
            if (instr == 1 || instr == 2) k += 2;
            if (instr == 3) k++;
        }

        aux[1] = k + 2;
    }
    if (instr == 1) {
        fd[1] = redirect_out(args[k + 1]);
        aux[1] = k + 2;
    }
    if (instr == 2) {
        fd[1] = redirect_out_append(args[k + 1]);
        aux[1] = k + 2;
    }
    if (instr == 3) {
        int fd1[2];
        pipe(fd1);

        fd[1] = fd1[1];
        fd[2] = fd1[0];
    }
}

void my_sh_execute_save(char **args, char *line, int save) {
    if (strcmp(args[0], "exit") == 0) save = 0;
    if (save) my_sh_save_history(line);
}

int my_sh_execute_chain(char **args, char *line) {
    int global_status = -1;
    int status;
    int execute = 1;

    int last = 0;
    int i;
    int j = 0;
    for (i = 0; args[i] != NULL; i++) {
        j += ((int) strlen(args[i]) + 1);

        if (args[i][strlen(args[i]) - 1] == ';') {
            if (global_status == -1) global_status = 1;

            if (execute) {
                char *aux = sub_str(line, last, j - 3);
                status = !my_sh_execute(aux, 0, 0);
                free(aux);
            }

            execute = 1;
            last = j;
            global_status = status && global_status;
        }

        if (strcmp(args[i], "&&") == 0) {
            if (global_status == -1) global_status = 1;

            if (execute) {
                char *aux = sub_str(line, last, j - 5);
                status = !my_sh_execute(aux, 0, 0);
                free(aux);

                execute = status;
            }

            last = j;
        }

        if (strcmp(args[i], "||") == 0) {
            if (global_status == -1) global_status = 1;

            if (execute) {
                char *aux = sub_str(line, last, j - 5);
                status = !my_sh_execute(aux, 0, 0);
                free(aux);

                execute = !status;
            }

            last = j;
        }
    }

    if (global_status != -1) {
        if (execute && args[i - 1][strlen(args[i - 1]) - 1] != ';') {
            char *aux = sub_str(line, last, j - 2);
            status = !my_sh_execute(aux, 0, 0);
            free(aux);
        }

        return !(status && global_status);
    }

    return global_status;
}

int my_sh_conditional(char **args, char *line) {
    int p = 0;

    int ind_then = -1;
    int ind_else = -1;
    int ind_end;

    int error = 0;

    char *c_condition;
    char *c_then;
    char *c_else;

    int i;
    int j;
    for (i = 1; args[i] != NULL; i++) {
        if (strcmp(args[i], "then") == 0 && p == 0) ind_then = i;
        if (strcmp(args[i], "else") == 0 && p == 0) ind_else = i;
        if (strcmp(args[i], "end") == 0 && p == 0) ind_end = i;
        if (strcmp(args[i], "if") == 0) p++;
        if (strcmp(args[i], "end") == 0) p--;
    }

    if (ind_end != i - 1 || ind_then == -1) error = 1;
    if (ind_then < 2 || ind_end - ind_then < 2) error = 1;
    if (ind_else != -1 && (ind_else - ind_then < 2 || ind_end - ind_else < 2)) error = 1;

    if (error) {
        fprintf(stderr, "%s: incorrect conditional\n", ERROR);
        return 1;
    }

    int size_total = 3;

    j = 0;
    for (i = 1; i < ind_then; i++) {
        j += ((int) strlen(args[i]) + 1);
    }
    c_condition = sub_str(line, size_total, j + size_total - 2);
    size_total += (j + 5);

    if (ind_else != -1) {
        j = 0;
        for (i = ind_then + 1; i < ind_else; i++) {
            j += ((int) strlen(args[i]) + 1);
        }
        c_then = sub_str(line, size_total, j + size_total - 2);
        size_total += (j + 5);

        j = 0;
        for (i = ind_else + 1; i < ind_end; i++) {
            j += ((int) strlen(args[i]) + 1);
        }
        c_else = sub_str(line, size_total, j + size_total - 2);
    } else {
        j = 0;
        for (i = ind_then + 1; i < ind_end; i++) {
            j += ((int) strlen(args[i]) + 1);
        }
        c_then = sub_str(line, size_total, j + size_total - 2);
    }

    int status = !my_sh_execute(c_condition, 0, 0);

    if (status) {
        return my_sh_execute(c_then, 0, 0);
    } else if (ind_else != -1) {
        return my_sh_execute(c_else, 0, 0);
    }

    return 1;
}

int my_sh_execute(char *line, int save, int possible_back) {
    char *new_line= my_sh_again(line);

    int status;
    char copy[strlen(new_line)];
    strcpy(copy, new_line);

    my_sh_encode_set(new_line);
    char **args = my_sh_split_line(new_line, MY_SH_TOK_DELIM);

    if (args[0] == NULL) {
        free(args);
        free(new_line);

        return 1;
    }

    my_sh_execute_save(args, copy, save);

    if (strcmp(args[array_size(args) - 1], "&") == 0 && possible_back) {
        int q = my_sh_background(copy);

        free(args);
        free(new_line);

        return q;
    }

    if (strcmp(args[0], "if") == 0) {
        int q = my_sh_conditional(args, copy);

        free(args);
        free(new_line);

        return q;
    }

    int chain = my_sh_execute_chain(args, copy);

    if (chain != -1) {
        free(args);
        free(new_line);

        return chain;
    }

    status = my_sh_execute_pipes(args);

    free(args);
    free(new_line);

    return status;
}

int my_sh_execute_pipes(char **args) {
    int fd[3];
    fd[2] = -1;
    int aux[2];
    int init = 0;
    int status = 0;

    while (1) {
        my_sh_new_args(init, args, fd[2], fd, aux);
        int end = aux[0];

        int c_status = my_sh_launch_not_out(args, init, end, fd[0], fd[1]);

        if (c_status == -1) {
            c_status = my_sh_launch(args, init, end, fd[0], fd[1]);
        }

        if (c_status != 0) status = c_status;

        init = aux[1];
        if (args[end] == NULL || args[init] == NULL)
            break;
    }

    return status;
}
