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


#ifndef GWHBCI_HBCI_P_H
#define GWHBCI_HBCI_P_H

#include "hbci_l.h"
#include <aqhbci/message.h>

#define AH_REGKEY_PATHS       "Software\\AqHbci\\Paths"
#define AH_REGKEY_XMLDATADIR  "xmldatadir"

#define AH_PM_LIBNAME       "aqhbci"
#define AH_PM_XMLDATADIR    "xmldatadir"


#define AH_HBCI_DEFAULT_CONNECT_TIMEOUT 30
#define AH_HBCI_DEFAULT_TRANSFER_TIMEOUT 60


struct AH_HBCI {
  GWEN_INHERIT_ELEMENT(AH_HBCI);

  AB_BANKING *banking;
  AB_PROVIDER *provider;

  char *productName;
  char *productVersion;

  GWEN_XMLNODE *defs;

  AH_MEDIUM_LIST *activeMedia;

  AH_BANK_LIST *banks;

  AH_DIALOG_LIST *dialogs;

  AH_HBCI_DIALOGUPFN dialogUpFn;
  AH_HBCI_DIALOGDOWNFN dialogDownFn;

  GWEN_TYPE_UINT32 libId;

  GWEN_TYPE_UINT32 counter;
  AH_MEDIUM *currentMedium;

  GWEN_DB_NODE *sharedRuntimeData;

  int transferTimeout;
  int connectTimeout;

};



AH_MEDIUM *AH_HBCI__FindMedium(AH_HBCI *hbci,
                               int country,
                               const char *bankId,
                               const char *userId);


int AH_HBCI_AddObjectPath(const AH_HBCI *hbci,
                          int country,
                          const char *bankId,
                          const char *accountId,
                          const char *userId,
                          const char *customerId,
                          GWEN_BUFFER *nbuf);

int AH_HBCI_SaveSettings(const char *path, GWEN_DB_NODE *db);
GWEN_DB_NODE *AH_HBCI_LoadSettings(const char *path);


int AH_HBCI_AddDefinitions(AH_HBCI *hbci, GWEN_XMLNODE *node);
GWEN_XMLNODE *AH_HBCI_LoadDefaultXmlFiles(const AH_HBCI *hbci);


AH_DIALOG *AH_HBCI_FindDialog(const AH_HBCI *hbci, const AH_CUSTOMER *cu);
AH_DIALOG *AH_HBCI_FindDialogByConnection(const AH_HBCI *hbci,
                                          const GWEN_NETCONNECTION *conn);


int AH_HBCI__LoadMedia(AH_HBCI *hbci, GWEN_DB_NODE *db);
int AH_HBCI__SaveMedia(AH_HBCI *hbci, GWEN_DB_NODE *db);


#endif /* GWHBCI_HBCI_P_H */



