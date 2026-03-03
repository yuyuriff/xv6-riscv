#include <unistd.h>
#include <sys/types.h>
#include <string.h>
#include <sys/wait.h>
#include <sys/param.h>
#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <stdint.h>

#define BUF_SIZE 4096

int
write_buf(int fd, const char* buf, int len)
{
    int i = 0;
    while (i < len) {
        int written = write(fd, buf + i, len - i);
        if (written <= 0) {
            return -1;
        }
        i += written;
    }
    return 0;
}

int
flush_buf(int fd, const char* buf, int* cur)
{
    if (*cur > 0) {
        if (write_buf(fd, buf, *cur) < 0) {
            return -1;
        }
        *cur = 0;
    }
    return 0;
}

int
add_to_buf(int fd, char *buf, int *cur, const char *str, int len)
{
  int i = 0;
  while (i < len) {
    if (*cur == BUF_SIZE) {
      if (flush_buf(fd, buf, cur) < 0) {
        return -1;
      }
    }

    int available = BUF_SIZE - *cur;
    int chunk_size = MIN(available, len - i);
    memmove(buf + *cur, str + i, chunk_size);

    *cur += chunk_size;
    i += chunk_size;
  }

  return 0;
}

int
main(int argc, char *argv[]) 
{   
    int pipefd[2];
    if (pipe(pipefd) < 0) {
        perror("pipe");
        exit(EXIT_FAILURE);
    }

    pid_t pid;
    pid = fork();

    if (pid < 0) {
        perror("fork");
        exit(EXIT_FAILURE);
    }
    else if (pid == 0) {
        if (close(pipefd[1]) < 0) {
            perror("close pipefd-1");
            exit(EXIT_FAILURE);
        }

        char buf[BUF_SIZE];

        ssize_t bytes_read = read(pipefd[0], buf, BUF_SIZE);
        while (bytes_read > 0) {
            if (write_buf(STDOUT_FILENO, buf, (int)bytes_read) < 0) {
                perror("write");
                exit(EXIT_FAILURE);
            }
            bytes_read = read(pipefd[0], buf, BUF_SIZE);
        }

        if (bytes_read < 0) {
            perror("read");
            exit(EXIT_FAILURE);
        }

        if (close(pipefd[0]) < 0) {
            perror("close pipefd-0");
            exit(EXIT_FAILURE);
        }

        exit(EXIT_SUCCESS);
    }
    else {
        if (close(pipefd[0]) < 0) {
            perror("parent close pipefd-0");
            exit(EXIT_FAILURE);
        }
        
        char buf[BUF_SIZE];
        int cur = 0;
        for (int i = 1; i < argc; i++) {
            int len = strlen(argv[i]);
            if (add_to_buf(pipefd[1], buf, &cur, argv[i], len) < 0) {
                perror("write");
                exit(EXIT_FAILURE);
            }
            if (add_to_buf(pipefd[1], buf, &cur, "\n", 1) < 0) {
                perror("write");
                exit(EXIT_FAILURE);
            }
        }

        if (flush_buf(pipefd[1], buf, &cur) < 0) {
            perror("write");
            exit(EXIT_FAILURE);
        }

        if (close(pipefd[1]) < 0) {
            perror("parent close pipefd-1");
            exit(EXIT_FAILURE);
        }

        int status;
        pid_t cpid = wait(&status);
        if (cpid < 0) {
            perror("wait");
            exit(EXIT_FAILURE);
        }
        printf("Child PID = %jd exited status = %d\n", (intmax_t)cpid, status);
    }

    exit(EXIT_SUCCESS);
}