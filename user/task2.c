#include "kernel/types.h"
#include "kernel/fcntl.h"
#include "user/user.h"

#define BUF_SIZE 256

void
write_buf(int fd, const char* buf, int len)
{
    int i = 0;
    while (i < len) {
        int written = write(fd, buf + i, len - i);
        if (written <= 0) {
            fprintf(2, "Write error\n");
            exit(1);
        }
        i += written;
    }
}

void
flush_buf(int fd, const char* buf, int* cur)
{
    if (*cur > 0) {
        write_buf(fd, buf, *cur);
        *cur = 0;
    }
}

void
add_to_buf(int fd, char *buf, int *cur, const char *str, int len)
{
  int i = 0;
  while (i < len) {
    if (*cur == BUF_SIZE) {
      flush_buf(fd, buf, cur);
    }

    int available = BUF_SIZE - *cur;
    int chunk_size = (len - i > available) ? available : len - i;
    memmove(buf + *cur, str + i, chunk_size);

    *cur += chunk_size;
    i += chunk_size;
  }
}

int
main(int argc, char *argv[]) 
{   
    int pipefd[2];
    if (pipe(pipefd) < 0) {
        fprintf(2, "Pipe error\n");
        exit(1);
    }

    int pid;
    pid = fork();

    if (pid < 0) {
        fprintf(2, "Fork error\n");
        exit(1);
    }
    else if (pid == 0) {
        if (close(0) < 0) {
            fprintf(2, "Close stdin error\n");
            exit(1);
        }
        if (dup(pipefd[0]) != 0) {
            fprintf(2, "Dup error\n");
            exit(1);
        }
        if (close(pipefd[0]) < 0) {
            fprintf(2, "Close pipefd[0] error\n");
            exit(1);
        }
        if (close(pipefd[1]) < 0) {
            fprintf(2, "Close pipefd[1] error\n");
            exit(1);
        }

        char *args[] = {"/wc", 0};
        exec("/wc", args);

        fprintf(2, "Exec error\n");
        exit(1);
    }
    else {
        if (close(pipefd[0]) < 0) {
            fprintf(2, "Parent close pipefd[0] error\n");
            exit(1);
        }
        
        char buf[BUF_SIZE];
        int cur = 0;
        for (int i = 1; i < argc; i++) {
            int len = strlen(argv[i]);
            add_to_buf(pipefd[1], buf, &cur, argv[i], len);
            add_to_buf(pipefd[1], buf, &cur, "\n", 1);
        }
        flush_buf(pipefd[1], buf, &cur);

        if (close(pipefd[1]) < 0) {
            fprintf(2, "Parent close pipefd[1] error\n");
            exit(1);
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