#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

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

int
atoi_with_sign(char* x_start, int sign_x) 
{
  if (sign_x) {
    return -(atoi(x_start + 1));
  } else {
    if (*x_start == '+') {
      x_start++;
    }
    return atoi(x_start);
  }
}

void
parse_ints(char* buf, int* a, int* b)
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

  *a = atoi_with_sign(a_start, sign_a);
  *b = atoi_with_sign(b_start, sign_b);
}
