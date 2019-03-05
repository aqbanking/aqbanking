/***************************************************************************
    begin       : Mon Mar 01 2004
    copyright   : (C) 2019 by Martin Preuss
    email       : martin@libchipcard.de

 ***************************************************************************
 *          Please see toplevel file COPYING for license details           *
 ***************************************************************************/


#ifndef AQEBICS_CLIENT_R_HPB_L_H
#define AQEBICS_CLIENT_R_HPB_L_H

#include "aqebics/client/provider_l.h"

#include <aqbanking/backendsupport/provider.h>
#include <aqbanking/backendsupport/user.h>
#include <aqbanking/backendsupport/httpsession.h>


int EBC_Provider_XchgHpbRequest(AB_PROVIDER *pro, GWEN_HTTP_SESSION *sess, AB_USER *u);
int EBC_Provider_XchgHpbRequest_H002(AB_PROVIDER *pro, GWEN_HTTP_SESSION *sess, AB_USER *u);
int EBC_Provider_XchgHpbRequest_H003(AB_PROVIDER *pro, GWEN_HTTP_SESSION *sess, AB_USER *u);
int EBC_Provider_XchgHpbRequest_H004(AB_PROVIDER *pro, GWEN_HTTP_SESSION *sess, AB_USER *u);


#endif

