/***************************************************************************
    begin       : Mon Mar 01 2004
    copyright   : (C) 2004 by Martin Preuss
    email       : martin@libchipcard.de

 ***************************************************************************
 *          Please see toplevel file COPYING for license details           *
 ***************************************************************************/

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif


#include "de_p.h"
#include "../generic/generic_l.h"

#include <aqbanking/banking.h>
#include <aqbanking/banking_be.h>
#include <gwenhywfar/debug.h>
#include <gwenhywfar/misc.h>
#include <gwenhywfar/inherit.h>
#include <gwenhywfar/text.h>
#include <gwenhywfar/directory.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include <errno.h>

#ifdef OS_WIN32
# define DIRSEP "\\"
#else
# define DIRSEP "/"
#endif



GWEN_INHERIT(AB_BANKINFO_PLUGIN, AB_BANKINFO_PLUGIN_DE);

/* interface to gwens plugin loader */
GWEN_PLUGIN *bankinfo_de_factory(GWEN_PLUGIN_MANAGER *pm,
                                 const char *name,
                                 const char *fileName) {
  GWEN_PLUGIN *pl;

  pl=AB_Plugin_BankInfo_new(pm, name, fileName);
  assert(pl);

  AB_Plugin_BankInfo_SetFactoryFn(pl, AB_Plugin_BankInfoDE_Factory);

  return pl;
}



/* interface to bankinfo plugin */
AB_BANKINFO_PLUGIN *AB_Plugin_BankInfoDE_Factory(GWEN_PLUGIN *pl, AB_BANKING *ab){
  AB_BANKINFO_PLUGIN *bip;
  AB_BANKINFO_PLUGIN_DE *bde;
#ifdef HAVE_KTOBLZCHECK
  const char *s;
  GWEN_STRINGLIST *paths;
#endif

  bip=AB_BankInfoPluginGENERIC_new(ab, "de");
  GWEN_NEW_OBJECT(AB_BANKINFO_PLUGIN_DE, bde);
  GWEN_INHERIT_SETDATA(AB_BANKINFO_PLUGIN, AB_BANKINFO_PLUGIN_DE,
                       bip, bde, AB_BankInfoPluginDE_FreeData);

  bde->banking=ab;
#ifdef HAVE_KTOBLZCHECK
  s=AccountNumberCheck_libraryVersion();
  if (s && GWEN_Text_ComparePattern(s, "1.8*", 0)!=-1) {
    DBG_WARN(AQBANKING_LOGDOMAIN,
             "WARNING:\n"
             "Bad version of KtoBlzCheck detected, "
             "please upgrade to 1.9.x or better.\n"
             " There is at least one known version of the 1.8 branch\n"
             " which corrupts the heap.\n");
  }

  /* try to find the data file */
  paths=AB_Banking_GetGlobalDataDirs();
  if (paths) {
    GWEN_BUFFER *fbuf;
    int rv;

    /* for debian look also in /var/lib */
    GWEN_StringList_AppendString(paths, "/var/lib", 0, 0);

    fbuf=GWEN_Buffer_new(0, 256, 0, 1);
    rv=GWEN_Directory_FindFileInPaths(paths,
				      "ktoblzcheck"
				      DIRSEP
				      "bankdata.txt",
				      fbuf);
    if (rv) {
      /* for debian look also in /var/lib/ktoblzcheck1 */
      rv=GWEN_Directory_FindFileInPaths(paths,
					"ktoblzcheck1"
					DIRSEP
					"bankdata.txt",
					fbuf);
      if (rv) {
	DBG_INFO(AQBANKING_LOGDOMAIN,
                 "File [%s] not found, falling back to default",
                 "libktoblzcheck1"
                 DIRSEP
                 "bankdata.txt");
      }
    }
    GWEN_StringList_free(paths);
    if (rv) {
      DBG_WARN(AQBANKING_LOGDOMAIN,
               "Bank data for KtoBlzCheck not found (%d), falling back to default", rv);
    }
    else {
      bde->checker=AccountNumberCheck_new_file(GWEN_Buffer_GetStart(fbuf));
      if (!bde->checker) {
	DBG_ERROR(AQBANKING_LOGDOMAIN,
		  "KtoBlzCheck returned an error");
	GWEN_Buffer_free(fbuf);
	AB_BankInfoPlugin_free(bip);
	return 0;
      }
    }
    GWEN_Buffer_free(fbuf);
  }

  if (bde->checker==NULL) {
    bde->checker=AccountNumberCheck_new();
    if (!bde->checker) {
      DBG_ERROR(AQBANKING_LOGDOMAIN,
		"KtoBlzCheck returned an error");
      AB_BankInfoPlugin_free(bip);
      return 0;
    }
  }
#endif
  AB_BankInfoPlugin_SetCheckAccountFn(bip, AB_BankInfoPluginDE_CheckAccount);
  return bip;
}



void GWENHYWFAR_CB AB_BankInfoPluginDE_FreeData(void *bp, void *p){
  AB_BANKINFO_PLUGIN_DE *bde;

  bde=(AB_BANKINFO_PLUGIN_DE*)p;

#ifdef HAVE_KTOBLZCHECK
  if (bde->checker)
    AccountNumberCheck_delete(bde->checker);
#endif
  GWEN_FREE_OBJECT(bde);
}



AB_BANKINFO_CHECKRESULT
AB_BankInfoPluginDE_CheckAccount(AB_BANKINFO_PLUGIN *bip,
                                 const char *branchId,
                                 const char *bankId,
                                 const char *accountId){
  AB_BANKINFO_PLUGIN_DE *bde;
#ifdef HAVE_KTOBLZCHECK
  AccountNumberCheck_Result res;
#endif
  AB_BANKINFO_CHECKRESULT cr;

  assert(bankId);
  assert(accountId);

  assert(bip);
  bde=GWEN_INHERIT_GETDATA(AB_BANKINFO_PLUGIN, AB_BANKINFO_PLUGIN_DE, bip);
  assert(bde);

#ifdef HAVE_KTOBLZCHECK
  assert(bde->checker);
  res=AccountNumberCheck_check(bde->checker,
                               bankId,
                               accountId);
  switch(res) {
  case 0:  cr=AB_BankInfoCheckResult_Ok; break;
  case 1:  cr=AB_BankInfoCheckResult_UnknownResult; break;
  case 2:  cr=AB_BankInfoCheckResult_NotOk; break;
  case 3:  cr=AB_BankInfoCheckResult_UnknownBank; break;
  default: cr=AB_BankInfoCheckResult_UnknownResult; break;
  } /* switch */
#else
  cr=AB_BankInfoCheckResult_UnknownResult;
#endif

  return cr;
}








