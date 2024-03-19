/***************************************************************************
 begin       : Wed Oct 03 2018
 copyright   : (C) 2018 by Martin Preuss
 email       : martin@libchipcard.de

 ***************************************************************************
 * This file is part of the project "AqBanking".                           *
 * Please see toplevel file COPYING of that project for license details.   *
 ***************************************************************************/


#ifdef AQBANKING_WITH_PLUGIN_BANKINFO_DE
# include "src/libs/plugins/bankinfo/de/de.h"
#endif



AB_BANKINFO_PLUGIN *AB_Banking_CreateImBankInfoPlugin(AB_BANKING *ab, const char *modname)
{
  if (modname && *modname) {
#ifdef AQBANKING_WITH_PLUGIN_BANKINFO_DE
    if (strcasecmp(modname, "de")==0)
      return AB_BankInfoPluginDE_new(ab);
#endif
    DBG_ERROR(AQBANKING_LOGDOMAIN, "Plugin [%s] not compiled-in", modname);
  }

  return NULL;
}



AB_BANKINFO_PLUGIN *AB_Banking_LoadBankInfoPlugin(AB_BANKING *ab, const char *modname)
{
  GWEN_PLUGIN *pl;

  pl=GWEN_PluginManager_GetPlugin(ab_pluginManagerBankInfo, modname);
  if (pl) {
    AB_BANKINFO_PLUGIN *bip;

    bip=AB_Plugin_BankInfo_Factory(pl, ab);
    if (!bip) {
      DBG_ERROR(AQBANKING_LOGDOMAIN, "Error in plugin [%s]: No bank info created", modname);
      return NULL;
    }
    return bip;
  }
  else {
    DBG_INFO(AQBANKING_LOGDOMAIN, "Plugin [%s] not found", modname);
    return NULL;
  }
}



AB_BANKINFO_PLUGIN *AB_Banking_FindBankInfoPlugin(AB_BANKING *ab, const char *country)
{
  AB_BANKINFO_PLUGIN *bip;

  bip=AB_BankInfoPlugin_List_First(ab_bankInfoPlugins);
  while (bip) {
    if (strcasecmp(AB_BankInfoPlugin_GetCountry(bip), country)==0)
      break;
    bip=AB_BankInfoPlugin_List_Next(bip);
  }

  return bip;
}



AB_BANKINFO_PLUGIN *AB_Banking_GetBankInfoPlugin(AB_BANKING *ab, const char *country)
{
  AB_BANKINFO_PLUGIN *bip;

  assert(ab);
  assert(country);

  bip=AB_Banking_FindBankInfoPlugin(ab, country);
  if (bip)
    return bip;
  bip=AB_Banking_CreateImBankInfoPlugin(ab, country);
  if (bip==NULL)
    bip=AB_Banking_LoadBankInfoPlugin(ab, country);
  if (bip)
    AB_BankInfoPlugin_List_Add(bip, ab_bankInfoPlugins);

  return bip;
}



AB_BANKINFO *AB_Banking_GetBankInfo(AB_BANKING *ab,
                                    const char *country,
                                    const char *branchId,
                                    const char *bankId)
{
  AB_BANKINFO_PLUGIN *bip;

  assert(ab);
  assert(country);
  bip=AB_Banking_GetBankInfoPlugin(ab, country);
  if (!bip) {
    DBG_INFO(AQBANKING_LOGDOMAIN, "BankInfo plugin for country \"%s\" not found", country);
    return 0;
  }

  return AB_BankInfoPlugin_GetBankInfo(bip, branchId, bankId);
}



int AB_Banking_GetBankInfoByTemplate(AB_BANKING *ab,
                                     const char *country,
                                     AB_BANKINFO *tbi,
                                     AB_BANKINFO_LIST2 *bl)
{
  AB_BANKINFO_PLUGIN *bip;

  assert(ab);
  assert(country);
  bip=AB_Banking_GetBankInfoPlugin(ab, country);
  if (!bip) {
    DBG_INFO(AQBANKING_LOGDOMAIN,
             "BankInfo plugin for country \"%s\" not found",
             country);
    return 0;
  }

  return AB_BankInfoPlugin_GetBankInfoByTemplate(bip, tbi, bl);
}



AB_BANKINFO_CHECKRESULT AB_Banking_CheckAccount(AB_BANKING *ab,
                                                const char *country,
                                                const char *branchId,
                                                const char *bankId,
                                                const char *accountId)
{
  AB_BANKINFO_PLUGIN *bip;

  assert(ab);
  assert(country);
  bip=AB_Banking_GetBankInfoPlugin(ab, country);
  if (!bip) {
    DBG_INFO(AQBANKING_LOGDOMAIN, "BankInfo plugin for country \"%s\" not found", country);
    return AB_BankInfoCheckResult_UnknownResult;
  }

  return AB_BankInfoPlugin_CheckAccount(bip, branchId, bankId, accountId);
}



int AB_Banking__TransformIban(const char *iban, int len, char *newIban, int maxLen)
{
  int i, j;
  const char *p;
  char *s;

  assert(iban);
  /* format IBAN */
  i=0;
  j=0;
  p=iban;
  s=newIban;
  while (j<len && i<maxLen) {
    int c;

    c=toupper(*p);
    if (c!=' ') {
      if (c>='A' && c<='Z') {
        c=10+(c-'A');
        *s='0'+(c/10);
        s++;
        i++;
        if (i>=maxLen) {
          DBG_ERROR(AQBANKING_LOGDOMAIN, "Bad IBAN (too long)");
          return -1;
        }
        *s='0'+(c%10);
        s++;
        i++;
      }
      else if (isdigit(c)) {
        *s=c;
        s++;
        i++;
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



int AB_Banking_CheckIban(const char *iban)
{
  char newIban[256];
  char tmp[10];
  int i;
  unsigned int j;
  const char *p;
  char *s;

  if (strlen(iban)<5) {
    DBG_INFO(AQBANKING_LOGDOMAIN, "Bad IBAN (too short) [%s]", iban);
    return -1;
  }
  p=iban;
  if (!(*p >= 'A' && *p++ <= 'Z' && *p >= 'A' && *p++ <= 'Z')) {
    DBG_INFO(AQBANKING_LOGDOMAIN, "Bad IBAN (country code not in upper case) [%s]", iban);
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
  if (AB_Banking__TransformIban(p, 4, s, sizeof(newIban)-strlen(newIban)-1)) {
    DBG_INFO(AQBANKING_LOGDOMAIN, "here");
    return -1;
  }

  /* calculate checksum in 9er steps */
  p=newIban;
  tmp[0]=0;
  j=0;
  while (*p) {
    for (i=strlen(tmp); i<9;  i++) {
      if (!*p)
        break;
      tmp[i]=*(p++);
    }
    tmp[i]=0;
    if (1!=sscanf(tmp, "%u", &j)) {
      DBG_INFO(AQBANKING_LOGDOMAIN, "Bad IBAN (bad char) [%s]", iban);
      return -1;
    }
    j=j%97; /* modulo 97 */
    snprintf(tmp, sizeof(tmp), "%u", j);
  } /* while */

  if (j!=1) {
    DBG_INFO(AQBANKING_LOGDOMAIN, "Bad IBAN (bad checksum) [%s]", iban);
    return 1;
  }

  DBG_DEBUG(AQBANKING_LOGDOMAIN, "IBAN is valid [%s]", iban);
  return 0;
}



int AB_Banking_MakeGermanIban(const char *bankCode, const char *accountNumber, GWEN_BUFFER *ibanBuf)
{
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
  while (*p) {
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



