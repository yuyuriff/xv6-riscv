#include "types.h"
#include "param.h"
#include "riscv.h"
#include "defs.h"
#include "spinlock.h"
#include "logger.h"

struct logger_ {
    int mask;
    uint ticks_limit;      // 0 for unlimited ticks
    struct spinlock lock;
};

static struct logger_ logger;

void
loggerinit(void)
{
    initlock(&logger.lock, "logger");
    logger.mask = 0;
    logger.ticks_limit = 0;
}

static uint
get_ticks(void)
{
    uint t;
    acquire(&tickslock);
    t = ticks;
    release(&tickslock);

    return t;
}

int
set_logger(int new_mask, int new_ticks)
{
    if (new_mask < 0 || (new_mask & ~LOG_ALL) || new_ticks < 0) {
        return -1;
    }
    
    uint t = get_ticks();

    acquire(&logger.lock);
    if (new_ticks > 0) {
        logger.ticks_limit = t + new_ticks;
    } else {
        logger.ticks_limit = 0;
    }
    logger.mask = new_mask;
    release(&logger.lock);

    return 0;
}

int
logger_active_for_task(int task)
{
    uint t = get_ticks();

    acquire(&logger.lock);

    if(logger.ticks_limit != 0 && t >= logger.ticks_limit){
        logger.mask = 0;
        logger.ticks_limit = 0;
    }
    int active = (logger.mask & task) != 0;

    release(&logger.lock);

    return active;
}

uint64
sys_logset(void)
{
    int mask;
    int new_ticks;

    argint(0, &mask);
    argint(1, &new_ticks);

    return set_logger(mask, new_ticks);
}
