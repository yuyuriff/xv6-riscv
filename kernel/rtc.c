#include "types.h"
#include "param.h"
#include "memlayout.h"
#include "riscv.h"
#include "spinlock.h"
#include "proc.h"
#include "defs.h"

#define Reg(reg) ((volatile uint32 *)(RTC0 + (reg)))

#define ReadReg(reg) (*(Reg(reg)))

static struct spinlock rtc_lock;

void
rtcinit(void)
{
    initlock(&rtc_lock, "rtc");
}

uint64
rtc_read(void)
{
    acquire(&rtc_lock);
    uint32 low = ReadReg(RTC_LOW);
    uint32 high = ReadReg(RTC_HIGH);
    release(&rtc_lock);

    uint64 time = ((uint64)high << 32) | low;
    return time;
}

uint64
sys_rtc(void)
{
    return rtc_read();
}
