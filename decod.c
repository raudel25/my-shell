//
// Created by raudel on 3/23/2023.
//

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "decod.h"

#define MY_SH_TOK_BUF_SIZE 1024

char **my_sh_split_line(char *line, char *split) {
    int buf_size = MY_SH_TOK_BUF_SIZE, position = 0;
    char **tokens = malloc(buf_size * sizeof(char *));
    char *token;

    if (!tokens) {
        fprintf(stderr, "my_sh: allocation error\n");
        exit(EXIT_FAILURE);
    }

    token = strtok(line, split);
    while (token != NULL) {
        tokens[position] = token;
        position++;

        if (position >= buf_size) {
            buf_size += MY_SH_TOK_BUF_SIZE;
            tokens = realloc(tokens, buf_size * sizeof(char *));
            if (!tokens) {
                fprintf(stderr, "my_sh: allocation error\n");
                exit(EXIT_FAILURE);
            }
        }

        token = strtok(NULL, split);
    }
    tokens[position] = NULL;
    return tokens;
}

char *my_sh_decod_line(char *line) {
    char *aux_line = (char *) malloc(3 * strlen(line));

    int len = (int) strlen(line);
    int j = 0;
    for (int i = 0; i < len; i++) {
        if (line[i] == '#')
            break;

        if (line[i] == ' ') {
            if (i != 0) {
                if (line[i - 1] == ' ') continue;
            } else
                continue;
        }

        if (line[i] == '<') {
            if (i != 0) {
                if (line[i - 1] != ' ') {
                    aux_line[j++] = ' ';
                }
            }
            aux_line[j++] = line[i];
            if (i != len - 1) {
                if (line[i + 1] != ' ') {
                    aux_line[j++] = ' ';
                }
            }

            continue;
        }

        if (line[i] == '>') {
            if (i != 0) {
                if (line[i - 1] != ' ' && line[i - 1] != '>') {
                    aux_line[j++] = ' ';
                }
            }
            aux_line[j++] = line[i];
            if (i != sizeof(line) - 1) {
                if (line[i + 1] != ' ' && line[i + 1] != '>') {
                    aux_line[j++] = ' ';
                }
            }

            continue;
        }

        if (line[i] == '|') {
            if (i != 0) {
                if (line[i - 1] != ' ' && line[i - 1] != '|') {
                    aux_line[j++] = ' ';
                }
            }
            aux_line[j++] = line[i];
            if (i != len - 1) {
                if (line[i + 1] != ' ' && line[i + 1] != '|') {
                    aux_line[j++] = ' ';
                }
            }

            continue;
        }

        if (line[i] == '&') {
            if (i != len - 1 && i != 0) {
                if (line[i - 1] != ' ' && line[i + 1] == '&') {
                    aux_line[j++] = ' ';
                }
            }
            aux_line[j++] = line[i];
            if (i != 0 && i != len - 1) {
                if (line[i + 1] != ' ' && line[i - 1] == '&') {
                    aux_line[j++] = ' ';
                }
            }

            continue;
        }

        aux_line[j++] = line[i];
    }

    char *new_line = (char *) malloc(j + 1);

    for (int x = 0; x < j; x++) {
        new_line[x] = aux_line[x];
    }
    free(aux_line);

    if (new_line[j - 1] != '\n') new_line[j++] = '\n';
    new_line[j] = 0;

    return (char *) new_line;
}

char *sub_str(char *line, int init, int end) {
    char *new_line = (char *) malloc(end - init + 1);

    int i;
    for (i = 0; i < end - init + 1; i++) {
        new_line[i] = line[i + init];
    }
    new_line[i] = 0;

    return new_line;
}

char *determinate_set_command(char *line) {
    int i;
    int l = -1;
    int r = -1;
    int find = 0;
    for (i = 0; i < strlen(line); i++) {
        if (line[i] == '`' && find) {
            r = i - 1;
            break;
        }
        if (line[i] == '`') {
            l = i + 1;
            find = 1;
        }
    }

    if (l > r || l == -1 || r == -1) return NULL;

    return sub_str(line, l, r);
}

int array_size(char **args) {
    int i;

    for (i = 0; args[i] != NULL; i++);

    return i;
}
