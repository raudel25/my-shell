//
// Created by raudel on 3/23/2023.
//

#ifndef SHELL_DECODE_H
#define SHELL_DECODE_H

char **my_sh_split_line(char *line, char *split);

char *my_sh_decode_line(char *line);

char *sub_str(const char *line, int init, int end);

void my_sh_decode_c(char **args);

int array_size(char **args);

void my_sh_encode_set(char *line);

int pat_equal(char *line, char *pat, int pos);

void push_str(char *line, char c);

#endif //SHELL_DECODE_H