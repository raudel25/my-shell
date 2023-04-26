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

#define CYAN "\033[0;36m"
#define RESET "\033[0m"
#define YELLOW "\033[0;33m"

#define MY_SH_TOK_DELIM " \t\r\n\a"

char *prompt() {
    char *line = NULL;

    char cwd[128];
    getcwd(cwd, sizeof(cwd));
    char head[256];
    snprintf(head, 256, "%smy_sh@%s:%s%s$ %s", YELLOW, pw->pw_name, CYAN, cwd, RESET);

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

        char *line_again = my_sh_again(line);
        char *new_line = my_sh_decode_line(line_again);
        if (line[0] != ' ') my_sh_save_history(line_again);

        my_sh_execute(new_line);

        free(line);
        free(new_line);
        free(line_again);
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
