#include "kernel/types.h"
#include "kernel/fcntl.h"
#include "user/user.h"

#define SLEEP_TIME 50

int
main(int argc, char *argv[]) 
{
    if (argc != 2 || (strcmp(argv[1], "1") != 0 && strcmp(argv[1], "0") != 0)) {
        fprintf(2, "Usage: task1 0|1\n");
        exit(1);
    }

    int mode = (strcmp(argv[1], "0") == 0) ? 0 : 1;

    int pid;
    pid = fork();

    if (pid < 0) {
        fprintf(2, "Fork error\n");
        exit(1);
    }
    else if (pid == 0) {
        pause(SLEEP_TIME);
        exit(1);
    }
    else {
        printf("Parent PID = %d Child PID = %d\n", getpid(), pid);
        if (mode) {
            if (kill(pid) < 0) {
                fprintf(2, "Kill error\n");
                exit(1);
            }
            printf("Child process killed\n");
        }
        int status;
        int cpid = wait(&status);
        if (cpid < 0) {
            fprintf(2, "Wait error\n");
            exit(1);
        }
        printf("Child PID = %d exited status = %d\n", cpid, status);
    }

    exit(0);
}