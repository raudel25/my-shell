//
// Created by raudel on 3/25/2023.
//

#ifndef SHELL_BUILTIN_H
#define SHELL_BUILTIN_H

#include "list.h"
#include "glist.h"

extern int current_pid;

extern int last_pid;

extern List *background_pid;

extern GList *variables_key;

extern GList *variables_value;

extern struct passwd *pw;

extern char *builtin_str[7];

extern int (*builtin_func[7])(char **);

extern char *builtin_str_out[4];

extern int (*builtin_func_out[4])(char **);

int my_sh_cd(char **args);

int my_sh_help(char **args);

int my_sh_exit();

int my_sh_num_builtins();

int my_sh_num_builtins_out();

void my_sh_init_variables();

int check_variable(char *variable);

void my_sh_save_history(char *line);

int my_sh_history();

int my_sh_unset(char **args);

int my_sh_foreground(char **args);

int my_sh_jobs();

int my_sh_get(char **args);

int my_sh_true();

int my_sh_false();

int my_sh_set(char **args);

int my_sh_background(char **args, int init, int end);

char *my_sh_again(char *line);

void my_sh_update_background();

void my_sh_load_history();

#endif //SHELL_BUILTIN_H