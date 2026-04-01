#include "kernel/types.h"
#include "user/user.h"
#include "kernel/fcntl.h"

int
main()
{
    int fd;
    unsigned char buf[16];
    int n;
    
    fd = open("null", O_RDONLY);
    printf("Test read null: ");
    if (fd >= 0) {
        n = read(fd, buf, 4);
        if (n != 0) {
            printf("FAIL\n");
        } else {
            printf("OK\n");
        }
        close(fd);
    } else {
        printf("Failed to open null\n");
    }
    
    fd = open("zero", O_RDONLY);
    printf("Test read zero: ");
    if (fd >= 0) {
        n = read(fd, buf, 2);
        if (buf[0] == 0 && buf[1] == 0 && n == 2) {
            printf("OK\n");
        } else {
            printf("FAIL\n");
        }
        close(fd);
    } else {
        printf("Failed to open zero\n");
    }
    
    fd = open("urandom", O_RDONLY);
    printf("Test read urandom: ");
    if (fd >= 0) {
        n = read(fd, buf, 8);
        if (n == 8) {
            printf("OK\n");
        } else {
            printf("FAIL\n");
        }
        close(fd);
    } else {
        printf("Failed to open urandom\n");
    }
    
    fd = open("nullstat", O_RDONLY);
    printf("Test read nullstat: ");
    if (fd >= 0) {
        uint64 count;
        n = read(fd, &count, sizeof(count));
        if (n == sizeof(count) && count == 0) {
            printf("OK\n");
        } else {
            printf("FAIL\n");
        }
        close(fd);
    } else {
        printf("Failed to open nullstat\n");
    }
    
    fd = open("null", O_WRONLY);
    printf("Test write null: ");
    if (fd >= 0) {
        n = write(fd, "test", 4);
        if (n == 4) {
            printf("OK\n");
        } else {
            printf("FAIL\n");
        }
        close(fd);
    } else {
        printf("Failed to open null\n");
    }
    
    fd = open("zero", O_WRONLY);
    printf("Test write zero: ");
    if (fd >= 0) {
        n = write(fd, "test", 4);
        if (n < 0) {
            printf("OK\n");
        } else {
            printf("FAIL\n");
        }
        close(fd);
    } else {
        printf("Failed to open zero\n");
    }
    
    fd = open("urandom", O_RDWR);
    printf("Test write urandom 1: ");
    if (fd >= 0) {
        uint64 seed = 0x12345678;
        n = write(fd, (char*)&seed, 4);
        if (n < 0) {
            printf("OK\n");
        } else {
            printf("FAIL\n");
        }
        printf("Test write urandom 2: ");
        n = write(fd, (char*)&seed, 8);
        if (n == 8) {
            printf("OK\n");
        } else {
            printf("FAIL\n");
        }
        close(fd);
    } else {
        printf("Failed to open urandom\n");
    }
    
    fd = open("nullstat", O_RDWR);
    printf("Test write nullstat: ");
    if (fd >= 0) {
        write(fd, "test", 4);  
        write(fd, "hello", 5);
        uint64 count;
        if (read(fd, &count, sizeof(count)) == sizeof(count) && count == 9) {
            printf("OK\n");
        } else {
            printf("FAIL\n");
        }
        close(fd);
    } else {
        printf("Failed to open nullstat\n");
    }
    
    printf("\nTest completed\n");
    exit(0);
}