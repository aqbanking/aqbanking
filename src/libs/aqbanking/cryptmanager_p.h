/***************************************************************************
 $RCSfile$
 -------------------
 cvs         : $Id$
 begin       : Mon Mar 01 2004
 copyright   : (C) 2004 by Martin Preuss
 email       : martin@libchipcard.de

 ***************************************************************************
 * This file is part of the project "AqBanking".                           *
 * Please see toplevel file COPYING of that project for license details.   *
 ***************************************************************************/


#ifndef AQBANKING_CRYPTMANAGER_P_H
#define AQBANKING_CRYPTMANAGER_P_H

#include "cryptmanager_l.h"



typedef struct AB_CRYPTMANAGER AB_CRYPTMANAGER;
struct AB_CRYPTMANAGER {
  AB_BANKING *banking;
  uint32_t showBoxId;
};
static void GWENHYWFAR_CB AB_CryptManager_FreeData(void *bp, void *p);


static int AB_CryptManager_GetPin(GWEN_PLUGIN_MANAGER *cm,
                                  GWEN_CRYPTTOKEN *token,
                                  GWEN_CRYPTTOKEN_PINTYPE pt,
                                  GWEN_CRYPTTOKEN_PINENCODING pe,
                                  uint32_t flags,
                                  unsigned char *buffer,
                                  unsigned int minLength,
                                  unsigned int maxLength,
                                  unsigned int *pinLength);

static int AB_CryptManager_SetPinStatus(GWEN_PLUGIN_MANAGER *pm,
                                        GWEN_CRYPTTOKEN *token,
                                        GWEN_CRYPTTOKEN_PINTYPE pt,
                                        GWEN_CRYPTTOKEN_PINENCODING pe,
                                        uint32_t flags,
                                        unsigned char *buffer,
                                        unsigned int pinLength,
                                        int isOk);



#endif /* AQBANKING_CRYPTMANAGER_P_H */
