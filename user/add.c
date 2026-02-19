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
get_sign(char* a)
{
  char* c = a;

  int minus = (*c == '-') ? 1 : 0;
  if (minus) {
    c++;
    if (*c == '\0') {
      return -1;
    }
  } else if (*c == '+') {
    c++;
    if (*c == '\0') {
      return -1;
    }
  }

  while (*c != '\0') {
    if (*c < '0' || *c > '9') {
      return -1;
    }
    c++;
  }

  return minus;
}

void
parse_ints(int* a, int* b)
{
  char* c = buf;
  while (*c == ' ') {
    c++;
  }

  if (*c == '\0') {
    fprintf(2, "Empty or whitespace input\n");
    exit(1);
  }

  char* a_start = c;

  while (*c != ' ' && *c != '\0') {
    c++;
  }
                                  
  if (*c == '\0') {
    fprintf(2, "Not enough numbers entered. Must be two separated by whitespace\n");
    exit(1);
  }

  char* a_end = c;
  
  while (*c == ' ') {
    c++;
  }

  if (*c == '\0') {
    fprintf(2, "Not enough numbers entered. Must be two separated by whitespace\n");
    exit(1);
  }

  char* b_start = c;

  *a_end = '\0';

  while (*c != ' ' && *c != '\0') {
    c++;
  }

  char* b_end = c;

  while (*c != '\0') {
    if (*c != ' ') {
      fprintf(2, "Too many numbers or tokens given\n");
      exit(1);
    }
    c++;
  }

  *b_end = '\0';

  int sign_a = get_sign(a_start);
  int sign_b = get_sign(b_start);

  if (sign_a == -1 || sign_b == -1) {
    fprintf(2, "Incorrect input\n");
    exit(1);
  }

  if (sign_a) {
    *a = -(atoi(a_start + 1));
  } else {
    if (*a_start == '+') {
      a_start++;
    }
    *a = atoi(a_start);
  }

  if (sign_b) {
    *b = -(atoi(b_start + 1));
  } else {
    if (*b_start == '+') {
      b_start++;
    }
    *b = atoi(b_start);
  }
}

int
main(int argc, char *argv[])
{
  custom_gets();
  printf("|%s|\n", buf);

  int a, b;
  parse_ints(&a, &b);

  long long sum = (long long)a + b;

  printf("%lld\n", sum);
}