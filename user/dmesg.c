#include "kernel/types.h"
#include "kernel/param.h"
#include "kernel/riscv.h"
#include "user/user.h"

#define DMESG_BUFSIZE (BUFPAGES * PGSIZE + 1)

static char buf[DMESG_BUFSIZE];

int
main(int argc, char **argv)
{
    if (dmesg(buf, sizeof(buf)) < 0) {
        fprintf(2, "dmesg: syscall failed\n");
        exit(1);
    }

    printf("%s", buf);
    exit(0);
}