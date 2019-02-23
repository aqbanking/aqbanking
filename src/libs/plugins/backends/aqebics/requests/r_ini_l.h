/***************************************************************************
    begin       : Mon Mar 01 2004
    copyright   : (C) 2019 by Martin Preuss
    email       : martin@libchipcard.de

 ***************************************************************************
 *          Please see toplevel file COPYING for license details           *
 ***************************************************************************/

#ifndef AQEBICS_CLIENT_R_INI_L_H
#define AQEBICS_CLIENT_R_INI_L_H

#include <aqbanking/provider.h>
#include <aqbanking/user.h>
#include <aqbanking/httpsession.h>


int EBC_Provider_XchgIniRequest(AB_PROVIDER *pro, GWEN_HTTP_SESSION *sess, AB_USER *u);
int EBC_Provider_XchgIniRequest_H002(AB_PROVIDER *pro, GWEN_HTTP_SESSION *sess, AB_USER *u);
int EBC_Provider_XchgIniRequest_H003(AB_PROVIDER *pro, GWEN_HTTP_SESSION *sess, AB_USER *u);
int EBC_Provider_XchgIniRequest_H004(AB_PROVIDER *pro, GWEN_HTTP_SESSION *sess, AB_USER *u);



#endif
