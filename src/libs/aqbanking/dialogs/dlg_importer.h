/***************************************************************************
 begin       : Tue Feb 10 2010
 copyright   : (C) 2010 by Martin Preuss
 email       : martin@aqbanking.de

 ***************************************************************************
 * This file is part of the project "AqBanking".                           *
 * Please see toplevel file COPYING of that project for license details.   *
 ***************************************************************************/

#ifndef AQBANKING_DLG_IMPORTER_H
#define AQBANKING_DLG_IMPORTER_H


#include <gwenhywfar/dialog.h>
#include <gwenhywfar/db.h>

#include <aqbanking/banking.h>
#include <aqbanking/imexporter.h>


#ifdef __cplusplus
extern "C" {
#endif



AQBANKING_API GWEN_DIALOG *AB_ImporterDialog_new(AB_BANKING *ab,
						 AB_IMEXPORTER_CONTEXT *ctx,
						 const char *finishedMessage);


#ifdef __cplusplus
}
#endif



#endif

