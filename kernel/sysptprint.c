#include "types.h"
#include "riscv.h"
#include "defs.h"
#include "param.h"
#include "spinlock.h"
#include "proc.h"

void
get_pteflags(pte_t pte, char out[8])
{
  out[0] = (pte & PTE_R) ? 'R' : '_';
  out[1] = (pte & PTE_W) ? 'W' : '_';
  out[2] = (pte & PTE_X) ? 'X' : '_';
  out[3] = (pte & PTE_U) ? 'U' : '_';
  out[4] = (pte & PTE_G) ? 'G' : '_';
  out[5] = (pte & PTE_A) ? 'A' : '_';
  out[6] = (pte & PTE_D) ? 'D' : '_';
  out[7] = '\0';
}

void
print_dots(int level)
{
    const char* dots = ".........";
    for (int i = 0; i < level; i++) {
        printf("%s", dots);
    }
}

void
print_by_level(pagetable_t pt, int level)
{
    for (int i = 0; i < 512; i++) {
        pte_t pte = pt[i];
        if ((pte & PTE_V) == 0) {
            continue;
        }

        char flags[8];
        get_pteflags(pte, flags);
        print_dots(level);

        uint64 pa = PTE2PA(pte);
        printf("0x%x -> %p %s\n", i, (void*)pa, flags);

        if (level < 2) {
            print_by_level((pagetable_t)pa, level + 1);
        }
    }
}

void
print_all_pt(pagetable_t pt)
{
    printf("PAGETABLE %p\n", (void *)pt);
    print_by_level(pt, 0);
}

uint64
sys_ptprint(void)
{
    print_all_pt(myproc()->pagetable);
    return 0;
}
