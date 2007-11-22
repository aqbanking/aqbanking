/***************************************************************************
 $RCSfile$
                             -------------------
    cvs         : $Id: us.c 1138 2007-01-22 15:05:39Z christian $
    begin       : Mon Mar 01 2004
    copyright   : (C) 2004 by Martin Preuss
    email       : martin@libchipcard.de

 ***************************************************************************
 *          Please see toplevel file COPYING for license details           *
 ***************************************************************************/

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif


#include "ca_p.h"
#include "../generic/generic_l.h"

#include <aqbanking/banking.h>
#include <gwenhywfar/debug.h>
#include <gwenhywfar/misc.h>
#include <gwenhywfar/inherit.h>
#include <gwenhywfar/text.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include <errno.h>



GWEN_INHERIT(AB_BANKINFO_PLUGIN, AB_BANKINFO_PLUGIN_CA);

/* interface to gwens plugin loader */
GWEN_PLUGIN *bankinfo_ca_factory(GWEN_PLUGIN_MANAGER *pm,
                                 const char *name,
                                 const char *fileName) {
  return GWEN_Plugin_new(pm, name, fileName);
}



/* interface to bankinfo plugin */
AB_BANKINFO_PLUGIN *ca_factory(AB_BANKING *ab, GWEN_DB_NODE *db){
  AB_BANKINFO_PLUGIN *bip;
  AB_BANKINFO_PLUGIN_CA *bde;

  bip=AB_BankInfoPluginGENERIC_new(ab, db, "ca");
  GWEN_NEW_OBJECT(AB_BANKINFO_PLUGIN_CA, bde);
  GWEN_INHERIT_SETDATA(AB_BANKINFO_PLUGIN, AB_BANKINFO_PLUGIN_CA,
                       bip, bde, AB_BankInfoPluginCA_FreeData);

  bde->banking=ab;
  bde->dbData=db;
  return bip;
}



void GWENHYWFAR_CB AB_BankInfoPluginCA_FreeData(void *bp, void *p){
  AB_BANKINFO_PLUGIN_CA *bde;

  bde=(AB_BANKINFO_PLUGIN_CA*)p;

  GWEN_FREE_OBJECT(bde);
}



