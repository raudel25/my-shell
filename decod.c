//
// Created by raudel on 3/23/2023.
//

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "decod.h"

#define MY_SH_TOK_BUFSIZE 64

char *my_sh_read_line() {
    char *line = NULL;
    ssize_t bufsize = 0; // have getline allocate a buffer for us

    if (getline(&line, &bufsize, stdin) == -1) {
        if (feof(stdin)) {
            exit(EXIT_SUCCESS);  // We recieved an EOF
        } else {
            perror("readline");
            exit(EXIT_FAILURE);
        }
    }

    return line;
}

char **my_sh_split_line(char *line,char *split)
{
    int bufsize = MY_SH_TOK_BUFSIZE, position = 0;
    char **tokens = malloc(bufsize * sizeof(char*));
    char *token;

    if (!tokens) {
        fprintf(stderr, "my_sh: allocation error\n");
        exit(EXIT_FAILURE);
    }

    token = strtok(line, split);
    while (token != NULL) {
        tokens[position] = token;
        position++;

        if (position >= bufsize) {
            bufsize += MY_SH_TOK_BUFSIZE;
            tokens = realloc(tokens, bufsize * sizeof(char*));
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

char *my_sh_decod_line(const char *line) {
    char *aux_line = (char *) malloc(3 * strlen(line));

    int j = 0;
    for (int i = 0; i < strlen(line); i++) {
        if (line[i] == '#')
            break;

        if (line[i] == ' ') {
            if (i != 0) {
                if (line[i - 1] == ' ') continue;
            } else
                continue;
        }
        if (line[i] == '<' || line[i] == '>' || line[i] == '|') {
            if (i != 0) {
                if (line[i - 1] != ' ' && !(line[i] == '>' && line[i - 1] == '>')) {
                    aux_line[j++] = ' ';
                }
            }
            aux_line[j++] = line[i];
            if (i != sizeof(line) - 1) {
                if (line[i + 1] != ' ' && !(line[i] == '>' && line[i + 1] == '>')) {
                    aux_line[j++] = ' ';
                }
            }

            continue;
        }

        aux_line[j++] = line[i];
    }

    char *new_line = (char *) malloc(j);

    for (int x = 0; x < j; x++) {
        new_line[x] = aux_line[x];
    }
    free(aux_line);

    if (new_line[j - 1] != '\n') new_line[j++] = '\n';
    new_line[j] = 0;

    return (char *) new_line;
}

char *eliminate_first(char *line) {
    char *new_line = (char *) malloc(strlen(line) - 1);

    int i;
    for (i = 0; i < strlen(line) - 1; i++) {
        new_line[i] = line[i + 1];
    }
    new_line[i] = 0;

    return new_line;
}
