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


#ifndef AQBANKING_PROVIDER_L_H
#define AQBANKING_PROVIDER_L_H

#define AB_PROVIDER_FOLDER "providers"
#define AB_PROVIDER_DEBUGGER_FOLDER "debugger"

#include <aqbanking/provider.h>
#include <aqbanking/provider_be.h>
#include <gwenhywfar/plugin.h>

GWEN_LIST_FUNCTION_DEFS(AB_PROVIDER, AB_Provider)


void AB_Provider_SetPlugin(AB_PROVIDER *pro, GWEN_PLUGIN *pl);
void AB_Provider_free(AB_PROVIDER *pro);


#endif /* AQBANKING_PROVIDER_L_H */


