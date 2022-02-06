/***************************************************************************
    begin       : Sat May 08 2010
    copyright   : (C) 2010 by Martin Preuss
    email       : martin@libchipcard.de

 ***************************************************************************
 *          Please see toplevel file COPYING for license details           *
 ***************************************************************************/

#ifndef AQPAYPAL_PROVIDER_REQUEST_H
#define AQPAYPAL_PROVIDER_REQUEST_H


#include <aqpaypal/user.h>
#include <aqpaypal/provider.h>

#include <gwenhywfar/db.h>



GWEN_DB_NODE *APY_Provider_SendRequestParseResponse(AB_PROVIDER *pro, AB_USER *u, const char *requestString, const char *jobName);

int APY_Provider_SetupUrlString(AB_PROVIDER *pro, AB_USER *u, GWEN_BUFFER *tbuf);



#endif

