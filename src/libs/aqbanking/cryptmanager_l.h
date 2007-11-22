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


#ifndef AQBANKING_CRYPTMANAGER_L_H
#define AQBANKING_CRYPTMANAGER_L_H

#error do not include me.

#define AB_CRYPTTOKEN_PLUGIN_FOLDER "crypttoken"


#include <aqbanking/banking.h>
#include <gwenhywfar/misc.h>
#include <gwenhywfar/list2.h>
#include <gwenhywfar/inherit.h>
#include <gwenhywfar/crypttoken.h>


GWEN_PLUGIN_MANAGER *AB_CryptManager_new(AB_BANKING *ab);




#endif /* AQBANKING_CRYPTMANAGER_L_H */
