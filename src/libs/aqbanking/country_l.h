/***************************************************************************
 $RCSfile$
 -------------------
 cvs         : $Id$
 begin       : Mon Mar 01 2004
 copyright   : (C) 2004 by Martin Preuss
 email       : martin@libchipcard.de

 ***************************************************************************
 * This file is part of the project "AqBanking".                           *
 * Please see toplevel file COPYING of that project for license details.   *
 ***************************************************************************/


#ifndef AQBANKING_COUNTRY_L_H
#define AQBANKING_COUNTRY_L_H

#include <aqbanking/country.h>


const AB_COUNTRY *AB_Country_FindByName(const char *name);
const AB_COUNTRY *AB_Country_FindByLocalName(const char *name);
const AB_COUNTRY *AB_Country_FindByCode(const char *code);
const AB_COUNTRY *AB_Country_FindByNumeric(int numid);

AB_COUNTRY_CONSTLIST2 *AB_Country_ListByName(const char *name);
AB_COUNTRY_CONSTLIST2 *AB_Country_ListByLocalName(const char *name);

#endif /* AQBANKING_COUNTRY_L_H */
