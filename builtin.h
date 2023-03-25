//
// Created by raudel on 3/25/2023.
//

#ifndef SHELL_BUILTIN_H
#define SHELL_BUILTIN_H

#endif //SHELL_BUILTIN_H

extern char* builtin_str[3];

extern int (*builtin_func[3])(char **);

int my_sh_cd(char **args);

int my_sh_help();

int my_sh_exit();

int my_sh_num_builtins();