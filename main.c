#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pwd.h>
#include <signal.h>
#include <string.h>
#include <readline/readline.h>

#include "decod.h"
#include "execute.h"
#include "list.h"
#include "builtin.h"
#include "glist.h"

#define BOLD_CYAN "\033[1;36m"
#define RESET "\033[0m"
#define BOLD_RED "\033[1;31m"

#define MY_SH_TOK_DELIM " \t\r\n\a"
#define MY_SH_MAX_HISTORY 10

int current_command;

void command_change(int up) {
    char **args = get_history();

    int i;
    for (i = 0; args[i] != NULL; i++) {
        if (args[i][0] != '#') break;
    }

    if (i == 0) {
        free(args);
        return;
    }

    int top = i < MY_SH_MAX_HISTORY ? i : MY_SH_MAX_HISTORY;
    if (current_command == -1) {
        current_command = top;
    }

    char *c = sub_str(args[top - current_command], 1, (int) strlen(args[top - current_command]) - 1);
    rl_replace_line("", 0);
    rl_insert_text(c);
    rl_redisplay();

    if (up && current_command != 1) current_command--;
    if (!up && current_command != MY_SH_MAX_HISTORY) current_command++;

    free(args);
    free(c);
}

static int up_arrow_callback() {
    command_change(1);
    return 0;
}

static int down_arrow_callback() {
    if (current_command != -1) command_change(0);
    return 0;
}

char *prompt() {
    char *line = NULL;
    current_command = -1;
    rl_bind_keyseq("\\e[A", up_arrow_callback);
    rl_bind_keyseq("\\e[B", down_arrow_callback);

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

        line = prompt();

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
