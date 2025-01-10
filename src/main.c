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

void check_files(int* error);
void check_dirs(int* error);

void check_statdir(const char* stat_dir, int* error) {
    struct stat st;
    if (stat(stat_dir, &st) != 0) {
        *error = 1;
        printf("Error: %s is not a file or directory.\n", stat_dir);
    } else if (!S_ISDIR(st.st_mode)) {
        *error = 1;
        printf("Error: %s is not a directory.\n", stat_dir);
    }
}

void check_envs() {
    int error = 0;
 
    const char* py_path = getenv("PYTHONPATH");
    if (!py_path) {
        printf("Error: PYTHONPATH is not set, are you sure you are in venv? [y/N] ");
        char ch = getchar();
        if (ch != 'y') {
            printf("Error: PYTHONPATH is not set.\n");
            error = 1;
        }
    } else {
        check_statdir(py_path, &error);
    }

    if (!MAX_LIMIT) {
        MAX_LIMIT = 10;
    }
    if (!CONFIG_PATH) {
        const char* HOME = getenv("HOME");
        if (!HOME) {
            error = 1;
            printf("Error: environmental variable HOME is not set(it's weird, report this).\n");
        } else {
            char tmp[strlen(HOME) + 20];
            snprintf(tmp, sizeof(tmp), "%s/.config/yuzutube", HOME);

            if (setenv("YUZU_CONFIG_PATH", tmp, 0) != 0) {
                error = 1;
                printf("Error: cannot set environmental variable YUZU_CONFIG_PATH.\n");
            } else if ((CONFIG_PATH = getenv("YUZU_CONFIG_PATH"))) {
                check_statdir(CONFIG_PATH, &error);
                
            } else {
                error = 1;
                printf("Error: environmental variable YUZU_CONFIG_PATH is not set.\n");
            } 
        }
        
    } else {
        check_statdir(CONFIG_PATH, &error);
    }
    if (!DOWNLOAD_DIR) {
        char cwd[PATH_MAX];
        if (getcwd(cwd, sizeof(cwd)) == NULL) {
            error = 1;
            printf("Error: something wrong with your shell, bro, or dev is screwed(report this).\n");
        } else if (setenv("YUZU_DOWNLOAD_DIR", cwd, 0) != 0) {
            error = 1;
            printf("Error: cannot set environmental variable YUZU_DOWNLOAD_DIR.\n");
        } else if ((DOWNLOAD_DIR = getenv("YUZU_DOWNLOAD_DIR"))) {
            check_statdir(DOWNLOAD_DIR, &error);

        } else {
            error = 1;
            printf("Error: environmental variable YUZU_DOWNLOAD_DIR is not set.\n");
        } 
    } else {
        check_statdir(DOWNLOAD_DIR, &error);

    }

    check_dirs(&error);
    check_files(&error); 

    if (error) {
        exit(1);
    }
}

void check_files(int* error) {
    char py_sender[strlen(CONFIG_PATH) + 26];
    snprintf(py_sender, sizeof(py_sender), "%s/bin/query_sender.py", CONFIG_PATH);
    if (access(py_sender, F_OK) != 0) {
        *error = 1;
        printf("Error: %s is inaccessible (check YUZU_CONFIG_PATH).\n", py_sender);
    }

    char py_downloader[strlen(CONFIG_PATH) + 26];
    snprintf(py_downloader, sizeof(py_downloader), "%s/bin/downloader.py", CONFIG_PATH);
    if (access(py_downloader, F_OK) != 0) {
        *error = 1;
        printf("Error: %s is inaccessible (check YUZU_CONFIG_PATH).\n", py_downloader);
    }

    char cookies_dir[strlen(CONFIG_PATH) + 26];
    snprintf(cookies_dir, sizeof(cookies_dir), "%s/conf/cookies.txt", CONFIG_PATH);
    if (access(cookies_dir, F_OK) != 0) {
        *error = 1;
        printf("Error: %s is inaccessible (copy cookies.txt to your YUZU_CONFIG_PATH/conf/ check YUZU_CONFIG_PATH).\n", cookies_dir);
    }
}

void check_dirs(int* error) {
    char dirs[3][strlen(CONFIG_PATH) + 7];

    snprintf(dirs[0], sizeof(dirs[0]), "%s/bin", CONFIG_PATH);
    snprintf(dirs[1], sizeof(dirs[1]), "%s/tmp", CONFIG_PATH);
    snprintf(dirs[2], sizeof(dirs[2]), "%s/conf", CONFIG_PATH);

    for (int i = 0; i < 3; ++i){
        struct stat st;
        if (stat((const char*)dirs[i], &st) != 0) {
            *error = 1;
            printf("Error: %s is not a file or directory.\n", dirs[i]);
        } else if (!S_ISDIR(st.st_mode)) {
            *error = 1;
            printf("Error: %s is not a directory.\n", dirs[i]);
        }
    }
}

volatile int shrinked = 0;
volatile int allow_shrink = 0;

volatile int RECEIVED = 0;
volatile int is_first_exec = 1;

volatile sig_atomic_t resize_flag = 0;
volatile pid_t py_proc = -1;

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



#include "read+parse.h"
#include "utf8_str.h"
#include "ncurses_funcs.h"



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

