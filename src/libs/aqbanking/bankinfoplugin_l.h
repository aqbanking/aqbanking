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


#ifndef AQBANKING_BANKINFOPLUGIN_L_H
#define AQBANKING_BANKINFOPLUGIN_L_H

#define AB_BANKINFO_PLUGIN_FOLDER "bankinfo"

#include "bankinfoplugin_be.h"
#include <gwenhywfar/plugin.h>


GWEN_LIST_FUNCTION_LIB_DEFS(AB_BANKINFO_PLUGIN,
                            AB_BankInfoPlugin,
                            AQBANKING_API)


void AB_BankInfoPlugin_SetPlugin(AB_BANKINFO_PLUGIN *bip,
                                 GWEN_PLUGIN *pl);


#endif /* AQBANKING_BANKINFOPLUGIN_L_H */

