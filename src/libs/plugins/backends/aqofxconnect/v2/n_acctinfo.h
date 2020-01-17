/***************************************************************************
 begin       : Mon Jan 13 2020
 copyright   : (C) 2020 by Martin Preuss
 email       : martin@libchipcard.de

 ***************************************************************************
 *          Please see toplevel file COPYING for license details           *
 ***************************************************************************/


#ifndef AO_N_ACCTINFO_H
#define AO_N_ACCTINFO_H


/* plugin headers */
#include <aqofxconnect/aqofxconnect.h>

/* aqbanking headers */
#include <aqbanking/backendsupport/user.h>

/* gwenhywfar headers */
#include <gwenhywfar/xml.h>



GWEN_XMLNODE *AO_V2_MkAcctInfoRqNode(AB_USER *u);



#endif



