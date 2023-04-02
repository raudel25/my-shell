//
// Created by raudel on 3/25/2023.
//

#ifndef SHELL_BUILTIN_H
#define SHELL_BUILTIN_H

#endif //SHELL_BUILTIN_H

#include "list.h"
#include "glist.h"

extern List *background_pid;

extern GList *background_command;

extern char *variables[26];

extern char *home;

extern char *builtin_str[6];

extern int (*builtin_func[6])(char **);

extern char *builtin_str_out[4];

extern int (*builtin_func_out[4])(char **);

int my_sh_cd(char **args);

int my_sh_help(char **args);

int my_sh_exit();

int my_sh_num_builtins();

int my_sh_num_builtins_out();

void my_sh_init_variables();

int check_variable(char *variable);

void save_history(char *line);

int my_sh_history();

char **get_history();

char *get_again(int ind, int last);

int my_sh_unset(char **args);

int my_sh_foreground(char **args);

int my_sh_jobs();

int my_sh_get(char **args);

int my_sh_true();

int my_sh_false();

void load_help();