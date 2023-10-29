/***************************************************************************
    begin       : Mon Mar 01 2004
    copyright   : (C) 2023 by Rico Rommel
    email       : rico@bierrommel.de

 ***************************************************************************
 *          Please see toplevel file COPYING for license details           *
 ***************************************************************************/

#ifndef AG_MERCHANT_H
#define AG_MERCHANT_H

#include "provider.h"
#include "gwenhywfar/json.h"



typedef struct AG_MERCHANT AG_MERCHANT ;

AG_MERCHANT *AG_MERCHANT_new ();
AG_MERCHANT *AG_MERCHANT_FromJsonElem(GWEN_JSON_ELEM *meta_elem);
void AG_MERCHANT_SetName(AG_MERCHANT *m, const char * name);
const char *AG_MERCHANT_GetName(AG_MERCHANT *m);
void AG_MERCHANT_free ( AG_MERCHANT *m);

#endif

