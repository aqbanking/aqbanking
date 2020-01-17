/***************************************************************************
 begin       : Mon Jan 13 2020
 copyright   : (C) 2020 by Martin Preuss
 email       : martin@libchipcard.de

 ***************************************************************************
 *          Please see toplevel file COPYING for license details           *
 ***************************************************************************/


#ifndef AO_N_MESSAGE_H
#define AO_N_MESSAGE_H


/* plugin headers */
#include <aqofxconnect/aqofxconnect.h>

/* aqbanking headers */
#include <aqbanking/backendsupport/user.h>

/* gwenhywfar headers */
#include <gwenhywfar/xml.h>



GWEN_XMLNODE *AO_V2_MkOfxHeader(AB_USER *u);
GWEN_XMLNODE *AO_V2_MkXmlHeader(void);



#endif



