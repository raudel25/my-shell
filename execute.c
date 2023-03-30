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
#include "execute.h"
#include "list.h"

#define MY_SH_TOK_DELIM " \t\r\n\a"

List *pid_history = NULL;

int null_entry;

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

int get_execute(char *variable) {
    if (variable == NULL) {
        for (int i = 0; i < 26; i++) {
            if (variables[i] != NULL) {
                printf("%c = ", (char) (i + 'a'));
                printf("%s", variables[i]);
                if (variables[i][strlen(variables[i]) - 1] != '\n') printf("\n");
            }
        }

        printf("%c",0);
        return 1;
    }

    if (check_variable(variable)) {
        for (int i = 0; i < 26; i++) {
            if (i != variable[0] - 'a')
                continue;

            if (variables[i] != NULL) {
                printf("%s", variables[i]);
                if (variables[i][strlen(variables[i]) - 1] != '\n') printf("\n");
                printf("%c",0);
                return 1;
            }
        }

        write(2, "my_sh: variable not found\n", 26);
    }

    printf("%c",0);
    return 0;
}

int my_sh_launch(char **args, int fd_in, int fd_out, int fd_next) {
    pid_t pid;
    int status;

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

        if (strcmp(args[0], "get") == 0) {
            exit(get_execute(args[1]));
        }

        if (execvp(args[0], args) == -1) {
            perror("my_shell");
        }
        exit(EXIT_FAILURE);
    } else if (pid < 0) {
        perror("my_shell");
    } else {
        append(pid_history, pid);

        if (fd_in != -1){
            close(fd_in);
        }
        if (fd_out != -1){
            close(fd_out);
        }

        if (fd_next == -1) {
            for (int i = 0; i < pid_history->len; ++i) {
                do {
                    int c_pid = pid_history->array[i];
                    waitpid(c_pid, &status, WUNTRACED);
                } while (!WIFEXITED(status) && !WIFSIGNALED(status));
            }

            clear(pid_history);
        }
    }

    return 1;
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

    if (null_entry) {

        fd[0] = redirect_in("/dev/null");
    }

}

int my_sh_again(char **args) {
    int status = 1;

    int q = 0;
    if (args[1] != NULL) {
        if (strlen(args[1]) == 1 && args[1][0] - '0' > 0 && args[1][0] - '0' < 10) q = args[1][0] - '0';
        if (strcmp(args[1], "10") == 0) q = 10;
    }

    char *c_again = get_again(q);

    if (q != 0 && c_again != NULL) {
        status = my_sh_execute(c_again, 1);
    } else
        printf("my_sh: incorrect command again\n");

    free(c_again);

    return status;
}

int my_sh_set(char **args, char *new_line) {
    int status = 1;

    if (args[1] != NULL && args[2] != NULL) {
        if (check_variable(args[1])) {
            if (variables[args[1][0] - 'a'] != NULL) {
                free(variables[args[1][0] - 'a']);
            }
            if (args[2][0] != '`') {
                variables[args[1][0] - 'a'] = (char *) malloc(strlen(args[2]));
                strcpy(variables[args[1][0] - 'a'], args[2]);
            } else {
                char *new_command = determinate_set_command(new_line);

                if (new_command != NULL) {
                    char *new_command_format = my_sh_decod_line(new_command);

                    int fd[2];
                    pipe(fd);

                    pid_t pid;

                    pid = fork();
                    if (pid == 0) {
                        dup2(fd[1], STDOUT_FILENO);
                        close(fd[1]);
                        close(fd[0]);

                        my_sh_execute(new_command_format, 0);
                        exit(EXIT_FAILURE);
                    } else if (pid > 0) {
                        do {
                            waitpid(pid, &status, WUNTRACED);
                        } while (!WIFEXITED(status) && !WIFSIGNALED(status));
                    }
                    char *buffer = (char *) malloc(1024);

                    close(fd[1]);
                    read(fd[0], buffer, 1024);
                    close(fd[0]);

                    variables[args[1][0] - 'a'] = (char *) malloc(strlen(buffer));
                    strcpy(variables[args[1][0] - 'a'], buffer);

                    free(buffer);
                    free(new_command);
                    free(new_command_format);
                } else {
                    printf("my_sh: incorrect command set\n");
                }
            }
        }
    } else
        printf("my_sh: incorrect command set\n");

    return status;
}

int my_sh_background(char *new_line) {
    int pid;

    pid = fork();
    if (pid == 0) {
        setpgid(0, 0);

        char *aux = sub_str(new_line, 0, (int) strlen(new_line) - 4);
        my_sh_execute(aux, 0);

        char *new_aux = (char *) malloc(strlen(aux) + 1);
        strcpy(new_aux, aux);
        strcat(new_aux, "\n");

        free(aux);
        free(new_aux);
        exit(EXIT_FAILURE);
    } else if (pid > 0) {
        setpgid(pid, pid);
        append(background_pid, pid);
        printf("[%d]\t%d\n", background_pid->len, pid);
    }

    return 1;
}

void my_sh_execute_save(char **args, char *line, int save) {
    if (strcmp(args[0], "exit") == 0 || strcmp(args[0], "again") == 0) save = 0;
    if (save) save_history(line);
}

int my_sh_execute(char *new_line, int save) {
    int status;
    char *aux = (char *) malloc(strlen(new_line));
    strcpy(aux, new_line);

    char **args = my_sh_split_line(new_line, MY_SH_TOK_DELIM);

    if (args[0] == NULL) {
        free(aux);
        free(args);

        return 1;
    }

    my_sh_execute_save(args, aux, save);

    for (int i = 0; i < my_sh_num_builtins(); i++) {
        if (strcmp(args[0], builtin_str[i]) == 0) {
            status = (*builtin_func[i])(args);

            free(aux);
            free(args);

            return status;
        }
    }

    if (strcmp(args[0], "again") == 0) {
        status = my_sh_again(args);
    } else if (strcmp(args[0], "set") == 0) {
        status = my_sh_set(args, aux);
    } else if (aux[strlen(aux) - 2] == '&') {
        status = my_sh_background(aux);
    } else {
        status = my_sh_execute_args(args);
    }

    free(aux);
    free(args);

    return status;
}

int my_sh_execute_args(char **args) {
    int fd[3];
    fd[2] = -1;
    int aux[2];
    int init = 0;
    int result;

    while (1) {
        my_sh_new_args(init, args, fd[2], fd, aux);
        int end = aux[0];

        char **new_args = (char **) malloc((end - init) * sizeof(char *));

        for (int i = init; i < end; i++) {
            new_args[i - init] = (char *) malloc(strlen(args[i]));
            strcpy(new_args[i-init],args[i]);
        }
        new_args[end - init] = NULL;

        result = my_sh_launch(new_args, fd[0], fd[1], fd[2]);
        free(new_args);

        init = aux[1];
        if (args[end] == NULL || args[init] == NULL)
            break;
    }

    return result;
}
