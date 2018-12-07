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


#ifndef AQBANKING_BANKINFOPLUGIN_P_H
#define AQBANKING_BANKINFOPLUGIN_P_H

#include "bankinfoplugin_l.h"
#include <gwenhywfar/types.h>
#include <gwenhywfar/plugin.h>


struct AB_BANKINFO_PLUGIN {
  GWEN_INHERIT_ELEMENT(AB_BANKINFO_PLUGIN);
  GWEN_LIST_ELEMENT(AB_BANKINFO_PLUGIN);
  uint32_t usage;

  char *country;

  GWEN_PLUGIN *plugin;

  AB_BANKINFOPLUGIN_GETBANKINFO_FN getBankInfoFn;
  AB_BANKINFOPLUGIN_CHECKACCOUNT_FN checkAccountFn;
  AB_BANKINFOPLUGIN_GETBANKINFOBYTMPLATE_FN getBankInfoByTemplateFn;
};



typedef struct AB_PLUGIN_BANKINFO AB_PLUGIN_BANKINFO;
struct AB_PLUGIN_BANKINFO {
  AB_PLUGIN_BANKINFO_FACTORY_FN pluginFactoryFn;
};

static void GWENHYWFAR_CB AB_Plugin_BankInfo_FreeData(void *bp, void *p);


#endif /* AQBANKING_BANKINFOPLUGIN_P_H */

