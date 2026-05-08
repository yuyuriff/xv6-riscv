#include "kernel/types.h"
#include "kernel/logger.h"
#include "user/user.h"

static int
isnum(char *s)
{
  if (*s == 0)
    return 0;

  while (*s) {
    if (*s < '0' || *s > '9')
      return 0;
    s++;
  }

  return 1;
}

static void
usage(void)
{
    fprintf(2, "usage:\n");
    fprintf(2, "  logset off\n");
    fprintf(2, "  logset all [ticks]\n");
    fprintf(2, "  logset syscall [ticks]\n");
    fprintf(2, "  logset irq [ticks]\n");
    fprintf(2, "  logset proc [ticks]\n");
    fprintf(2, "  logset exec [ticks]\n");
    exit(1);
}

int
main(int argc, char **argv)
{
    int mask = 0;
    int ticks = 0;

    if (argc < 2)
        usage();

    if (strcmp(argv[1], "off") == 0) {
        if(argc != 2)
            usage();

        if (logset(0, 0) < 0) {
            fprintf(2, "logset: failed\n");
            exit(1);
        }
        exit(0);
    }

    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "all") == 0){
            mask |= LOG_ALL;
        } else if (strcmp(argv[i], "syscall") == 0) {
            mask |= LOG_SYSCALL;
        } else if (strcmp(argv[i], "irq") == 0) {
            mask |= LOG_INTR;
        } else if (strcmp(argv[i], "proc") == 0) {
            mask |= LOG_PROC;
        } else if (strcmp(argv[i], "exec") == 0) {
            mask |= LOG_EXEC;
        } else if (isnum(argv[i])) {
            ticks = atoi(argv[i]);
        } else {
            usage();
        }
    }

    if (logset(mask, ticks) < 0){
        fprintf(2, "logset: failed\n");
        exit(1);
    }

    exit(0);
}