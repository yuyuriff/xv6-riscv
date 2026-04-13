#include "kernel/types.h"
#include "user/user.h"
#include "kernel/fcntl.h"
#include "kernel/stat.h"

#define BUF_SIZE 32

int 
hexdump(int fd, int n) {
    unsigned char buf[BUF_SIZE];
    int size = BUF_SIZE;
    int read_size;
    int target = n;
    
    while (target > 0) {
        int expected_read_size = (target < size) ? target : size;
        read_size = read(fd, buf, expected_read_size);

        if (read_size < 0) {
            return -1;
        }

        if (read_size == 0) {
            printf("EOF\n");
            return 0;
        }
        
        for (int i = 0; i < read_size; i++) {
            char c = buf[i];
            if (!(c & (15 << 4))) {
                printf("0");
            }
            printf("%x ", c);
        }
        
        target -= read_size;
    }
    
    printf("\n");
    return 0;
}

int
main(int argc, char *argv[]) {    
    if (argc != 3) {
        fprintf(2, "Hexdump needs 2 args\n");
        exit(1);
    }

    int fd;
    int n;

    n = atoi(argv[1]);
    if (n <= 0) {
        fprintf(2, "Requires positive number of bytes\n");
        exit(1);
    }
    
    char *f = argv[2];
    struct stat st;
    
    if (stat(f, &st) < 0) {
        fprintf(2, "No such file\n");
        exit(1);
    }

    fd = open(f, O_RDONLY);
    if (fd < 0) {
        fprintf(2, "Cannot open file\n");
        exit(1);
    }

    int code = hexdump(fd, n);
    if (code < 0) {
        fprintf(2, "Read error");
        exit(1);
    }

    close(fd);
    exit(0);
}