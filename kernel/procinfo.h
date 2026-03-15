typedef struct procinfo {
    int pid;
    int state;
    char name[16];
    int ppid;       // -1 if no parent
} procinfo_s;
