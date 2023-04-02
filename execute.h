//
// Created by raudel on 3/23/2023.
//

#ifndef SHELL_EXECUTE_H
#define SHELL_EXECUTE_H

#endif //SHELL_EXECUTE_H

#include "list.h"

extern List *pid_history;

int my_sh_execute(char *new_line, int save, int possible_back);

int my_sh_execute_args(char **args, char *line);

int my_sh_background(char *line);

int my_sh_set(char **args, char *line);

int my_sh_again(char **args);
