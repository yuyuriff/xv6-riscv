#include "kernel/types.h"
#include "user/user.h"
#include "kernel/stat.h"    

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

// mutex is not readable, writable, cannot call fstat
static void
test_rw_fstat() {
  int fd = create_mutex();

  char c;
  if (read(fd, &c, 1) >= 0) {
      fail_output_and_exit("mutex shouldn't be readable");
  }
  if (write(fd, "c", 1) >= 0) {
      fail_output_and_exit("mutex shouldn't be writable");
  }

  struct stat s;
  if (fstat(fd, &s) >= 0) {
      fail_output_and_exit("fstat on mutex should fail");
  }

  close(fd);
  fprintf(1, "Test rw fstat passed\n");
}

// mutex can be closed by current proc
// mutex cannot be unlocked by parent
static void
test_close_unlock() {
  int fd = create_mutex();
  if (mutex_lock(fd) < 0) {
      fail_output_and_exit("unable to lock created mutex");
  }
  close(fd);
  fprintf(1, "Test close normally passed\n");

  fd = create_mutex();
  int pid = fork();
  if (pid < 0) {
      fail_output_and_exit("fork error"); 
  }

  if (pid == 0) {
    if (mutex_lock(fd) < 0) {
        fail_output_and_exit("couldn't lock child");
    }
    pause(5);
    if (mutex_unlock(fd) < 0) {
        fail_output_and_exit("couldn't unlock child");
    }
    close(fd);
    exit(0);
  } else {
    pause(2);
    if (mutex_unlock(fd) != -2) {
        fail_output_and_exit("mutex must not be unlocked by parent");
    }
    close(fd);
    int a;
    wait(&a);
  }
  fprintf(1, "Test close unlock passed\n");
}

// exit closes mutex (can be locked again)
static void
test_exit() {
  int fd = create_mutex();
  int pid = fork();
  if (pid < 0) {
      fail_output_and_exit("fork error"); 
  }
  if (pid == 0) {
    if (mutex_lock(fd) < 0) {
        fail_output_and_exit("couldn't lock child");
    }
    exit(0);
  } else {
    int a;
    wait(&a);
    if (mutex_lock(fd) < 0) {
        fail_output_and_exit("couldn't lock parent - child mutex must be freed");
    }
    close(fd);
  }
  fprintf(1, "Test exit passed\n");
}

// filedup makes ref++
// close from parent doesn't destroy mutex
static void
test_fork_dup() {
  int fd = create_mutex();
  int pid = fork();
  if (pid < 0) {
      fail_output_and_exit("fork error"); 
  }
  if (pid == 0) {
    if (mutex_lock(fd) < 0) {
        fail_output_and_exit("couldn't lock child");
    }
    pause(10);
    if (mutex_unlock(fd) < 0) {
        fail_output_and_exit("couldn't unlock child");
    }
    close(fd);
    exit(0);
  } else {
    close(fd);
    int a;
    wait(&a);
  }
  fprintf(1, "Test fork passed\n");
}

int
main(int argc, char **argv) {
  test_rw_fstat();
  test_close_unlock();
  test_exit();
  test_fork_dup();

  fprintf(1, "SUCCESS: all tests passed\n");
  exit(0);
}