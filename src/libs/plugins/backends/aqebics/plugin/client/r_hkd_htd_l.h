/***************************************************************************
    begin       : Fri Dec 28 2018
    copyright   : (C) 2018 by Martin Preuss
    email       : martin@libchipcard.de

 ***************************************************************************
 *          Please see toplevel file COPYING for license details           *
 ***************************************************************************/

#ifndef AQEBICS_CLIENT_R_HKD_HTD_L_H
#define AQEBICS_CLIENT_R_HKD_HTD_L_H

#include <aqebics/provider.h>

#include <gwenhywfar/httpsession.h>



int EBC_Provider_XchgHkdRequest(AB_PROVIDER *pro, GWEN_HTTP_SESSION *sess, AB_USER *u);
int EBC_Provider_XchgHtdRequest(AB_PROVIDER *pro, GWEN_HTTP_SESSION *sess, AB_USER *u);


#endif
