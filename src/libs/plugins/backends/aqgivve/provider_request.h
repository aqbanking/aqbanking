/***************************************************************************
    begin       : Mon Mar 01 2004
    copyright   : (C) 2023 by Rico Rommel
    email       : rico@bierrommel.de

 ***************************************************************************
 *          Please see toplevel file COPYING for license details           *
 ***************************************************************************/

#ifndef AG_PROVIDER_REQUEST_H
#define AG_PROVIDER_REQUEST_H

#include <aqbanking/banking.h>
#include <aqbanking/backendsupport/user.h>
#include <gwenhywfar/httpsession.h>
#include "voucherlist.h"
#include "gwenhywfar/json.h"

char *AG_Provider_Request_GetToken(AB_USER *user);
AG_VOUCHERLIST *AG_Provider_Request_GetVoucherList(char *token);
AB_TRANSACTION_LIST *AG_Provider_Request_GetTransactions(AB_ACCOUNT *account,
                                                         const GWEN_DATE *start_date,
                                                         const GWEN_DATE *end_date,
                                                         char *token);
AB_BALANCE *AG_Provider_Request_GetBalance(AB_ACCOUNT *account, char* token );

#endif
