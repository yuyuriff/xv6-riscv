#include "kernel/types.h"
#include "user/user.h"

int
main(int argc, char *argv[])
{
  if (ps_listinfo(0, 123) <= 0) {
    fprintf(2, "TEST plist null FAILED\n");
    exit(1);
  }

  procinfo_s *pi_table = (procinfo_s *)malloc(10 * sizeof(procinfo_s));
  if (ps_listinfo(pi_table, 1) != -1) {
    fprintf(2, "TEST small buf FAILED\n");
    exit(1);
  }
  if (ps_listinfo(pi_table, 0) != -1) {
    fprintf(2, "TEST 0 buf FAILED\n");
    exit(1);
  }

  int n = ps_listinfo(pi_table, 10);
  if (n < 0 || n > 10){
    fprintf(2, "TEST ok FAILED\n");
    exit(1);
  }

  if (ps_listinfo((procinfo_s *)1, 10) != -2){
    fprintf(2, "TEST bad addr FAILED\n");
    exit(1);
  }

  free(pi_table);

  fprintf(1, "ALL TESTS SUCCESSFUL\n");

  exit(0);
}
