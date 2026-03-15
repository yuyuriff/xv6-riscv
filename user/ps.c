#include "kernel/types.h"
#include "user/user.h"

void
print_str_with_padding(const char *s, int col_width)
{
    int padding = col_width - strlen(s);
    printf("%s", s);
    for (int i = 0; i < padding; i++) {
        printf(" ");
    }
}

int
count_pos_digits(int n)
{
    if (n == 0) {
        return 1;
    }
    int count = 0;
    while (n > 0) {
        n /= 10;
        count++;
    }
    return count;
}

void
print_int_with_padding(int n, int col_width)
{
    int padding = col_width - count_pos_digits(n);
    printf("%d", n);
    for (int i = 0; i < padding; i++) {
        printf(" ");
    }
}

int
main(int argc, char *argv[])
{
    int lim;
    lim = ps_listinfo(0, 1);
    if (lim < 0) {
        fprintf(2, "ps_listinfo error\n");
        exit(1);
    }

    procinfo_s *plist = (procinfo_s *)malloc(lim * sizeof(procinfo_s));
    if (!plist) {
        fprintf(2, "Malloc error\n");
        exit(1);
    }

    int n = ps_listinfo(plist, lim);
    while (n == -1) {
        lim *= 2;
        free(plist);
        plist = (procinfo_s *)malloc(lim * sizeof(procinfo_s));
        if (!plist) {
            fprintf(2, "Malloc error\n");
            exit(1);
        }
        n = ps_listinfo(plist, lim);
    }
    if (n == -2) {
        fprintf(2, "Copyout to user space error\n");
        exit(1);
    }

    char* pnames = (char*)malloc(n * 16);
    if (!pnames) {
        fprintf(2, "Malloc error\n");
        exit(1);
    }
    memset(pnames, '\0', n * 16);
    procinfo_s *p;
    for (p = plist; p < &plist[n]; p++) {
        int ppid = p->ppid;
        if (ppid == -1) {
            continue;
        }
        for (int i = 0; i < n; i++) {
            procinfo_s *pi = &plist[i];
            int j = p - plist;
            if (pi->pid == ppid) {
                memmove(&pnames[j * 16], pi->name, 16);
                pnames[j * 16 + 15] = '\0';
            }
        }
    }

    print_str_with_padding("PID", 11);
    print_str_with_padding("NAME", 17);
    print_str_with_padding("STATE", 12);
    print_str_with_padding("PPID", 11);
    print_str_with_padding("PNAME", 16);
    printf("\n");

    char* str_states[6] = { "unused", "used", "sleep", "runnable", "running", "zombie" };
    for (p = plist; p < &plist[n]; p++) {
        print_int_with_padding(p->pid, 11);
        print_str_with_padding(p->name, 17);
        print_str_with_padding(str_states[p->state], 12);
        if (p->ppid == -1) {
            print_str_with_padding("-", 11);
            print_str_with_padding("-", 16);
        } else {
            print_int_with_padding(p->ppid, 11);
            int i = p - plist;
            print_str_with_padding(&pnames[i * 16], 16);
        }
        printf("\n");
    }

    free(pnames);
    free(plist);
    exit(0);
}