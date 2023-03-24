#include <stdio.h>
#include <stdlib.h>
#include "decod.h"
#include "execute.h"


void lsh_loop(void)
{
    char *line;
    char **args;
    int status;

    do {
        printf("> ");
        line = lsh_read_line();
        args = lsh_split_line(line);
        status = lsh_execute(args);

        free(line);
        free(args);
    } while (status);
}

int main() {
    lsh_loop();;
    return 0;
}
