/***************************************************************************
 $RCSfile: provider.h,v $
                             -------------------
    cvs         : $Id$
    begin       : Mon Mar 01 2004
    copyright   : (C) 2004 by Martin Preuss
    email       : martin@libchipcard.de

 ***************************************************************************
 *          Please see toplevel file COPYING for license details           *
 ***************************************************************************/

#ifndef AY_PROVIDER_H
#define AY_PROVIDER_H


#include <aqyellownet/aqyellownet.h>
#include <aqyellownet/user.h>

#include <aqbanking/banking_be.h>
#include <aqbanking/provider_be.h>



#define AQYELLOWNET_LOGDOMAIN "aqyellownet"

#ifdef __cplusplus
extern "C" {
#endif

AQYELLOWNET_API AB_PROVIDER *AY_Provider_new(AB_BANKING *ab);

AQYELLOWNET_API int AY_Provider_GetConnectTimeout(const AB_PROVIDER *pro);
AQYELLOWNET_API int AY_Provider_GetTransferTimeout(const AB_PROVIDER *pro);


#ifdef __cplusplus
}
#endif


#endif

