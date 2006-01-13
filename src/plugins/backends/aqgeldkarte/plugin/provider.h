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

#ifndef AG_PROVIDER_H
#define AG_PROVIDER_H

#include <aqgeldkarte/aqgeldkarte.h>

#include <aqbanking/banking_be.h>
#include <aqbanking/provider_be.h>

#include <chipcard2-client/client/card.h>


#ifdef __cplusplus
extern "C" {
#endif

AQGELDKARTE_API
AB_PROVIDER *AG_Provider_new(AB_BANKING *ab);

AQGELDKARTE_API
LC_CARD *AG_Provider_MountCard(AB_PROVIDER *pro, AB_ACCOUNT *acc);


#ifdef __cplusplus
}
#endif


#endif

