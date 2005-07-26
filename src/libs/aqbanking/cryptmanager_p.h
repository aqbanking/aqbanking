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


#ifndef AQBANKING_CRYPTMANAGER_P_H
#define AQBANKING_CRYPTMANAGER_P_H

#include "cryptmanager_l.h"



typedef struct AB_CRYPTMANAGER AB_CRYPTMANAGER;
struct AB_CRYPTMANAGER {
  AB_BANKING *banking;
  GWEN_TYPE_UINT32 showBoxId;
};
void AB_CryptManager_FreeData(void *bp, void *p);


int AB_CryptManager_GetPin(GWEN_PLUGIN_MANAGER *cm,
                           GWEN_CRYPTTOKEN *token,
                           GWEN_CRYPTTOKEN_PINTYPE pt,
                           GWEN_CRYPTTOKEN_PINENCODING pe,
                           GWEN_TYPE_UINT32 flags,
                           unsigned char *buffer,
                           unsigned int minLength,
                           unsigned int maxLength,
                           unsigned int *pinLength);

int AB_CryptManager_BeginEnterPin(GWEN_PLUGIN_MANAGER *cm,
                                  GWEN_CRYPTTOKEN *token,
                                  GWEN_CRYPTTOKEN_PINTYPE pt);
int AB_CryptManager_EndEnterPin(GWEN_PLUGIN_MANAGER *cm,
                                GWEN_CRYPTTOKEN *token,
                                GWEN_CRYPTTOKEN_PINTYPE pt,
                                int ok);
int AB_CryptManager_InsertToken(GWEN_PLUGIN_MANAGER *cm,
                                GWEN_CRYPTTOKEN *token);
int AB_CryptManager_InsertCorrectToken(GWEN_PLUGIN_MANAGER *cm,
                                       GWEN_CRYPTTOKEN *token);

int AB_CryptManager_ShowMessage(GWEN_PLUGIN_MANAGER *cm,
                                GWEN_CRYPTTOKEN *token,
				const char *title,
                                const char *msg);




#endif /* AQBANKING_CRYPTMANAGER_P_H */
