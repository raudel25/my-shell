//
// Created by raudel on 3/23/2023.
//

#ifndef SHELL_EXECUTE_H
#define SHELL_EXECUTE_H

#include "list.h"

int my_sh_execute(char *new_line, int save, int possible_back);

int my_sh_execute_pipes(char **args);

#endif //SHELL_EXECUTE_H