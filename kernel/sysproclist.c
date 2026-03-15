#include "types.h"
#include "riscv.h"
#include "defs.h"
#include "param.h"
#include "memlayout.h"
#include "spinlock.h"
#include "proc.h"
#include "vm.h"
#include "procinfo.h"

extern struct proc proc[NPROC];
extern struct spinlock wait_lock;

// -1 means buffer is not sufficient
// -2 means unsuccessful copyout
uint64
sys_ps_listinfo(void)
{
    uint64 plistaddr;
    int lim;
    argaddr(0, &plistaddr);
    argint(1, &lim);

    int count = 0;
    struct proc *p;
    if (plistaddr == 0) {
        for (p = proc; p < &proc[NPROC]; p++) {
            acquire(&p->lock);
            if (p->state != UNUSED) {
                count++;
            }
            release(&p->lock);
        }
        return count;
    } else {
        procinfo_s pi;
        for (p = proc; p < &proc[NPROC]; p++) {
            acquire(&wait_lock);
            acquire(&p->lock);

            if (p->state == UNUSED) {
                release(&p->lock);
                release(&wait_lock);
                continue;
            }

            count++;
            if (count > lim) {
                release(&p->lock);
                release(&wait_lock);
                return -1;
            }

            pi.pid = p->pid;
            memmove(pi.name, p->name, 16);
            pi.state = (int)p->state;
            pi.ppid = p->parent ? p->parent->pid : -1;
            release(&p->lock);
            release(&wait_lock);

            uint64 i = (uint64)count - 1;
            int result = copyout(myproc()->pagetable, plistaddr + i * sizeof(procinfo_s), (char*)&pi, sizeof(pi));

            if (result < 0) {
                return -2;
            }
        }

        return count;
    }
}
