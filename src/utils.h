//
// Created by raudel on 3/23/2023.
//

#ifndef SHELL_UTILS_H
#define SHELL_UTILS_H

#include "list.h"

extern List *special_spaces;

char **my_sh_split_line(char *line, char *split);

char *my_sh_format_line(char *line);

char *sub_str(const char *line, int init, int end);

void my_sh_decode_c(char **args);

int array_size(char **args);

int pat_equal(char *line, char *pat, int pos);

void push_str(char *line, char c,int *len);

char *array_to_str(char **args, int init, int end);

#endif //SHELL_UTILS_H