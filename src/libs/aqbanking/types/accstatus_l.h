/***************************************************************************
 $RCSfile$
                             -------------------
    cvs         : $Id$
    begin       : Mon Apr 05 2004
    copyright   : (C) 2004 by Martin Preuss
    email       : martin@libchipcard.de

 ***************************************************************************
 * This file is part of the project "AqBanking".                           *
 * Please see toplevel file COPYING of that project for license details.   *
 ***************************************************************************/


#ifndef AQBANKING_ACCSTATUS_L_H
#define AQBANKING_ACCSTATUS_L_H

#include <aqbanking/accstatus.h>
#include <gwenhywfar/misc.h>

GWEN_LIST_FUNCTION_DEFS(AB_ACCOUNT_STATUS, AB_AccountStatus)

AB_ACCOUNT_STATUS_LIST *AB_AccountStatus_List_dup(const AB_ACCOUNT_STATUS_LIST *asl);


#endif /* AQBANKING_ACCSTATUS_L_H */


