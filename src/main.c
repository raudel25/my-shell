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
#include "glist.h"

#define BOLD_CYAN "\033[1;36m"
#define RESET "\033[0m"
#define BOLD_RED "\033[1;31m"

#define MY_SH_TOK_DELIM " \t\r\n\a"
#define MY_SH_MAX_HISTORY 20

int current_command;
char **history;
int top;

void command_change() {
    int len = (int) strlen(history[top - current_command]) - 1;
    char c[len];

    int i;
    for (i = 0; i < len; i++) {
        c[i] = history[top - current_command][i + 1];
    }
    c[i] = 0;

    rl_replace_line("", 0);
    rl_insert_text(c);
    rl_redisplay();
}


void load_history() {
    history = get_history();

    int i;
    for (i = 0; history[i] != NULL; i++) {
        if (history[i][0] != '#') break;
    }

    top = i < MY_SH_MAX_HISTORY ? i : MY_SH_MAX_HISTORY;
    current_command = -1;
}

static int up_arrow_callback() {
    if (current_command != 1 && current_command != -1 && top != 0) {
        current_command--;
        command_change();
    }
    if (current_command == -1 && top != 0) {
        current_command = top;
        command_change();
    }

    return 0;
}

static int down_arrow_callback() {
    if (current_command != top && current_command != -1 && top != 0) {
        current_command++;
        command_change();
    }
    return 0;
}

char *prompt() {
    load_history();

    char *line = NULL;
    rl_bind_keyseq("\\e[A", up_arrow_callback);
    rl_bind_keyseq("\\e[B", down_arrow_callback);

    char cwd[128];
    getcwd(cwd, sizeof(cwd));
    char head[256];
    snprintf(head, 256, "%smy_sh@%s:%s%s$ %s", BOLD_RED, pw->pw_name, BOLD_CYAN, cwd, RESET);

    line = readline(head);

    free(history);
    return line;
}

void my_sh_ctrl_c() {
    if (current_pid == -1) {
        current_command = -1;
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

        my_sh_execute(new_line, save, 1);

        free(line);
        free(new_line);
    } while (1);
}

int main() {
    uid_t uid;

    uid = getuid();
    pw = getpwuid(uid);

    background_pid = createList();
    background_command = createListG();
    my_sh_init_variables();

    signal(SIGINT, my_sh_ctrl_c);

    my_sh_loop();
}
