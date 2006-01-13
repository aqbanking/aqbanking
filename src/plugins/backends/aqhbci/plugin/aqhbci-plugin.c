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

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include "banking/provider.h"

#include <gwenhywfar/plugin.h>


/* interface to gwens plugin loader */
AQBANKING_EXPORT GWEN_PLUGIN *provider_aqhbci_factory(GWEN_PLUGIN_MANAGER *pm,
                                                      const char *name,
                                                      const char *fileName) {
  return GWEN_Plugin_new(pm, name, fileName);
}


AQBANKING_EXPORT AB_PROVIDER *aqhbci_factory(AB_BANKING *ab){
  return AH_Provider_new(ab, AH_PROVIDER_NAME);
}
