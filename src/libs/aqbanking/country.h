/***************************************************************************
 $RCSfile$
 -------------------
 cvs         : $Id$
 begin       : Mon Mar 01 2004
 copyright   : (C) 2004 by Martin Preuss
 email       : martin@libchipcard.de

 ***************************************************************************
 *          Please see toplevel file COPYING for license details           *
 ***************************************************************************/


#ifndef AQBANKING_COUNTRY_H
#define AQBANKING_COUNTRY_H

#include <gwenhywfar/list2.h>
#include <aqbanking/error.h>

typedef struct AB_COUNTRY AB_COUNTRY;
GWEN_CONSTLIST2_FUNCTION_LIB_DEFS(AB_COUNTRY, AB_Country, AQBANKING_API)

#include <aqbanking/banking.h>


AQBANKING_API 
const char *AB_Country_GetName(const AB_COUNTRY *cntry);

AQBANKING_API 
const char *AB_Country_GetCode(const AB_COUNTRY *cntry);

AQBANKING_API 
int AB_Country_GetNumericCode(const AB_COUNTRY *cntry);

AQBANKING_API 
const char *AB_Country_GetLocalName(const AB_COUNTRY *cntry);


#endif /* AQBANKING_COUNTRY_H */
