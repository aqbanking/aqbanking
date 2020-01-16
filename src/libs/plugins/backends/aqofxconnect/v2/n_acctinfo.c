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



/*
 * <ACCTINFORQ>
 *   <DTACCTUP>19691231</DTACCTUP>
 * </ACCTINFORQ>
 */




GWEN_XMLNODE *AO_V2_MkAcctInfoRqNode(AB_USER *u)
{
  GWEN_XMLNODE *xmlAcctInfoRq;

  xmlAcctInfoRq=GWEN_XMLNode_new(GWEN_XMLNodeTypeTag, "ACCTINFORQ");
  GWEN_XMLNode_SetCharValue(xmlAcctInfoRq, "DTACCTUP", "19691231");

  return xmlAcctInfoRq;
}






