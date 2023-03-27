#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pwd.h>
#include <string.h>

#include "decod.h"
#include "execute.h"
#include "list.h"
#include "builtin.h"

#define BOLD_CYAN "\033[1;36m"
#define WHITE "\033[0m"
#define BOLD_RED "\033[1;31m"

#define MY_SH_TOK_DELIM " \t\r\n\a"

void head_shell(char *name) {
    char cwd[1024];
    getcwd(cwd, sizeof(cwd));
    printf("%smy_shell@%s:%s%s$ %s", BOLD_RED, name, BOLD_CYAN, cwd, WHITE);
}

void my_sh_loop(void) {
    char *line;
    char **args;
    int status;

    struct passwd *pw;
    uid_t uid;

    uid = getuid();
    pw = getpwuid(uid);

    home = (char *) malloc(strlen(pw->pw_dir));
    strcpy(home,pw->pw_dir);

    do {
        head_shell(pw->pw_name);
        line = my_sh_read_line();

        char *new_line = my_sh_decod_line(line);
        char *aux = (char *) malloc(strlen(new_line));
        strcpy(aux, new_line);

        args = my_sh_split_line(new_line, MY_SH_TOK_DELIM);


        if (strcmp(args[0], "again") == 0) {
            int q = 0;
            if (args[1] != NULL) {
                if (strlen(args[1]) == 1 && args[1][0] - '0' > 0 && args[1][0] - '0' < 10) q = args[1][0] - '0';
                if (strcmp(args[1], "10") == 0) q = 10;
            }

            char *c_again = get_again(q);

            if (q != 0 && c_again != NULL) {

                save_history(c_again);
                char **new_args = my_sh_split_line(c_again, MY_SH_TOK_DELIM);

                status = my_sh_execute(new_args);

                free(new_args);
            } else
                printf("my_sh: incorrect command again\n");

            free(c_again);
        } else {
            if (line[0] != ' ') save_history(aux);

            status = my_sh_execute(args);
        }

        free(aux);
        free(line);
        free(new_line);
        free(args);
    } while (status);
}

int main() {
    pid_history = createList();
    my_sh_loop();
    return 0;
}
