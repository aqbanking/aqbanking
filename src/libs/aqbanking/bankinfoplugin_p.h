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


#ifndef AQBANKING_BANKINFOPLUGIN_P_H
#define AQBANKING_BANKINFOPLUGIN_P_H

#include "bankinfoplugin_l.h"
#include <gwenhywfar/types.h>


struct AB_BANKINFO_PLUGIN {
  GWEN_INHERIT_ELEMENT(AB_BANKINFO_PLUGIN);
  GWEN_LIST_ELEMENT(AB_BANKINFO_PLUGIN);
  GWEN_TYPE_UINT32 usage;

  char *country;

  GWEN_LIBLOADER *libLoader;

  AB_BANKINFOPLUGIN_GETBANKINFO_FN getBankInfoFn;
  AB_BANKINFOPLUGIN_CHECKACCOUNT_FN checkAccountFn;
  AB_BANKINFOPLUGIN_GETBANKINFOBYTMPLATE_FN getBankInfoByTemplateFn;
};


#endif /* AQBANKING_BANKINFOPLUGIN_P_H */

