



#include <gwenhywfar/logger.h>
#include <gwenhywfar/db.h>
#include <gwenhywfar/debug.h>
#include <aqbanking/banking.h>
#include <aqbanking/banking_be.h>
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



int test4(int argc, char **argv) {
  AB_BANKING *ab;
  int rv;
  GWEN_BUFFER *pbuf;

  fprintf(stderr, "Creating AB_Banking...\n");
  ab=AB_Banking_new("abtest", "./aqbanking.conf");

  fprintf(stderr, "Initializing AB_Banking...\n");
  rv=AB_Banking_Init(ab);
  if (rv) {
    fprintf(stderr, "Could not init AqBanking (%d)\n", rv);
    return 2;
  }

  pbuf=GWEN_Buffer_new(0, 256, 0, 1);
  if (AB_Banking_FindDebugger(ab, "aqhbci", "kde;qt;gnome;gtk", pbuf)) {
    fprintf(stderr, "Debugger not found.\n");
    return 2;
  }
  fprintf(stderr, "Debugger found: %s\n",
          GWEN_Buffer_GetStart(pbuf));
  GWEN_Buffer_free(pbuf);

  pbuf=GWEN_Buffer_new(0, 256, 0, 1);
  if (AB_Banking_FindWizard(ab, "aqhbci", "kde;qt;gnome;gtk", pbuf)) {
    fprintf(stderr, "Wizard not found.\n");
    return 2;
  }
  fprintf(stderr, "Wizard found: %s\n",
          GWEN_Buffer_GetStart(pbuf));
  GWEN_Buffer_free(pbuf);

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



int test5(int argc, char **argv) {
  int rv;
  GWEN_DB_NODE *db;
  GWEN_DB_NODE *dbParams;

  db=GWEN_DB_Group_new("test");
  dbParams=GWEN_DB_Group_new("params");
  GWEN_DB_SetCharValue(dbParams, GWEN_DB_FLAGS_DEFAULT,
                       "params/type", "mt940");
  rv=GWEN_DB_ReadFileAs(db, "test.swift", "swift", dbParams,
			GWEN_PATH_FLAGS_CREATE_GROUP);
  if (rv) {
    DBG_ERROR(0, "Error reading file");
    return 2;
  }
  GWEN_DB_Dump(db, stderr, 2);

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

  GWEN_Logger_SetLevel(AQBANKING_LOGDOMAIN, GWEN_LoggerLevelInfo);

  if (strcasecmp(cmd, "test1")==0)
    rv=test1(argc, argv);
  else if (strcasecmp(cmd, "test2")==0)
    rv=test2(argc, argv);
  else if (strcasecmp(cmd, "test3")==0)
    rv=test3(argc, argv);
  else if (strcasecmp(cmd, "test4")==0)
    rv=test4(argc, argv);
  else if (strcasecmp(cmd, "test5")==0)
    rv=test5(argc, argv);
  else {
    fprintf(stderr, "Unknown command \"%s\"", cmd);
    rv=1;
  }

  return rv;
}

