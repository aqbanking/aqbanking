/***************************************************************************
    begin       : Mon Mar 01 2004
    copyright   : (C) 2018 by Martin Preuss
    email       : martin@libchipcard.de

 ***************************************************************************
 *          Please see toplevel file COPYING for license details           *
 ***************************************************************************/

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif


#include "de.h"
#include "../generic/generic_l.h"

#include <aqbanking/banking.h>
#include <gwenhywfar/debug.h>



AB_BANKINFO_PLUGIN *AB_BankInfoPluginDE_new(AB_BANKING *ab){
  AB_BANKINFO_PLUGIN *bip;

  bip=AB_BankInfoPluginGENERIC_new(ab, "de");
  if (bip==NULL) {
    DBG_INFO(AQBANKING_LOGDOMAIN, "here");
    return NULL;
  }

  return bip;
}




