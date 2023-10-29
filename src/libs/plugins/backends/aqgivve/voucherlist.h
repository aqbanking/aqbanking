/***************************************************************************
    begin       : Mon Mar 01 2004
    copyright   : (C) 2023 by Rico Rommel
    email       : rico@bierrommel.de

 ***************************************************************************
 *          Please see toplevel file COPYING for license details           *
 ***************************************************************************/

#ifndef AG_VOUCHER_LIST_H
#define AG_VOUCHER_LIST_H

#include "provider.h"
#include "voucher.h"

typedef struct AG_VOUCHERLIST AG_VOUCHERLIST; 


AG_VOUCHERLIST *AG_VOUCHERLIST_new();
void AG_VOUCHERLIST_free ( AG_VOUCHERLIST *list, int free_vouchers);

void AG_VOUCHERLIST_AddCard ( AG_VOUCHERLIST *list, AG_VOUCHER *card);

AG_VOUCHER *AG_VOUCHERLIST_Get_Card_By_ID ( AG_VOUCHERLIST *list, const char *id);
AG_VOUCHER *AG_VOUCHERLIST_Get_Card_By_Index ( AG_VOUCHERLIST *list, int index);
int AG_VOUCHERLIST_Get_TotalEntries ( AG_VOUCHERLIST *list);

#endif
