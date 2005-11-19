#include "cbanking.h"

int main(int argc, char *argv[])
{
  AB_BANKING *testobject = CBanking_new("testlib", "testname");
  AB_Banking_free(testobject);

  return 0;
}
