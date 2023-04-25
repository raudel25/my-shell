#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pwd.h>
#include <signal.h>
#include <string.h>
#include <readline/readline.h>

#include "decode.h"
#include "execute.h"
#include "list.h"
#include "builtin.h"

#define BOLD_CYAN "\033[1;36m"
#define RESET "\033[0m"
#define BOLD_RED "\033[1;31m"

#define MY_SH_TOK_DELIM " \t\r\n\a"

char *prompt() {
    char *line = NULL;

    char cwd[128];
    getcwd(cwd, sizeof(cwd));
    char head[256];
    snprintf(head, 256, "%smy_sh@%s:%s%s$ %s", BOLD_RED, pw->pw_name, BOLD_CYAN, cwd, RESET);

    line = readline(head);

    return line;
}

void my_sh_ctrl_c() {
    if (current_pid == -1) {
        printf("\n");
        rl_on_new_line();
        rl_replace_line("", 0);
        rl_redisplay();

        return;
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
        my_sh_update_background();

        line = prompt();

        char *new_line = my_sh_decode_line(line);
        int save = line[0] != ' ';

        my_sh_execute(new_line, save);

        free(line);
        free(new_line);
    } while (1);
}

int main() {
    uid_t uid;

    uid = getuid();
    pw = getpwuid(uid);

    background_pid = createList();
    my_sh_init_variables();
    my_sh_load_history();

    signal(SIGINT, my_sh_ctrl_c);

    my_sh_loop();
}
