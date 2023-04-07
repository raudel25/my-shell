//
// Created by raudel on 3/23/2023.
//

#ifndef SHELL_DECOD_H
#define SHELL_DECOD_H

#endif //SHELL_DECOD_H

char **my_sh_split_line(char *line, char *split);

char *my_sh_decod_line(char *line);

char *sub_str(char *line, int init, int end);

void my_sh_decod_set(char *line);

int array_size(char **args);

void my_sh_encod_set(char *line);