//
// Created by raudel on 3/23/2023.
//

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "decode.h"

#define ERROR "\033[1;31mmy_sh\033[0m"
#define MY_SH_TOK_BUF_SIZE 1024

char **my_sh_split_line(char *line, char *split) {
    int buf_size = MY_SH_TOK_BUF_SIZE, position = 0;
    char **tokens = malloc(buf_size * sizeof(char *));
    char *token;

    token = strtok(line, split);
    while (token != NULL) {
        tokens[position] = token;
        position++;

        if (position >= buf_size) {
            buf_size += MY_SH_TOK_BUF_SIZE;
            tokens = realloc(tokens, buf_size * sizeof(char *));
            if (!tokens) {
                fprintf(stderr, "%s: allocation error\n", ERROR);
                fprintf(stderr, "my_sh: \n");
                exit(EXIT_FAILURE);
            }
        }

        token = strtok(NULL, split);
    }
    tokens[position] = NULL;
    return tokens;
}

void my_sh_encode_set(char *line) {
    int c_set = 0;

    for (int i = 0; i < strlen(line); i++) {
        if (i > 0 && (line[i - 1] == ' ' || line[i - 1] == '#') && line[i] == '`') c_set++;
        if (i > 0 && (line[i - 1] != ' ' && line[i - 1] != '#') && line[i] == '`') c_set--;

        if (c_set != 0) {
            if (line[i] == ' ') line[i] = '#';
        }
    }
}

char *my_sh_decode_line(char *line) {
    char aux_line[strlen(line)];

    char pat[7] = {'<', ';', '(', ')', '>', '|', '&'};

    int c = 0;

    int len = (int) strlen(line);
    int j = 0;
    for (int i = 0; i < len; i++) {
        if (line[i] == '#')
            break;

        if (line[i] == '"') {
            c = !c;
            continue;
        }

        if (c) {
            if (line[i] == ' ') aux_line[j++] = '#';
            else aux_line[j++] = line[i];
            continue;
        }

        if (line[i] == '\\') {
            if (i != len - 1 && line[i + 1] == ' ') {
                aux_line[j++] = '#';
                i++;
            }
            continue;
        }

        if (line[i] == ' ') {
            if (i != 0) {
                if (line[i - 1] == ' ') continue;
            } else
                continue;
        }

        int stop = 0;

        for (int x = 0; x < 7; x++) {
            if (line[i] == pat[x]) {
                if (i != 0) {
                    if (line[i - 1] != ' ' && line[i - 1] != pat[x]) {
                        aux_line[j++] = ' ';
                    }
                }
                aux_line[j++] = line[i];
                if (i != sizeof(line) - 1) {
                    if (line[i + 1] != ' ' && (x < 4 || line[i + 1] != pat[x])) {
                        aux_line[j++] = ' ';
                    }
                }

                stop = 1;
                break;
            }
        }

        if (stop) continue;

        aux_line[j++] = line[i];
    }

    if (c) {
        fprintf(stderr, "%s: incorrect format of line\n", ERROR);

        return "false";
    }

    char *new_line = (char *) malloc(MY_SH_TOK_BUF_SIZE);

    for (int x = 0; x < j; x++) {
        new_line[x] = aux_line[x];
    }

    if (new_line[j - 1] != '\n') new_line[j++] = '\n';
    new_line[j] = 0;

    return (char *) new_line;
}

char *sub_str(const char *line, int init, int end) {
    char *new_line = (char *) malloc(sizeof(char) * (end - init + 1));

    int i;
    for (i = 0; i < end - init + 1; i++) {
        new_line[i] = line[i + init];
    }
    new_line[i] = 0;

    return new_line;
}

void my_sh_decode_c(char **args) {
    for (int i = 0; args[i] != NULL; i++) {
        int len = (int) strlen(args[i]);

        for (int j = 0; j < len; j++) {
            if (args[i][j] == '#') args[i][j] = ' ';
        }
    }
}

int array_size(char **args) {
    int i;

    for (i = 0; args[i] != NULL; i++);

    return i;
}

int pat_equal(char *line, char *pat, int pos) {
    int len = (int) strlen(line);
    int len_pat = (int) strlen(pat);

    if (len - pos < len_pat || pos < 0) return 0;

    int equal = 1;
    for (int i = 0; i < len_pat; i++) {
        if (line[pos + i] != pat[i]) equal = 0;
    }

    return equal;
}

char *array_to_str(char **args, int init, int end) {
    char *new_line = (char *) malloc(MY_SH_TOK_BUF_SIZE);
    strcpy(new_line, "");

    for (int i = init; i < end - 1; i++) {
        strcat(new_line, args[i]);
        strcat(new_line, " ");
    }
    strcat(new_line, args[end - 1]);

    return new_line;
}

void push_str(char *line, char c) {
    int w = (int) strlen(line);
    line[w++] = c;
    line[w] = 0;
}
