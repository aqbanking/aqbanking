/***************************************************************************
 begin       : Mon Mar 01 2004
 copyright   : (C) 2004-2013 by Martin Preuss
 email       : martin@libchipcard.de

 ***************************************************************************
 * This file is part of the project "AqBanking".                           *
 * Please see toplevel file COPYING of that project for license details.   *
 ***************************************************************************/

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

/* don't warn about our own deprecated functions */
#define AQBANKING_NOWARN_DEPRECATED 


#include "banking_p.h"
#include "provider_l.h"
#include "imexporter_l.h"
#include "bankinfoplugin_l.h"
#include "i18n_l.h"
#include "country_l.h"
#include "userfns_l.h"

#include <gwenhywfar/version.h>
#include <gwenhywfar/gwenhywfar.h>
#include <gwenhywfar/debug.h>
#include <gwenhywfar/misc.h>
#include <gwenhywfar/directory.h>
#include <gwenhywfar/libloader.h>
//#include <gwenhywfar/bio_file.h>
#include <gwenhywfar/text.h>
#include <gwenhywfar/xml.h>
#include <gwenhywfar/pathmanager.h>
#include <gwenhywfar/gui.h>
#include <gwenhywfar/dbio.h>
#include <gwenhywfar/ctplugin.h>
#include <gwenhywfar/configmgr.h>
#include <gwenhywfar/syncio_file.h>

#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <ctype.h>
#include <stdio.h>

/* includes for high level API */
#include "jobgetbalance.h"
#include "jobgettransactions.h"
#include "jobgetstandingorders.h"
#include "jobsingletransfer.h"
#include "jobsingledebitnote.h"
#include "jobeutransfer.h"
#include "jobgetdatedtransfers.h"


#ifdef OS_WIN32
# define ftruncate chsize
# define DIRSEP "\\"
#else
# define DIRSEP "/"
#endif

GWEN_INHERIT_FUNCTIONS(AB_BANKING)

#include <aqbanking/error.h>


#include "banking_init.c"
#include "banking_account.c"
#include "banking_user.c"
#include "banking_online.c"
#include "banking_deprec.c"
#include "banking_cfg.c"
#include "banking_imex.c"




AB_BANKING *AB_Banking_new(const char *appName,
			   const char *dname,
			   uint32_t extensions){
  AB_BANKING *ab;
  GWEN_BUFFER *nbuf;
  char buffer[256];
  int err;

  assert(appName);
  err=GWEN_Init();
  if (err) {
    DBG_ERROR_ERR(AQBANKING_LOGDOMAIN, err);
    abort();
  }
  DBG_INFO(AQBANKING_LOGDOMAIN,
           "Application \"%s\" compiled with extensions %08x",
           appName, extensions);

  nbuf=GWEN_Buffer_new(0, 256, 0, 1);
  if (GWEN_Text_EscapeToBufferTolerant(appName, nbuf)) {
    DBG_ERROR(AQBANKING_LOGDOMAIN,
              "Bad application name, aborting.");
    GWEN_Buffer_free(nbuf);
    abort();
  }
  else {
    char *s;

    s=GWEN_Buffer_GetStart(nbuf);
    while(*s) {
      *s=tolower(*s);
      s++;
    }
  }

  GWEN_NEW_OBJECT(AB_BANKING, ab);
  GWEN_INHERIT_INIT(AB_BANKING, ab);
  ab->providers=AB_Provider_List_new();
  ab->users=AB_User_List_new();
  ab->accounts=AB_Account_List_new();
  ab->appEscName=strdup(GWEN_Buffer_GetStart(nbuf));
  ab->appName=strdup(appName);
  ab->activeProviders=GWEN_StringList_new();
  ab->cryptTokenList=GWEN_Crypt_Token_List2_new();

  GWEN_StringList_SetSenseCase(ab->activeProviders, 0);

  GWEN_Buffer_free(nbuf);

  AB_Banking__GetConfigManager(ab, dname);

  ab->appExtensions=extensions;

  if (getcwd(buffer, sizeof(buffer)-1)==0) {
    DBG_ERROR(AQBANKING_LOGDOMAIN,
              "getcwd: %s", strerror(errno));
  }
  else {
    struct stat st;

    if (stat(buffer, &st)) {
      DBG_ERROR(AQBANKING_LOGDOMAIN,
                "stat(%s): %s", buffer, strerror(errno));
    }
    else {
      ab->startFolder=strdup(buffer);
    }
  }

  return ab;
}



void AB_Banking_free(AB_BANKING *ab){
  if (ab) {
    DBG_INFO(AQBANKING_LOGDOMAIN, "Destroying AB_BANKING");

    GWEN_INHERIT_FINI(AB_BANKING, ab);

    AB_Banking_ClearCryptTokenList(ab);
    GWEN_Crypt_Token_List2_free(ab->cryptTokenList);
    AB_Account_List_free(ab->accounts);
    AB_User_List_free(ab->users);
    AB_Provider_List_free(ab->providers);
    GWEN_StringList_free(ab->activeProviders);
    GWEN_ConfigMgr_free(ab->configMgr);
    free(ab->startFolder);
    free(ab->appName);
    free(ab->appEscName);
    free(ab->dataDir);
    GWEN_FREE_OBJECT(ab);
    GWEN_Fini();
  }
}



int AB_Banking_GetUniqueId(AB_BANKING *ab){
  int rv;
  int uid=0;
  GWEN_DB_NODE *dbConfig=NULL;

  rv=GWEN_ConfigMgr_LockGroup(ab->configMgr,
			      AB_CFG_GROUP_MAIN,
			      "uniqueId");
  if (rv<0) {
    DBG_ERROR(AQBANKING_LOGDOMAIN, "Unable to lock main config (%d)", rv);
    return rv;
  }

  rv=GWEN_ConfigMgr_GetGroup(ab->configMgr,
			     AB_CFG_GROUP_MAIN,
			     "uniqueId",
			     &dbConfig);
  if (rv<0) {
    DBG_ERROR(AQBANKING_LOGDOMAIN, "Unable to read main config (%d)", rv);
    return rv;
  }

  uid=GWEN_DB_GetIntValue(dbConfig, "uniqueId", 0, 0);
  uid++;
  GWEN_DB_SetIntValue(dbConfig, GWEN_DB_FLAGS_OVERWRITE_VARS,
                      "uniqueId", uid);
  rv=GWEN_ConfigMgr_SetGroup(ab->configMgr,
			     AB_CFG_GROUP_MAIN,
			     "uniqueId",
			     dbConfig);
  if (rv<0) {
    DBG_ERROR(AQBANKING_LOGDOMAIN, "Unable to write main config (%d)", rv);
    GWEN_ConfigMgr_UnlockGroup(ab->configMgr,
			       AB_CFG_GROUP_MAIN,
			       "uniqueId");
    GWEN_DB_Group_free(dbConfig);
    return rv;
  }
  GWEN_DB_Group_free(dbConfig);

  /* unlock */
  rv=GWEN_ConfigMgr_UnlockGroup(ab->configMgr,
				AB_CFG_GROUP_MAIN,
				"uniqueId");
  if (rv<0) {
    DBG_ERROR(AQBANKING_LOGDOMAIN, "Unable to unlock main config (%d)", rv);
    return rv;
  }

  return uid;
}



int AB_Banking_SetUniqueId(AB_BANKING *ab, uint32_t uid){
  int rv;
  GWEN_DB_NODE *dbConfig=NULL;

  rv=GWEN_ConfigMgr_LockGroup(ab->configMgr,
			      AB_CFG_GROUP_MAIN,
			      "uniqueId");
  if (rv<0) {
    DBG_ERROR(AQBANKING_LOGDOMAIN, "Unable to lock main config (%d)", rv);
    return rv;
  }

  rv=GWEN_ConfigMgr_GetGroup(ab->configMgr,
			     AB_CFG_GROUP_MAIN,
			     "uniqueId",
			     &dbConfig);
  if (rv<0) {
    DBG_ERROR(AQBANKING_LOGDOMAIN, "Unable to read main config (%d)", rv);
    return rv;
  }

  GWEN_DB_SetIntValue(dbConfig, GWEN_DB_FLAGS_OVERWRITE_VARS,
                      "uniqueId", uid);
  rv=GWEN_ConfigMgr_SetGroup(ab->configMgr,
			     AB_CFG_GROUP_MAIN,
			     "uniqueId",
			     dbConfig);
  if (rv<0) {
    DBG_ERROR(AQBANKING_LOGDOMAIN, "Unable to write main config (%d)", rv);
    GWEN_ConfigMgr_UnlockGroup(ab->configMgr,
			       AB_CFG_GROUP_MAIN,
			       "uniqueId");
    GWEN_DB_Group_free(dbConfig);
    return rv;
  }
  GWEN_DB_Group_free(dbConfig);

  /* unlock */
  rv=GWEN_ConfigMgr_UnlockGroup(ab->configMgr,
				AB_CFG_GROUP_MAIN,
				"uniqueId");
  if (rv<0) {
    DBG_ERROR(AQBANKING_LOGDOMAIN, "Unable to unlock main config (%d)", rv);
    return rv;
  }

  return 0;
}



GWEN_CONFIGMGR *AB_Banking_GetConfigMgr(AB_BANKING *ab) {
  assert(ab);
  return ab->configMgr;
}



const char *AB_Banking_GetAppName(const AB_BANKING *ab){
  assert(ab);
  return ab->appName;
}



const char *AB_Banking_GetEscapedAppName(const AB_BANKING *ab){
  assert(ab);
  return ab->appEscName;
}



AB_BANKINFO_PLUGIN *AB_Banking__LoadBankInfoPlugin(AB_BANKING *ab,
						   const char *modname){
  GWEN_PLUGIN *pl;

  pl=GWEN_PluginManager_GetPlugin(ab_pluginManagerBankInfo, modname);
  if (pl) {
    AB_BANKINFO_PLUGIN *bip;

    bip=AB_Plugin_BankInfo_Factory(pl, ab);
    if (!bip) {
      DBG_ERROR(AQBANKING_LOGDOMAIN,
		"Error in plugin [%s]: No bank info created",
		modname);
      return NULL;
    }
    return bip;
  }
  else {
    DBG_INFO(AQBANKING_LOGDOMAIN, "Plugin [%s] not found", modname);
    return NULL;
  }
}



AB_BANKINFO_PLUGIN *AB_Banking__GetBankInfoPlugin(AB_BANKING *ab,
                                                  const char *country) {
  AB_BANKINFO_PLUGIN *bip;

  assert(ab);
  assert(country);

  bip=AB_BankInfoPlugin_List_First(ab_bankInfoPlugins);
  while(bip) {
    if (strcasecmp(AB_BankInfoPlugin_GetCountry(bip), country)==0)
      return bip;
    bip=AB_BankInfoPlugin_List_Next(bip);
  }

  bip=AB_Banking__LoadBankInfoPlugin(ab, country);
  if (!bip) {
    DBG_ERROR(AQBANKING_LOGDOMAIN,
              "BankInfo plugin for country \"%s\" not found",
              country);
    return 0;
  }
  AB_BankInfoPlugin_List_Add(bip, ab_bankInfoPlugins);
  return bip;
}



AB_BANKINFO *AB_Banking_GetBankInfo(AB_BANKING *ab,
                                    const char *country,
                                    const char *branchId,
                                    const char *bankId){
  AB_BANKINFO_PLUGIN *bip;

  assert(ab);
  assert(country);
  bip=AB_Banking__GetBankInfoPlugin(ab, country);
  if (!bip) {
    DBG_INFO(AQBANKING_LOGDOMAIN,
             "BankInfo plugin for country \"%s\" not found",
             country);
    return 0;
  }

  return AB_BankInfoPlugin_GetBankInfo(bip, branchId, bankId);
}



int AB_Banking_GetBankInfoByTemplate(AB_BANKING *ab,
                                     const char *country,
                                     AB_BANKINFO *tbi,
                                     AB_BANKINFO_LIST2 *bl){
  AB_BANKINFO_PLUGIN *bip;

  assert(ab);
  assert(country);
  bip=AB_Banking__GetBankInfoPlugin(ab, country);
  if (!bip) {
    DBG_INFO(AQBANKING_LOGDOMAIN,
             "BankInfo plugin for country \"%s\" not found",
             country);
    return 0;
  }

  return AB_BankInfoPlugin_GetBankInfoByTemplate(bip, tbi, bl);
}



AB_BANKINFO_CHECKRESULT
AB_Banking_CheckAccount(AB_BANKING *ab,
                        const char *country,
                        const char *branchId,
                        const char *bankId,
                        const char *accountId) {
  AB_BANKINFO_PLUGIN *bip;

  assert(ab);
  assert(country);
  bip=AB_Banking__GetBankInfoPlugin(ab, country);
  if (!bip) {
    DBG_INFO(AQBANKING_LOGDOMAIN,
             "BankInfo plugin for country \"%s\" not found",
             country);
    return AB_BankInfoCheckResult_UnknownResult;
  }

  return AB_BankInfoPlugin_CheckAccount(bip, branchId, bankId, accountId);
}



int AB_Banking__GetDebuggerPath(AB_BANKING *ab,
                                const char *backend,
                                GWEN_BUFFER *pbuf){
  const char *s;

  GWEN_Buffer_AppendString(pbuf,
                           AQBANKING_PLUGINS
			   DIRSEP
			   AB_PROVIDER_DEBUGGER_FOLDER
                           DIRSEP);
  s=backend;
  while(*s) GWEN_Buffer_AppendByte(pbuf, tolower(*(s++)));

  return 0;
}



void *AB_Banking_GetUserData(AB_BANKING *ab) {
  assert(ab);
  return ab->user_data;
}


void AB_Banking_SetUserData(AB_BANKING *ab, void *user_data) {
  assert(ab);
  ab->user_data = user_data;
}






/* __________________________________________________________________________
 * AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA
 *                             High Level API
 * YYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYY
 */



AB_ACCOUNT *AB_Banking__GetAccount(AB_BANKING *ab, const char *accountId){
  int rv;
  GWEN_DB_NODE *dbData=NULL;

  rv=AB_Banking_LoadAppConfig(ab, &dbData);
  if (rv<0) {
    DBG_ERROR(AQBANKING_LOGDOMAIN,
	      "Unable to load app config");
    return NULL;
  }
  else {
    uint32_t uniqueId;
    AB_ACCOUNT *a;
    GWEN_DB_NODE *db;

    assert(dbData);
    uniqueId=0;
    db=GWEN_DB_GetGroup(dbData, GWEN_PATH_FLAGS_NAMEMUSTEXIST,
			"banking/aliases");
    if (db)
      uniqueId=GWEN_DB_GetIntValue(db, accountId, 0, 0);
    GWEN_DB_Group_free(dbData);
    if (!uniqueId) {
      /* should not happen anyway */
      DBG_ERROR(AQBANKING_LOGDOMAIN,
		"Account has no unique id assigned to the alias [%s].",
		accountId);
      return NULL;
    }

    a=AB_Banking_GetAccount(ab, uniqueId);
    if (!a) {
      DBG_ERROR(AQBANKING_LOGDOMAIN, "Account with alias \"%s\" not found",
		accountId);
      return NULL;
    }

    return a;
  }
}



AB_ACCOUNT *AB_Banking_GetAccountByAlias(AB_BANKING *ab,
					 const char *accountId){
  return AB_Banking__GetAccount(ab, accountId);
}



void AB_Banking_SetAccountAlias(AB_BANKING *ab, AB_ACCOUNT *a, const char *alias){
  int rv;
  GWEN_DB_NODE *dbConfig=NULL;
  GWEN_DB_NODE *db;

  rv=AB_Banking_LockAppConfig(ab);
  if (rv<0) {
    DBG_ERROR(AQBANKING_LOGDOMAIN, "here (%d)", rv);
    return;
  }

  rv=AB_Banking_LoadAppConfig(ab, &dbConfig);
  if (rv<0) {
    DBG_ERROR(AQBANKING_LOGDOMAIN, "here (%d)", rv);
    AB_Banking_UnlockAppConfig(ab);
    return;
  }

  db=GWEN_DB_GetGroup(dbConfig, GWEN_DB_FLAGS_DEFAULT, "banking/aliases");
  GWEN_DB_SetIntValue(db, GWEN_DB_FLAGS_OVERWRITE_VARS,
		      alias,
		      AB_Account_GetUniqueId(a));

  rv=AB_Banking_SaveAppConfig(ab, dbConfig);
  if (rv<0) {
    DBG_ERROR(AQBANKING_LOGDOMAIN, "here (%d)", rv);
    AB_Banking_UnlockAppConfig(ab);
    return;
  }

  rv=AB_Banking_UnlockAppConfig(ab);
  if (rv<0) {
    DBG_ERROR(AQBANKING_LOGDOMAIN, "here (%d)", rv);
    return;
  }
}



const AB_COUNTRY *AB_Banking_FindCountryByName(AB_BANKING *ab,
                                               const char *name){
  assert(ab);
  return AB_Country_FindByName(name);
}



const AB_COUNTRY *AB_Banking_FindCountryByLocalName(AB_BANKING *ab,
                                               const char *name){
  assert(ab);
  return AB_Country_FindByLocalName(name);
}



const AB_COUNTRY *AB_Banking_FindCountryByCode(AB_BANKING *ab,
                                               const char *code){
  assert(ab);
  return AB_Country_FindByCode(code);
}



const AB_COUNTRY *AB_Banking_FindCountryByNumeric(AB_BANKING *ab,
                                                  int numid){
  assert(ab);
  return AB_Country_FindByNumeric(numid);
}



AB_COUNTRY_CONSTLIST2 *AB_Banking_ListCountriesByName(AB_BANKING *ab,
                                                      const char *name){
  assert(ab);
  return AB_Country_ListByName(name);
}



AB_COUNTRY_CONSTLIST2 *AB_Banking_ListCountriesByLocalName(AB_BANKING *ab,
                                                           const char *name){
  assert(ab);
  return AB_Country_ListByLocalName(name);
}



int AB_Banking__TransformIban(const char *iban, int len,
			      char *newIban, int maxLen) {
  int i, j;
  const char *p;
  char *s;

  assert(iban);
  /* format IBAN */
  i=0;
  j=0;
  p=iban;
  s=newIban;
  while(j<len && i<maxLen) {
    int c;

    c=toupper(*p);
    if (c!=' ') {
      if (c>='A' && c<='Z') {
	c=10+(c-'A');
	*s='0'+(c/10);
	s++; i++;
	if (i>=maxLen) {
	  DBG_ERROR(AQBANKING_LOGDOMAIN, "Bad IBAN (too long)");
	  return -1;
	}
	*s='0'+(c%10);
	s++; i++;
      }
      else if (isdigit(c)) {
	*s=c;
	s++; i++;
      }
      else {
	DBG_ERROR(AQBANKING_LOGDOMAIN, "Bad IBAN (bad char)");
	return -1;
      }
    }
    p++;
    j++;
  } /* while */
  if (j<len) {
    DBG_ERROR(AQBANKING_LOGDOMAIN, "Bad IBAN (too long)");
    return -1;
  }
  *s=0;

  return 0;
}



int AB_Banking_CheckIban(const char *iban) {
  char newIban[256];
  char tmp[10];
  int i;
  unsigned int j;
  const char *p;
  char *s;

  if (strlen(iban)<5) {
    DBG_ERROR(AQBANKING_LOGDOMAIN, "Bad IBAN (too short)");
    return -1;
  }
  p=iban;
  if (!(*p >= 'A' && *p++ <= 'Z' && *p >= 'A' && *p++ <= 'Z')) {
    DBG_ERROR(AQBANKING_LOGDOMAIN, "Bad IBAN (country code not in upper case)");
    return -1;
  }
  p+=2;

  /* convert IBAN+4 to buffer */
  if (AB_Banking__TransformIban(p, strlen(p),
				newIban, sizeof(newIban)-1)) {
    DBG_INFO(AQBANKING_LOGDOMAIN, "here");
    return -1;
  }

  /* append country and checksum */
  p=iban;
  s=newIban+strlen(newIban);
  if (AB_Banking__TransformIban(p, 4, s, sizeof(newIban)-strlen(newIban)-1)){
    DBG_INFO(AQBANKING_LOGDOMAIN, "here");
    return -1;
  }

  /* calculate checksum in 9er steps */
  p=newIban;
  tmp[0]=0;
  j=0;
  while(*p) {
    for (i=strlen(tmp); i<9;  i++) {
      if (!*p)
        break;
      tmp[i]=*(p++);
    }
    tmp[i]=0;
    if (1!=sscanf(tmp, "%u", &j)) {
      DBG_ERROR(AQBANKING_LOGDOMAIN, "Bad IBAN (bad char)");
      return -1;
    }
    j=j%97; /* modulo 97 */
    snprintf(tmp, sizeof(tmp), "%u", j);
  } /* while */

  if (j!=1) {
    DBG_ERROR(AQBANKING_LOGDOMAIN, "Bad IBAN (bad checksum)");
    return 1;
  }

  DBG_INFO(AQBANKING_LOGDOMAIN, "IBAN is valid");
  return 0;
}



int AB_Banking_MakeGermanIban(const char *bankCode, const char *accountNumber, GWEN_BUFFER *ibanBuf) {
  GWEN_BUFFER *tbuf;
  int i;
  char numbuf[32];
  char tmp[10];
  int rv;
  unsigned int j;
  const char *p;

  /* create BBAN */
  tbuf=GWEN_Buffer_new(0, 256, 0, 1);

  /* bank code */
  i=strlen(bankCode);
  if (i<8)
    GWEN_Buffer_FillWithBytes(tbuf, '0', 8-i);
  rv=AB_Banking__TransformIban(bankCode, strlen(bankCode), numbuf, sizeof(numbuf)-1);
  if (rv<0) {
    GWEN_Buffer_free(tbuf);
    DBG_ERROR(AQBANKING_LOGDOMAIN, "Bad bank code (bad char) (%d)", rv);
    return rv;
  }
  GWEN_Buffer_AppendString(tbuf, numbuf);

  /* account number */
  i=strlen(accountNumber);
  if (i<10)
    GWEN_Buffer_FillWithBytes(tbuf, '0', 10-i);
  rv=AB_Banking__TransformIban(accountNumber, strlen(accountNumber), numbuf, sizeof(numbuf)-1);
  if (rv<0) {
    GWEN_Buffer_free(tbuf);
    DBG_ERROR(AQBANKING_LOGDOMAIN, "Bad account number (bad char) (%d)", rv);
    return rv;
  }
  GWEN_Buffer_AppendString(tbuf, numbuf);

  /* add "DE00" */
  GWEN_Buffer_AppendString(tbuf, "131400");

  /* calculate checksum in 9er steps */
  p=GWEN_Buffer_GetStart(tbuf);
  tmp[0]=0;
  j=0;
  while(*p) {
    for (i=strlen(tmp); i<9;  i++) {
      if (!*p)
        break;
      tmp[i]=*(p++);
    }
    tmp[i]=0;
    if (1!=sscanf(tmp, "%u", &j)) {
      GWEN_Buffer_free(tbuf);
      DBG_ERROR(AQBANKING_LOGDOMAIN, "Bad IBAN (bad char)");
      return -1;
    }
    j=j%97; /* modulo 97 */
    snprintf(tmp, sizeof(tmp), "%u", j);
  } /* while */

  /* j contains the modulus */
  snprintf(tmp, sizeof(tmp), "%02u", 98-j);

  GWEN_Buffer_AppendString(ibanBuf, "DE"); /* DE */
  GWEN_Buffer_AppendString(ibanBuf, tmp);  /* checksum */
  i=strlen(bankCode);                      /* bank code */
  if (i<8)
    GWEN_Buffer_FillWithBytes(ibanBuf, '0', 8-i);
  GWEN_Buffer_AppendString(ibanBuf, bankCode);

  i=strlen(accountNumber);                  /* account number */
  if (i<10)
    GWEN_Buffer_FillWithBytes(ibanBuf, '0', 10-i);
  GWEN_Buffer_AppendString(ibanBuf, accountNumber);


  DBG_INFO(AQBANKING_LOGDOMAIN, "IBAN is %s", GWEN_Buffer_GetStart(ibanBuf));
  GWEN_Buffer_free(tbuf);
  return 0;
}



GWEN_STRINGLIST *AB_Banking_GetGlobalDataDirs(void) {
  GWEN_STRINGLIST *sl;

  sl=GWEN_PathManager_GetPaths(AB_PM_LIBNAME, AB_PM_DATADIR);
  return sl;
}



GWEN_STRINGLIST *AB_Banking_GetGlobalSysconfDirs(void) {
  GWEN_STRINGLIST *sl;

  sl=GWEN_PathManager_GetPaths(AB_PM_LIBNAME, AB_PM_SYSCONFDIR);
  return sl;
}



void AB_Banking_GetVersion(int *major,
			   int *minor,
			   int *patchlevel,
			   int *build) {
  if (major)
    *major=AQBANKING_VERSION_MAJOR;
  if (minor)
    *minor=AQBANKING_VERSION_MINOR;
  if (patchlevel)
    *patchlevel=AQBANKING_VERSION_PATCHLEVEL;
  if (build)
    *build=AQBANKING_VERSION_BUILD;
}



GWEN_DIALOG *AB_Banking_GetNewUserDialog(AB_BANKING *ab,
					 const char *backend,
					 int mode) {
  AB_PROVIDER *pro;
  GWEN_DIALOG *dlg;

  assert(ab);
  pro=AB_Banking_GetProvider(ab, backend);
  if (!pro) {
    DBG_ERROR(AQBANKING_LOGDOMAIN, "Backend \"%s\" not found", backend);
    return NULL;
  }

  dlg=AB_Provider_GetNewUserDialog(pro, mode);
  if (dlg==NULL) {
    DBG_INFO(AQBANKING_LOGDOMAIN,
	     "Provider did not return a NewUser dialog (backend=%s, mode=%d)",
	     backend, mode);
    return NULL;
  }

  return dlg;
}






