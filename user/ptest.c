#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

#define PTE_A (1L << 6) // accessed
#define PTE_D (1L << 7) // dirty 

static void
fail_and_exit(const char *msg)
{
    fprintf(2, "ERROR: %s\n", msg);
    exit(1);
}

void
check_flags(const char *name, void *p, int len)
{
    int a = check_ad(p, len, PTE_A);
    int d = check_ad(p, len, PTE_D);
    int ad = check_ad(p, len, PTE_A | PTE_D);
    if (a < 0 || d < 0 || ad < 0) {
        fail_and_exit("check ad returned error");
    }
    printf("%s: A=%d D=%d AD=%d\n", name, a, d, ad);
}

static int global = 1010110;

int
main(void)
{
    int stack_var = 156536;
    int stack_array[512];

    printf("Initial:\n");
    ptprint();

    printf("\nCheck stack and global:\n");
    check_flags("global", (void *)&global, sizeof(global));
    check_flags("stack_var", (void *)&stack_var, sizeof(stack_var));
    check_flags("stack_array", (void *)stack_array, 512 * sizeof(int));

    int size = 2 * 4096 + 16;
    char *heap_array = malloc(size);
    if (!heap_array) {
        fail_and_exit("malloc failed");
    }

    printf("\nHeap array allocated:\n");
    ptprint();
    if (remove_ad(heap_array, size, PTE_A | PTE_D) != 0) {
        fail_and_exit("remove_ad heap failed");
    }

    volatile int tmp;
    tmp = global;
    tmp += stack_var;
    tmp += stack_array[10];
    tmp += heap_array[12];

    printf("\nAfter read:\n");
    check_flags("global", (void *)&global, sizeof(global));
    check_flags("stack_var", (void *)&stack_var, sizeof(stack_var));
    check_flags("stack_array", (void *)stack_array, 512 * sizeof(int));
    check_flags("heap_array", (void *)heap_array, size);

    global++;
    stack_var++;
    stack_array[23]++;
    heap_array[1245]++;

    printf("\nAfter read and write:\n");
    check_flags("global", (void *)&global, sizeof(global));
    check_flags("stack_var", (void *)&stack_var, sizeof(stack_var));
    check_flags("stack_array", (void *)stack_array, 512 * sizeof(int));
    check_flags("heap_array", (void *)heap_array, size);

    printf("\nAfter clear AD:\n");
    if (remove_ad(&global, sizeof(global), PTE_A | PTE_D) != 0) {
        fail_and_exit("remove_ad global failed");
    }
    if (remove_ad(&stack_var, sizeof(stack_var), PTE_A | PTE_D) != 0) {
        fail_and_exit("remove_ad stack_var failed");
    }
    if (remove_ad(stack_array, sizeof(stack_array), PTE_A | PTE_D) != 0) {
        fail_and_exit("remove_ad stack_array failed");
    }
    if (remove_ad(heap_array, size, PTE_A | PTE_D) != 0) {
        fail_and_exit("remove_ad heap failed");
    }
    ptprint();
    check_flags("global", (void *)&global, sizeof(global));
    check_flags("stack_var", (void *)&stack_var, sizeof(stack_var));
    check_flags("stack_array", (void *)stack_array, 512 * sizeof(int));
    check_flags("heap_array", (void *)heap_array, size);

    free(heap_array);
    printf("\nAfter free heap:\n");
    ptprint();

    printf("\nALL OK\n");
    exit(0);
}