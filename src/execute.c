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

void my_sh_execute_save(char *line, int save) {
    if (save) my_sh_save_history(line);
}

int my_sh_conditional(char **args, int ind_if, int ind_then, int ind_else, int ind_end, int fd_in, int fd_out) {
    if (fd_in != -1) {
        close(fd_in);
    }

    int temp_stdout;

    if (fd_out != -1) {
        fflush(stdout);

        temp_stdout = dup(fileno(stdout));

        dup2(fd_out, fileno(stdout));
        close(fd_out);
    }

    int status = my_sh_parser(args, ind_if + 1, ind_then, -1, -1);

    if (ind_else != -1) {
        if (!status) {
            status = my_sh_parser(args, ind_then + 1, ind_else, -1, -1);
        } else {
            status = my_sh_parser(args, ind_else + 1, ind_end, -1, -1);
        }
    } else {
        if (!status) {
            status = my_sh_parser(args, ind_then + 1, ind_end, -1, -1);
        }
    }

    if (fd_out != -1) {
        fflush(stdout);
        dup2(temp_stdout, fileno(stdout));
        close(temp_stdout);
    }

    return status;
}

void my_sh_execute(char *line, int save) {
    char *new_line = my_sh_again(line);

    char copy[strlen(new_line)];
    strcpy(copy, new_line);

    my_sh_encode_set(new_line);
    char **args = my_sh_split_line(new_line, MY_SH_TOK_DELIM);

    if (args[0] == NULL) {
        free(args);
        free(new_line);

        return;
    }

    my_sh_execute_save(copy, save);

    my_sh_parser(args, 0, array_size(args), -1, -1);

    free(args);
    free(new_line);
}

int my_sh_execute_simple(char **args, int init, int end, int fd_int, int fd_out) {
    int status = my_sh_launch_not_out(args, init, end, fd_int, fd_out);

    if (status == -1) {
        status = my_sh_launch(args, init, end, fd_int, fd_out);
    }

    return status;
}

int my_sh_redirect_in(char **args, int init, int end, int fd_in, int fd_out, int pos) {
    if (fd_in != -1) {
        close(fd_in);
    }
    if (init == pos || end - pos != 2) {
        if (fd_out != -1) {
            close(fd_out);
        }
        fprintf(stderr, "%s: incorrect command redirect\n", ERROR);
        return 1;
    }

    int fd = redirect_in(args[pos + 1]);

    if (fd == -1) {
        fprintf(stderr, "%s: incorrect command redirect\n", ERROR);
        return 1;
    }

    return my_sh_parser(args, init, pos, fd, fd_out);
}

int my_sh_redirect_out(char **args, int init, int end, int fd_in, int fd_out, int pos) {
    if (fd_out != -1) {
        close(fd_out);
    }
    if (init == pos || end - pos != 2) {
        if (fd_in != -1) {
            close(fd_in);
        }
        fprintf(stderr, "%s: incorrect command redirect\n", ERROR);
        return 1;
    }

    int fd = redirect_out(args[pos + 1]);

    return my_sh_parser(args, init, pos, fd_in, fd);
}

int my_sh_redirect_out_append(char **args, int init, int end, int fd_in, int fd_out, int pos) {
    if (fd_out != -1) {
        close(fd_out);
    }
    if (init == pos || end - pos != 2) {
        if (fd_in != -1) {
            close(fd_in);
        }
        fprintf(stderr, "%s: incorrect command redirect\n", ERROR);
        return 1;
    }

    int fd = redirect_out_append(args[pos + 1]);

    return my_sh_parser(args, init, pos, fd_in, fd);
}

int my_sh_pipes(char **args, int init, int end, int fd_in, int fd_out, int pos) {
    if (init == pos || end - 1 == pos) {
        if (fd_in != -1) {
            close(fd_in);
        }
        if (fd_out != -1) {
            close(fd_out);
        }
        fprintf(stderr, "%s: incorrect command pipes\n", ERROR);
        return 1;
    }
    int fd[2];
    pipe(fd);

    int status1 = my_sh_parser(args, init, pos, fd_in, fd[1]);
    int status2 = my_sh_parser(args, pos + 1, end, fd[0], fd_out);

    return status1 | status2;
}

int my_sh_and_or(char **args, int init, int end, int pos, int and) {
    if (init == pos || end - 1 == pos) {
        fprintf(stderr, "%s: incorrect command chain\n", ERROR);
        return 1;
    }
    int status = my_sh_parser(args, init, pos, -1, -1);

    if (and && !status) {
        status = my_sh_parser(args, pos + 1, end, -1, -1);
    }

    if (!and && status) {
        status = my_sh_parser(args, pos + 1, end, -1, -1);
    }

    return status;
}

int my_sh_multiple(char **args, int init, int end, int pos) {
    int status1 = 0;
    int status2 = 0;

    if (pos != init) {
        status1 = my_sh_parser(args, init, pos, -1, -1);
    }
    if (pos != end - 1) {
        status2 = my_sh_parser(args, pos + 1, end, -1, -1);
    }

    return status1 | status2;
}

int my_sh_conditional_execute(char **args, int init, int end, int fd_in, int fd_out) {
    int p = 0;

    int ind_then = -1;
    int ind_else = -1;
    int ind_end;

    int error = 0;

    int i;
    for (i = init + 1; i < end; i++) {
        if (strcmp(args[i], "then") == 0 && p == 0) ind_then = i;
        if (strcmp(args[i], "else") == 0 && p == 0) ind_else = i;
        if (strcmp(args[i], "end") == 0 && p == 0) ind_end = i;
        if (strcmp(args[i], "if") == 0) p++;
        if (strcmp(args[i], "end") == 0) p--;
    }

    if (ind_end == -1 || ind_then == -1) error = 1;
    if (ind_then - init < 2 || ind_end - ind_then < 2) error = 1;
    if (ind_else != -1 && (ind_else - ind_then < 2 || ind_end - ind_else < 2)) error = 1;

    if (error) {
        if (fd_in != -1) {
            close(fd_in);
        }
        if (fd_out != -1) {
            close(fd_out);
        }
        fprintf(stderr, "%s: incorrect command conditional\n", ERROR);
        return 1;
    }

    if (ind_end == end - 1) {
        return my_sh_conditional(args, init, ind_then, ind_else, ind_end, fd_in, fd_out);
    }

    int status1 = my_sh_conditional(args, init, ind_then, ind_else, ind_end, fd_in, fd_out);
    int status2 = my_sh_parser(args, ind_end + 1, end, fd_in, fd_out);

    return status1 | status2;
}

int my_sh_background_execute(char **args, int init, int end, int pos) {
    if (init == pos) {
        fprintf(stderr, "%s: incorrect command background\n", ERROR);
        return 1;
    }

    int status = 0;
    my_sh_background(args, init, pos);

    if (pos != end - 1) {
        status = my_sh_parser(args, pos + 1, end, -1, -1);
    }

    return status;
}

int my_sh_parser(char **args, int init, int end, int fd_in, int fd_out) {
    int ind = init;
    int priority = 0;
    int c_cond = 0;

    for (int i = init; i < end; i++) {
        int aux_priority = 0;

        if (strcmp(args[i], "if") == 0) {
            c_cond++;
        }
        if (strcmp(args[i], "end") == 0) {
            c_cond--;
        }
        if (c_cond != 0 || strcmp(args[i], "if") == 0 || strcmp(args[i], "end") == 0) {
            continue;
        }

        if (strcmp(args[i], "<") == 0) aux_priority = 1;
        if (strcmp(args[i], ">") == 0) aux_priority = 2;
        if (strcmp(args[i], ">>") == 0) aux_priority = 2;
        if (strcmp(args[i], "|") == 0) aux_priority = 3;
        if (strcmp(args[i], "&&") == 0) aux_priority = 4;
        if (strcmp(args[i], "||") == 0) aux_priority = 4;
        if (strcmp(args[i], ";") == 0) aux_priority = 5;
        if (strcmp(args[i], "&") == 0) aux_priority = 6;

        if (aux_priority >= priority) {
            priority = aux_priority;
            ind = i;
        }
    }

    if (strcmp(args[ind], "<") == 0) {
        return my_sh_redirect_in(args, init, end, fd_in, fd_out, ind);
    }
    if (strcmp(args[ind], "|") == 0) {
        return my_sh_pipes(args, init, end, fd_in, fd_out, ind);
    }
    if (strcmp(args[ind], ">") == 0) {
        return my_sh_redirect_out(args, init, end, fd_in, fd_out, ind);
    };
    if (strcmp(args[ind], ">") == 0) {
        return my_sh_redirect_out_append(args, init, end, fd_in, fd_out, ind);
    };
    if (strcmp(args[ind], "&&") == 0) {
        return my_sh_and_or(args, init, end, ind, 1);
    }
    if (strcmp(args[ind], "||") == 0) {
        return my_sh_and_or(args, init, end, ind, 0);
    }
    if (strcmp(args[ind], ";") == 0) {
        return my_sh_multiple(args, init, end, ind);
    };
    if (strcmp(args[ind], "if") == 0) {
        return my_sh_conditional_execute(args, init, end, fd_in, fd_out);
    }
    if (strcmp(args[ind], "&") == 0) {
        return my_sh_background_execute(args, init, end, ind);
    }

    return my_sh_execute_simple(args, init, end, fd_in, fd_out);
}