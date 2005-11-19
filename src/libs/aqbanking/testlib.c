#include "banking.h"

int main(int argc, char *argv[])
{
  AB_BANKING *testobject = AB_Banking_new("testlib", "testname");
  AB_Banking_free(testobject);

  return 0;
}
