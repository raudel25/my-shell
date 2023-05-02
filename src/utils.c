//
// Created by raudel on 3/23/2023.
//

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "utils.h"

#define ERROR "\033[0;31mmy_sh\033[0m"
#define MY_SH_TOK_BUF_SIZE 1024

List *special_spaces = NULL;

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

int format_set(const char *line, char *new_line, int i, int *j) {
    if (line[i] == '`') {
        if (i == 0 || line[i - 1] == ' ') {
            push_str(new_line, '(', j);
            push_str(new_line, ' ', j);
        } else {
            push_str(new_line, ' ', j);
            push_str(new_line, ')', j);
        }

        return 1;
    }

    return 0;
}

int format_spaces(const char *line, int i) {
    if (line[i] == ' ') {
        if (i != 0) {
            if (line[i - 1] == ' ') return 1;
        } else
            return 1;
    }

    return 0;
}

int format_special(char *line, char *new_line, int *i, int *j) {
    char special[5] = {' ', '\\', '"', '`', '#'};

    if (line[*i] == '\\') {
        for (int x = 0; x < 5; x++) {
            if (*i != strlen(line) - 1 && line[*i + 1] == special[x]) {
                if (special[x] == ' ') {
                    append(special_spaces, *j);
                    push_str(new_line, '#', j);
                } else {
                    push_str(new_line, special[x], j);
                }
                (*i)++;
            }
        }
        return 1;
    }

    return 0;
}

int format_pat(char *line, char *new_line, int i, int *j) {
    char pat[7] = {'<', ';', '(', ')', '>', '|', '&'};

    for (int x = 0; x < 7; x++) {
        if (line[i] == pat[x]) {
            if (i != 0) {
                if (line[i - 1] != ' ' && line[i - 1] != pat[x]) {
                    push_str(new_line, ' ', j);
                }
            }
            push_str(new_line, line[i], j);
            if (i != sizeof(line) - 1) {
                if (line[i + 1] != ' ' && (x < 4 || line[i + 1] != pat[x])) {
                    push_str(new_line, ' ', j);
                }
            }

            return 1;
        }
    }

    return 0;
}

char *my_sh_format_line(char *line) {
    char *new_line = (char *) malloc(MY_SH_TOK_BUF_SIZE);
    new_line[0] = 0;

    clear(special_spaces);

    int c = 0;

    int len = (int) strlen(line);
    int j = 0;
    for (int i = 0; i < len; i++) {
        if (line[i] == '"') {
            c = !c;
            continue;
        }

        if (c) {
            if (line[i] == ' ') {
                append(special_spaces, j);
                push_str(new_line, '#', &j);
            } else push_str(new_line, line[i], &j);
            continue;
        }

        if (line[i] == '#')
            break;

        int stop = format_special(line, new_line, &i, &j);
        stop = stop || format_spaces(line, i);
        stop = stop || format_set(line, new_line, i, &j);
        stop = stop || format_pat(line, new_line, i, &j);

        if (stop) continue;

        push_str(new_line, line[i], &j);
    }

    if (c) {
        fprintf(stderr, "%s: incorrect format of line\n", ERROR);
        free(new_line);

        return NULL;
    }

    return new_line;
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
    int x = 0;
    int y = 0;
    for (int i = 0; args[i] != NULL; i++) {
        int len = (int) strlen(args[i]);

        for (int j = 0; j < len; j++) {
            if (special_spaces->len != y && x == special_spaces->array[y]) {
                args[i][j] = ' ';
                y++;
            }
            x++;
        }
        x++;
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

void push_str(char *line, char c, int *len) {
    line[(*len)++] = c;
    line[(*len)] = 0;
}
