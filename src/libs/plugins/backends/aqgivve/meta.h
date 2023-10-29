/***************************************************************************
    begin       : Mon Mar 01 2004
    copyright   : (C) 2023 by Rico Rommel
    email       : rico@bierrommel.de

 ***************************************************************************
 *          Please see toplevel file COPYING for license details           *
 ***************************************************************************/

#ifndef AG_META_H
#define AG_META_H

#include "meta.h"
#include "gwenhywfar/json.h"

typedef struct AG_META AG_META;

AG_META *AG_META_new();
AG_META *AG_META_FromJsonElem(GWEN_JSON_ELEM *meta_elem);

int AG_META_GetCurrentPage(AG_META *meta);
void AG_META_SetCurrentPage(AG_META *meta, int cp);
int AG_META_GetTotalPages(AG_META *meta);
void AG_META_SetTotalPages(AG_META *meta, int tp);
int AG_META_GetPageSize(AG_META *meta);
void AG_META_SetPageSize(AG_META *meta, int ps);
int AG_META_GetTotalEntries(AG_META *meta);
void AG_META_SetTotalEntries(AG_META *meta, int te);
void AG_META_free(AG_META *meta);

#endif
