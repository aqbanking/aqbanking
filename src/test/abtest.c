



#include <gwenhywfar/logger.h>
#include <aqbanking/banking.h>
#include <stdio.h>
#include <assert.h>
#include <string.h>



int test1(int argc, char **argv) {
  AB_BANKING *ab;
  int rv;

  fprintf(stderr, "Creating AB_Banking...\n");
  ab=AB_Banking_new("abtest", "./aqbanking.conf");

  fprintf(stderr, "Initializing AB_Banking...\n");
  rv=AB_Banking_Init(ab);
  if (rv) {
    fprintf(stderr, "Could not init AqBanking (%d)\n", rv);
    return 2;
  }

  fprintf(stderr, "Deinitializing AB_Banking...\n");
  rv=AB_Banking_Fini(ab);
  if (rv) {
    fprintf(stderr, "Could not deinit AqBanking (%d)\n", rv);
    return 2;
  }

  fprintf(stderr, "Freeing AB_Banking...\n");
  AB_Banking_free(ab);

  fprintf(stderr, "Finished\n");
  return 0;
}


int test2(int argc, char **argv) {
  AB_BANKING *ab;
  int rv;
  GWEN_PLUGIN_DESCRIPTION_LIST2 *pdl;
  GWEN_PLUGIN_DESCRIPTION_LIST2_ITERATOR *pit;
  GWEN_PLUGIN_DESCRIPTION *pd;

  fprintf(stderr, "Creating AB_Banking...\n");
  ab=AB_Banking_new("abtest", "./aqbanking.conf");

  fprintf(stderr, "Initializing AB_Banking...\n");
  rv=AB_Banking_Init(ab);
  if (rv) {
    fprintf(stderr, "Could not init AqBanking (%d)\n", rv);
    return 2;
  }

  pdl=AB_Banking_GetProviderDescrs(ab);
  if (!pdl) {
    fprintf(stderr, "No providers...\n");
    return 2;
  }

  pit=GWEN_PluginDescription_List2_First(pdl);
  assert(pit);
  pd=GWEN_PluginDescription_List2Iterator_Data(pit);
  assert(pd);
  while(pd) {
    fprintf(stderr, "Backend:\n");
    fprintf(stderr, "Name        : %s (%s)\n",
            GWEN_PluginDescription_GetName(pd),
            GWEN_PluginDescription_GetVersion(pd));
    fprintf(stderr, "Author      : %s\n",
            GWEN_PluginDescription_GetAuthor(pd));
    fprintf(stderr, "Short Descr.: %s\n",
            GWEN_PluginDescription_GetShortDescr(pd));
    pd=GWEN_PluginDescription_List2Iterator_Next(pit);
  } /* while */

  fprintf(stderr, "Deinitializing AB_Banking...\n");
  rv=AB_Banking_Fini(ab);
  if (rv) {
    fprintf(stderr, "Could not deinit AqBanking (%d)\n", rv);
    return 2;
  }

  fprintf(stderr, "Freeing AB_Banking...\n");
  AB_Banking_free(ab);

  fprintf(stderr, "Finished\n");
  return 0;
}



int test3(int argc, char **argv) {
  AB_BANKING *ab;
  int rv;
  AB_PROVIDER *pro;
  AB_ACCOUNT_LIST2 *al;
  AB_ACCOUNT_LIST2_ITERATOR *ait;
  AB_ACCOUNT *a;

  fprintf(stderr, "Creating AB_Banking...\n");
  ab=AB_Banking_new("abtest", "./aqbanking.conf");

  fprintf(stderr, "Initializing AB_Banking...\n");
  rv=AB_Banking_Init(ab);
  if (rv) {
    fprintf(stderr, "Could not init AqBanking (%d)\n", rv);
    return 2;
  }

  fprintf(stderr, "Loading provider...\n");
  pro=AB_Banking_GetProvider(ab, "aqhbci");
  if (!pro) {
    fprintf(stderr, "Could not load provider.\n");
    return 2;
  }
  else {
    fprintf(stderr, "Provider loaded.\n");
  }

  al=AB_Provider_GetAccountList(pro);
  if (!al) {
    fprintf(stderr, "No accounts.\n");
    return 2;
  }

  ait=AB_Account_List2_First(al);
  assert(ait);
  a=AB_Account_List2Iterator_Data(ait);
  assert(a);
  while(a) {
    fprintf(stderr, "Account:\n");
    fprintf(stderr, "Bank code     : %s (%s)\n",
            AB_Account_GetBankCode(a),
            AB_Account_GetBankName(a));
    fprintf(stderr, "Account number: %s (%s)\n",
            AB_Account_GetAccountNumber(a),
            AB_Account_GetAccountName(a));
    fprintf(stderr, "Account owner : %s\n",
            AB_Account_GetOwnerName(a));
    a=AB_Account_List2Iterator_Next(ait);
  } /* while */

  fprintf(stderr, "Deinitializing AB_Banking...\n");
  rv=AB_Banking_Fini(ab);
  if (rv) {
    fprintf(stderr, "Could not deinit AqBanking (%d)\n", rv);
    return 2;
  }

  fprintf(stderr, "Freeing AB_Banking...\n");
  AB_Banking_free(ab);

  fprintf(stderr, "Finished\n");
  return 0;
}



int test4(int argc, char **argv) {
  AB_BANKING *ab;
  int rv;

  fprintf(stderr, "Creating AB_Banking...\n");
  ab=AB_Banking_new("abtest", "./aqbanking.conf");

  fprintf(stderr, "Initializing AB_Banking...\n");
  rv=AB_Banking_Init(ab);
  if (rv) {
    fprintf(stderr, "Could not init AqBanking (%d)\n", rv);
    return 2;
  }


  fprintf(stderr, "Importing accounts...\n");
  rv=AB_Banking_ImportProviderAccounts(ab, "aqhbci");
  if (rv) {
    fprintf(stderr, "Could not import accounts\n");
    return 2;
  }
  fprintf(stderr, "Accounts imported\n");

  fprintf(stderr, "Deinitializing AB_Banking...\n");
  rv=AB_Banking_Fini(ab);
  if (rv) {
    fprintf(stderr, "Could not deinit AqBanking (%d)\n", rv);
    return 2;
  }

  fprintf(stderr, "Freeing AB_Banking...\n");
  AB_Banking_free(ab);

  fprintf(stderr, "Finished\n");
  return 0;
}



int main(int argc, char **argv) {
  const char *cmd;
  int rv;

  if (argc<2) {
    fprintf(stderr, "Usage: %s COMMAND\n", argv[0]);
    return 1;
  }

  cmd=argv[1];

  GWEN_Logger_SetLevel(0, GWEN_LoggerLevelInfo);

  if (strcasecmp(cmd, "test1")==0)
    rv=test1(argc, argv);
  else if (strcasecmp(cmd, "test2")==0)
    rv=test2(argc, argv);
  else if (strcasecmp(cmd, "test3")==0)
    rv=test3(argc, argv);
  else if (strcasecmp(cmd, "test4")==0)
    rv=test4(argc, argv);
  else {
    fprintf(stderr, "Unknown command \"%s\"", cmd);
    rv=1;
  }

  return rv;
}

