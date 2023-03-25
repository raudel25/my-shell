#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#include "decod.h"
#include "execute.h"
#include "vec.h"

#define BOLD_CYAN "\033[1;36m"
#define WHITE "\033[0m"
#define BOLD_RED "\033[1;31m"
#define HOST_NAME_MAX 10

//char* nsh_get_home_adr(){
//    int len = (int)strlen(getlogin());
//    char* path = calloc((strlen("/home/") + len + 1) * sizeof(char), sizeof(char));
//    strcpy(path,"/home/");
//    strcat(path,getlogin());
//    return path;
//}
//
//void head_shell(){
//    int capacity = 64;
//    char *buf = calloc(capacity,1);
//    char*to_free = buf;
//    while (getcwd(buf,capacity) == NULL){
//        capacity *= 2;
//        buf = realloc(buf,capacity);
//    }
//    fflush(0);
//    char hostname[HOST_NAME_MAX];
//    gethostname(hostname,HOST_NAME_MAX);
//
//    char* home_path = nsh_get_home_adr();
//    int hp_len = (int)strlen(home_path);
//    if (strlen(buf) >= hp_len){
//        char c = buf[hp_len];
//        buf[hp_len] = 0;
//        if (strcmp(buf,home_path) == 0){
//            buf += hp_len - 1;
//            buf[0] = '~';
//            buf[1] = c;
//        } else buf[hp_len] = 0;
//    }
//    free(home_path);
//
//    printf("%s%s@%s%s:%s%s%s$ ", BOLD_RED, getlogin(), hostname, WHITE, BOLD_CYAN, buf, WHITE);
//    free(to_free);
//}

void head_shell() {
    struct passwd *pw;
    uid_t uid;
    fflush(0);

    uid = getuid();
    char *hostname=NULL;
//    pw = getpwuid(uid)char hostname[HOST_NAME_MAX];
//    gethostname(hostname,HOST_NAME_MAX);

    char cwd[1024];
    getcwd(cwd, sizeof(cwd));

    printf("%smy_shell:%s%s$ %s", BOLD_RED, BOLD_CYAN, cwd, WHITE);
}

void my_sh_loop(void) {
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
    pid_history = vector_create();
    my_sh_loop();
    return 0;
}
