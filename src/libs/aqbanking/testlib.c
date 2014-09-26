
#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include "banking.h"
#include "types/value.h"

#include <gwenhywfar/gwenhywfar.h>
#include <gwenhywfar/cgui.h>



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


int test2(int argc, char **argv) {
  int rv;
  AB_BANKING *ab;
  GWEN_GUI *gui;

  rv=GWEN_Init();
  if (rv) {
    fprintf(stderr, "ERROR: Unable to init Gwen.\n");
    exit(2);
  }

  gui=GWEN_Gui_CGui_new();
  GWEN_Gui_SetGui(gui);

  ab=AB_Banking_new("testlib", NULL, 0);

  rv=AB_Banking_HasConf4(ab);
  if (!rv) {
    fprintf(stderr, "Config for AqBanking 4 found\n");
    return 0;
  }
  fprintf(stderr, "Config for AqBanking 4 not found (%d)\n", rv);

  rv=AB_Banking_HasConf3(ab);
  if (!rv) {
    fprintf(stderr, "Config for AqBanking 3 found\n");
    return 0;
  }
  fprintf(stderr, "Config for AqBanking 3 not found (%d)\n", rv);

  rv=AB_Banking_HasConf2(ab);
  if (!rv) {
    fprintf(stderr, "Config for AqBanking 2 found\n");
    return 0;
  }
  fprintf(stderr, "Config for AqBanking 2 not found (%d)\n", rv);

  return 0;
}



int test3(int argc, char **argv) {
  int rv;
  AB_BANKING *ab;
  GWEN_GUI *gui;

  rv=GWEN_Init();
  if (rv) {
    fprintf(stderr, "ERROR: Unable to init Gwen.\n");
    exit(2);
  }

  gui=GWEN_Gui_CGui_new();
  GWEN_Gui_SetGui(gui);

  ab=AB_Banking_new("testlib", NULL, 0);

  rv=AB_Banking_HasConf4(ab);
  if (!rv) {
    fprintf(stderr, "Config for AqBanking 4 found\n");
    return 0;
  }
  fprintf(stderr, "Config for AqBanking 4 not found (%d)\n", rv);

  rv=AB_Banking_HasConf3(ab);
  if (!rv) {
    fprintf(stderr, "Config for AqBanking 3 found, importing\n");
    rv=AB_Banking_ImportConf3(ab);
    if (rv<0) {
      fprintf(stderr, "Error importing configuration (%d)\n", rv);
      return 2;
    }
    return 0;
  }
  fprintf(stderr, "Config for AqBanking 3 not found (%d)\n", rv);

  rv=AB_Banking_HasConf2(ab);
  if (!rv) {
    fprintf(stderr, "Config for AqBanking 2 found, importing\n");
    rv=AB_Banking_ImportConf3(ab);
    if (rv<0) {
      fprintf(stderr, "Error importing configuration (%d)\n", rv);
      return 2;
    }
    return 0;
  }
  fprintf(stderr, "Config for AqBanking 2 not found (%d)\n", rv);

  return 0;
}



int test4(int argc, char **argv) {
  int rv;
//  AB_BANKING *ab;
  GWEN_GUI *gui;
  GWEN_BUFFER *tbuf;

  rv=GWEN_Init();
  if (rv) {
    fprintf(stderr, "ERROR: Unable to init Gwen.\n");
    exit(2);
  }

  gui=GWEN_Gui_CGui_new();
  GWEN_Gui_SetGui(gui);

//  ab=AB_Banking_new("testlib", NULL, 0);

  if (argc<3) {
    fprintf(stderr, "Missing bank code and account number\n");
    return 1;
  }

  tbuf=GWEN_Buffer_new(0, 256, 0, 1);
  rv=AB_Banking_MakeGermanIban(argv[1], argv[2], tbuf);
  if (rv<0) {
    fprintf(stderr, "ERROR: %d\n", rv);
    return 2;
  }

  rv=AB_Banking_CheckIban(GWEN_Buffer_GetStart(tbuf));
  if (rv != 0) {
    fprintf(stderr, "Bad IBAN (%s)\n", GWEN_Buffer_GetStart(tbuf));
    return 2;
  }
  fprintf(stderr, "Verified IBAN: %s\n", GWEN_Buffer_GetStart(tbuf));


  return 0;
}



int test5(int argc, char **argv) {
  AB_VALUE *v1;
  GWEN_BUFFER *tbuf;

  tbuf=GWEN_Buffer_new(0, 256, 0, 1);
  v1=AB_Value_fromString("11,90");
  if (v1==NULL) {
    fprintf(stderr, "ERROR: v1\n");
    return 1;
  }
  AB_Value_toHbciString(v1, tbuf);
  if (strcmp(GWEN_Buffer_GetStart(tbuf), "11,9")!=0) {
    fprintf(stderr, "ERROR: Bad HBCI string (%s)\n", GWEN_Buffer_GetStart(tbuf));
    return 2;
  }
  GWEN_Buffer_free(tbuf);
  AB_Value_free(v1);


  tbuf=GWEN_Buffer_new(0, 256, 0, 1);
  v1=AB_Value_fromString("11,91");
  if (v1==NULL) {
    fprintf(stderr, "ERROR: v1\n");
    return 1;
  }
  AB_Value_toHbciString(v1, tbuf);
  if (strcmp(GWEN_Buffer_GetStart(tbuf), "11,91")!=0) {
    fprintf(stderr, "ERROR: Bad HBCI string (%s)\n", GWEN_Buffer_GetStart(tbuf));
    return 2;
  }
  GWEN_Buffer_free(tbuf);
  AB_Value_free(v1);


  tbuf=GWEN_Buffer_new(0, 256, 0, 1);
  v1=AB_Value_fromString("1190");
  if (v1==NULL) {
    fprintf(stderr, "ERROR: v1\n");
    return 1;
  }
  AB_Value_toHbciString(v1, tbuf);
  if (strcmp(GWEN_Buffer_GetStart(tbuf), "1190,")!=0) {
    fprintf(stderr, "ERROR: Bad HBCI string (%s)\n", GWEN_Buffer_GetStart(tbuf));
    return 2;
  }
  GWEN_Buffer_free(tbuf);
  AB_Value_free(v1);


  tbuf=GWEN_Buffer_new(0, 256, 0, 1);
  v1=AB_Value_fromString("11,00");
  if (v1==NULL) {
    fprintf(stderr, "ERROR: v1\n");
    return 1;
  }
  AB_Value_toHbciString(v1, tbuf);
  if (strcmp(GWEN_Buffer_GetStart(tbuf), "11,")!=0) {
    fprintf(stderr, "ERROR: Bad HBCI string (%s)\n", GWEN_Buffer_GetStart(tbuf));
    return 2;
  }
  GWEN_Buffer_free(tbuf);
  AB_Value_free(v1);



  fprintf(stderr, "Ok.\n");
  return 0;
}



int main(int argc, char *argv[]){
#if 1
  return test5(argc, argv);
#else
  AB_BANKING *ab;

  ab=AB_Banking_new("testlib", "testname",
		    AB_BANKING_EXTENSION_NONE);

  AB_Banking_free(ab);

  return 0;
#endif
}
