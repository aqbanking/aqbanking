#include "cbanking.h"

int main(int argc, char *argv[])
{
  AB_BANKING *testobject = CBanking_new("testlib", "testname");

  if (argc > 1) {
    char buffer[50];
    int rv;

    rv = AB_Banking_ShowBox(testobject, 0,
			    "This is a ShowBox test title",
			    "This is a ShowBox test.");
    printf("AB_Banking_ShowBox: rv=%d\n", rv);
    AB_Banking_HideBox(testobject, rv);
    printf("AB_Banking_HideBox called.\n\n");

    rv = AB_Banking_InputBox(testobject, 0,
				 "This is a InputBox test title",
				 "Just enter something.",
				 buffer,
				 1, 40);
    printf("AB_Banking_InputBox: rv=%d, result=\"%s\"\n\n",
	   rv, buffer);

    rv = AB_Banking_MessageBox(testobject, 0,
			       "Third test title, this time MessageBox",
			       "Just press the first or second button.",
			       "First button.", "Second button", NULL);
    printf("AB_Banking_MessageBox: rv=%d; button=%s\n", rv,
	   (rv == 1 ? "first" : (rv == 2 ? "second" : "unknown")));
  } else
    printf("For interaction test, run this:   ./testlib 1\n");

  AB_Banking_free(testobject);

  return 0;
}
