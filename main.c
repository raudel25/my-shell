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

void my_sh_loop() {
    char *line;
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
        int save=line[0]!=' ';

        status= my_sh_execute(new_line,save);

        free(line);
        free(new_line);
    } while (status);
}

int main() {
    pid_history = createList();
    background_pid = createList();
    my_sh_init_variables();

    my_sh_loop();

    free(home);
    free(pid_history);

    return 0;
}
