#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pwd.h>
#include "decod.h"
#include "execute.h"

void head_shell(){
    struct passwd *pw;
    uid_t uid;

    uid = getuid();
    pw = getpwuid(uid);

    char cwd[1024];
    getcwd(cwd, sizeof(cwd));

    printf("myshell@");
    printf("%s", pw->pw_name);
    printf(":%s", cwd);
    printf(" $");
}

void lsh_loop(void)
{
    char *line;
    char **args;
    int status;

    do {
        head_shell();
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
