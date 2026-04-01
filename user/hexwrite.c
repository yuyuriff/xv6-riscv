#include "kernel/types.h"
#include "user/user.h"
#include "kernel/fcntl.h"

#define BUF_SIZE 64

int
write_buf(int fd, const char* buf, int len) {
    int i = 0;
    while (i < len) {
        int written = write(fd, buf + i, len - i);
        if (written <= 0) {
            return -1;
        }
        i += written;
    }
    return 0;
}

int
flush_buf(int fd, const char* buf, int* cur) {
    if (*cur > 0) {
        if (write_buf(fd, buf, *cur) < 0) {
            return -1;
        }
        *cur = 0;
    }
    return 0;
}

int
add_to_buf(int fd, char *buf, int *cur, const char *str, int len) {
  int i = 0;
  while (i < len) {
    if (*cur == BUF_SIZE) {
      if (flush_buf(fd, buf, cur) < 0) {
        return -1;
      }
    }

    int available = BUF_SIZE - *cur;
    int chunk_size = (len - i > available) ? available : len - i;
    memmove(buf + *cur, str + i, chunk_size);

    *cur += chunk_size;
    i += chunk_size;
  }

  return 0;
}

int
hex_to_int(char c) {
    if (c >= '0' && c <= '9')
    return c - '0';
    if (c >= 'A' && c <= 'F')
        return c - 'A' + 10;
    return -1;
}

int 
hexwrite(int fd, char *str) {
    char bytes[BUF_SIZE];
    int cur = 0;
    int len = strlen(str);

    if (len % 2) {
        return -3;
    }

    for (int i = 0; i < len; i += 2) {
        int fst = hex_to_int(str[i]);
        int snd = hex_to_int(str[i + 1]);

        if (fst == -1 || snd == -1) {
            return -2;
        }

        uint8 n = (fst << 4) | snd;

        if (add_to_buf(fd, bytes, &cur, (char*)&n, 1)  < 0) {
            return -1;
        }
    }

    if (flush_buf(fd, bytes, &cur) < 0) {
        return -1;
    }

    return 0;
}

int
main(int argc, char *argv[]) {    
    if (argc != 3) {
        fprintf(2, "Hexwrite needs 2 args\n");
        exit(1);
    }

    int fd;
    char *str;

    str = argv[1];
    
    if (strcmp(argv[2], "null") == 0) {
        if ((fd = open("null", O_WRONLY)) < 0) {
            fprintf(2, "Cannot open null\n");
            exit(1);
        }
    }
    else if (strcmp(argv[2], "urandom") == 0) {
        if ((fd = open("urandom", O_WRONLY)) < 0) {
            fprintf(2, "Cannot open urandom\n");
            exit(1);
        }
    } else if (strcmp(argv[2], "zero") == 0) {
        if ((fd = open("zero", O_WRONLY)) < 0) {
            fprintf(2, "Cannot open zero\n");
            exit(1);
        }
    } else if (strcmp(argv[2], "nullstat") == 0) {
        if ((fd = open("nullstat", O_WRONLY)) < 0) {
            fprintf(2, "Cannot open nullstat\n");
            exit(1);
        }
    } else {
        fprintf(2, "Unknown device");
        exit(1);
    }
    
    int code = hexwrite(fd, str);
    if (code == -1) {
        fprintf(2, "Write error\n");
        exit(1);
    } else if (code == -2) {
        fprintf(2, "Invalid hex number\n");
        exit(1);
    } else if (code == -3) {
        fprintf(2, "Hex string must have even length\n");
        exit(1);
    }
    close(fd);
    exit(0);
}