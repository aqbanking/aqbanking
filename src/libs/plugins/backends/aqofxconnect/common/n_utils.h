/***************************************************************************
 begin       : Mon Jan 13 2020
 copyright   : (C) 2020 by Martin Preuss
 email       : martin@libchipcard.de

 ***************************************************************************
 *          Please see toplevel file COPYING for license details           *
 ***************************************************************************/


#ifndef AO_N_UTILS_H
#define AO_N_UTILS_H


#include <aqbanking/types/imexporter_context.h>


/* gwenhywfar headers */
#include <gwenhywfar/xml.h>
#include <gwenhywfar/gwendate.h>
#include <gwenhywfar/gwentime.h>



void AO_Provider_Util_SetDateValue(GWEN_XMLNODE *xmlNode, const GWEN_DATE *da, uint32_t userFlags, const char *varName);
void AO_Provider_Util_SetTimeValue(GWEN_XMLNODE *xmlNode, const GWEN_TIME *ti, uint32_t userFlags, const char *varName);
void AO_Provider_Util_SetCurrentTimeValue(GWEN_XMLNODE *xmlNode, uint32_t userFlags, const char *varName);


void AO_Provider_Util_ListAccounts(AB_IMEXPORTER_CONTEXT *ctx);


char *AO_Provider_Util_GenerateUuid(void);


#endif


