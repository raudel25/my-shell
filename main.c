#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pwd.h>

#include "decod.h"
#include "execute.h"
#include "list.h"

#define BOLD_CYAN "\033[1;36m"
#define WHITE "\033[0m"
#define BOLD_RED "\033[1;31m"

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

    do {
        head_shell(pw->pw_name);
        line = my_sh_read_line();
        args = my_sh_split_line(line);
        status = my_sh_execute(args);

        free(line);
        free(args);
    } while (status);
}

int main() {
    pid_history = createList();
    my_sh_loop();
    return 0;
}
