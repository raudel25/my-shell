#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pwd.h>

#include "decod.h"
#include "execute.h"

#define BOLD_CYAN "\033[1;36m"
#define WHITE "\033[0m"
#define BOLD_RED "\033[1;31m"

void head_shell(){
    struct passwd *pw;
    uid_t uid;

    uid = getuid();
    pw = getpwuid(uid);

    char cwd[1024];
    getcwd(cwd, sizeof(cwd));

    printf("%s%s%s:%s%s$ %s",BOLD_RED,"my_shell@",pw->pw_name,BOLD_CYAN,cwd,WHITE);
}

void my_sh_loop(void)
{
    char *line;
    char **args;
    int status;

    do {
        head_shell();
        line = my_sh_read_line();
        args = my_sh_split_line(line);
        status = my_sh_execute(args);

        free(line);
        free(args);
    } while (status);
}

int main() {
    my_sh_loop();;
    return 0;
}
