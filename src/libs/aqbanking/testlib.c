#include "banking.h"
#include "types/value.h"


int test1(int argc, char **argv) {
  AB_VALUE *v1;
  AB_VALUE *v2;
  AB_VALUE *v3;
  int rv;

  v1=AB_Value_fromString("99999266603139355");
  if (v1==NULL) {
    fprintf(stderr, "ERROR: v1\n");
    return 1;
  }
  fprintf(stderr, "v1: %f\n",
	  AB_Value_GetValueAsDouble(v1));
  v2=AB_Value_fromString("10/1");
  if (v2==NULL) {
    fprintf(stderr, "ERROR: v2\n");
    return 1;
  }
  fprintf(stderr, "v2: %f\n",
	  AB_Value_GetValueAsDouble(v2));
  v3=AB_Value_dup(v1);
  if (v3==NULL) {
    fprintf(stderr, "ERROR: v3\n");
    return 1;
  }
  fprintf(stderr, "v3: %f\n",
	  AB_Value_GetValueAsDouble(v3));
  rv=AB_Value_MultValue(v3, v2);
  if (rv) {
    fprintf(stderr, "ERROR: v3*v2\n");
    return 1;
  }
  if (AB_Value_GetValueAsDouble(v3)!=12345.0) {
    fprintf(stderr, "Bad result: %f\n",
	    AB_Value_GetValueAsDouble(v3));
    return 1;
  }

  fprintf(stderr, "Ok.\n");
  return 0;
}



int main(int argc, char *argv[]){
#if 0
  return test1(argc, argv);
#else
  AB_BANKING *ab;

  ab=AB_Banking_new("testlib", "testname",
		    AB_BANKING_EXTENSION_NONE);

  AB_Banking_free(ab);

  return 0;
#endif
}
