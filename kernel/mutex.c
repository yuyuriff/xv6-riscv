#include "types.h"
#include "riscv.h"
#include "defs.h"
#include "fs.h"
#include "spinlock.h"
#include "sleeplock.h"
#include "param.h"
#include "mutex.h"
#include "file.h"

void free_on_error(struct mutex *m, struct file **f, struct sleeplock *lock) {
  if (lock) {
      kfree((char*)lock);
  }
  if (m) {
      kfree((char*)m);
  }
  if (*f) {
      fileclose(*f);
  }
}

int
mutexalloc(struct file **f) {
  struct mutex *m;
  struct sleeplock *lock;

  m = 0; lock = 0; *f = 0;
  *f = filealloc();
  if (!(*f)) {
      free_on_error(m, f, lock);
      return -1;
  }
  m = (struct mutex*)kalloc();
  if (!m) {
      free_on_error(m ,f, lock);
      return -1;
  }
  lock = (struct sleeplock*)kalloc();
  if (!lock) {
      free_on_error(m, f, lock);
      return -1;
  }
  m->lock = lock;
  initsleeplock(m->lock, "mutex lock");

  (*f)->type = FD_MUTEX;
  (*f)->readable = 0;
  (*f)->writable = 0;
  (*f)->mutex = m;

  printf("mutexalloc m=%p lk=%p\n", m, m->lock);
  return 0;
}

void
mutexclose(struct mutex *m) {
  printf("mutexclose m=%p lk=%p\n", m, m->lock);
  if (!m) {
      return;
  }
  if (m->lock) {
      kfree((char*)m->lock);
  }
  kfree((char*)m);
}
