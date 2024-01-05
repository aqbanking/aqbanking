/***************************************************************************
    begin       : Mon Mar 01 2004
    copyright   : (C) 2023 by Rico Rommel
    email       : rico@bierrommel.de

 ***************************************************************************
 *          Please see toplevel file COPYING for license details           *
 ***************************************************************************/


#ifndef AG_USERDIALOG_P_H
#define AG_USERDIALOG_P_H

#include "userdialog.h"
#include <aqbanking/backendsupport/providerqueue.h>


struct AG_USER_DIALOG {
  AB_PROVIDER *provider;
  AB_USER *user;
};


#endif

