#include "gbanking.h"

int main(int argc, char *argv[])
{
  AB_BANKING *testobject = GBanking_new("testlib", "testname");
  AB_Banking_free(testobject);

  return 0;
}
