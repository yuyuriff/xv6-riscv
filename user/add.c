#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

// int32 from -2,147,483,648 to 2,147,483,647 so 22 chars + whitespace + /0 is 24 chars
#define max_size 32
char buf[max_size];

void
custom_gets()
{
  int i, return_val;
  char c;

  for (i = 0; ; ) {
    return_val = read(0, &c, 1);

    if (return_val < 1) {
        if (return_val != 0) {
          fprintf(2, "Read error\n");
          exit(1);
        }
        break;
    }

    if(c == '\n' || c == '\r') {
      break;
    }

    if (i + 1 < max_size) {
      buf[i] = c;
    }
    i++;
  }

  if (i + 1 >= max_size) {
    fprintf(2, "Buffer overflow\n");
    exit(1);
  }

  buf[i] = '\0';
}

int
main(int argc, char *argv[])
{
  custom_gets();
  printf("|%s|\n", buf);

  int a, b;
  parse_ints(buf, &a, &b);

  long long sum = (long long)a + b;

  printf("%lld\n", sum);

  exit(0);
}