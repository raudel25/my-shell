//
// Created by raudel on 3/25/2023.
//

#ifndef SHELL_BUILTIN_H
#define SHELL_BUILTIN_H

#endif //SHELL_BUILTIN_H

#include "list.h"

extern List* background_pid;

extern char *variables[26];

extern char *home;

extern char *builtin_str[5];

extern int (*builtin_func[5])(char **);

int my_sh_cd(char **args);

int my_sh_help();

int my_sh_exit();

int my_sh_num_builtins();

void my_sh_init_variables();

int check_variable(char *variable);

void save_history(char *line);

int my_sh_history();

char **get_history();

char *get_again(int ind);

int my_sh_unset(char **args);