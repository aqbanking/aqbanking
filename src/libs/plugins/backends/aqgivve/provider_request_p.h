/***************************************************************************
    begin       : Mon Mar 01 2004
    copyright   : (C) 2023 by Rico Rommel
    email       : rico@bierrommel.de

 ***************************************************************************
 *          Please see toplevel file COPYING for license details           *
 ***************************************************************************/

#ifndef AG_PROVIDER_REQUEST_P_H
#define AG_PROVIDER_REQUEST_P_H

#include "provider_request.h"
#include "gwenhywfar/json.h"



GWEN_DATE *parseDate(const char *date_str);
AB_VALUE *parseMoney (GWEN_JSON_ELEM *value_elem);
AB_BALANCE *parseBalance(GWEN_JSON_ELEM *balance_elem);
AB_TRANSACTION *parseTransaction(GWEN_JSON_ELEM *data_elem);
char *createDateFilter( const GWEN_DATE *date, const char *op);
GWEN_JSON_ELEM *getElement(GWEN_JSON_ELEM *parent, char *key);

#endif
