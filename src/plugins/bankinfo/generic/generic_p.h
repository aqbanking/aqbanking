/***************************************************************************
 $RCSfile$
                             -------------------
    cvs         : $Id$
    begin       : Mon Mar 01 2004
    copyright   : (C) 2004 by Martin Preuss
    email       : martin@libchipcard.de

 ***************************************************************************
 *          Please see toplevel file COPYING for license details           *
 ***************************************************************************/

#ifndef AQBANKING_BANKINFO_GENERIC_P_H
#define AQBANKING_BANKINFO_GENERIC_P_H

#include "generic_l.h"



typedef struct AB_BANKINFO_PLUGIN_GENERIC AB_BANKINFO_PLUGIN_GENERIC;
struct AB_BANKINFO_PLUGIN_GENERIC {
  AB_BANKING *banking;
  char *country;
  char *dataDir;
};


void GWENHYWFAR_CB AB_BankInfoPluginGENERIC_FreeData(void *bp, void *p);

AB_BANKINFO *AB_BankInfoPluginGENERIC_GetBankInfo(AB_BANKINFO_PLUGIN *bip,
                                                  const char *branchId,
                                                  const char *bankId);

AB_BANKINFO *AB_BankInfoPluginGENERIC__SearchbyCode(AB_BANKINFO_PLUGIN *bip,
                                                    const char *bankId);
int AB_BankInfoPluginGENERIC_SearchbyTemplate(AB_BANKINFO_PLUGIN *bip,
                                              AB_BANKINFO *tbi,
                                              AB_BANKINFO_LIST2 *bl);

void AB_BankInfoPluginGENERIC__GetDataDir(AB_BANKINFO_PLUGIN *bip,
                                          GWEN_BUFFER *pbuf);

AB_BANKINFO *AB_BankInfoPluginGENERIC__ReadBankInfo(AB_BANKINFO_PLUGIN *bip,
                                                    const char *num);

int AB_BankInfoPluginGENERIC__AddById(AB_BANKINFO_PLUGIN *bip,
                                      const char *bankId,
                                      AB_BANKINFO_LIST2 *bl);

int AB_BankInfoPluginGENERIC__AddByBic(AB_BANKINFO_PLUGIN *bip,
                                       const char *bic,
                                       AB_BANKINFO_LIST2 *bl);
int AB_BankInfoPluginGENERIC__AddByNameAndLoc(AB_BANKINFO_PLUGIN *bip,
                                              const char *name,
                                              const char *loc,
                                              AB_BANKINFO_LIST2 *bl);

int AB_BankInfoPluginGENERIC__CmpTemplate(AB_BANKINFO *bi,
                                          const AB_BANKINFO *tbi,
					  uint32_t flags);


int AB_BankInfoPluginGENERIC_AddByTemplate(AB_BANKINFO_PLUGIN *bip,
                                           AB_BANKINFO *tbi,
                                           AB_BANKINFO_LIST2 *bl,
                                           uint32_t flags);

#define AB_BANKINFO_GENERIC__FLAGS_COUNTRY  0x00000001
#define AB_BANKINFO_GENERIC__FLAGS_BRANCHID 0x00000002
#define AB_BANKINFO_GENERIC__FLAGS_BANKID   0x00000004
#define AB_BANKINFO_GENERIC__FLAGS_BIC      0x00000008
#define AB_BANKINFO_GENERIC__FLAGS_BANKNAME 0x00000010
#define AB_BANKINFO_GENERIC__FLAGS_LOCATION 0x00000020
#define AB_BANKINFO_GENERIC__FLAGS_CITY AB_BANKINFO_GENERIC__FLAGS_LOCATION
#define AB_BANKINFO_GENERIC__FLAGS_STREET   0x00000040
#define AB_BANKINFO_GENERIC__FLAGS_ZIPCODE  0x00000080
#define AB_BANKINFO_GENERIC__FLAGS_REGION   0x00000100
#define AB_BANKINFO_GENERIC__FLAGS_PHONE    0x00000200
#define AB_BANKINFO_GENERIC__FLAGS_FAX      0x00000400
#define AB_BANKINFO_GENERIC__FLAGS_EMAIL    0x00000800
#define AB_BANKINFO_GENERIC__FLAGS_WEBSITE  0x00001000


#endif /* AQBANKING_BANKINFO_GENERIC_P_H */




