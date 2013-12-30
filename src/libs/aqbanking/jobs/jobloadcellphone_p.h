/***************************************************************************
 begin       : Mon Mar 01 2004
 copyright   : (C) 2004-2013 by Martin Preuss
 email       : martin@libchipcard.de

 ***************************************************************************
 * This file is part of the project "AqBanking".                           *
 * Please see toplevel file COPYING of that project for license details.   *
 ***************************************************************************/


#ifndef AQBANKING_JOBLOADCELLPHONE_P_H
#define AQBANKING_JOBLOADCELLPHONE_P_H


#include "jobloadcellphone_l.h"


typedef struct AB_JOBLOADCELLPHONE AB_JOBLOADCELLPHONE;
struct AB_JOBLOADCELLPHONE {
  AB_CELLPHONE_PRODUCT *product;
  char *phoneNumber;
  AB_VALUE *value;

  AB_CELLPHONE_PRODUCT_LIST *productList;

};
static void GWENHYWFAR_CB AB_JobLoadCellPhone_FreeData(void *bp, void *p);



#endif

