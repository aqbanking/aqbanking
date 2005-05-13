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


#ifndef CBANKING_CBANKING_H
#define CBANKING_CBANKING_H


#include <aqbanking/banking.h>



AB_BANKING *CBanking_new(const char *appName,
                         const char *fname);


const char *CBanking_GetCharSet(const AB_BANKING *ab);
void CBanking_SetCharSet(AB_BANKING *ab, const char *s);

/** Takes over the DB */
void CBanking_SetPinDb(AB_BANKING *ab, GWEN_DB_NODE *dbPins);

#endif

