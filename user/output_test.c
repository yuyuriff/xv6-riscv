#include "kernel/types.h"
#include "user/user.h"

static void
fail_output_and_exit(const char *msg) {
  fprintf(2, "FAILED: %s\n", msg);
  exit(1);
}

static int
create_mutex() {
  int fd = -1;
  if (mutex(&fd) < 0) {
      fail_output_and_exit("couldn't create mutex");
  }
  if (fd < 0) {
      fail_output_and_exit("fd invalid value");
  }
  return fd;
}

static void
print_args(int argc, char** argv, int pid, int fd, int mutex) {
  for (int i = 1; i < argc; i++) {
    char *s = argv[i];
    for (int j = 0; s[j] != '\0'; j++) {
      if (mutex) {
        if (mutex_lock(fd) < 0)
          fail_output_and_exit("mutex lock failed");
      }

      printf("%d: arg %d, char '%c'\n", pid, i, s[j]);

      if (mutex) {
        if (mutex_unlock(fd) < 0)
          fail_output_and_exit("mutex unlock failed");
      }
    }
  }
}

int
main(int argc, char **argv) {
  if (argc < 2) {
    fail_output_and_exit("not enough args");
  }

  printf("No mutex:\n");
  int fd = -1;
  int pid = fork();

  if (pid < 0) {
      fail_output_and_exit("fork error"); 
  }
  if (pid == 0) {
    print_args(argc, argv, getpid(), fd, 0);
    exit(0);
  } else {
    print_args(argc, argv, getpid(), fd, 0);
    int a;
    wait(&a);
  }

  printf("With mutex:\n");
  fd = create_mutex();
  pid = fork();

  if (pid < 0) {
    fail_output_and_exit("fork error"); 
  }
  if (pid == 0) {
    print_args(argc, argv, getpid(), fd, 1);
    exit(0);
  } else {
    print_args(argc, argv, getpid(), fd, 1);
    int a;
    wait(&a);
  }

  close(fd);
  exit(0);
}
