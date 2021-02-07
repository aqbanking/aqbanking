/***************************************************************************
 begin       : Mon Jan 13 2020
 copyright   : (C) 2020 by Martin Preuss
 email       : martin@libchipcard.de

 ***************************************************************************
 *          Please see toplevel file COPYING for license details           *
 ***************************************************************************/

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

/* plugin headers */
#include "n_acctinfo.h"
#include "n_utils.h"
#include "aqofxconnect/user.h"



/*
 * <ACCTINFORQ>
 *   <DTACCTUP>19691231</DTACCTUP>
 * </ACCTINFORQ>
 */




GWEN_XMLNODE *AO_Provider_MkAcctInfoRqNode(AB_USER *u)
{
  GWEN_XMLNODE *xmlMsg;
  GWEN_XMLNODE *xmlTrnRq;
  GWEN_XMLNODE *xmlRq;

  xmlMsg=GWEN_XMLNode_new(GWEN_XMLNodeTypeTag, "SIGNUPMSGSRQV1");

  xmlTrnRq=GWEN_XMLNode_new(GWEN_XMLNodeTypeTag, "ACCTINFOTRNRQ");
  GWEN_XMLNode_AddChild(xmlMsg, xmlTrnRq);

  AO_Provider_Util_SetCurrentTimeValue(xmlTrnRq, AO_User_GetFlags(u), "TRNUID");

  xmlRq=GWEN_XMLNode_new(GWEN_XMLNodeTypeTag, "ACCTINFORQ");
  GWEN_XMLNode_AddChild(xmlTrnRq, xmlRq);

  GWEN_XMLNode_SetCharValue(xmlRq, "DTACCTUP", "19900101");

  return xmlMsg;
}






