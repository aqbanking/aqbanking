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

#undef BUIDLING_AQDTAUS


#include "provider_l.h"
#include "aqdtaus_l.h"

#include <gwenhywfar/plugin.h>


/* interface to gwens plugin loader */
AQBANKING_EXPORT
GWEN_PLUGIN *provider_aqdtaus_factory(GWEN_PLUGIN_MANAGER *pm,
                                      const char *name,
                                      const char *fileName) {
  return GWEN_Plugin_new(pm, name, fileName);
}



/* interface to AqBankings plugin loader */
AQBANKING_EXPORT AB_PROVIDER *aqdtaus_factory(AB_BANKING *ab){
  return AD_Provider_new(ab);
}



