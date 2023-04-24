//
// Created by raudel on 3/23/2023.
//

#ifndef SHELL_EXECUTE_H
#define SHELL_EXECUTE_H

#include "list.h"

int my_sh_execute(char *new_line, int save, int possible_back);

int my_sh_execute_pipes(char **args);

int my_sh_parser(char **args, int init, int end,int fd_int,int fd_out);

#endif //SHELL_EXECUTE_H
