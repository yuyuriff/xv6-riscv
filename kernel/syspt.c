#include "types.h"
#include "riscv.h"
#include "defs.h"
#include "param.h"
#include "spinlock.h"
#include "proc.h"

uint64
sys_remove_ad(void)
{
    uint64 srcva;
    int len;
    int mask;

    argaddr(0, &srcva);
    argint(1, &len);
    argint(2, &mask);

    if (len <= 0 || (mask & ~(PTE_A | PTE_D)) != 0) {
        return -1;
    }

    if (mask == 0) {
        return 0;
    }

    pagetable_t pt = myproc()->pagetable;
    uint64 n, va;
    pte_t *pte;

    while (len > 0) {
        va = PGROUNDDOWN(srcva);
        pte = walk(pt, va, 0);
        if (pte == 0 || (*pte & PTE_V) == 0 || (*pte & PTE_U) == 0) {
            return -2;
        }

        *pte &= ~((uint64)mask);
        n = PGSIZE - (srcva - va);
        if (n > len) {
            n = len;
        }
        len -= n;
        srcva = va + PGSIZE;
    }

    sfence_vma();

    return 0;
}

uint64
sys_check_ad(void) 
{
    uint64 srcva;
    int len;
    int mask;

    argaddr(0, &srcva);
    argint(1, &len);
    argint(2, &mask);

    if (len <= 0 || (mask & ~(PTE_A | PTE_D)) != 0) {
        return -1;
    }

    if (mask == 0) {
        return 0;
    }

    pagetable_t pt = myproc()->pagetable;
    uint64 n, va;
    pte_t *pte;
    int flagged = 0;

    while (len > 0) {
        va = PGROUNDDOWN(srcva);
        pte = walk(pt, va, 0);
        if (pte == 0 || (*pte & PTE_V) == 0 || (*pte & PTE_U) == 0) {
            return -2;
        }

        if (((*pte) & (uint64)mask) != 0) {
            flagged = 1;
        }

        n = PGSIZE - (srcva - va);
        if (n > len) {
            n = len;
        }
        len -= n;
        srcva = va + PGSIZE;
    }

    return flagged;
}