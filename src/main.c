#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <cjson/cJSON.h>
#include <ncurses.h>
#include <locale.h>
#include <wchar.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <limits.h>

/*------------------------------------------------------*/
/*-------nah, bro, u better close this file-------------*/
/*-------the fucking hell going on in here--------------*/
/*------------------------------------------------------*/

#define MAX_INPUT_Q 250

int MAX_LIMIT;
const char* CONFIG_PATH = NULL;
const char* DOWNLOAD_DIR = NULL;

volatile int shrinked = 0;
volatile int allow_shrink = 0;

volatile int RECEIVED = 0;
volatile int is_first_exec = 1;

volatile sig_atomic_t resize_flag = 0;
volatile pid_t py_proc = -1;

#include "utf8_str.h"
#include "read+parse.h"
#include "ncurses_funcs.h"
#include "check_config.h"

void handle_resize(int sig) {
    resize_flag = 1;
}
void handle_sigint(int sig) {
    endwin();
    if (py_proc != -1) {
        killpg(py_proc, SIGKILL);
        waitpid(py_proc, NULL, 0);
    }
    exit(1);
}
void handle_sigterm(int sig) { exit(0); }
void handle_sigsegv(int sig) {
    endwin();
    printf("Error: that's a segfault how tf did you get it? (report dev)\n");
    kill(getppid(), SIGTERM);
    exit(1);
}




int main(int argc, char* argv[]) {
    if (argc > 2 || (argc > 1 && strcmp(argv[1], "--help") == 0)) {
        printf(" Usage: yuzutube [OPTION]\n");
        printf("\t--min-width\tallows terminal width < 60\n");
        printf("\t--help\tdisplay this message\n");
        exit(0);
    }


    if (argc > 1 && strcmp(argv[1], "--min-width") == 0) {
        allow_shrink = 1;
    }

    setlocale(LC_ALL, "");
    signal(SIGWINCH, handle_resize);
    signal(SIGINT, handle_sigint);
    signal(SIGTERM, handle_sigterm);
    signal(SIGSEGV, handle_sigsegv);

    char* max_limit_str = getenv("YUZU_MAX_LIMIT");
    if (max_limit_str) {
        MAX_LIMIT = atoi(max_limit_str);
    }
    CONFIG_PATH = getenv("YUZU_CONFIG_PATH");
    DOWNLOAD_DIR = getenv("YUZU_DOWNLOAD_DIR");

    check_envs();  

    pid_t pid;

    while (1) {
        pid = fork(); 

        if (pid < 0) {
            endwin();
            perror("Fork failed");
            exit(1);
        } else if (pid == 0) {
            //setpgid(0,0);
            draw_main();
            exit(0);
        } else {
            while (!resize_flag) {
                usleep(100000);
            }
            if (is_first_exec) {
                is_first_exec = 0;
            }
            resize_flag = 0;
            kill(pid, SIGKILL);
            waitpid(pid, NULL, 0); 
            if (py_proc != -1) {
                killpg(py_proc, SIGKILL);
                waitpid(py_proc, NULL, 0);
                py_proc = -1;
            }
        }

    }

    return 0;
}

