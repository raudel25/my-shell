#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pwd.h>
#include <string.h>
#include <signal.h>

#include "decod.h"
#include "execute.h"
#include "list.h"
#include "builtin.h"
#include "glist.h"

#define BOLD_CYAN "\033[1;36m"
#define RESET "\033[0m"
#define BOLD_RED "\033[1;31m"

#define MY_SH_TOK_DELIM " \t\r\n\a"


void head_shell() {
    char cwd[1024];
    getcwd(cwd, sizeof(cwd));
    printf("%smy_sh@%s:%s%s$ %s", BOLD_RED, pw->pw_name, BOLD_CYAN, cwd, RESET);
    fflush(stdout);
}

void my_sh_ctrl_c() {
    if (current_pid == -1) {
        printf("\n");
        head_shell();

        return;
    }

    for (int i = 0; i < pipes_pid->len; i++) {
        if (current_pid == last_pid) {
            kill(pipes_pid->array[i], SIGINT);
        } else {
            kill(pipes_pid->array[i], SIGKILL);
        }
    }

    if (current_pid == last_pid) {
        kill(current_pid, SIGINT);
    } else {
        kill(current_pid, SIGKILL);
    }

    last_pid = current_pid;

    printf("\n");
}


_Noreturn void my_sh_loop() {
    char *line;

    do {
        current_pid = -1;
        last_pid = -1;

        head_shell();
        line = my_sh_read_line();

        char *new_line = my_sh_decod_line(line);
        int save = line[0] != ' ';

        my_sh_execute(new_line, save, 1);

        free(line);
        free(new_line);
    } while (1);
}

int main() {
    uid_t uid;

    uid = getuid();
    pw = getpwuid(uid);

    pipes_pid = createList();
    background_pid = createList();
    background_command = createListG();
    my_sh_init_variables();

    signal(SIGINT, my_sh_ctrl_c);

    my_sh_loop();
}
