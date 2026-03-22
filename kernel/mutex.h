struct mutex {
  struct sleeplock *lock;
};

int
mutexalloc(struct file **f);

void
mutexclose(struct mutex *m);
