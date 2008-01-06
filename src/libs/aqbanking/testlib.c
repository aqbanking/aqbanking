#include "banking.h"
#include "types/value.h"


void dumpNumDenom(const char *t, const AB_VALUE *v) {
  char numbuf[256];

  if (!AB_Value_GetNumDenomString(v, numbuf, sizeof(numbuf))) {
    fprintf(stderr, "%s (num/den): %s\n", t, numbuf);
  }
  else {
    fprintf(stderr, "%s (num/den): Error\n", t);
  }
}


int test1(int argc, char **argv) {
  AB_VALUE *v1;
  AB_VALUE *v2;
  AB_VALUE *v3;
  AB_VALUE *v4;
  int rv;

  v1=AB_Value_fromString("987654321.12345");
  if (v1==NULL) {
    fprintf(stderr, "ERROR: v1\n");
    return 1;
  }
  fprintf(stderr, "v1          : %f\n",
	  AB_Value_GetValueAsDouble(v1));
  dumpNumDenom("v1", v1);

  v2=AB_Value_fromString("10/1");
  if (v2==NULL) {
    fprintf(stderr, "ERROR: v2\n");
    return 1;
  }
  fprintf(stderr, "v2          : %f\n",
	  AB_Value_GetValueAsDouble(v2));
  dumpNumDenom("v2", v2);

  v3=AB_Value_dup(v1);
  if (v3==NULL) {
    fprintf(stderr, "ERROR: v3\n");
    return 1;
  }
  fprintf(stderr, "v3          : %f\n",
	  AB_Value_GetValueAsDouble(v3));
  dumpNumDenom("v3", v3);

  v4=AB_Value_fromString("-1250,");
  if (v4==NULL) {
    fprintf(stderr, "ERROR: v4\n");
    return 1;
  }
  fprintf(stderr, "v4          : %f\n",
	  AB_Value_GetValueAsDouble(v4));
  dumpNumDenom("v4", v4);

  rv=AB_Value_MultValue(v3, v2);
  if (rv) {
    fprintf(stderr, "ERROR: v3*v2\n");
    return 1;
  }

  fprintf(stderr, "r           : %f\n",
	  AB_Value_GetValueAsDouble(v3));
  dumpNumDenom("r ", v3);

  if (AB_Value_GetValueAsDouble(v3)!=9876543211.2345) {
    fprintf(stderr, "Bad result : %f\n",
	    AB_Value_GetValueAsDouble(v3));
    return 1;
  }
  dumpNumDenom("v3", v3);

  fprintf(stderr, "Ok.\n");
  return 0;
}



int main(int argc, char *argv[]){
#if 1
  return test1(argc, argv);
#else
  AB_BANKING *ab;

  ab=AB_Banking_new("testlib", "testname",
		    AB_BANKING_EXTENSION_NONE);

  AB_Banking_free(ab);

  return 0;
#endif
}
