#include "types.h"
#include "param.h"
#include "spinlock.h"
#include "sleeplock.h"
#include "fs.h"
#include "file.h"
#include "memlayout.h"
#include "riscv.h"
#include "defs.h"
#include "proc.h"

#define ZERO_BUF_SIZE 32
char zero_buf[ZERO_BUF_SIZE] = {0};

struct {
    struct spinlock lock;
    uint64 seed;
} urandom;

struct {
    struct spinlock lock;
    uint64 stat;
} nullstat;

#define LCG_A 1664525
#define LCG_C 1013904223

static uint64
urand_get_next() {
    uint64 next = urandom.seed * LCG_A + LCG_C;
    urandom.seed = next;
    return next;
}

int
zeroread(int user_dst, uint64 dst, int n, int m) {
    int target = n;
    int size = ZERO_BUF_SIZE;
    int read_size;
    uint64 i = dst;

    while (target > 0) {
        read_size = (target < size) ? target : size;
        if (either_copyout(user_dst, i, zero_buf, read_size) == -1) {
            break;
        }

        target -= read_size;
        i += read_size;
    }

    return n - target;
}

int
urandomread(int user_dst, uint64 dst, int n, int m) {
    int target = n;
    int size = sizeof(uint64);
    int read_size;
    uint64 random_number;
    uint64 i = dst;

    // ensure one seed throughout a single write
    acquire(&urandom.lock);
    while (target > 0) {
        read_size = (target < size) ? target : size;
        random_number = urand_get_next();

        if (either_copyout(user_dst, i, (char*)&random_number, read_size) == -1) {
            release(&urandom.lock);
            return -1;
        }

        target -= read_size;
        i += read_size;
    }
    release(&urandom.lock);

    return n - target;
}

int
nullstatread(int user_dst, uint64 dst, int n, int m) {
    if (n != sizeof(nullstat.stat)) {
        return -1;
    }
    acquire(&nullstat.lock);
    uint64 stat = nullstat.stat;
    release(&nullstat.lock);

    if (either_copyout(user_dst, dst, (char*)&stat, sizeof(stat)) == -1) {
        return -1;
    }
    return sizeof(uint64);
}

int
urandomwrite(int user_src, uint64 src, int n, int m) {
    if (n != sizeof(urandom.seed)) {
        return -1;
    }

    uint32 new_seed;
    if (either_copyin(&new_seed, user_src, src, n) == -1) {
        return -1;
    }

    acquire(&urandom.lock);
    urandom.seed = new_seed;
    release(&urandom.lock);

    return n;
}

int
nullstatwrite(int user_src, uint64 src, int n, int m) {
    acquire(&nullstat.lock);
    nullstat.stat += n;
    release(&nullstat.lock);

    return n;
}

int
driverread(int user_dst, uint64 dst, int n, int m) {
    switch (m) {
        case NULL_MINOR:
            return 0;
            break;
        case ZERO_MINOR:
            return zeroread(user_dst, dst, n, m);
            break;
        case URAND_MINOR:
            return urandomread(user_dst, dst, n, m);  
        case NULLSTAT_MINOR:
            return nullstatread(user_dst, dst, n, m);
        default:
            return -2;
            break;
    }
}

int
driverwrite(int user_src, uint64 src, int n, int m) {
    switch (m) {
        case NULL_MINOR:
            return n;
            break;
        case ZERO_MINOR:
            return -1;
            break;
        case URAND_MINOR:
            return urandomwrite(user_src, src, n, m);  
        case NULLSTAT_MINOR:
            return nullstatwrite(user_src, src, n, m);
        default:
            return -2;
            break;
    }
}

void
nullstat_init() {
    initlock(&nullstat.lock, "nullstat");
    nullstat.stat = 0;
}

void
urand_init() {
    initlock(&urandom.lock, "urandom");
    urandom.seed = r_time();
}

void
driverinit() {
    urand_init();
    nullstat_init();

    devsw[NEW_DRIVER].read = driverread;
    devsw[NEW_DRIVER].write = driverwrite;
}


